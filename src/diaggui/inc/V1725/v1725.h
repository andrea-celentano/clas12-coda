/*Copied from CODA*/

#ifndef V1725_H
#define V1725_H

/* single channel registers map (addresses for channel 1 will be from 0x1100, and so on) */
typedef struct v1725channel_struct /*1n00-1nFC*/
{
  /*0x1000-0x1020*/ unsigned int blank0[9];
  /*0x1024*/        volatile unsigned int dummy32;
  /*0x1028*/        volatile unsigned int gain;
  /*0x102C-0x106C*/ unsigned int blank1[17];
  /*0x1070*/        volatile unsigned int pulse_width;
  /*0x1074-0x107C*/ unsigned int blank2[3];
  /*0x1080*/        volatile unsigned int threshold;
  /*0x1084*/        volatile unsigned int self_trigger_logic;
  /*0x1088*/        volatile unsigned int status;
  /*0x108C*/        volatile unsigned int amc_fpga_firmware_rev;
  /*0x1090-0x1904*/ unsigned int blank3[2];
  /*0x1098*/        volatile unsigned int dac_offset;
  /*0x109C-0x10A4*/ unsigned int blank4[3];
  /*0x10A8*/        volatile unsigned int temperature_monitor;
  /*0x10AC-0x10FC*/ unsigned int blank5[21];
} V1725CH;


/* ROM address map */
struct v1725_ROM_struct {
  /*0xF000*/ volatile unsigned int checksum;
  /*0xF004*/ volatile unsigned int checksum_length2;
  /*0xF008*/ volatile unsigned int checksum_length1;
  /*0xF00C*/ volatile unsigned int checksum_length0;
  /*0xF010*/ volatile unsigned int constant2;
  /*0xF014*/ volatile unsigned int constant1;
  /*0xF018*/ volatile unsigned int constant0;
  /*0xF01C*/ volatile unsigned int c_code;
  /*0xF020*/ volatile unsigned int r_code;
  /*0xF024*/ volatile unsigned int oui2;
  /*0xF028*/ volatile unsigned int oui1;
  /*0xF02C*/ volatile unsigned int oui0;
  /*0xF030*/ volatile unsigned int vers;
  /*0xF034*/ volatile unsigned int form_factor;
  /*0xF038*/ volatile unsigned int boardID1;
  /*0xF03C*/ volatile unsigned int boardID0;
  /*0xF040*/ volatile unsigned int revis3;
  /*0xF044*/ volatile unsigned int revis2;
  /*0xF048*/ volatile unsigned int revis1;
  /*0xF04C*/ volatile unsigned int revis0;
  /*0xF050*/ volatile unsigned int FLASHtype;
  /*0xF054-0xF07C*/ unsigned int blank0[11];
  /*0xF080*/ volatile unsigned int semum1;
  /*0xF084*/ volatile unsigned int semum0;
  /*0xF088*/ volatile unsigned int VCXOtype;

} V1725ROM;



/* registers address map */
typedef struct v1725_struct
{
  /*0x0000-0x0FFC*/ volatile unsigned int data[1024];
  /*0x1000-0x1FFC*/	volatile struct v1725channel_struct chan[16];
  /*0x2000-0x7FFC*/ unsigned int blank0[6144];
  /*0x8000*/        volatile unsigned int chan_config;
  /*0x8004*/        volatile unsigned int chan_config_bit_set;
  /*0x8008*/        volatile unsigned int chan_config_bit_clear;
  /*0x800C*/        volatile unsigned int buffer_organization;
  /*0x8010-0x801C*/ unsigned int blank1[4];
  /*0x8020*/        volatile unsigned int custom_size;
  /*0x8024-0x8098*/ unsigned int blank2[30];
  /*0x809C*/        volatile unsigned int channel_calibration;
  /*0x80A0-0x80FC*/ unsigned int blank3[24];
  /*0x8100*/        volatile unsigned int acquisition_control;
  /*0x8104*/        volatile unsigned int acquisition_status;
  /*0x8108*/        volatile unsigned int sw_trigger;
  /*0x810C*/        volatile unsigned int trigger_source_enable_mask;
  /*0x8110*/        volatile unsigned int front_panel_trigger_out_enable_mask;
  /*0x8114*/        volatile unsigned int post_trigger_setting;
  /*0x8118*/        volatile unsigned int front_panel_io_data;
  /*0x811C*/        volatile unsigned int front_panel_io_control;
  /*0x8120*/        volatile unsigned int channel_enable_mask;
  /*0x8124*/        volatile unsigned int roc_fpga_firmware_rev;
  /*0x8128*/        unsigned int blank4;
  /*0x812C*/        volatile unsigned int event_stored;
  /*0x8130-0x8134*/ unsigned int blank5[2];
  /*0x8138*/        volatile unsigned int monitor_dac;
  /*0x813C*/        volatile unsigned int sw_clk_sync;
  /*0x8140*/        volatile unsigned int board_info;
  /*0x8144*/        volatile unsigned int monitor_mode;
  /*0x8148*/        unsigned int blank6;
  /*0x814C*/        volatile unsigned int event_size;
  /*0x8150-0x8164*/ unsigned int blank7[6];
  /*0x8168*/		volatile unsigned int fan_control;
  /*0x816C*/        volatile unsigned int memory_buffer_almost_full_level;
  /*0x8170*/        volatile unsigned int run_start_stop_delay;
  /*0x8174*/        unsigned int blank8;
  /*0x8178*/        volatile unsigned int board_fail_status;
  /*0x817C-0x819C*/ unsigned int blank9[9];
  /*0x81A0*/        volatile unsigned int front_panel_lvds_io_new_features;
  /*0x81A4-0x81B0*/ unsigned int blank10[4];
  /*0x81B4*/		volatile unsigned int buffer_occupancy_gain;
  /*0x81B8-0x81BC*/	unsigned int blank11[2];
  /*0x81C0*/        volatile unsigned int channels_shutdown;
  /*0x81C4*/		volatile unsigned int extended_veto_delay;
  /*0x81C8-0xEEFC*/ unsigned int blank12[6990];

  /*0xEF00*/        volatile unsigned int readout_control;
  /*0xEF04*/        volatile unsigned int readout_status;
  /*0xEF08*/        volatile unsigned int geo_address/*board_id*/;
  /*0xEF0C*/        volatile unsigned int multicast_base_address_and_control;
  /*0xEF10*/        volatile unsigned int relocation_address;
  /*0xEF14*/        volatile unsigned int interrupt_status_id;
  /*0xEF18*/        volatile unsigned int interrupt_event_number;
  /*0xEF1C*/        volatile unsigned int blt_event_number;
  /*0xEF20*/        volatile unsigned int scratch;
  /*0xEF24*/        volatile unsigned int sw_reset;
  /*0xEF28*/        volatile unsigned int sw_clear;
  /*0xEF2C*/        volatile unsigned int flash_enable;
  /*0xEF30*/        volatile unsigned int flash_data;
  /*0xEF34*/        volatile unsigned int config_reload;
  /*0xEF38-0xEFFC*/ unsigned int blank13[50];
  /*0xF000-0xF088*/ volatile struct v1725_ROM_struct rom;
} V1725;


#endif
