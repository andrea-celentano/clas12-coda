
/* guictof.h */

/*
#define NU 68
#define NV 62
#define NW 62
*/
#define NU 32
#define NV 32
#define NW 32


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


class GUIMainFrame : public TGMainFrame {

private:

   TGDockableFrame    *fMenuDock;
   TGCompositeFrame   *fStatusFrame;
   TGCanvas           *fCanvasWindow;
   TileFrame          *fContainer;
   TGTextEntry        *fTestText;
   TGButton           *fTestButton;
   TGColorSelect      *fColorSel;

   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuCTOF, *fMenuView, *fMenuHelp;
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

   GUIMainFrame(const TGWindow *p, UInt_t w, UInt_t h, char *host);
   virtual ~GUIMainFrame();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t);

   void ClearScalersDlg() {fScalersDlg = NULL;}
   void ClearDsc2Dlg() {fDsc2Dlg = NULL;}
   void ClearDelaysDlg() {fDelaysDlg = NULL;}
   //void ClearScopeDlg() {fScopeDlg = NULL;}  // sergey: crash here !!!
};



class ScalersDlg : public TGTransientFrame {

private:
   GUIMainFrame       *fMain;
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
   TRootEmbeddedCanvas *fEc1, *fEc2, *fEc3, *fEc4, *fEc5, *fEc6;
   Int_t                fFirstEntry;
   Int_t                fLastEntry;
   Bool_t               fFillHistos;
   TH1F                *fHpxS1, *fHpxS2, *fHpxS3, *fHpxS4, *fHpxS5, *fHpxS6;

   TGTextBuffer *tbufU1[32], *tbufV1[32], *tbufW1[32];
   TGTextBuffer *tbufU2[32], *tbufV2[32], *tbufW2[32];

   TGTextEntry *tentU1[32], *tentV1[32], *tentW1[32];
   TGTextEntry *tentU2[32], *tentV2[32], *tentW2[32];

   UInt_t U1[46], V1[46], W1[46];
   UInt_t REF1;

   UInt_t U2[46], V2[46], W2[46];
   UInt_t REF2;

   void FillHistos();

public:
   ScalersDlg(const TGWindow *p, GUIMainFrame *main, UInt_t w, UInt_t h,
               UInt_t options = kVerticalFrame);
   virtual ~ScalersDlg();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

   virtual void ReadVME();
   virtual void UpdateGUI();
};





class Dsc2Dlg : public TGTransientFrame {

private:
   GUIMainFrame       *fMain;
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

   TGTextBuffer *tbufU1[NU], *tbufV1[NV], *tbufW1[NW];
   TGTextEntry *tentU1[NU], *tentV1[NV], *tentW1[NW];
   UInt_t U1[NU], V1[NV], W1[NW];
   UInt_t refU1[NU], refV1[NV], refW1[NW];

   UInt_t ref[16];

   void FillHistos();
   Int_t HistAccumulate;

public:
   Dsc2Dlg(const TGWindow *p, GUIMainFrame *main, UInt_t w, UInt_t h,
               UInt_t options = kVerticalFrame);
   virtual ~Dsc2Dlg();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

   virtual void ReadVME();
   virtual void UpdateGUI();
};



class DelaysDlg : public TGTransientFrame {

private:
   GUIMainFrame        *fMain;
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

   TGNumberEntry *tnumU1[46], *tnumV1[46], *tnumW1[46];

   UInt_t U1[46], V1[46], W1[46];
   UInt_t U1GUI[46], V1GUI[46], W1GUI[46];

public:
   DelaysDlg(const TGWindow *p, GUIMainFrame *main, UInt_t w, UInt_t h,
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
  GUIMainFrame   *fGUIMainFrame;   //display to which this timer belongs

public:
  MyTimer(GUIMainFrame *m, Long_t ms);
  Bool_t  Notify();
};
