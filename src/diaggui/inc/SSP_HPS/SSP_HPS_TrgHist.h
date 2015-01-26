#ifndef SSP_HPS_TrgHist_H
#define SSP_HPS_TrgHist_H

#include <stdlib.h>
#include "ModuleFrame.h"
#include "RootHeader.h"

#define UDPATETIME_MAX			60

class SSP_HPS_TrgHist	: public TGCompositeFrame
{
public:
	SSP_HPS_TrgHist(const TGWindow *p, ModuleFrame *pModule) : TGCompositeFrame(p, 400, 400)
	{
		static int inst = 0;

		SetLayoutManager(new TGVerticalLayout(this));

		pM = pModule;

		TGCompositeFrame *pTF1;

		AddFrame(pTF1 = new TGHorizontalFrame(this), new TGLayoutHints(kLHintsExpandX | kLHintsTop));
			pTF1->AddFrame(pButtonNormalize = new TGTextButton(pTF1, new TGHotString("Normalize")), new TGLayoutHints(kLHintsCenterY | kLHintsLeft));
				pButtonNormalize->AllowStayDown(kTRUE);
			pTF1->AddFrame(pButtonAutoUpdate = new TGTextButton(pTF1, new TGHotString("Update Mode: Manual"), BTN_AUTOUPDATE), new TGLayoutHints(kLHintsCenterY | kLHintsLeft));
				pButtonAutoUpdate->SetWidth(80);
				pButtonAutoUpdate->SetEnabled(kTRUE);
				pButtonAutoUpdate->AllowStayDown(kTRUE);
				pButtonAutoUpdate->Associate(this);
			pTF1->AddFrame(pButtonManualUpdate = new TGTextButton(pTF1, new TGHotString("Manual Update"), BTN_MANUALUPDATE), new TGLayoutHints(kLHintsCenterY | kLHintsLeft));
				pButtonManualUpdate->Associate(this);
			pTF1->AddFrame(pSliderUpdateTime = new TGHSlider(pTF1, 100, kSlider1 | kScaleBoth, SDR_UPDATETIME), new TGLayoutHints(kLHintsExpandX | kLHintsCenterY | kLHintsLeft));
				pSliderUpdateTime->SetRange(0, UDPATETIME_MAX);
//				pSliderUpdateTime->SetEnabled(kFALSE);
				pSliderUpdateTime->SetPosition(5);
				pSliderUpdateTime->Associate(this);

		AddFrame(pCanvasRates = new TRootEmbeddedCanvas("c1", this, 1300, 125));//, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
				
		TGCanvas *pTGCanvas;
		AddFrame(pTGCanvas = new TGCanvas(this), new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
			pTGCanvas->SetContainer(pTF1 = new TGVerticalFrame(pTGCanvas->GetViewPort()));
				pTF1->AddFrame(pCanvas = new TRootEmbeddedCanvas("c1", pTF1, 1300, 2300));//, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

		gStyle->SetPalette(1, NULL);

		pCanvas->GetCanvas()->Divide(1,4);

		pCanvas->GetCanvas()->cd(1);
		pHistLatency = new TH1I("ClusterLatency", "ClusterLatency", 1024, 0.0, 4.0*1024.0);
		pHistLatency->GetXaxis()->SetTitle("Latency(ns)");
		pHistLatency->GetXaxis()->SetRangeUser(0.0, 4096.0);
		pHistLatency->GetYaxis()->SetTitle("Counts");
		pHistLatency->GetYaxis()->CenterTitle();
		pHistLatency->SetLineColor(kBlue);
		//pHistLatency->SetStats(0);
		pHistLatency->Draw();
		//pLegend = new TLegend(0.8,0.8,0.9,0.9);
		//pLegend->AddEntry(pHistLatency, "GTP Clusters");
		//pLegend->Draw();

		pCanvas->GetCanvas()->cd(2);
		pHistPosition = new TH2I("ClusterPosition", "ClusterPosition;X;Y", 46, -22.0, 24.0, 11, -5.0, 6.0);
		pHistPosition->SetStats(0);
		pHistPosition->GetXaxis()->CenterLabels();
		pHistPosition->GetXaxis()->SetNdivisions(46, kFALSE);
		pHistPosition->GetXaxis()->SetTickLength(1);
		pHistPosition->GetYaxis()->CenterLabels();
		pHistPosition->GetYaxis()->SetNdivisions(11, kFALSE);
		pHistPosition->GetYaxis()->SetTickLength(1);
		pHistPosition->Draw("COLZTEXT");

		int x = 23;
		for(int n = 1; n <= 46; n++)
		{
			pHistPosition->GetXaxis()->SetBinLabel(n,Form("%d", x));
			x--;
			if(x == 0) x--;
		}
		
		pCanvas->GetCanvas()->cd(3);
		pHistEnergy = new TH1I("ClusterEnergy", "ClusterEnergy", 1024, 0.0, 8.0*1024.0);
		pHistEnergy->GetXaxis()->SetTitle("Energy(MeV)");
		pHistEnergy->GetXaxis()->SetRangeUser(0.0, 8192.0);
		pHistEnergy->GetYaxis()->SetTitle("Counts");
		pHistEnergy->GetYaxis()->CenterTitle();
		pHistEnergy->SetLineColor(kBlue);
		//pHistEnergy->SetStats(0);
		pHistEnergy->Draw();
		//pLegend = new TLegend(0.8,0.8,0.9,0.9);
		//pLegend->AddEntry(pHistEnergy, "GTP Clusters");
		//pLegend->Draw();

		pCanvas->GetCanvas()->cd(4);
		pHistNHits = new TH1I("ClusterNHits", "ClusterNHits", 9, 0.0, 9.0);
		pHistNHits->GetXaxis()->SetTitle("NHits");
		pHistNHits->GetXaxis()->SetRangeUser(0.0, 9.0);
		pHistNHits->GetXaxis()->CenterLabels();
		pHistNHits->GetYaxis()->SetTitle("Counts");
		pHistNHits->GetYaxis()->CenterTitle();
		pHistNHits->SetLineColor(kBlue);
		//pHistNHits->SetStats(0);
		pHistNHits->Draw();
		//pLegend = new TLegend(0.8,0.8,0.9,0.9);
		//pLegend->AddEntry(pHistNHits, "GTP Clusters");
		//pLegend->Draw();

		pCanvas->GetCanvas()->Modified();
		pCanvas->GetCanvas()->Update();

		pTimerUpdate = new TTimer(this, 1000*pSliderUpdateTime->GetPosition(), kTRUE);

		HpsCluster_HistCtrl		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0500);
		HpsCluster_HistLatency	= (volatile unsigned int *)((int)pM->BaseAddr + 0x0510);
		HpsCluster_HistPosition	= (volatile unsigned int *)((int)pM->BaseAddr + 0x0514);
		HpsCluster_HistEnergy	= (volatile unsigned int *)((int)pM->BaseAddr + 0x0518);
		HpsCluster_HistNHits		= (volatile unsigned int *)((int)pM->BaseAddr + 0x051C);
		HpsCluster_RefTime		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0520);

		HpsSingles0_Pass			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0780);
		HpsSingles0_Tot			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0784);
		HpsSingles1_Pass			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0880);
		HpsSingles1_Tot			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0884);
		HpsPairs0_Pass				= (volatile unsigned int *)((int)pM->BaseAddr + 0x0980);
		HpsPairs0_SumPass			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0984);
		HpsPairs0_DiffPass		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0988);
		HpsPairs0_EDPass			= (volatile unsigned int *)((int)pM->BaseAddr + 0x098C);
		HpsPairs0_CoplanarPass	= (volatile unsigned int *)((int)pM->BaseAddr + 0x0990);
		HpsPairs0_TriggerPass	= (volatile unsigned int *)((int)pM->BaseAddr + 0x0994);
		HpsPairs1_Pass				= (volatile unsigned int *)((int)pM->BaseAddr + 0x0A80);
		HpsPairs1_SumPass			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0A84);
		HpsPairs1_DiffPass		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0A88);
		HpsPairs1_EDPass			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0A8C);
		HpsPairs1_CoplanarPass	= (volatile unsigned int *)((int)pM->BaseAddr + 0x0A90);
		HpsPairs1_TriggerPass	= (volatile unsigned int *)((int)pM->BaseAddr + 0x0A94);
		HpsScaler_Disable			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0300);
		HpsScaler_Sysclk50		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0304);
		
		HpsScaler_Trig1			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0310);
		HpsScaler_Busy				= (volatile unsigned int *)((int)pM->BaseAddr + 0x0348);
		HpsScaler_BusyCycles		= (volatile unsigned int *)((int)pM->BaseAddr + 0x034C);
		HpsScaler_P2LVDSOut0		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0370);
		HpsScaler_P2LVDSOut1		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0374);
		HpsScaler_P2LVDSOut2		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0378);
		HpsScaler_P2LVDSOut3		= (volatile unsigned int *)((int)pM->BaseAddr + 0x037C);
		HpsScaler_P2LVDSOut4		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0380);
		HpsScaler_P2LVDSOut5		= (volatile unsigned int *)((int)pM->BaseAddr + 0x0384);
		HpsScaler_TrigBusy0		= (volatile unsigned int *)((int)pM->BaseAddr + 0x2110);
		HpsScaler_TrigBusy1		= (volatile unsigned int *)((int)pM->BaseAddr + 0x2114);
		HpsScaler_TrigBusy2		= (volatile unsigned int *)((int)pM->BaseAddr + 0x2118);
		HpsScaler_TrigBusy3		= (volatile unsigned int *)((int)pM->BaseAddr + 0x211C);
		HpsScaler_TrigBusy4		= (volatile unsigned int *)((int)pM->BaseAddr + 0x2120);
		HpsScaler_TrigBusy5		= (volatile unsigned int *)((int)pM->BaseAddr + 0x2124);

		HpsScaler_Cosmic			= (volatile unsigned int *)((int)pM->BaseAddr + 0x0B2C);
		HpsScaler_Led				= (volatile unsigned int *)((int)pM->BaseAddr + 0x0B1C);

		inst++;
	}
	
	virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t)
	{
		switch(GET_MSG(msg))
		{
		case kC_COMMAND:
			switch(GET_SUBMSG(msg))
			{
			case kCM_BUTTON:
				switch(parm1)
				{
					case BTN_AUTOUPDATE:
						if(pButtonAutoUpdate->IsDown())
						{
							pButtonAutoUpdate->SetText(new TGHotString("Update Mode: Auto"));
//							pSliderUpdateTime->SetEnabled(kTRUE);
							pTimerUpdate->Start(1000*pSliderUpdateTime->GetPosition(), kTRUE);
						}
						else
						{
							pButtonAutoUpdate->SetText(new TGHotString("Update Mode: Manual"));
//							pSliderUpdateTime->SetEnabled(kFALSE);
							pTimerUpdate->TurnOff();
						}
						break;
					case BTN_MANUALUPDATE:
						UpdateHistogram();
						break;
					default:
						printf("button id %d pressed\n", (int)parm1);
						break;
				}
				break;
			case kC_HSLIDER:
				switch(parm1)
				{
					case SDR_UPDATETIME:
						pTimerUpdate->TurnOff();
						pTimerUpdate->Start(1000*pSliderUpdateTime->GetPosition(), kTRUE);
						break;
					default:
						printf("slider id %d pressed\n", (int)parm1);
						break;
				}
				break;
			}
			break;
		}
		return kTRUE;
	}

	virtual Bool_t HandleTimer(TTimer *t)
	{
		if(pTimerUpdate->HasTimedOut())
		{
			UpdateHistogram();
			if(pButtonAutoUpdate->IsDown())
				pTimerUpdate->Start(1000*pSliderUpdateTime->GetPosition(), kTRUE);
		}
		return kTRUE;
	}

	void UpdateLatencyHistogram(float scale, Bool_t normalize)
	{
		unsigned int buf[1024];

		pM->BlkReadReg32(HpsCluster_HistLatency, buf, 1024, CRATE_MSG_FLAGS_NOADRINC);
		
		pCanvas->GetCanvas()->cd(1);
		pHistLatency->Reset();

		pCanvas->GetCanvas()->SetLogy(1);

		if(normalize)
			pHistLatency->GetYaxis()->SetTitle("Hz");
		else
			pHistLatency->GetYaxis()->SetTitle("Counts");
		
		for(int i = 0; i < 1024; i++)
		{
			if(normalize)
			{
				float val = (float)buf[i] * scale;
				buf[i] = (int)val;
			}

			if(buf[i] > 0x80000000)
				pHistLatency->Fill(4*i, 0x7FFFFFFF);
			else
				pHistLatency->Fill(4*i, buf[i]);
		}

		pCanvas->GetCanvas()->Modified();
		pCanvas->GetCanvas()->Update();
	}

	void UpdatePositionHistogram(float scale, Bool_t normalize)
	{
		unsigned int buf[1024];
		float rate_top = 0.0, rate_bot = 0.0;
		static bool called=0;

    static TPaveText tt1(0.1,0.9,0.3,1.0,"NDC");
    static TPaveText tt2(0.7,0.91,0.9,0.99,"NDC");
    static TPaveText ttT(-22+13+0.05,6-5,-22+22,7-5-0.05);
    static TPaveText ttB(-22+13+0.05,4-5+0.05,-22+22,5-5);
    static TPaveText ttM(-22+0+0.05,5-5+0.05,-22+13,6-5);
    static TBox bb;
    static TLine ll;

    if (!called)
    {
        called=1;
        bb.SetFillStyle(1001);
        bb.SetFillColor(kWhite);
        bb.SetLineWidth(1);
        bb.SetLineColor(kBlack);
        tt1.SetBorderSize(0);
        tt2.SetBorderSize(0);
        tt1.SetFillColor(kWhite);
        tt2.SetFillColor(kWhite);
        ttT.SetBorderSize(0);
        ttB.SetBorderSize(0);
        ttT.SetFillColor(kWhite);
        ttB.SetFillColor(kWhite);
        ttM.SetBorderSize(0);
        ttM.SetFillColor(kWhite);
        ttM.SetTextColor(kRed);
    }

    unsigned int max=0;
		
		pM->BlkReadReg32(HpsCluster_HistPosition, buf, 1024, CRATE_MSG_FLAGS_NOADRINC);

		pCanvas->GetCanvas()->cd(2);
		pHistPosition->SetMinimum(0);
		pHistPosition->Reset();

		pCanvas->GetCanvas()->SetLogz(1);

		int x, y;
		for(int i = 0; i < 1024; i++)
		{
			if(normalize)
			{
				float val = (float)buf[i] * scale;
				buf[i] = (int)val;
			}

			if(buf[i] > max)
				max = buf[i];

			if(buf[i] > 0x80000000)
				buf[i] = 0x7FFFFFFF;

			x = (i>>0) & 0x3F;
			y = (i>>6) & 0xF;

			if(x & 0x20) x |= 0xFFFFFFC0;
			if(y & 0x08) y |= 0xFFFFFFF0;

			if(y > 0)
				rate_top += buf[i];
			else
				rate_bot += buf[i];

			pHistPosition->Fill(x, y, buf[i]);
		}

    bb.DrawBox(-9+0.05,-1,0,1.97);
    bb.DrawBox(-24,0,24.05,0.97);
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmin(),pHistPosition->GetYaxis()->GetXmin(),
                pHistPosition->GetXaxis()->GetXmax(),pHistPosition->GetYaxis()->GetXmin());
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmin(),pHistPosition->GetYaxis()->GetXmax(),
                pHistPosition->GetXaxis()->GetXmax(),pHistPosition->GetYaxis()->GetXmax());
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmin(),pHistPosition->GetYaxis()->GetXmin(),
                pHistPosition->GetXaxis()->GetXmin(),0);
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmax(),pHistPosition->GetYaxis()->GetXmin(),
                pHistPosition->GetXaxis()->GetXmax(),0);
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmin(),pHistPosition->GetYaxis()->GetXmax(),
                pHistPosition->GetXaxis()->GetXmin(),1);
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmax(),pHistPosition->GetYaxis()->GetXmax(),
                pHistPosition->GetXaxis()->GetXmax(),1);
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmax(),0,0,0);
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmax(),1,0,1);
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmin(),0,-9,0);
    ll.DrawLine(pHistPosition->GetXaxis()->GetXmin(),1,-9,1);
    ll.DrawLine(-9,-1,0,-1);
    ll.DrawLine(-9,2,0,2);
    ll.DrawLine(-9,1,-9,2);
    ll.DrawLine(-9,-1,-9,0);
    ll.DrawLine(0,-1,0,0);
    ll.DrawLine(0,1,0,2);
                
    tt1.Clear();
    tt2.Clear();
    ttT.Clear();
    ttB.Clear();
    ttM.Clear();

		if(normalize)
		{
			tt1.AddText(Form("Total Rate:  %.1E Hz",rate_top+rate_bot));
			tt2.AddText(Form("Total Rate:  %.1f kHz",(rate_top+rate_bot)/1000.0));
			ttT.AddText(Form("%.2f kHz",rate_top/1000.0));
			ttB.AddText(Form("%.2f kHz",rate_bot/1000.0));
			ttM.AddText(Form("MAX SINGLE CRYSTAL = %.2f kHz",(float)max/1000));
		}
		else
		{
			tt1.AddText(Form("Total Count:  %.1E",rate_top+rate_bot));
			ttM.AddText(Form("MAX SINGLE CRYSTAL = %.2f count",(float)max));
		}
		tt1.Draw();
		tt2.Draw();
		ttT.Draw();
		ttB.Draw();
		ttM.Draw();
		pCanvas->GetCanvas()->Modified();
		pCanvas->GetCanvas()->Update();
	}

	void UpdateEnergyHistogram(float scale, Bool_t normalize)
	{
		unsigned int buf[1024];

		pM->BlkReadReg32(HpsCluster_HistEnergy, buf, 1024, CRATE_MSG_FLAGS_NOADRINC);
		
		pCanvas->GetCanvas()->cd(3);
		pHistEnergy->Reset();

		pCanvas->GetCanvas()->SetLogy(1);

		if(normalize)
			pHistEnergy->GetYaxis()->SetTitle("Hz");
		else
			pHistEnergy->GetYaxis()->SetTitle("Counts");
		
		for(int i = 0; i < 1024; i++)
		{
			if(normalize)
			{
				float val = (float)buf[i] * scale;
				buf[i] = (int)val;
			}

			if(buf[i] > 0x80000000)
				pHistEnergy->Fill(8*i, 0x7FFFFFFF);
			else
				pHistEnergy->Fill(8*i, buf[i]);
		}

		pCanvas->GetCanvas()->Modified();
		pCanvas->GetCanvas()->Update();
	}

	void UpdateNHitsHistogram(float scale, Bool_t normalize)
	{
		unsigned int buf[16];

		pM->BlkReadReg32(HpsCluster_HistNHits, buf, 16, CRATE_MSG_FLAGS_NOADRINC);
		
		pCanvas->GetCanvas()->cd(4);
		pHistNHits->Reset();

		pCanvas->GetCanvas()->SetLogy(1);

		if(normalize)
			pHistNHits->GetYaxis()->SetTitle("Hz");
		else
			pHistNHits->GetYaxis()->SetTitle("Counts");
		
		for(int i = 0; i < 16; i++)
		{
			if(normalize)
			{
				float val = (float)buf[i] * scale;
				buf[i] = (int)val;
			}
			
			if(buf[i] > 0x80000000)
				pHistNHits->Fill(i, 0x7FFFFFFF);
			else
				pHistNHits->Fill(i, buf[i]);
		}

		pCanvas->GetCanvas()->Modified();
		pCanvas->GetCanvas()->Update();
	}

	void UpdateScalers(Bool_t normalize)
	{
		float singles_pass[2], singles_tot[2];
		float pairs_pass[2], pairs_sumpass[2], pairs_diffpass[2];
		float pairs_edpass[2], pairs_coplanarpass[2], pairs_triggerpass[2];
		float trig1, busy, busycycles, p2lvdsout[6], trigbusy[6];
		float led, cosmic;
		float ref, sysclk;

		pM->WriteReg32(HpsScaler_Disable, 1);
		singles_pass[0] = (float)pM->ReadReg32(HpsSingles0_Pass);
		singles_tot[0] = (float)pM->ReadReg32(HpsSingles0_Tot);
		singles_pass[1] = (float)pM->ReadReg32(HpsSingles1_Pass);
		singles_tot[1] = (float)pM->ReadReg32(HpsSingles1_Tot);
		pairs_pass[0] = (float)pM->ReadReg32(HpsPairs0_Pass);
		pairs_sumpass[0] = (float)pM->ReadReg32(HpsPairs0_SumPass);
		pairs_diffpass[0] = (float)pM->ReadReg32(HpsPairs0_DiffPass);
		pairs_edpass[0] = (float)pM->ReadReg32(HpsPairs0_EDPass);
		pairs_coplanarpass[0] = (float)pM->ReadReg32(HpsPairs0_CoplanarPass);
		pairs_triggerpass[0] = (float)pM->ReadReg32(HpsPairs0_TriggerPass);
		pairs_pass[1] = (float)pM->ReadReg32(HpsPairs1_Pass);
		pairs_sumpass[1] = (float)pM->ReadReg32(HpsPairs1_SumPass);
		pairs_diffpass[1] = (float)pM->ReadReg32(HpsPairs1_DiffPass);
		pairs_edpass[1] = (float)pM->ReadReg32(HpsPairs1_EDPass);
		pairs_coplanarpass[1] = (float)pM->ReadReg32(HpsPairs1_CoplanarPass);
		pairs_triggerpass[1] = (float)pM->ReadReg32(HpsPairs1_TriggerPass);
		
		trig1 = (float)pM->ReadReg32(HpsScaler_Trig1);
		busy = (float)pM->ReadReg32(HpsScaler_Busy);
		busycycles = (float)pM->ReadReg32(HpsScaler_BusyCycles);
		p2lvdsout[0] = (float)pM->ReadReg32(HpsScaler_P2LVDSOut0);
		p2lvdsout[1] = (float)pM->ReadReg32(HpsScaler_P2LVDSOut1);
		p2lvdsout[2] = (float)pM->ReadReg32(HpsScaler_P2LVDSOut2);
		p2lvdsout[3] = (float)pM->ReadReg32(HpsScaler_P2LVDSOut3);
		p2lvdsout[4] = (float)pM->ReadReg32(HpsScaler_P2LVDSOut4);
		p2lvdsout[5] = (float)pM->ReadReg32(HpsScaler_P2LVDSOut5);
		trigbusy[0] = (float)pM->ReadReg32(HpsScaler_TrigBusy0);
		trigbusy[1] = (float)pM->ReadReg32(HpsScaler_TrigBusy1);
		trigbusy[2] = (float)pM->ReadReg32(HpsScaler_TrigBusy2);
		trigbusy[3] = (float)pM->ReadReg32(HpsScaler_TrigBusy3);
		trigbusy[4] = (float)pM->ReadReg32(HpsScaler_TrigBusy4);
		trigbusy[5] = (float)pM->ReadReg32(HpsScaler_TrigBusy5);
		
		led = (float)pM->ReadReg32(HpsScaler_Led);
		cosmic = (float)pM->ReadReg32(HpsScaler_Cosmic);
	
		sysclk = (float)pM->ReadReg32(HpsScaler_Sysclk50);
				
		pM->WriteReg32(HpsScaler_Disable, 0);

printf("busycycles=%f\n", busycycles);
		
		if(/*normalize && */(sysclk <= 0.0))
		{
			printf("Error: UpdateScalers() ref not valid - normalization will not be done\n");
			normalize = kFALSE;
		}
		else
		{
			ref = 50.0E6 / sysclk;

			singles_pass[0] *= ref;
			singles_tot[0] *= ref;
			singles_pass[1] *= ref;
			singles_tot[1] *= ref;
			pairs_pass[0] *= ref;
			pairs_sumpass[0] *= ref;
			pairs_diffpass[0] *= ref;
			pairs_edpass[0] *= ref;
			pairs_coplanarpass[0] *= ref;
			pairs_triggerpass[0] *= ref;
			pairs_pass[1] *= ref;
			pairs_sumpass[1] *= ref;
			pairs_diffpass[1] *= ref;
			pairs_edpass[1] *= ref;
			pairs_coplanarpass[1] *= ref;
			pairs_triggerpass[1] *= ref;
			
			trig1 *= ref;
			busy *= ref;
			busycycles = ref * busycycles / 250.0E6;
			p2lvdsout[0] = ref * p2lvdsout[0];
			p2lvdsout[1] = ref * p2lvdsout[1];
			p2lvdsout[2] = ref * p2lvdsout[2];
			p2lvdsout[3] = ref * p2lvdsout[3];
			p2lvdsout[4] = ref * p2lvdsout[4];
			p2lvdsout[5] = ref * p2lvdsout[5];
			trigbusy[0] = ref * trigbusy[0] / 250.0E6;
			trigbusy[1] = ref * trigbusy[1] / 250.0E6; 
			trigbusy[2] = ref * trigbusy[2] / 250.0E6;
			trigbusy[3] = ref * trigbusy[3] / 250.0E6;
			trigbusy[4] = ref * trigbusy[4] / 250.0E6;
			trigbusy[5] = ref * trigbusy[5] / 250.0E6;
			
			led *= ref;
			cosmic *= ref;
		}

		static bool called=0;

 		static TPaveText tt_col0(0.0,0.0,0.1,1.0,"NDC");
		static TPaveText tt_col1(0.1,0.0,0.2,1.0,"NDC");
		static TPaveText tt_col2(0.2,0.0,0.3,1.0,"NDC");
		static TPaveText tt_col3(0.3,0.0,0.4,1.0,"NDC");
		static TPaveText tt_col4(0.4,0.0,0.5,1.0,"NDC");
		static TPaveText tt_col5(0.5,0.0,0.6,1.0,"NDC");
		static TPaveText tt_col6(0.6,0.0,0.7,1.0,"NDC");
		static TPaveText tt_col7(0.7,0.0,0.8,1.0,"NDC");
		
		if (!called)
		{
			called=1;

			tt_col0.SetBorderSize(0);
			tt_col1.SetBorderSize(0);
			tt_col2.SetBorderSize(0);
			tt_col3.SetBorderSize(0);
			tt_col4.SetBorderSize(0);
			tt_col5.SetBorderSize(0);
			tt_col6.SetBorderSize(0);
			tt_col7.SetBorderSize(0);

			tt_col0.SetFillColor(kWhite);
			tt_col1.SetFillColor(kWhite);
			tt_col2.SetFillColor(kWhite);
			tt_col3.SetFillColor(kWhite);
			tt_col4.SetFillColor(kWhite);
			tt_col5.SetFillColor(kWhite);
			tt_col6.SetFillColor(kWhite);
			tt_col7.SetFillColor(kWhite);
		}
		
		pCanvasRates->GetCanvas()->cd();

		tt_col0.Clear();
		tt_col1.Clear();
		tt_col2.Clear();
		tt_col3.Clear();
		tt_col4.Clear();
		tt_col5.Clear();
		tt_col6.Clear();
		tt_col7.Clear();
		
		tt_col0.SetTextAlign(12);
		tt_col1.SetTextAlign(12);
		tt_col2.SetTextAlign(12);
		tt_col3.SetTextAlign(12);
		tt_col4.SetTextAlign(12);
		tt_col5.SetTextAlign(12);
		tt_col6.SetTextAlign(12);
		tt_col7.SetTextAlign(12);

		tt_col0.SetTextSize(0.1);
		tt_col1.SetTextSize(0.1);
		tt_col2.SetTextSize(0.1);
		tt_col3.SetTextSize(0.1);
		tt_col4.SetTextSize(0.1);
		tt_col5.SetTextSize(0.1);
		tt_col6.SetTextSize(0.1);
		tt_col7.SetTextSize(0.1);

		tt_col0.AddText("SINGLES TRIGGER 0");
		tt_col0.AddText("tot");
		tt_col0.AddText("pass");
		tt_col0.AddText("trig");
		tt_col0.AddText("deadtime");
		tt_col0.AddText("SINGLES TRIGGER 1");
		tt_col0.AddText("tot");
		tt_col0.AddText("pass");
		tt_col0.AddText("trig");
		tt_col0.AddText("deadtime");

		tt_col1.AddText("");
		tt_col1.AddText(Form("%fHz", singles_tot[0]));
		tt_col1.AddText(Form("%fHz", singles_pass[0]));
		tt_col1.AddText(Form("%fHz", p2lvdsout[0]));
		tt_col1.AddText(Form("%f%c", 100.0*trigbusy[0], '%'));
		tt_col1.AddText("");
		tt_col1.AddText(Form("%fHz", singles_tot[1]));
		tt_col1.AddText(Form("%fHz", singles_pass[1]));
		tt_col1.AddText(Form("%fHz", p2lvdsout[1]));
		tt_col1.AddText(Form("%f%c", 100.0*trigbusy[1], '%'));
				
		tt_col2.AddText("PAIR TRIGGER 0");
		tt_col2.AddText("tot");
		tt_col2.AddText("sumpass");
		tt_col2.AddText("diffpass");
		tt_col2.AddText("edpass");
		tt_col2.AddText("coplanarpass");
		tt_col2.AddText("pass");
		tt_col2.AddText("trig");
		tt_col2.AddText("deadtime");
	
		tt_col3.AddText("");
		tt_col3.AddText(Form("%fHz", pairs_pass[0]));
		tt_col3.AddText(Form("%fHz", pairs_sumpass[0]));
		tt_col3.AddText(Form("%fHz", pairs_diffpass[0]));
		tt_col3.AddText(Form("%fHz", pairs_edpass[0]));
		tt_col3.AddText(Form("%fHz", pairs_coplanarpass[0]));
		tt_col3.AddText(Form("%fHz", pairs_triggerpass[0]));
		tt_col3.AddText(Form("%fHz", p2lvdsout[2]));
		tt_col3.AddText(Form("%f%c", 100.0*trigbusy[2], '%'));

		tt_col4.AddText("PAIR TRIGGER 1");
		tt_col4.AddText("tot");
		tt_col4.AddText("sumpass");
		tt_col4.AddText("diffpass");
		tt_col4.AddText("edpass");
		tt_col4.AddText("coplanarpass");
		tt_col4.AddText("pass");
		tt_col4.AddText("trig");
		tt_col4.AddText("deadtime");
		
		tt_col5.AddText("");
		tt_col5.AddText(Form("%fHz", pairs_pass[1]));
		tt_col5.AddText(Form("%fHz", pairs_sumpass[1]));
		tt_col5.AddText(Form("%fHz", pairs_diffpass[1]));
		tt_col5.AddText(Form("%fHz", pairs_edpass[1]));
		tt_col5.AddText(Form("%fHz", pairs_coplanarpass[1]));
		tt_col5.AddText(Form("%fHz", pairs_triggerpass[1]));
		tt_col5.AddText(Form("%fHz", p2lvdsout[3]));
		tt_col5.AddText(Form("%f%c", 100.0*trigbusy[3], '%'));

		tt_col6.AddText("OTHER TRIGGERS");
		tt_col6.AddText("led");
		tt_col6.AddText("cosmic");
		tt_col6.AddText("trig");
		tt_col6.AddText("pulser");
		tt_col6.AddText("DAQ trigger");
		tt_col6.AddText("trigger deadtime");
		
		tt_col7.AddText("");
		tt_col7.AddText(Form("%fHz", led));
		tt_col7.AddText(Form("%fHz", cosmic));
		tt_col7.AddText(Form("%fHz", p2lvdsout[4]));
		tt_col7.AddText(Form("%fHz", p2lvdsout[5]));
		tt_col7.AddText(Form("%fHz", trig1));
		tt_col7.AddText(Form("%f%c", 100.0*busycycles, '%'));
		
		tt_col0.Draw();
		tt_col1.Draw();
		tt_col2.Draw();
		tt_col3.Draw();
		tt_col4.Draw();
		tt_col5.Draw();
		tt_col6.Draw();
		tt_col7.Draw();

		pCanvasRates->GetCanvas()->Modified();
		pCanvasRates->GetCanvas()->Update();
	}

	void UpdateHistogram(Bool_t bReadout = kTRUE)
	{
		// histograms
		pM->WriteReg32(HpsCluster_HistCtrl, 0x00);	// disable histograms

		Bool_t normalize = pButtonNormalize->IsDown();
		
		unsigned int ref = pM->ReadReg32(HpsCluster_RefTime);
		float scale = ref;
		if(normalize && (scale <= 0.0))
		{
			printf("SSP cluster histogram reference time invalid. not normalizing data.\n");
			normalize = kFALSE;
		}
		else
		{
			scale = scale * 256.0f / 250.0E6;
			scale = 1.0f / scale;
		}

		UpdateLatencyHistogram(scale, normalize);
		UpdatePositionHistogram(scale, normalize);
		UpdateEnergyHistogram(scale, normalize);
		UpdateNHitsHistogram(scale, normalize);

		pM->WriteReg32(HpsCluster_HistCtrl, 0x1F);	// enable histograms

		// scalers
		UpdateScalers(pButtonNormalize->IsDown());
	}

private:

	enum Buttons
	{
		BTN_AUTOUPDATE		= 1002,
		BTN_MANUALUPDATE	= 1003,
		SDR_UPDATETIME		= 1100
	};
	
	volatile unsigned int	*HpsCluster_HistCtrl;
	volatile unsigned int	*HpsCluster_HistLatency;
	volatile unsigned int	*HpsCluster_HistPosition;
	volatile unsigned int	*HpsCluster_HistEnergy;
	volatile unsigned int	*HpsCluster_HistNHits;
	volatile unsigned int	*HpsCluster_RefTime;
	
	volatile unsigned int	*HpsScaler_Disable;
	volatile unsigned int	*HpsScaler_Sysclk50;
	volatile unsigned int	*HpsSingles0_Pass;
	volatile unsigned int	*HpsSingles0_Tot;
	volatile unsigned int	*HpsSingles1_Pass;
	volatile unsigned int	*HpsSingles1_Tot;
	volatile unsigned int	*HpsPairs0_Pass;
	volatile unsigned int	*HpsPairs0_SumPass;
	volatile unsigned int	*HpsPairs0_DiffPass;
	volatile unsigned int	*HpsPairs0_EDPass;
	volatile unsigned int	*HpsPairs0_CoplanarPass;
	volatile unsigned int	*HpsPairs0_TriggerPass;
	volatile unsigned int	*HpsPairs1_Pass;
	volatile unsigned int	*HpsPairs1_SumPass;
	volatile unsigned int	*HpsPairs1_DiffPass;
	volatile unsigned int	*HpsPairs1_EDPass;
	volatile unsigned int	*HpsPairs1_CoplanarPass;
	volatile unsigned int	*HpsPairs1_TriggerPass;
	
	volatile unsigned int	*HpsScaler_Trig1;
	volatile unsigned int	*HpsScaler_Busy;
	volatile unsigned int	*HpsScaler_BusyCycles;
	volatile unsigned int	*HpsScaler_P2LVDSOut0;
	volatile unsigned int	*HpsScaler_P2LVDSOut1;
	volatile unsigned int	*HpsScaler_P2LVDSOut2;
	volatile unsigned int	*HpsScaler_P2LVDSOut3;
	volatile unsigned int	*HpsScaler_P2LVDSOut4;
	volatile unsigned int	*HpsScaler_P2LVDSOut5;
	volatile unsigned int	*HpsScaler_TrigBusy0;
	volatile unsigned int	*HpsScaler_TrigBusy1;
	volatile unsigned int	*HpsScaler_TrigBusy2;
	volatile unsigned int	*HpsScaler_TrigBusy3;
	volatile unsigned int	*HpsScaler_TrigBusy4;
	volatile unsigned int	*HpsScaler_TrigBusy5;
	volatile unsigned int	*HpsScaler_Cosmic;
	volatile unsigned int	*HpsScaler_Led;
	
	ModuleFrame				*pM;

	TTimer					*pTimerUpdate;

	TRootEmbeddedCanvas	*pCanvas;
	TRootEmbeddedCanvas	*pCanvasRates;

	TH1I						*pHistLatency;
	TH2I						*pHistPosition;
	TH1I						*pHistEnergy;
	TH1I						*pHistNHits;

	TGSlider					*pSliderUpdateTime;

	TGTextButton			*pButtonAutoUpdate;
	TGTextButton			*pButtonManualUpdate;
	TGTextButton			*pButtonNormalize;
};

#endif
