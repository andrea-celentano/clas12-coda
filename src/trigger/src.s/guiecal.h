
/* guiecal.h */

#define NUVW 36

enum ETestCommandIdentifiers {
   M_FILE_OPEN,
   M_FILE_SAVE,
   M_FILE_SAVEAS,
   M_FILE_PRINT,
   M_FILE_PRINTSETUP,
   M_FILE_EXIT,

   M_REGISTERS,
   M_DELAYS,
   M_SCALERS,
   M_DSC2,
   M_SCOPE_ASCII,
   M_SCOPE_CANVAS,

   M_VIEW_ENBL_DOCK,
   M_VIEW_ENBL_HIDE,
   M_VIEW_DOCK,
   M_VIEW_UNDOCK,

   M_HELP_CONTENTS,
   M_HELP_SEARCH,
   M_HELP_ABOUT
};


class ScalersDlg;
class Dsc2Dlg;
class DelaysDlg;
class ScopeDlg;
class MyTimer;

class TileFrame : public TGCompositeFrame {

private:
   TGCanvas *fCanvas;

public:
   TileFrame(const TGWindow *p);
   virtual ~TileFrame() { }

   void SetCanvas(TGCanvas *canvas) { fCanvas = canvas; }
   Bool_t HandleButton(Event_t *event);
};


class ECALMainFrame : public TGMainFrame {

private:

   TGDockableFrame    *fMenuDock;
   TGCompositeFrame   *fStatusFrame;
   TGCanvas           *fCanvasWindow;
   TileFrame          *fContainer;
   TGTextEntry        *fTestText;
   TGButton           *fTestButton;
   TGColorSelect      *fColorSel;

   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuECAL, *fMenuView, *fMenuHelp;
   TGPopupMenu        *fCascadeMenu, *fCascade1Menu, *fCascade2Menu;
   TGPopupMenu        *fMenuNew1, *fMenuNew2;
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;

   TGCompositeFrame    *fHor1;
   TGTextButton        *fExit;
   TGGroupFrame        *fGframe;
   TGNumberEntry       *fNumber;
   TGLabel             *fLabel;

   TGGroupFrame        *fF6;
   TGLayoutHints       *fL3;

   TGTextButton *btConnect, *btDisconnect;

   MyTimer *tt;

   char hostname[80];

public:
   ScalersDlg         *fScalersDlg;
   Dsc2Dlg            *fDsc2Dlg;
   DelaysDlg          *fDelaysDlg;
   ScopeDlg           *fScopeDlg;

   ECALMainFrame(const TGWindow *p, UInt_t w, UInt_t h, char *host);
   virtual ~ECALMainFrame();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t);

   void ClearScalersDlg() {fScalersDlg = NULL;}
   void ClearDsc2Dlg() {fDsc2Dlg = NULL;}
   void ClearDelaysDlg() {fDelaysDlg = NULL;}
   //void ClearScopeDlg() {fScopeDlg = NULL;}  // sergey: crash here !!!
};



class ScalersDlg : public TGTransientFrame {

private:
   ECALMainFrame       *fMain;
   TGCompositeFrame    *fFrame1, *fF1, *fF2, *fF3, *fF4, *fF5;
   TGGroupFrame        *fF6, *fF7;
   TGButton            *fOkButton, *fCancelButton, *fStartB, *fStopB;
   TGButton            *fBtn1, *fBtn2, *fChk1, *fChk2, *fRad1, *fRad2;
   TGPictureButton     *fPicBut1;
   TGCheckButton       *fCheck1;
   TGCheckButton       *fCheckMulti;
   TGListBox           *fListBox;
   TGComboBox          *fCombo;
   TGTab               *fTab;
   TGTextEntry         *fTxt1, *fTxt2;
   TGLayoutHints       *fL1, *fL2, *fL3, *fL4;
   TRootEmbeddedCanvas *fEc1, *fEc2, *fEc3;
   Int_t                fFirstEntry;
   Int_t                fLastEntry;
   Bool_t               fFillHistos;
   TH1F                *fHpxU1, *fHpxV1, *fHpxW1;

   TGTextBuffer *tbufU1[NUVW], *tbufV1[NUVW], *tbufW1[NUVW];
   TGTextEntry *tentU1[NUVW], *tentV1[NUVW], *tentW1[NUVW];
   UInt_t U1[NUVW], V1[NUVW], W1[NUVW];
   UInt_t REF1, REF2;

   void FillHistos();
   Int_t HistAccumulate;

public:
   ScalersDlg(const TGWindow *p, ECALMainFrame *main, UInt_t w, UInt_t h,
               UInt_t options = kVerticalFrame);
   virtual ~ScalersDlg();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

   virtual void ReadVME();
   virtual void UpdateGUI();
};


class Dsc2Dlg : public TGTransientFrame {

private:
   ECALMainFrame       *fMain;
   TGCompositeFrame    *fFrame1, *fF1, *fF2, *fF3, *fF4, *fF5, *fF51;
   TGGroupFrame        *fF6, *fF7;
   TGButton            *fOkButton, *fCancelButton, *fStartB, *fStopB;
   TGButton            *fBtn1, *fBtn2, *fChk1, *fChk2, *fRad1, *fRad2;
   TGPictureButton     *fPicBut1;
   TGCheckButton       *fCheck1;
   TGCheckButton       *fCheckMulti;
   TGListBox           *fListBox;
   TGComboBox          *fCombo;
   TGTab               *fTab;
   TGTextEntry         *fTxt1, *fTxt2;
   TGLayoutHints       *fL1, *fL2, *fL3, *fL4;
   TRootEmbeddedCanvas *fEc1, *fEc2, *fEc3, *fEc4, *fEc5, *fEc6;
   Int_t                fFirstEntry;
   Int_t                fLastEntry;
   Bool_t               fFillHistos;
   TH1F                *fHpxU1, *fHpxV1, *fHpxW1, *fHpxU2, *fHpxV2, *fHpxW2;

   TGTextBuffer *tbufU1[NUVW], *tbufV1[NUVW], *tbufW1[NUVW];
   TGTextEntry *tentU1[NUVW], *tentV1[NUVW], *tentW1[NUVW];
   UInt_t U1[NUVW], V1[NUVW], W1[NUVW];
   UInt_t refU1[NUVW], refV1[NUVW], refW1[NUVW];

   TGTextBuffer *tbufU2[NUVW], *tbufV2[NUVW], *tbufW2[NUVW];
   TGTextEntry *tentU2[NUVW], *tentV2[NUVW], *tentW2[NUVW];
   UInt_t U2[NUVW], V2[NUVW], W2[NUVW];
   UInt_t refU2[NUVW], refV2[NUVW], refW2[NUVW];

   UInt_t ref[16];

   void FillHistos();
   Int_t HistAccumulate;

public:
   Dsc2Dlg(const TGWindow *p, ECALMainFrame *main, UInt_t w, UInt_t h,
               UInt_t options = kVerticalFrame);
   virtual ~Dsc2Dlg();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

   virtual void ReadVME();
   virtual void UpdateGUI();
};



class DelaysDlg : public TGTransientFrame {

private:
   ECALMainFrame       *fMain;
   TGCompositeFrame    *fFrame1, *fF1, *fF2, *fF3, *fF4, *fF5;
   TGGroupFrame        *fF6, *fF7;
   TGButton            *fOkButton, *fCancelButton, *fStartB, *fStopB;
   TGButton            *fBtn1, *fBtn2, *fChk1, *fChk2, *fRad1, *fRad2;
   TGPictureButton     *fPicBut1;
   TGCheckButton       *fCheck1;
   TGCheckButton       *fCheckMulti;
   TGListBox           *fListBox;
   TGComboBox          *fCombo;
   TGTab               *fTab;
   TGTextEntry         *fTxt1, *fTxt2;
   TGLayoutHints       *fL1, *fL2, *fL3, *fL4;
   TRootEmbeddedCanvas *fEc1, *fEc2;
   Int_t                fFirstEntry;
   Int_t                fLastEntry;
   //TGRadioButton       *fRad1;

   TGNumberEntry *tnumU1[NUVW], *tnumV1[NUVW], *tnumW1[NUVW];

   UInt_t U1[NUVW], V1[NUVW], W1[NUVW];
   UInt_t U1GUI[NUVW], V1GUI[NUVW], W1GUI[NUVW];

public:
   DelaysDlg(const TGWindow *p, ECALMainFrame *main, UInt_t w, UInt_t h,
               UInt_t options = kVerticalFrame);
   virtual ~DelaysDlg();
   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

   virtual void ReadVME();
   virtual void WriteVME();
   virtual Bool_t ReadGUI();
   virtual Bool_t UpdateGUI();
};


class RegistersDlg : public TGTransientFrame {

private:
   TGVerticalFrame      *fF1;
   TGVerticalFrame      *fF2;
   TGHorizontalFrame    *fF[13];
   TGLayoutHints        *fL1;
   TGLayoutHints        *fL2;
   TGLayoutHints        *fL3;
   TGLabel              *fLabel[13];
   TGNumberEntry        *fNumericEntries[13];
   TGCheckButton        *fLowerLimit;
   TGCheckButton        *fUpperLimit;
   TGNumberEntry        *fLimits[2];
   TGCheckButton        *fPositive;
   TGCheckButton        *fNonNegative;
   TGButton             *fSetButton;
   TGButton             *fExitButton;

   static const char *const numlabel[13];
   static const Double_t numinit[13];

public:
   RegistersDlg(const TGWindow *p, const TGWindow *main);
   virtual ~RegistersDlg();
   virtual void CloseWindow();

   void SetLimits();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t);
};


class MyTimer : public TTimer
{
private:
  ECALMainFrame   *fECALMainFrame;   //display to which this timer belongs

public:
  MyTimer(ECALMainFrame *m, Long_t ms);
  Bool_t  Notify();
};
