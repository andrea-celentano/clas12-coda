#if defined(VXWORKS) || defined(Linux_vme)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vscmLib.h"

#ifndef VXWORKS
#include "jvme.h"

typedef struct {
  unsigned int triggers;
  unsigned int adc[8];
} data_counts;

data_counts
vscmParseFIFO(DMANODE* event, FILE *f)
{
  int i, n = event->length;
  long long trig1 = 0, trigtime = 0;
  data_counts cnt = {0, {0,0,0,0,0,0,0,0}};

  for(i = 0; i < n; i++) {
    uint32_t word = LSWAP(event->data[i]);
#ifdef DEBUG
    printf("0x%08X - ", word);
#endif
  
    if(word & 0x80000000) {
      int type = (word >> 27) & 0xF;
      switch(type) {
        case DATA_TYPE_BLKHDR:
#ifdef DEBUG
          printf(" {BLKHDR} SLOTID: %d", (word>>22)&0x1f);
          printf(" NEVENTS: %d", (word>>11)&0x7ff);
          printf(" BLOCK: %d\n", (word>>0)&0x7ff);
#endif
          break;
        case DATA_TYPE_BLKTLR:
#ifdef DEBUG
          printf(" {BLKTLR} SLOTID: %d", (word>>22)&0x1f);
          printf(" NWORDS: %d\n", (word>>0)&0x3fffff);
#endif
          break;
        case DATA_TYPE_EVTHDR:
/*
          if(((word & 0x7ffffff) % 100000) == 0)
            printf("Event: %d\n", word & 0x7ffffff);
*/
#ifdef DEBUG
          printf(" {EVTHDR} EVENT: %d\n", (word>>0)&0x7ffffff);
#endif
          break;
        case DATA_TYPE_TRGTIME:
          cnt.triggers++;
          trig1 = word & 0xffffff;
#ifdef DEBUG
          printf(" {TRGTIME1} %lld\n", trig1);
#endif
          break;
        case DATA_TYPE_BCOTIME:
/*
          if(f) 
            fprintf(f, "* %d, %d\n", (word & 0xff), (word >> 16) & 0xff);
*/
#ifdef DEBUG
          printf(" {BCOTIME}  %d, %d\n", (word & 0xff), (word >> 16) & 0xff); 
#endif
          break;
        case DATA_TYPE_FSSREVT:
          if(f) {
            fprintf(f, "%u, %u, %u, %u, %u\n",
                  (word>>22) & 1, (word>>19) & 7, (word>>12) & 0x7f,
                  (word>>4) & 0xff, word & 7);
          }
          cnt.adc[(word & 0x7)]++;
#ifdef DEBUG
          printf(" {FSSREVT}");
          printf(" HFCB: %1u", (word>>22)&1);
          printf(" CHIPID: %1u", (word>>19)&0x7);
          printf(" CH: %3u", (word>>12)&0x7F);
          printf(" BCO: %3u", (word>>4)&0xFF);
          printf(" ADC: %1u\n", (word>>0)&0x7);
#endif
          break;
        case DATA_TYPE_DNV:
#ifdef DEBUG
          printf(" {***DNV***}\n");
#endif
          break;
        case DATA_TYPE_FILLER:
#ifdef DEBUG
          printf(" {FILLER}\n");
#endif
          break;
        default:
#ifdef DEBUG
          printf(" {***DATATYPE ERROR***}\n");
#endif
          break;
      }
    }
    else {
      trigtime = (trig1 << 24) + word;
/*
      if(f)
        fprintf(f, "%lld\n", trigtime);
*/
#ifdef DEBUG
      printf(" {TRGTIME2} %lld\n", trigtime);
#endif
    }
  }
  return cnt;
}
#endif

typedef struct {
  unsigned int amp;
  unsigned int start;
  uint32_t hits[256];
} threshold_scan;

void
fssrGainScan(int id, char *filename, \
              int beg_chip, int end_chip, \
              int beg_chan, int end_chan,
              int start_thr, int chan_mult)
{
  FILE *fd;
  char fname[256], reg27[20];
  int thr, ichip, ich;
  int i, j;
  int zero_count, amp, min_hits;
  int base_thr = start_thr;
  int temp_start_thr, zero_status, a;

  uint32_t pulser_rate = vscmGetPulserRate(id);
  uint32_t scaler[128];

  /* Pulser Settings */
  const int start_amp = 75;
  const int end_amp = 125;
  const int step_amp = 25;
  int steps = ((end_amp - start_amp) / step_amp) + 1;

  threshold_scan *data;

#ifdef VXWORKS
  int ticks = 1;
#else
  struct timespec ts; 
  ts.tv_sec = 0;
  ts.tv_nsec = 16666667; /* ~16.6 ms (similar to 1 tick on VxWorks) */
#endif

  if (vscmIsNotInit(&id, __func__))
    return;

  if ((end_chip - beg_chip) > 3) {
    logMsg("ERROR: %s: Chip range must be 4 chips or less\n", __func__);
    logMsg("ERROR: %s: Output files will be overwritten if >4\n", __func__);
    return;
  }

  if ((chan_mult < 1) || ((chan_mult & (chan_mult - 1)) != 0)) {
    logMsg("ERROR: %s: Channel multiplier (%d) ", __func__, chan_mult);
    logMsg("must be a power of 2\n");
    return;
  }

  data = malloc(steps * chan_mult * sizeof(threshold_scan));

  /* Temporarily disable both pulser outputs */
  vscmPulser(id, 0, 0, 1);

#ifdef VXWORKS
  min_hits = (int)(pulser_rate * (ticks / (float)sysClkRateGet()) * 0.99);
#else
  min_hits = (int)(pulser_rate * (float)(ts.tv_nsec * 1.0e-9) * 0.99);
#endif
  logMsg("INFO: %s: Min Hits = %d\n", __func__, min_hits);

  for (ichip = beg_chip; ichip <= end_chip; ichip++) {
#ifdef DEBUG
    fssrStatus(id, ichip);
#endif

    if (filename) {
      sprintf(fname,"%s_u%1d", filename, ((ichip % 4) + 1));
      fd = fopen(fname, "w");
      if (!fd) {
        logMsg("ERROR: %s: Opening file: %s\n", __func__, fname);
        free(data);
        return;
      }
      else {
        logMsg("INFO: %s: Output file: %s\n", __func__, fname);
      }
    }
    else
      logMsg("Chip %d\n", ichip);

    fssrParseControl(id, ichip, reg27);
    if (filename) {
      fprintf(fd, "%s\n", reg27);
    }
    else
      printf("%s\n", reg27);

    for (i = 0; i < (128 / chan_mult); i++) {
      start_thr = base_thr;
      thr = start_thr;
      temp_start_thr = 1;

      fssrKillMaskDisableAll(id, ichip);
      fssrInjectMaskDisableAll(id, ichip);
      printf("Chan: ");

      for (j = 0; j < chan_mult; j++) {
        ich = i + (j * (128 / chan_mult));
        printf("%d ", ich);
        fssrKillMaskEnableSingle(id, ichip, ich);
        fssrInjectMaskEnableSingle(id, ichip, ich);
      }
      printf("\n");

      for (a = 0; a < steps; a++) {
        amp = start_amp + (a * step_amp);
        thr = start_thr;
        zero_count = 0;
        printf("Amp: %d Start: %d\n", amp, thr);

        vscmPulser(id, ((ichip > 3) ? 2 : 1), amp, (int)(min_hits / 0.99));

        while(thr < 256 && zero_count < 2) {
          zero_status = 0;
          temp_start_thr = 0;
          fssrSetThreshold(id, ichip, 0, thr);

          vscmDisableChipScaler(id, ichip);
          vscmClearStripScalers(id, ichip);
          vscmEnableChipScaler(id, ichip);
          vscmDisableChipScaler(id, ichip);
          vscmEnableChipScaler(id, ichip);

          vscmPulserStart(id);
   
          /* Gather data by sleeping */
#ifdef VXWORKS
          taskDelay(ticks);
#else
          nanosleep(&ts, NULL);
#endif

          vscmDisableChipScaler(id, ichip);
          vscmReadStripScalers(id, ichip, scaler);

          for (j = 0; j < chan_mult; j++) {
            ich = i + (j * (128 / chan_mult));

            if (thr == start_thr) {
              data[(a * chan_mult) + j].amp = amp;
              data[(a * chan_mult) + j].start = start_thr;
            }
            data[(a * chan_mult) + j].hits[thr] = scaler[ich];
   
            if (scaler[ich] > min_hits)
              temp_start_thr++;

            if (scaler[ich] == 0)
              zero_status++;
          }

          if (zero_status == chan_mult)
            zero_count++;
          else
            zero_count = 0;

          if (temp_start_thr == chan_mult)
            start_thr = thr;

          thr++;
        }
      }

      /* Save the data struct to file */
      if (filename) {
        for (j = 0; j < chan_mult; j++) {
          ich = i + (j * (128 / chan_mult));
          fprintf(fd, "%d\n", ich);
          for (a = 0; a < steps; a++) {

            fprintf(fd, "%d, ", data[(a * chan_mult) + j].amp);
            fprintf(fd, "%d ", data[(a * chan_mult) + j].start);
            zero_count = 0;
            thr = data[(a *chan_mult) + j].start;

            while(thr < 256 && zero_count < 2) {
              fprintf(fd, ",%d ", data[(a * chan_mult) + j].hits[thr]);

              if (data[(a * chan_mult) + j].hits[thr] == 0)
                zero_count++;
              else
                zero_count = 0;
              thr++;
            }
            fprintf(fd, "\n");
          }
          fprintf(fd, "\n");
        }
        fflush(fd);
      }
    } /* End of Channel loop */
    printf("\n");
    if (filename)
      fclose(fd);
  } /* End of Chip loop */

  free(data);
}

void
fssrThresholdScan(int id, char *filename, \
                  int beg_chip, int end_chip, \
                  int beg_chan, int end_chan, \
                  int start_thr)
{
  FILE *fd;
  char fname[256], reg27[20];
  int thr, ich, ichip;
  int zero_count;
  uint32_t scaler[128];
#ifdef DEBUG
  float ratio, ref;
  uint32_t gothitref;
  uint32_t pulser_rate = vscmGetPulserRate(id);
#endif

#ifdef VXWORKS
  int ticks = 1;
#else
  struct timespec ts; 
  ts.tv_sec = 0;
  ts.tv_nsec = 16666667; /* ~16.6 ms (similar to 1 tick on VxWorks) */
#endif

  if (vscmIsNotInit(&id, __func__))
    return;

  for (ichip = beg_chip; ichip <= end_chip; ichip++) {
    if (filename) {
      sprintf(fname,"%s_u%1d", filename, ((ichip % 4) + 1));
      fd = fopen(fname, "w");
      if (!fd) {
        logMsg("ERROR: %s: Opening file: %s\n", __func__, fname);
        return;
      }
      else {
        logMsg("INFO: %s: Output file: %s\n", __func__, fname);
      }
    }

#ifdef DEBUG
    fssrStatus(id, ichip);
#endif

    fssrParseControl(id, ichip, reg27);
    if (filename) {
      fprintf(fd, "%s\n", reg27);
    }
    else
      printf("%s\n", reg27);

    for (ich = beg_chan; ich <= end_chan; ich++) {
      if (!filename)
        printf("Chan %d\n",ich);
      else
        fprintf(fd, "%d\n0, %d", ich, start_thr);

      fssrKillMaskDisableAll(id, ichip);
      fssrKillMaskEnableSingle(id, ichip, ich);
      fssrInjectMaskDisableAll(id, ichip);
      fssrInjectMaskEnableSingle(id, ichip, ich);

      thr = start_thr;
      scaler[ich] = 100000;
      zero_count = 0;

      while ((thr < 256) && (zero_count <= 2)) {
        fssrSetThreshold(id, ichip, 0, thr);

        vscmDisableChipScaler(id, ichip);
        vscmClearStripScalers(id, ichip);
        vscmEnableChipScaler(id, ichip);

        /* Gather data by sleeping */
#ifdef VXWORKS
        taskDelay(ticks);
#else
        nanosleep(&ts, NULL);
#endif

        vscmDisableChipScaler(id, ichip);
        vscmReadStripScalers(id, ichip, scaler);

#ifdef DEBUG
        ref = (float)vmeRead32(&VSCMpr[id]->Fssr[ichip].ScalerRef);
        ratio = (scaler[ich]) / (ref * (float)pulser_rate / VSCM_SYS_CLK);
        gothitref = vmeRead32(&VSCMpr[id]->Fssr[ichip].ScalerGotHit);
        printf("chp=%d chn=%3d thr=%3d scaler=%7d ghit=%7d ref=%.0f -> ratio=%.1f\n", \
                ichip, ich, thr, scaler[ich], gothitref, ref, ratio);
        fflush(stdout);
#endif

        if (filename) {
          fprintf(fd, ", %d", scaler[ich]);
          printf("\rChan %d - %d", ich, thr);fflush(stdout);
        }
        else {
#ifndef DEBUG
          printf("%d, %d\n", thr, scaler[ich]);
#endif
        }

        thr++;
        if (scaler[ich] == 0)
          zero_count++;
        else
          zero_count = 0;
      }

      if (filename) {
        fprintf(fd, "\n\n");
        fflush(fd);
      }
      printf("\n");
    } /* End of Channel loop */
    if (filename) {
      fclose(fd);
    }
  } /* End of Chip loop */
}

/* 
 * Returns 1 if all channels are good, 0 if any are dead
 * 
 * The status parameter is an optional array of the 
 * individual channel status, note this is opposite
 * of the return value, i.e. 1 = channel is bad 
 */
int
fssrChanStatus(int id, int chip, uint32_t *status)
{
  uint32_t dead[4] = {0, 0, 0, 0};
  uint32_t scaler[128];
  int i, arr;

  const int AMP = 100;
  const int THR = 50;

  if (vscmIsNotInit(&id, __func__))
    return -1;

  vscmPulser(id, ((chip > 3) ? 2 : 1), AMP, 10000);
  vscmPulserStart(id);

  printf("Slot %2d, Chip %1d\n", id, chip);
  printf("---------------\n");

  for (i = 0; i < 128; i++) {
    fssrKillMaskDisableAll(id, chip);
    fssrKillMaskEnableSingle(id, chip, i);

    fssrSetThreshold(id, chip, 0, THR);

    vscmDisableChipScaler(id, chip);
    vscmClearStripScalers(id, chip);
    vscmEnableChipScaler(id, chip);

    /* Gather data by sleeping */
    taskDelay(1);

    vscmDisableChipScaler(id, chip);
    vscmReadStripScalers(id, chip, scaler);

    if (scaler[i] == 0) {
      printf("Channel %d BAD\n", i);
      arr = (int)((i / 128.0) * 4);
      dead[arr] |= (1 << (i % 32));
    }
  }

  if (status)
    memcpy(status, dead, sizeof(status));

  vscmPulser(id, ((chip > 3) ? 2 : 1), 0, 1);

  if (dead[0] > 0 || dead[1] > 0 || dead[2] > 0 || dead[3] > 0)
    return 0;
  else {
    printf("All Channels Good\n");
    return 1;
  }
}

void
vscmScalerDelayTest(int id, int ticks, int loop)
{
  uint32_t ref;
  int c;
#ifndef VXWORKS
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 16666667 * ticks;
#endif

  if (vscmIsNotInit(&id, __func__))
    return;

  for (c = 0; c < loop; c++) {
//    vscmLatchScalers(id);
    vmeWrite32(&VSCMpr[id]->ScalerLatch, 1<<31);
    /* Gather data by sleeping */
#ifdef VXWORKS
    taskDelay(ticks);
#else
    nanosleep(&ts, NULL);
#endif

//    vscmLatchScalers(id);
    vmeWrite32(&VSCMpr[id]->ScalerLatch, 1<<31);
    ref = vmeRead32(&VSCMpr[id]->ScalerVmeClk);

    printf("%u\n", ref);
  }
}

/* Compares the Chip ID in the status word with VSCM Register */
int
fssrChipIDTest(int id, int chip)
{
  uint32_t res, addr;

#ifdef VXWORKS
  int ticks = 1;
#else
  struct timespec ts; 
  ts.tv_sec = 0;
  ts.tv_nsec = 16666667; /* ~16.6 ms (similar to 1 tick on VxWorks) */
#endif

  if (vscmIsNotInit(&id, __func__))
    return 1;

  // Only use the lower 3 bits i.e. Chip ID wire bonds
  if (chip >=0 && chip <= 3) {
    addr = vmeRead32(&VSCMpr[id]->FssrAddrH1) >> (chip * 8) & 7;
  } 
  else if (chip >= 4 && chip <= 7) {
    addr = vmeRead32(&VSCMpr[id]->FssrAddrH2) >> ((chip - 4) * 8) & 7;
  }

  // Make sure that the VSCM register has been set
  if (addr == 0) {
    printf("ERROR: %s: Need to set Chip ID register first\n", __func__);
    return 1;
  }

  printf("Chip #%d: ID ... ", chip);
  fssrMasterReset(id);
  fssrSetActiveLines(id, chip, FSSR_ALINES_1);
  vscmEnableChipScaler(id, chip);
  // Latch once to clear scalers
  vscmLatchChipScaler(id, chip);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);

  // First make sure we actually got some new status words
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].ScalerStatusWord);
  if (res < 1) {
    printf("%sFAIL%s - No status words\n", "\x1B[1;31m", "\x1B[0m");
    return 1;
  }
  // Now check the status word
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastStatusWord);
  if (addr != (((res >> 14) & 7))) {
    printf("%sFAIL%s - Expected ", "\x1B[1;31m", "\x1B[0m");
    printf("%d got ", addr);
    printf("%d\n", ((res >> 14) & 7));
    return 1;
  }
  else {
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");
    return 0;
  }
}

static uint32_t
rand32()
{
  uint32_t x;

  x = rand() & 0xff;
  x |= (rand() & 0xff) << 8;
  x |= (rand() & 0xff) << 16;
  x |= (rand() & 0xff) << 24;

  return x;
}

int
fssrRegisterTest(int id, int chip)
{
  int c, r, i;
  int bad = 0;
  uint32_t t, res;
  int start = 0, stop = 7;
  uint32_t mask[4] = {0, 0, 0, 0}, readmask[4];

  if (vscmIsNotInit(&id, __func__))
    return 1;

  if (chip >= 0 && chip < 8) {
    start = stop = chip;
  }

  for (c = start; c <= stop; c++) {
    if ((c > 0) && (start != stop)) printf("\n");
    printf("Chip #%d\n", c);
    /* can't read 28 & 30, so stop at 27 */
    for (r = 1; r < 28; r++) {
      /* skip non-existant registers */
      if (r == 21 || r == 22 || r == 23 || r == 25 || r == 26)
        continue;
      /* skip registers that always return 0 */
      else if (r == 2 || r == 19 || r == 20 || r == 24)
        continue;
      /* kill & inject masks */
      else if (r == 17 || r == 18) {
        /* Try setting mask to all zeros */
        mask[0] = mask[1] = mask[2] = mask[3] = 0;
        fssrTransfer(id, c, r, FSSR_CMD_WRITE, 128, mask);
        fssrTransfer(id, c, r, FSSR_CMD_READ, 129, readmask);
        printf("\rReg #%2d - %-9s ... ", r, "All Zeros");

        if (fssrMaskCompare(mask, readmask)) {
          printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
          bad |= 1;
          continue;
        }
        /* 5 passes of random 32-bit numbers for mask */
        for (i = 1; i <= 5; i++) {
          mask[0] = rand32();
          mask[1] = rand32();
          mask[2] = rand32();
          mask[3] = rand32();

          fssrTransfer(id, c, r, FSSR_CMD_WRITE, 128, mask);
					fssrTransfer(id, c, r, FSSR_CMD_READ, 129, readmask);
          printf("\rReg #%2d - %-8s%1d ... ", r, "Random #", i);

          if (fssrMaskCompare(mask, readmask)) {
            printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
            bad |= 1;
            break;
          }
        }
        /* Check to see if all 5 passes of random numbers finished */
        if (i < 5)
          continue;
        /* Try setting mask to all ones */
        mask[0] = mask[1] = mask[2] = mask[3] = 0xffffffff;
        fssrTransfer(id, c, r, FSSR_CMD_WRITE, 128, mask);
				fssrTransfer(id, c, r, FSSR_CMD_READ, 129, readmask);
        printf("\rReg #%2d - %-9s ... ", r, "All Ones");

        if (fssrMaskCompare(mask, readmask)) {
          printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
          bad |= 1;
          continue;
        }
        /* If made it this far haven't had any errors, so report a pass */
        printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");
      }
      /* handle 8-bit registers */
      else if ((r >= 1 && r <= 15) || r == 27) {
        for (t = 0; t < 256; t++) {
          fssrTransfer(id, c, r, FSSR_CMD_WRITE, 8, &t);
          fssrTransfer(id, c, r, FSSR_CMD_READ, 9, &res);
          printf("\rReg #%2d - %-9d ... ", r, res & 0xFF);
          if ((res & 0xFF) != t) {
            printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
            bad |= 1;
            break;
          }
        }
        if (t == 256)
          printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");
      }
      /* handle active lines register */
      else if (r == 16) {
        for (t = 0; t < 4; t++) {
          fssrTransfer(id, c, r, FSSR_CMD_WRITE, 2, &t);
          fssrTransfer(id, c, r, FSSR_CMD_READ, 3, &res);
          printf("\rReg #%2d - %-9d ... ", r, res & 0xFF);
          if ((res & 0xFF) != t) {
            printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
            bad |= 1;
            break;
          }
        }
        if (t == 4)
          printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");
      }
    }
  }
  fssrMasterReset(id);
  return bad;
}

int
fssrDiffLineTest(int id, int chip)
{
  uint32_t res, ref;
  int bad = 0;
  int tstat1 = 0, tstat2 = 0;
  int i;
  uint32_t buf[100];

#ifdef VXWORKS
  int ticks = 1;
#else
  struct timespec ts; 
  ts.tv_sec = 0;
  ts.tv_nsec = 16666667; /* ~16.6 ms (similar to 1 tick on VxWorks) */
#endif

  if (vscmIsNotInit(&id, __func__))
    return 1;

  // Check Output Clock by counting number of words
  // Note that this could fail due to Out1 being bad
  printf("%-11s ... ", "OutClk");
  fssrSetActiveLines(id, chip, FSSR_ALINES_1);
  vscmLatchScalers(id); 
  vscmLatchChipScaler(id, chip);	
  vscmEnableChipScaler(id, chip);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmDisableChipScaler(id, chip);	

  res = vmeRead32(&VSCMpr[id]->Fssr[chip].ScalerWords);
  ref = vmeRead32(&VSCMpr[id]->Fssr[chip].ScalerRef);
  // 70e6 = MCLK rate, 12.0 factor comes from using 1 output line
  if ((int)(res - ((double)ref / VSCM_SYS_CLK * (70e6 / 12.0))) != 0) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");

  // Check MasterReset by setting regsiter 3 to 0xFF then
  // verifiying is default value (139) after reset
  printf("%-11s ... ", "MasterReset");
  fssrTransfer(id, chip, 3, FSSR_CMD_SET, 1, NULL);
  fssrMasterReset(id);
  fssrTransfer(id, chip, 3, FSSR_CMD_READ, 9, &res);
  if (res != 139) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");

  // Check GotHit and CoreTalking debug lines by first setting 
  // threshold to max so that there should be 0 hits on both.
  // Next lower threshold slightly and inject 1000 pulses (300mV)
  // and check that both counts are within 0.5% (to allow for noise)
  vscmLatchScalers(id); 
  vscmLatchChipScaler(id, chip);	
  fssrSetControl(id, chip, 0x1F);
  fssrSetActiveLines(id, chip, FSSR_ALINES_6);
  fssrRejectHits(id, chip, 0);
  fssrSCR(id, chip);
  fssrSendData(id, chip, 1);
  vscmEnableChipScaler(id, chip);

  fssrSetThreshold(id, chip, 0, 255);
  vscmDisableChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].ScalerGotHit);
  if (res != 0)
    tstat1 |= 1;
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].ScalerCoreTalking);
  if (res != 0)
    tstat2 |= 1;
 
  fssrSetThreshold(id, chip, 0, 200);
  vscmPulser(id, ((chip > 3) ? 2 : 1), 300, 1000);
  fssrInjectMaskEnableSingle(id, chip, 87);
  vscmEnableChipScaler(id, chip);
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmDisableChipScaler(id, chip);	
  printf("%-11s ... ", "GotHit");
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].ScalerGotHit);
  if ((res > 1005 || res < 995) || tstat1) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");

  printf("%-11s ... ", "CoreTalking");
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].ScalerCoreTalking);
  if ((res > 1005 || res < 995) || tstat2) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");

  tstat1 = 0;
  // Check output line 6 via status word, it has bits 23-20
  // bit 23 - SendData (register 19)
  // bit 22 - RejectHits (register 20)
  // bit 21 - output lines MSB (register 16)
  // bit 20 - output lines LSB
  // Only check top two bits (0b1100) of status word
  printf("%-11s ... ", "Out6");
  fssrMasterReset(id);
  fssrSetActiveLines(id, chip, FSSR_ALINES_6);
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastStatusWord);
  if (((res >> 20) & 0xC) != 4)
    tstat1 = 1;
  fssrRejectHits(id, chip, 0);
  fssrSCR(id, chip);
  fssrSendData(id, chip, 1);  
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastStatusWord);
  if (((res >> 20) & 0xC) != 8 || tstat1) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");

  tstat1 = 0;
  // Check output line 5 via status word, it has bits 19-16
  // bit 19 - AqBCO
  // bit 18 - Mod255 (bit 4 of register 27)
  // bit 17 - Pulser Active (register 2)
  // bit 16 - Chip ID MSB
  // Only check middle two bits (0b0110) of status word
  printf("%-11s ... ", "Out5");
  fssrMasterReset(id);
  fssrSetActiveLines(id, chip, FSSR_ALINES_6);
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastStatusWord);
  if (((res >> 16) & 6) != 0)
    tstat1 = 1;
  fssrSetControl(id, chip, (1 << 4)); // Set bit 18
  fssrInternalPulserEnable(id, chip); // Set bit 17
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastStatusWord);
  if (((res >> 16) & 6) != 6 || tstat1) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");

  tstat1 = 0;
  // Check output line 4 via data word, it has bits 15-12
  // bit 15 - Logic Set - bit 3
  // bit 14 - Logic Set - bit 2
  // bit 13 - Logic Set - bit 1
  // bit 12 - Logic Set - bit 0
  printf("%-11s ... ", "Out4");
  fssrMasterReset(id);
  fssrSetControl(id, chip, 0x1F);
  fssrSetActiveLines(id, chip, FSSR_ALINES_6);
  fssrRejectHits(id, chip, 0);
  fssrSCR(id, chip);
  fssrSendData(id, chip, 1);
  vscmFifoClear(id);

  // First select channel 4 which has column set: 0b01010 = 10
  fssrSetThreshold(id, chip, 0, 200);
  vscmPulser(id, ((chip > 3) ? 2 : 1), 300, 1000);
  fssrKillMaskDisableAll(id, chip);
  fssrKillMaskEnableSingle(id, chip, 4);
  fssrInjectMaskDisableAll(id, chip);
  fssrInjectMaskEnableSingle(id, chip, 4);
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if (((res >> 12) & 0x1F) != 10)
    tstat1 = 1;

  // Next select channel 77 which has column set: 0b10101 = 21
  fssrKillMaskDisableAll(id, chip);
  fssrKillMaskEnableSingle(id, chip, 77);
  fssrInjectMaskDisableAll(id, chip);
  fssrInjectMaskEnableSingle(id, chip, 77);
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if (((res >> 12) & 0x1F) != 21 || tstat1) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");

  tstat1 = tstat2 = 0;
  // Check output line 3 via data word, it has bits 11-8 
  // bit 11 - BCO # - bit 8
  // bit 10 - BCO # - bit 7
  // bit  9 - BCO # - bit 6
  // bit  8 - BCO # - bit 5
  printf("%-11s ... ", "Out3");
  fssrMasterReset(id);
  fssrSetControl(id, chip, 0x1F);
  fssrSetActiveLines(id, chip, FSSR_ALINES_6);
  fssrRejectHits(id, chip, 0);
  fssrSCR(id, chip);
  fssrSendData(id, chip, 1);
  vscmFifoClear(id);

  fssrSetThreshold(id, chip, 0, 200);
  vscmPulser(id, ((chip > 3) ? 2 : 1), 300, 1000);
  // +5 to move to next BCO and +8 to center pulse
  vscmPulserDelay(id, 13);
  fssrKillMaskDisableAll(id, chip);
  fssrKillMaskEnableSingle(id, chip, 87);
  fssrInjectMaskDisableAll(id, chip);
  fssrInjectMaskEnableSingle(id, chip, 87);

  // First sync to BCO 0
  vscmPulserBCOSync(id, 0, 1);
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if (((res >> 4) & 0xF0) != 0)
    tstat1 = 1;

  // Next sync to BCO 80 (0b0101xxxx)
  vscmPulserBCOSync(id, 80, 1);
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if (((res >> 4) & 0xF0) != 80)
    tstat2 = 1;

  // Finally sync to BCO 160 (0b1010xxxx)
  vscmPulserBCOSync(id, 160, 1);
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if (((res >> 4) & 0xF0) != 160 || tstat1 || tstat2) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");

  tstat1 = tstat2 = 0;
  // Check output line 2 via data word, it has bits 7-4 
  // bit 7 - BCO # - bit 4
  // bit 5 - BCO # - bit 3
  // bit 5 - BCO # - bit 2
  // bit 4 - BCO # - bit 1
  printf("%-11s ... ", "Out2");
  fssrMasterReset(id);
  fssrSetControl(id, chip, 0x1F);
  fssrSetActiveLines(id, chip, FSSR_ALINES_6);
  fssrRejectHits(id, chip, 0);
  fssrSCR(id, chip);
  fssrSendData(id, chip, 1);
  vscmFifoClear(id);

  fssrSetThreshold(id, chip, 0, 200);
  vscmPulser(id, ((chip > 3) ? 2 : 1), 300, 1000);
  // +5 to move to next BCO and +8 to center pulse
  // Will need to subtract 2 from BCO number
  vscmPulserDelay(id, 13);
  fssrKillMaskDisableAll(id, chip);
  fssrKillMaskEnableSingle(id, chip, 87);
  fssrInjectMaskDisableAll(id, chip);
  fssrInjectMaskEnableSingle(id, chip, 87);

  // First sync to BCO 0
  vscmPulserBCOSync(id, 0, 1);
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if ((((res >> 4) & 0xF) - 2) != 0)
    tstat1 = 1;

  // Next sync to BCO 5 (0bxxxx0101)
  vscmPulserBCOSync(id, 5, 1);
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if ((((res >> 4) & 0xF) - 2) != 5)
    tstat2 = 1;

  // Finally sync to BCO 10 (0bxxxx1010)
  vscmPulserBCOSync(id, 10, 1);
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if ((((res >> 4) & 0xF) - 2) != 10 || tstat1 || tstat2) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");

  tstat1 = 0;
  // Check output line 1 via data word, it has bits 3-0
  // bit 3 - fADC - bit 3
  // bit 2 - fADC - bit 2
  // bit 1 - fADC - bit 1
  // bit 0 - Word Mark
  printf("%-11s ... ", "Out1");
  fssrMasterReset(id);
  fssrSetControl(id, chip, 0x10);
  fssrSetActiveLines(id, chip, FSSR_ALINES_6);
  fssrRejectHits(id, chip, 0);
  fssrSCR(id, chip);
  fssrSendData(id, chip, 1);
  vscmFifoClear(id);

  fssrSetThreshold(id, chip, 0, 100);
  vscmPulser(id, ((chip > 3) ? 2 : 1), 300, 1000);
  // Disable Pulser Delay and BCO Sync from previous output checks
  vscmPulserDelay(id, 0);
  vscmPulserBCOSync(id, 0, 0);
  fssrKillMaskDisableAll(id, chip);
  fssrKillMaskEnableSingle(id, chip, 87);
  fssrInjectMaskDisableAll(id, chip);
  fssrInjectMaskEnableSingle(id, chip, 87);

  // All fADC discriminators are at 255 after reset, fADC should = 0
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if (((res >> 1) & 7) != 0)
    tstat1 = 1;

  // Set all fADC discriminators to 0, fADC should = 7
  for (i = 1; i < 8; i++) {
    fssrSetThreshold(id, chip, i, 0);
  }
  vscmPulserStart(id);
#ifdef VXWORKS
  taskDelay(ticks);
#else
  nanosleep(&ts, NULL);
#endif
  vscmLatchChipScaler(id, chip);
  res = vmeRead32(&VSCMpr[id]->Fssr[chip].LastDataWord);
  if (((res >> 1) & 7) != 7 || tstat1) {
    printf("%sFAIL%s\n", "\x1B[1;31m", "\x1B[0m");
    bad |= 1;
  }
  else
    printf("%sok%s\n", "\x1B[1;32m", "\x1B[0m");
 
  return bad; 
}

#else /* Dummy Version */

void
vscmScan_dummy()
{
  return;
}

#endif
