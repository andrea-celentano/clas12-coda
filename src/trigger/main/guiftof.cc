
// guiftof.cxx: gui for FTOF v1495-based trigger

#include <stdlib.h>

#include <TROOT.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <TVirtualPadEditor.h>
#include <TGResourcePool.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGFSContainer.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGComboBox.h>
#include <TGTab.h>
#include <TGSlider.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGTextEdit.h>
#include <TGShutter.h>
#include <TGProgressBar.h>
#include <TGColorSelect.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TH1.h>
#include <TH2.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TFile.h>
#include <TKey.h>
#include <TGDockableFrame.h>
#include <TGFontDialog.h>
#include <TPolyLine.h>
#include <TRootCanvas.h>
#include <TText.h>

#include "FTOFTriggerBoardRegs.h"

#include "guiftof.h"

#include "cratemsgclient.h"
#include "libtcp.h"
#include "libdb.h"

#include "scope.h"

CrateMsgClient *tcp; //sergey: global for now, will find appropriate place later

const char *filetypes[] = { "All files",     "*",
                            "ROOT files",    "*.root",
                            "ROOT macros",   "*.C",
                            0,               0 };

/********************************/
/* MyTimer class implementation */

MyTimer::MyTimer(GUIMainFrame *m, Long_t ms) : TTimer(ms, kTRUE)
{
  fGUIMainFrame = m;
  gSystem->AddTimer(this);
}

Bool_t MyTimer::Notify()
{
  // That function will be called in case of timeout INSTEAD OF
  // standart Notify() function from TTimer class

  if(fGUIMainFrame->fScalersDlg) fGUIMainFrame->fScalersDlg->ReadVME();
  if(fGUIMainFrame->fDsc2Dlg)    fGUIMainFrame->fDsc2Dlg->ReadVME();
  if(fGUIMainFrame->fDelaysDlg) fGUIMainFrame->fDelaysDlg->ReadVME();
  this->Reset();

  return kTRUE;
}
//------------------------------------------------------------------------------



/**********************************/
/* TileFrame class implementation */

TileFrame::TileFrame(const TGWindow *p) : TGCompositeFrame(p, 10, 10, kHorizontalFrame, GetWhitePixel())
{
   // Create tile view container. Used to show colormap.

   fCanvas = 0;
   SetLayoutManager(new TGTileLayout(this, 8));

   // Handle only buttons 4 and 5 used by the wheel mouse to scroll
   gVirtualX->GrabButton(fId, kButton4, kAnyModifier,
                         kButtonPressMask | kButtonReleaseMask,
                         kNone, kNone);
   gVirtualX->GrabButton(fId, kButton5, kAnyModifier,
                         kButtonPressMask | kButtonReleaseMask,
                         kNone, kNone);
}

Bool_t TileFrame::HandleButton(Event_t *event)
{
   // Handle wheel mouse to scroll.

   Int_t page = 0;
   if (event->fCode == kButton4 || event->fCode == kButton5) {
      if (!fCanvas) return kTRUE;
      if (fCanvas->GetContainer()->GetHeight())
         page = Int_t(Float_t(fCanvas->GetViewPort()->GetHeight() *
                              fCanvas->GetViewPort()->GetHeight()) /
                              fCanvas->GetContainer()->GetHeight());
   }

   if (event->fCode == kButton4) {
      //scroll up
      Int_t newpos = fCanvas->GetVsbPosition() - page;
      if (newpos < 0) newpos = 0;
      fCanvas->SetVsbPosition(newpos);
      return kTRUE;
   }
   if (event->fCode == kButton5) {
      // scroll down
      Int_t newpos = fCanvas->GetVsbPosition() + page;
      fCanvas->SetVsbPosition(newpos);
      return kTRUE;
   }
   return kTRUE;
}



/**************************************/
/* GUIMainFrame class implementation */

GUIMainFrame::GUIMainFrame(const TGWindow *p, UInt_t w, UInt_t h, char *host) : TGMainFrame(p, w, h)
{

   // create VME communication
   strcpy(hostname,host);

   fScalersDlg = NULL;
   fDsc2Dlg = NULL;
   fDelaysDlg = NULL;

   // Create main frame. A TGMainFrame is a top level window.

   // use hierarchical cleaning
   SetCleanup(kDeepCleanup);

   // Create menubar and popup menus. The hint objects are used to place
   // and group the different menu widgets with respect to each other.
   fMenuDock = new TGDockableFrame(this); // create menu dock (upper horizontal bar for 'File' etc)
   AddFrame(fMenuDock, new TGLayoutHints(kLHintsExpandX, 0, 0, 1, 0)); // add menu dock to the main window
   fMenuDock->SetWindowName("GUIFTOF Menu"); // ???

   // create several layouts 
   fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
   fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
   fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

   // menu item 'File'
   fMenuFile = new TGPopupMenu(fClient->GetRoot());
   fMenuFile->AddEntry("&Open...", M_FILE_OPEN);
   fMenuFile->AddEntry("&Save", M_FILE_SAVE);
   fMenuFile->AddEntry("S&ave as...", M_FILE_SAVEAS);
   fMenuFile->AddEntry("&Close", -1);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("&Print", M_FILE_PRINT);
   fMenuFile->AddEntry("P&rint setup...", M_FILE_PRINTSETUP);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("E&xit", M_FILE_EXIT);

   fMenuFile->DisableEntry(M_FILE_SAVEAS);
   fMenuFile->HideEntry(M_FILE_PRINT);

   // menu item 'FTOF'
   fMenuFTOF = new TGPopupMenu(fClient->GetRoot());
   fMenuFTOF->AddLabel("FTOF monitoring and control");
   fMenuFTOF->AddSeparator();
   fMenuFTOF->AddEntry("&Registers", M_REGISTERS);
   fMenuFTOF->AddEntry("&Delays", M_DELAYS);
   fMenuFTOF->AddEntry("&Scalers", M_SCALERS);
   fMenuFTOF->AddEntry("&Dsc2", M_DSC2);
   fMenuFTOF->AddEntry("&Scope (ASCII)", M_SCOPE_ASCII);
   fMenuFTOF->AddEntry("&Scope (Canvas)", M_SCOPE_CANVAS);

   // menu item 'View'
   fMenuView = new TGPopupMenu(gClient->GetRoot());
   fMenuView->AddEntry("&Dock", M_VIEW_DOCK);
   fMenuView->AddEntry("&Undock", M_VIEW_UNDOCK);
   fMenuView->AddSeparator();
   fMenuView->AddEntry("Enable U&ndock", M_VIEW_ENBL_DOCK);
   fMenuView->AddEntry("Enable &Hide", M_VIEW_ENBL_HIDE);
   fMenuView->DisableEntry(M_VIEW_DOCK);

   fMenuDock->EnableUndock(kTRUE);
   fMenuDock->EnableHide(kTRUE);
   fMenuView->CheckEntry(M_VIEW_ENBL_DOCK);
   fMenuView->CheckEntry(M_VIEW_ENBL_HIDE);

   // 'menu item 'Help'
   fMenuHelp = new TGPopupMenu(fClient->GetRoot());
   fMenuHelp->AddEntry("&Contents", M_HELP_CONTENTS);
   fMenuHelp->AddEntry("&Search...", M_HELP_SEARCH);
   fMenuHelp->AddSeparator();
   fMenuHelp->AddEntry("&About", M_HELP_ABOUT);

   // Menu button messages are handled by the main frame (i.e. "this") ProcessMessage() method.
   fMenuFile->Associate(this);
   fMenuFTOF->Associate(this);
   fMenuView->Associate(this);
   fMenuHelp->Associate(this);

   // create menu bar and actually add created above menus to it
   fMenuBar = new TGMenuBar(fMenuDock, 1, 1, kHorizontalFrame);
   fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
   fMenuBar->AddPopup("&FTOF", fMenuFTOF, fMenuBarItemLayout);
   fMenuBar->AddPopup("&View", fMenuView, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarHelpLayout);

   // add menu bar to the dock
   fMenuDock->AddFrame(fMenuBar, fMenuBarLayout);

   //
   // at that moment we have only top menu bar with menus
   //
   /*
   MapSubwindows();
   Resize();
   MapWindow();
   return;
   */

   
   // Create TGCanvas and a canvas container which uses a tile layout manager
   fCanvasWindow = new TGCanvas(this, 500, 300);
   fContainer = new TileFrame(fCanvasWindow->GetViewPort());
   fContainer->SetCanvas(fCanvasWindow);
   fCanvasWindow->SetContainer(fContainer);
   fContainer->SetCleanup(kDeepCleanup); // use hierarchical cleaning for container
   AddFrame(fCanvasWindow, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 2, 2));
   

   AddFrame(btConnect = new TGTextButton(this, "Connect", 11));
   btConnect->Associate(this);
   btConnect->Resize(90, btConnect->GetDefaultHeight());
   AddFrame(btConnect);

   AddFrame(btDisconnect = new TGTextButton(this, "Disconnect", 12));
   btDisconnect->Associate(this);
   btDisconnect->Resize(90, btDisconnect->GetDefaultHeight());
   btDisconnect->SetEnabled(kFALSE);
   AddFrame(btDisconnect);


   SetWindowName("GUIFTOF");
   MapSubwindows();  // force drawing of all created subwindows; actual drawing is done by MapWindow()

   // we need to use GetDefault...() to initialize the layout algorithm...
   Resize();   // resize to default size
   MapWindow(); // actual window drawing
   Print();

   tt = new MyTimer(this, 1000);
}

GUIMainFrame::~GUIMainFrame()
{
   // Delete all created widgets.

   delete fMenuFile;
   delete fMenuFTOF;
   delete fMenuView;
   delete fMenuHelp;
   delete fContainer;
}

void GUIMainFrame::CloseWindow()
{
   // Got close message for this MainFrame. Terminate the application
   // or returns from the TApplication event loop (depending on the
   // argument specified in TApplication::Run()).

   gApplication->Terminate(0);
}

Bool_t GUIMainFrame::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
  Int_t ret;
   // Handle messages send to the GUIMainFrame object. E.g. all menu button
   // messages.

   UInt_t board_addr[2];
   board_addr[0] = FTOF_BOARD_ADDRESS_1;
   if(FTOF_BOARD_ADDRESS_1 != FTOF_BOARD_ADDRESS_2) board_addr[1] = FTOF_BOARD_ADDRESS_2;
   else                                             board_addr[1] = 0;

   char SignalNames0[SCOPE_CHANNELCOUNT][20];
   char SignalNames1[SCOPE_CHANNELCOUNT][20];
   int i, sig = 0;
   for(i=1; i<=12; i++) sprintf(SignalNames0[sig++],"TRIG%02d", i);
   for(i=1; i<=NU; i++) sprintf(SignalNames0[sig++],"U1_%02d", i);
   for(i=1; i<=NV; i++) sprintf(SignalNames0[sig++],"V1_%02d", i);
   for(i=1; i<=NW; i++) sprintf(SignalNames0[sig++],"W1_%02d", i);
   while(sig<SCOPE_CHANNELCOUNT) sprintf(SignalNames0[sig++],"unused");
   printf("sig=%d\n",sig);

   switch (GET_MSG(msg)) {

      case kC_COMMAND:
         switch (GET_SUBMSG(msg)) {

            case kCM_BUTTON:
	       //printf("Button was pressed, id = %ld\n", parm1);
			  if(parm1 == 11) // Connect
			  {
				printf("Connect reached\n");fflush(stdout);

                tcp = new CrateMsgClient(hostname,6102);
                if(tcp->IsValid())
                {
                  printf("Connected\n");
				  btConnect->SetEnabled(kFALSE);
				  btDisconnect->SetEnabled(kTRUE);

		unsigned short sval = 0xFBFB;
		ret = tcp->Read16((unsigned int)(FTOF_BOARD_ADDRESS_1+0x800C), &sval);
        printf("ret=%d, VME FIRMWARE val=0x%04x\n",ret,sval);
				  
		unsigned int val = 0xFBFBFBFB;
		ret = tcp->Read32((unsigned int)(FTOF_BOARD_ADDRESS_1+0x1000), &val);
        printf("ret=%d, USER FIRMWARE val=0x%08x\n",ret,val);
                }
                else
                {
                  printf("NOT CONNECTED - EXIT\n");
                  exit(0);
                }

                if(fDelaysDlg)
				{
				  fDelaysDlg->ReadVME();
				  fDelaysDlg->UpdateGUI();
				}
                if(fScalersDlg)
				{
				  fScalersDlg->ReadVME();
				  fScalersDlg->UpdateGUI();
				}
                if(fDsc2Dlg)
				{
				  fDsc2Dlg->ReadVME();
				  fDsc2Dlg->UpdateGUI();
				}

			  }
			  else if(parm1 == 12) // Disconnect
			  {
			    printf("Disconnect reached\n");
                tcp->Disconnect();
			    btConnect->SetEnabled(kTRUE);
			    btDisconnect->SetEnabled(kFALSE);
			  }
              break;

            case kCM_MENUSELECT:
               //printf("Pointer over menu entry, id=%ld\n", parm1);
               break;

            case kCM_MENU:
               switch (parm1) {

                  case M_FILE_OPEN:
                     {
                        static TString dir(".");
                        TGFileInfo fi;
                        fi.fFileTypes = filetypes;
                        fi.fIniDir    = StrDup(dir);
                        new TGFileDialog(fClient->GetRoot(), this, kFDOpen, &fi);
                        printf("Open file: %s (dir: %s)\n", fi.fFilename,
                               fi.fIniDir);
                        dir = fi.fIniDir;
                     }
                     break;

                  case M_FILE_SAVE:
                     printf("M_FILE_SAVE\n");
                     break;

                  case M_FILE_PRINT:
                     printf("M_FILE_PRINT\n");
                     printf("Hiding itself, select \"Print Setup...\" to enable again\n");
                     fMenuFile->HideEntry(M_FILE_PRINT);
                     break;

                  case M_FILE_PRINTSETUP:
                     printf("M_FILE_PRINTSETUP\n");
                     printf("Enabling \"Print\"\n");
                     fMenuFile->EnableEntry(M_FILE_PRINT);
                     break;

                  case M_FILE_EXIT:
                     CloseWindow();   // this also terminates theApp
                     break;

                  case M_REGISTERS:
                     new RegistersDlg(fClient->GetRoot(), this);
                     break;

                  case M_DELAYS:
                     fDelaysDlg = new DelaysDlg(fClient->GetRoot(), this, 600, 300);
                     break;

                  case M_SCALERS:
                     fScalersDlg = new ScalersDlg(fClient->GetRoot(), this, 600, 300);
                     break;

                  case M_DSC2:
                     fDsc2Dlg = new Dsc2Dlg(fClient->GetRoot(), this, 600, 300);
                     break;

                  case M_SCOPE_ASCII:
					fScopeDlg = new ScopeDlg(fClient->GetRoot(), this, 800, 800, board_addr, SignalNames0, SignalNames1, 0);
                     break;

                  case M_SCOPE_CANVAS:
					fScopeDlg = new ScopeDlg(fClient->GetRoot(), this, 800, 800, board_addr, SignalNames0, SignalNames1, 1);
                     break;

                  case M_VIEW_ENBL_DOCK:
                     fMenuDock->EnableUndock(!fMenuDock->EnableUndock());
                     if (fMenuDock->EnableUndock()) {
                        fMenuView->CheckEntry(M_VIEW_ENBL_DOCK);
                        fMenuView->EnableEntry(M_VIEW_UNDOCK);
                     } else {
                        fMenuView->UnCheckEntry(M_VIEW_ENBL_DOCK);
                        fMenuView->DisableEntry(M_VIEW_UNDOCK);
                     }
                     break;

                  case M_VIEW_ENBL_HIDE:
                     fMenuDock->EnableHide(!fMenuDock->EnableHide());
                     if (fMenuDock->EnableHide()) {
                        fMenuView->CheckEntry(M_VIEW_ENBL_HIDE);
                     } else {
                        fMenuView->UnCheckEntry(M_VIEW_ENBL_HIDE);
                     }
                     break;

                  case M_VIEW_DOCK:
                     fMenuDock->DockContainer();
                     fMenuView->EnableEntry(M_VIEW_UNDOCK);
                     fMenuView->DisableEntry(M_VIEW_DOCK);
                     break;

                  case M_VIEW_UNDOCK:
                     fMenuDock->UndockContainer();
                     fMenuView->EnableEntry(M_VIEW_DOCK);
                     fMenuView->DisableEntry(M_VIEW_UNDOCK);
                     break;

                  default:
                     break;
               }
            default:
               break;
         }
      default:
         break;
   }

   if (fMenuDock->IsUndocked()) {
      fMenuView->EnableEntry(M_VIEW_DOCK);
      fMenuView->DisableEntry(M_VIEW_UNDOCK);
   } else {
      fMenuView->EnableEntry(M_VIEW_UNDOCK);
      fMenuView->DisableEntry(M_VIEW_DOCK);
   }

   return kTRUE;
}


/***********************************/
/* ScalersDlg class implementation */

ScalersDlg::ScalersDlg(const TGWindow *p, GUIMainFrame *main,
					   UInt_t w, UInt_t h, UInt_t options) : TGTransientFrame(p, main, w, h, options)
{

  fMain = main; // remember mainframe

   // Create a dialog window. A dialog window pops up with respect to its
   // "main" window.

   // use hierarchical cleani
   SetCleanup(kDeepCleanup);

   fFrame1 = new TGHorizontalFrame(this, 60, 20, kFixedWidth);

   fOkButton = new TGTextButton(fFrame1, "&Ok", 1);
   fOkButton->Associate(this);
   fCancelButton = new TGTextButton(fFrame1, "&Cancel", 2);
   fCancelButton->Associate(this);

   fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,
                           2, 2, 2, 2);
   fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1);

   fFrame1->AddFrame(fOkButton, fL1);
   fFrame1->AddFrame(fCancelButton, fL1);

   fFrame1->Resize(150, fOkButton->GetDefaultHeight());
   AddFrame(fFrame1, fL2);


   //--------- create Tab widget and some composite frames for Tabs

   fTab = new TGTab(this, 300, 300);
   fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

   TGCompositeFrame *tf;


   /* scalers */
   tf = fTab->AddTab("Scalers");
   tf->SetLayoutManager(new TGHorizontalLayout(tf));

   fF6 = new TGGroupFrame(tf, "Scalers", kVerticalFrame);
   fF6->SetTitlePos(TGGroupFrame::kRight); // right aligned
   tf->AddFrame(fF6, fL3);

   // 14 columns, n rows
   fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 14, 1));

   
   char buff1[100];
   int jj;


   // first board

   for(jj=0; jj<32; jj++)
   {
     // U1
     if(jj<16)
	 {
       if(jj%2) sprintf(buff1, "  S1 R%1d ->", (jj/2)+1);
       else     sprintf(buff1, "  S1 L%1d ->", (jj/2)+1);
	 }
     else if(jj<32)
	 {
       if(jj%2) sprintf(buff1, "  S4 R%1d ->", ((jj-16)/2)+1);
	   else     sprintf(buff1, "  S4 L%1d ->", ((jj-16)/2)+1);
	 }
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
     tbufU1[jj] = new TGTextBuffer(10); //arg: the number of digits
     tbufU1[jj]->AddText(0,"0");
     tentU1[jj] = new TGTextEntry(fF6, tbufU1[jj]);
     tentU1[jj]->Resize(80, tentU1[jj]->GetDefaultHeight()); // 1st arg: the number of pixels
	 tentU1[jj]->SetEnabled(kFALSE);
     tentU1[jj]->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
     fF6->AddFrame(tentU1[jj]);
   }

   for(jj=0; jj<32; jj++)
   {
     // V1
     if(jj<16)
	 {
       if(jj%2) sprintf(buff1, "  S2 R%1d ->", (jj/2)+1);
       else     sprintf(buff1, "  S2 L%1d ->", (jj/2)+1);
	 }
     else if(jj<32)
	 {
       if(jj%2) sprintf(buff1, "  S5 R%1d ->", ((jj-16)/2)+1);
	   else     sprintf(buff1, "  S5 L%1d ->", ((jj-16)/2)+1);
	 }
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
     tbufV1[jj] = new TGTextBuffer(10);
     tbufV1[jj]->AddText(0,"0");
     tentV1[jj] = new TGTextEntry(fF6, tbufV1[jj]);
     tentV1[jj]->Resize(80, tentV1[jj]->GetDefaultHeight());
	 tentV1[jj]->SetEnabled(kFALSE);
     tentV1[jj]->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
     fF6->AddFrame(tentV1[jj]);
   }

   for(jj=0; jj<32; jj++)
   {
     // W1
     if(jj<16)
	 {
       if(jj%2) sprintf(buff1, "  S3 R%1d ->", (jj/2)+1);
       else     sprintf(buff1, "  S3 L%1d ->", (jj/2)+1);
	 }
     else if(jj<32)
	 {
       if(jj%2) sprintf(buff1, "  S6 R%1d ->", ((jj-16)/2)+1);
	   else     sprintf(buff1, "  S6 L%1d ->", ((jj-16)/2)+1);
	 }
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
     tbufW1[jj] = new TGTextBuffer(10);
     tbufW1[jj]->AddText(0,"0");
     tentW1[jj] = new TGTextEntry(fF6, tbufW1[jj]);
     tentW1[jj]->Resize(80, tentW1[jj]->GetDefaultHeight());
     tentW1[jj]->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
	 tentW1[jj]->SetEnabled(kFALSE);
     fF6->AddFrame(tentW1[jj]);
   }
   
   // second board

   for(jj=0; jj<32; jj++)
   {
     // U2
     if(jj<16)
	 {
       if(jj%2) sprintf(buff1, "  S1 R%1d ->", (jj/2)+9);
       else     sprintf(buff1, "  S1 L%1d ->", (jj/2)+9);
	 }
     else if(jj<32)
	 {
       if(jj%2) sprintf(buff1, "  S4 R%1d ->", ((jj-16)/2)+9);
	   else     sprintf(buff1, "  S4 L%1d ->", ((jj-16)/2)+9);
	 }
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
     tbufU2[jj] = new TGTextBuffer(10); //arg: the number of digits
     tbufU2[jj]->AddText(0,"0");
     tentU2[jj] = new TGTextEntry(fF6, tbufU2[jj]);
     tentU2[jj]->Resize(80, tentU2[jj]->GetDefaultHeight()); // 1st arg: the number of pixels
	 tentU2[jj]->SetEnabled(kFALSE);
     tentU2[jj]->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
     fF6->AddFrame(tentU2[jj]);
   }

   for(jj=0; jj<32; jj++)
   {
     // V2
     if(jj<16)
	 {
       if(jj%2) sprintf(buff1, "  S2 R%1d ->", (jj/2)+9);
       else     sprintf(buff1, "  S2 L%1d ->", (jj/2)+9);
	 }
     else if(jj<32)
	 {
       if(jj%2) sprintf(buff1, "  S5 R%1d ->", ((jj-16)/2)+9);
	   else     sprintf(buff1, "  S5 L%1d ->", ((jj-16)/2)+9);
	 }
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
     tbufV2[jj] = new TGTextBuffer(10);
     tbufV2[jj]->AddText(0,"0");
     tentV2[jj] = new TGTextEntry(fF6, tbufV2[jj]);
     tentV2[jj]->Resize(80, tentV2[jj]->GetDefaultHeight());
	 tentV2[jj]->SetEnabled(kFALSE);
     tentV2[jj]->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
     fF6->AddFrame(tentV2[jj]);
   }

   for(jj=0; jj<32; jj++)
   {
     // W2
     if(jj<16)
	 {
       if(jj%2) sprintf(buff1, "  S3 R%1d ->", (jj/2)+9);
       else     sprintf(buff1, "  S3 L%1d ->", (jj/2)+9);
	 }
     else if(jj<32)
	 {
       if(jj%2) sprintf(buff1, "  S6 R%1d ->", ((jj-16)/2)+9);
	   else     sprintf(buff1, "  S6 L%1d ->", ((jj-16)/2)+9);
	 }
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
     tbufW2[jj] = new TGTextBuffer(10);
     tbufW2[jj]->AddText(0,"0");
     tentW2[jj] = new TGTextEntry(fF6, tbufW2[jj]);
     tentW2[jj]->Resize(80, tentW2[jj]->GetDefaultHeight());
     tentW2[jj]->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
	 tentW2[jj]->SetEnabled(kFALSE);
     fF6->AddFrame(tentW2[jj]);
   }
   

   
   fF6->Resize(); // resize to default size



   /* histos */
   fFillHistos = kFALSE;
   fHpxS1 = 0;
   fHpxS2 = 0;
   fHpxS3 = 0;
   fHpxS4 = 0;
   fHpxS5 = 0;
   fHpxS6 = 0;

   tf = fTab->AddTab("Histos");
   fF3 = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
   fStartB = new TGTextButton(fF3, "Start &Filling Hists", 40);
   fStopB  = new TGTextButton(fF3, "&Stop Filling Hists", 41);
   fStartB->Associate(this);
   fStopB->Associate(this);
   fF3->AddFrame(fStartB, fL3);
   fF3->AddFrame(fStopB, fL3);

   fF5 = new TGCompositeFrame(tf, 60, 60, kHorizontalFrame);

   fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
                           kLHintsExpandY, 5, 5, 5, 5);
   fEc1 = new TRootEmbeddedCanvas("ec1", fF5, 100, 100);
   fF5->AddFrame(fEc1, fL4);
   fEc2 = new TRootEmbeddedCanvas("ec2", fF5, 100, 100);
   fF5->AddFrame(fEc2, fL4);
   fEc3 = new TRootEmbeddedCanvas("ec3", fF5, 100, 100);
   fF5->AddFrame(fEc3, fL4);
   fEc4 = new TRootEmbeddedCanvas("ec4", fF5, 100, 100);
   fF5->AddFrame(fEc4, fL4);
   fEc5 = new TRootEmbeddedCanvas("ec5", fF5, 100, 100);
   fF5->AddFrame(fEc5, fL4);
   fEc6 = new TRootEmbeddedCanvas("ec6", fF5, 100, 100);
   fF5->AddFrame(fEc6, fL4);

   tf->AddFrame(fF3, fL3);
   tf->AddFrame(fF5, fL4);

   fEc1->GetCanvas()->SetBorderMode(0);
   fEc2->GetCanvas()->SetBorderMode(0);
   fEc3->GetCanvas()->SetBorderMode(0);
   fEc4->GetCanvas()->SetBorderMode(0);
   fEc5->GetCanvas()->SetBorderMode(0);
   fEc6->GetCanvas()->SetBorderMode(0);

   // make tab yellow
   Pixel_t yellow;
   fClient->GetColorByName("yellow", yellow);
   TGTabElement *tabel = fTab->GetTabTab("Tab 3");;
   /*tabel->ChangeBackground(yellow);*/




   TGLayoutHints *fL5 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
                                          kLHintsExpandY, 2, 2, 5, 1);
   AddFrame(fTab, fL5);

   MapSubwindows();
   Resize();   // resize to default size

   // position relative to the parent's window
   CenterOnParent();

   SetWindowName("Dialog");

   MapWindow();
   //fClient->WaitFor(this);    // otherwise canvas contextmenu does not work

   {
     // read VME and update GUI for the first time
     ReadVME();
     UpdateGUI();
   }
}


ScalersDlg::~ScalersDlg()
{
   // Delete ScalersDlg widgets.

}

void ScalersDlg::FillHistos()
{
   // Fill histograms till user clicks "Stop Filling" button.

   if (!fHpxS1) {
      fHpxS1 = new TH1F("hpxS1","S1 scalers",32,0.,32.);
      fHpxS2 = new TH1F("hpxS2","S2 scalers",32,0.,32.);
      fHpxS3 = new TH1F("hpxS3","S3 scalers",32,0.,32.);
      fHpxS4 = new TH1F("hpxS3","S4 scalers",32,0.,32.);
      fHpxS5 = new TH1F("hpxS3","S5 scalers",32,0.,32.);
      fHpxS6 = new TH1F("hpxS3","S6 scalers",32,0.,32.);
      //fHpxpy = new TH2F("hpxpy","py vs px",40,-4,4,40,-4,4);
      fHpxS1->SetFillColor(kRed);
      fHpxS2->SetFillColor(kRed);
      fHpxS3->SetFillColor(kRed);
      fHpxS4->SetFillColor(kRed);
      fHpxS5->SetFillColor(kRed);
      fHpxS6->SetFillColor(kRed);
   }

   TCanvas *c1 = fEc1->GetCanvas();
   TCanvas *c2 = fEc2->GetCanvas();
   TCanvas *c3 = fEc3->GetCanvas();
   TCanvas *c4 = fEc4->GetCanvas();
   TCanvas *c5 = fEc5->GetCanvas();
   TCanvas *c6 = fEc6->GetCanvas();

   if(fFillHistos)
   {
	 Int_t jj;
	 Double_t xx,ww;

	 fHpxS1->Reset();
	 fHpxS2->Reset();
	 fHpxS3->Reset();
	 fHpxS4->Reset();
	 fHpxS5->Reset();
	 fHpxS6->Reset();

     // first board
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)jj;
	   ww = (Double_t)U1[jj];
	   fHpxS1->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)jj;
	   ww = (Double_t)U1[jj+16];
	   fHpxS4->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)jj;
	   ww = (Double_t)V1[jj];
	   fHpxS2->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)jj;
	   ww = (Double_t)V1[jj+16];
	   fHpxS5->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)jj;
	   ww = (Double_t)W1[jj];
	   fHpxS3->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)jj;
	   ww = (Double_t)W1[jj+16];
	   fHpxS6->Fill(xx,ww);
	 }

	 /*
     // second board
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)(jj+16);
	   ww = (Double_t)U2[jj];
	   fHpxS1->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)(jj+16);
	   ww = (Double_t)U2[jj+16];
	   fHpxS4->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)(jj+16);
	   ww = (Double_t)V2[jj];
	   fHpxS2->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)(jj+16);
	   ww = (Double_t)V2[jj+16];
	   fHpxS5->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)(jj+16);
	   ww = (Double_t)W2[jj];
	   fHpxS3->Fill(xx,ww);
	 }
	 for(jj=0; jj<16; jj++)
	 {
	   xx = (Double_t)(jj+16);
	   ww = (Double_t)W2[jj+16];
	   fHpxS6->Fill(xx,ww);
	 }
	 */
	 

     c1->cd();
     fHpxS1->Draw();
     c2->cd();
     fHpxS2->Draw();
     c3->cd();
     fHpxS3->Draw();
     c4->cd();
     fHpxS4->Draw();
     c5->cd();
     fHpxS5->Draw();
     c6->cd();
     fHpxS6->Draw();

     c1->Modified();
     c1->Update();
     c2->Modified();
     c2->Update();
     c3->Modified();
     c3->Update();
     c4->Modified();
     c4->Update();
     c5->Modified();
     c5->Update();
     c6->Modified();
     c6->Update();
     //gSystem->ProcessEvents();  // handle GUI events
   }
}

void ScalersDlg::CloseWindow()
{
   // Called when window is closed (via the window manager or not).
   // Let's stop histogram filling...
   fFillHistos = kFALSE;
   // Add protection against double-clicks
   fOkButton->SetState(kButtonDisabled);
   fCancelButton->SetState(kButtonDisabled);
   // ... and close the Ged editor if it was activated.
   if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
      TVirtualPadEditor::Terminate();
   DeleteWindow();

   fMain->ClearScalersDlg(); // clear pointer to ourself, so FainFrame will stop reading scalers from VME

}


Bool_t ScalersDlg::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   // Process messages coming from widgets associated with the dialog.

   switch (GET_MSG(msg)) {
      case kC_COMMAND:

         switch (GET_SUBMSG(msg)) {
            case kCM_BUTTON:
               switch(parm1) {
                  case 1:
                  case 2:
                     printf("\nTerminating dialog: %s pressed\n",
                            (parm1 == 1) ? "OK" : "Cancel");
                     CloseWindow();
                     break;
                  case 40:  // start histogram filling
                     fFillHistos = kTRUE;
                     //FillHistos();
                     break;
                  case 41:  // stop histogram filling
                     fFillHistos = kFALSE;
                     break;

                 default:
                     break;
               }
               break;
			   /*
            case kCM_RADIOBUTTON:
               switch (parm1) {
                  case 81:
                     fRad2->SetState(kButtonUp);
                     break;
                  case 82:
                     fRad1->SetState(kButtonUp);
                     break;
               }
               break;
			   */
			   /*
            case kCM_CHECKBUTTON:
               switch (parm1) {
                  case 92:
                     fListBox->SetMultipleSelections(fCheckMulti->GetState());
                     break;
                  default:
                     break;
               }
               break;
			   */
            case kCM_TAB:
               printf("Tab item %ld activated\n", parm1);
			   //if(parm1==5)
			   //{
               //  printf("Scalers !!!\n");
			   //}
               break;
            default:
               break;
         }
         break;

      default:
         break;
   }
   return kTRUE;
}

void ScalersDlg::ReadVME()
{

  {
    Int_t jj;
    UInt_t tmp;
    for(jj=0; jj<46; jj++) {U1[jj] = 0;U2[jj] = 0;}
	for(jj=0; jj<46; jj++) {V1[jj] = 0;V2[jj] = 0;}
	for(jj=0; jj<46; jj++) {W1[jj] = 0;W2[jj] = 0;}

    tmp = 0x0000;
	tcp->Write32(FTOF_BOARD_ADDRESS_1 + FTOF_ENABLE_SCALERS, &tmp);
	tcp->Write32(FTOF_BOARD_ADDRESS_2 + FTOF_ENABLE_SCALERS, &tmp);

	for(jj=0; jj<46; jj++) tcp->Read32(FTOF_BOARD_ADDRESS_1 + FTOF_U_SCALER_BASE + jj*4, &U1[jj]);
	for(jj=0; jj<46; jj++) tcp->Read32(FTOF_BOARD_ADDRESS_1 + FTOF_V_SCALER_BASE + jj*4, &V1[jj]);
	for(jj=0; jj<46; jj++) tcp->Read32(FTOF_BOARD_ADDRESS_1 + FTOF_W_SCALER_BASE + jj*4, &W1[jj]);	
	for(jj=0; jj<46; jj++) tcp->Read32(FTOF_BOARD_ADDRESS_2 + FTOF_U_SCALER_BASE + jj*4, &U2[jj]);
	for(jj=0; jj<46; jj++) tcp->Read32(FTOF_BOARD_ADDRESS_2 + FTOF_V_SCALER_BASE + jj*4, &V2[jj]);
	for(jj=0; jj<46; jj++) tcp->Read32(FTOF_BOARD_ADDRESS_2 + FTOF_W_SCALER_BASE + jj*4, &W2[jj]);	
	
    tcp->Read32(FTOF_BOARD_ADDRESS_1 + FTOF_REF_SCALER, &REF1);
    tcp->Read32(FTOF_BOARD_ADDRESS_2 + FTOF_REF_SCALER, &REF2);

    tmp = 0x0001;
	tcp->Write32(FTOF_BOARD_ADDRESS_1 + FTOF_ENABLE_SCALERS, &tmp);
	tcp->Write32(FTOF_BOARD_ADDRESS_2 + FTOF_ENABLE_SCALERS, &tmp);

	// normalize
    if(REF1>0)
	{
      Float_t norm = 40000000./((Float_t)REF1);
	  for(jj=0; jj<46; jj++) U1[jj] = (Int_t)(((Float_t)U1[jj])*norm);
	  for(jj=0; jj<46; jj++) V1[jj] = (Int_t)(((Float_t)V1[jj])*norm);
	  for(jj=0; jj<46; jj++) W1[jj] = (Int_t)(((Float_t)W1[jj])*norm);
	}
    if(REF2>0)
	{
      Float_t norm = 40000000./((Float_t)REF2);
	  for(jj=0; jj<46; jj++) U2[jj] = (Int_t)(((Float_t)U2[jj])*norm);
	  for(jj=0; jj<46; jj++) V2[jj] = (Int_t)(((Float_t)V2[jj])*norm);
	  for(jj=0; jj<46; jj++) W2[jj] = (Int_t)(((Float_t)W2[jj])*norm);
	}

	
    UpdateGUI();
    FillHistos();
	
  }

}

void ScalersDlg::UpdateGUI()
{
  //printf("ScalersDlg::UpdateGUI reached\n");
   Int_t jj;
   Char_t str[10];
   
   for(jj=0; jj<32; jj++)
   {
     sprintf(str,"%8d",U1[jj]);
     tentU1[jj]->SetText(str);
   }
   /*
   for(jj=0; jj<32; jj++)
   {
     sprintf(str,"%8d",V1[jj]);
     tentV1[jj]->SetText(str);
   }
   for(jj=0; jj<32; jj++)
   {
     sprintf(str,"%8d",W1[jj]);
     tentW1[jj]->SetText(str);
   }
   */
}







/***********************************/
/* Dsc2Dlg class implementation */

Dsc2Dlg::Dsc2Dlg(const TGWindow *p, GUIMainFrame *main,
					   UInt_t w, UInt_t h, UInt_t options) : TGTransientFrame(p, main, w, h, options)
{

  fMain = main; // remember mainframe

   // Create a dialog window. A dialog window pops up with respect to its
   // "main" window.

   // use hierarchical cleani
   SetCleanup(kDeepCleanup);

   fFrame1 = new TGHorizontalFrame(this, 60, 20, kFixedWidth);

   fOkButton = new TGTextButton(fFrame1, "&Ok", 1);
   fOkButton->Associate(this);
   fCancelButton = new TGTextButton(fFrame1, "&Cancel", 2);
   fCancelButton->Associate(this);

   fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,
                           2, 2, 2, 2);
   fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1);

   fFrame1->AddFrame(fOkButton, fL1);
   fFrame1->AddFrame(fCancelButton, fL1);

   fFrame1->Resize(150, fOkButton->GetDefaultHeight());
   AddFrame(fFrame1, fL2);

   //--------- create Tab widget and some composite frames for Tabs

   fTab = new TGTab(this, 300, 300);
   fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

   TGCompositeFrame *tf;


   /* scalers */
   tf = fTab->AddTab("Scalers");
   tf->SetLayoutManager(new TGHorizontalLayout(tf));

   fF6 = new TGGroupFrame(tf, "Scalers", kVerticalFrame);
   fF6->SetTitlePos(TGGroupFrame::kRight); // right aligned
   tf->AddFrame(fF6, fL3);

   // 12 columns, n rows
   fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 12, 1));
   char buff1[100];
   int jj;
   for(jj=0; jj<NU; jj++)
   {
     // U1
     sprintf(buff1, "    U1 [%2d] ->", jj+1);
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
     tbufU1[jj] = new TGTextBuffer(10); //arg: the number of digits
     tbufU1[jj]->AddText(0,"0");
     tentU1[jj] = new TGTextEntry(fF6, tbufU1[jj]);
     tentU1[jj]->Resize(80, tentU1[jj]->GetDefaultHeight()); // 1st arg: the number of pixels
	 tentU1[jj]->SetEnabled(kFALSE);
     tentU1[jj]->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
     fF6->AddFrame(tentU1[jj]);
   }
   for(jj=0; jj<NV; jj++)
   {
     // V1
     sprintf(buff1, "    V1 [%2d] ->", jj+1);
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
     tbufV1[jj] = new TGTextBuffer(10);
     tbufV1[jj]->AddText(0,"0");
     tentV1[jj] = new TGTextEntry(fF6, tbufV1[jj]);
     tentV1[jj]->Resize(80, tentV1[jj]->GetDefaultHeight());
	 tentV1[jj]->SetEnabled(kFALSE);
     tentV1[jj]->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
     fF6->AddFrame(tentV1[jj]);
   }
   for(jj=0; jj<NW; jj++)
   {
     // W1
     sprintf(buff1, "    W1 [%2d] ->", jj+1);
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
     tbufW1[jj] = new TGTextBuffer(10);
     tbufW1[jj]->AddText(0,"0");
     tentW1[jj] = new TGTextEntry(fF6, tbufW1[jj]);
     tentW1[jj]->Resize(80, tentW1[jj]->GetDefaultHeight());
     tentW1[jj]->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
	 tentW1[jj]->SetEnabled(kFALSE);
     fF6->AddFrame(tentW1[jj]);
   }
   
   fF6->Resize(); // resize to default size




   /* histos */

   fFillHistos = kFALSE;
   fHpxU1 = 0;
   fHpxV1 = 0;
   fHpxW1 = 0;

   tf = fTab->AddTab("Histos");
   fF3 = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
   fStartB = new TGTextButton(fF3, "Start &Filling Hists", 40);
   fStopB  = new TGTextButton(fF3, "&Stop Filling Hists", 41);
   fStartB->Associate(this);
   fStopB->Associate(this);
   fF3->AddFrame(fStartB, fL3);
   fF3->AddFrame(fStopB, fL3);
   /*
   fF3->AddFrame(fRad1 = new TGRadioButton(fF3, "&Radio 1", 81), fL3);
   fRad1->Associate(this);
   */
   fF3->AddFrame(fChk1 = new TGCheckButton(fF3, "A&ccumulate", 71), fL3);
   fChk1->Associate(this);

   fF5 = new TGCompositeFrame(tf, 60, 60, kHorizontalFrame);

   //fF5->SetLayoutManager(new TGMatrixLayout(fF5, 0, 3, 2));

   fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5);
   //fL4 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);

   fEc1 = new TRootEmbeddedCanvas("ec1", fF5, 100, 100);
   fF5->AddFrame(fEc1, fL4);
   fEc2 = new TRootEmbeddedCanvas("ec2", fF5, 100, 100);
   fF5->AddFrame(fEc2, fL4);
   fEc3 = new TRootEmbeddedCanvas("ec3", fF5, 100, 100);
   fF5->AddFrame(fEc3, fL4);

   tf->AddFrame(fF3, fL3);
   tf->AddFrame(fF5, fL4);


   fEc1->GetCanvas()->SetBorderMode(0);
   fEc2->GetCanvas()->SetBorderMode(0);
   fEc3->GetCanvas()->SetBorderMode(0);

   // make tab yellow
   Pixel_t yellow;
   fClient->GetColorByName("yellow", yellow);
   TGTabElement *tabel = fTab->GetTabTab("Tab 3");;
   /*tabel->ChangeBackground(yellow);*/




   TGLayoutHints *fL5 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsExpandY, 2, 2, 5, 1);
   AddFrame(fTab, fL5);

   MapSubwindows();
   Resize();   // resize to default size

   // position relative to the parent's window
   CenterOnParent();

   SetWindowName("Dialog");

   MapWindow();
   //fClient->WaitFor(this);    // otherwise canvas contextmenu does not work

   HistAccumulate = 0;
   {
     // read VME and update GUI for the first time
     ReadVME();
     UpdateGUI();
   }
}


Dsc2Dlg::~Dsc2Dlg()
{
   // Delete Dsc2Dlg widgets.

}

void Dsc2Dlg::FillHistos()
{
   // Fill histograms till user clicks "Stop Filling" button.

   if (!fHpxU1)
   {
	 fHpxU1 = new TH1F("hpxU","U-plane scalers",NU,0.,(float)NU);
     fHpxV1 = new TH1F("hpxV","V-plane scalers",NV,0.,(float)NV);
     fHpxW1 = new TH1F("hpxW","W-plane scalers",NW,0.,(float)NW);
     fHpxU1->SetFillColor(kRed);
     fHpxV1->SetFillColor(kRed);
     fHpxW1->SetFillColor(kRed);
   }

   TCanvas *c1 = fEc1->GetCanvas();
   TCanvas *c2 = fEc2->GetCanvas();
   TCanvas *c3 = fEc3->GetCanvas();

   if(fFillHistos)
   {
	 Int_t jj;
	 Double_t xx,ww;

	 if(!HistAccumulate)
	 {
	   fHpxU1->Reset();
	   fHpxV1->Reset();
	   fHpxW1->Reset();
	 }

	 for(jj=0; jj<NU; jj++)
	 {
	   xx = (Double_t)jj;
	   ww = (Double_t)U1[jj];
	   fHpxU1->Fill(xx,ww);
	 }

	 for(jj=0; jj<NV; jj++)
	 {
	   xx = (Double_t)jj;
	   ww = (Double_t)V1[jj];
	   fHpxV1->Fill(xx,ww);
	 }

	 for(jj=0; jj<NW; jj++)
	 {
	   xx = (Double_t)jj;
	   ww = (Double_t)W1[jj];
	   fHpxW1->Fill(xx,ww);
	 }
	 

     c1->cd();
     fHpxU1->Draw();
     c2->cd();
     fHpxV1->Draw();
     c3->cd();
     fHpxW1->Draw();

     c1->Modified();
     c1->Update();
     c2->Modified();
     c2->Update();
     c3->Modified();
     c3->Update();
     //gSystem->ProcessEvents();  // handle GUI events
   }
}

void Dsc2Dlg::CloseWindow()
{
   // Called when window is closed (via the window manager or not).
   // Let's stop histogram filling...
   fFillHistos = kFALSE;
   // Add protection against double-clicks
   fOkButton->SetState(kButtonDisabled);
   fCancelButton->SetState(kButtonDisabled);
   // ... and close the Ged editor if it was activated.
   if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
      TVirtualPadEditor::Terminate();
   DeleteWindow();

   fMain->ClearDsc2Dlg(); // clear pointer to ourself, so FainFrame will stop reading scalers from VME

}


Bool_t Dsc2Dlg::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
  //Int_t status;

   // Process messages coming from widgets associated with the dialog.

   switch (GET_MSG(msg)) {
      case kC_COMMAND:

         switch (GET_SUBMSG(msg)) {
            case kCM_BUTTON:
               switch(parm1) {
                  case 1:
                  case 2:
                     printf("\nTerminating dialog: %s pressed\n",
                            (parm1 == 1) ? "OK" : "Cancel");
                     CloseWindow();
                     break;
                  case 40:  // start histogram filling
                     fFillHistos = kTRUE;
                     //FillHistos();
                     break;
                  case 41:  // stop histogram filling
                     fFillHistos = kFALSE;
                     break;

                 default:
                     break;
               }
               break;
			   /*
            case kCM_RADIOBUTTON:
               switch (parm1) {
                  case 81:
					 printf("RADIOBUTTON 81: %d\n",fRad1->GetState());
                     if(fRad1->GetState()) fRad1->SetState(kButtonUp);
                     else fRad1->SetState(kButtonDown);
                     break;
                  case 82:
                     fRad1->SetState(kButtonUp);
                     break; 
               }
               break;
			   */
			   
            case kCM_CHECKBUTTON:
               switch (parm1) {
                  case 71:
					 printf("CHECKBUTTON 71\n");
                     HistAccumulate = fChk1->GetState();
					 printf("CHECKBUTTON 71: status=%d\n",HistAccumulate);
                     
                     /*fListBox->SetMultipleSelections(fCheck1->GetState());*/
                     break;

                  default:
                     break;
               }
               break;
			   
            case kCM_TAB:
               printf("Tab item %ld activated\n", parm1);
			   //if(parm1==5)
			   //{
               //  printf("Scalers !!!\n");
			   //}
               break;
            default:
               break;
         }
         break;

      default:
         break;
   }
   return kTRUE;
}

#define NDSC 12

void Dsc2Dlg::ReadVME()
{
  Int_t ii, jj, ndsc;
  UInt_t tmp, addr[NDSC];

  ndsc = NDSC;

  addr[0]  = FTOF_DSC2_ADDRESS_SLOT2;
  addr[1]  = FTOF_DSC2_ADDRESS_SLOT4;
  addr[2]  = FTOF_DSC2_ADDRESS_SLOT5;
  addr[3]  = FTOF_DSC2_ADDRESS_SLOT7;
  addr[4]  = FTOF_DSC2_ADDRESS_SLOT8;
  addr[5]  = FTOF_DSC2_ADDRESS_SLOT10;
  addr[6]  = FTOF_DSC2_ADDRESS_SLOT12;
  addr[7]  = FTOF_DSC2_ADDRESS_SLOT14;
  addr[8]  = FTOF_DSC2_ADDRESS_SLOT15;
  addr[9]  = FTOF_DSC2_ADDRESS_SLOT17;
  addr[10] = FTOF_DSC2_ADDRESS_SLOT18;
  addr[11] = FTOF_DSC2_ADDRESS_SLOT20;

  {
    /* should do in dsc2Config, here just in case; will move it to constructor, should be called once */
    tmp = 0x0004;
    for(ii=0; ii<NDSC; ii++) tcp->Write32(addr[ii] + FTOF_DSC2_SCALER_GATE, &tmp);

    tmp = 0x0000;
    for(ii=0; ii<NDSC; ii++) tcp->Write32(addr[ii] + FTOF_DSC2_SCALER_LATCH, &tmp);

    for(jj=0; jj<NU; jj++) U1[jj] = 0;
    for(jj=0; jj<NV; jj++) V1[jj] = 0;
    for(jj=0; jj<NW; jj++) W1[jj] = 0;


    for(ii=0; ii<NDSC; ii++) tcp->Read32(addr[ii] + FTOF_DSC2_SCALER_REF, &ref[ii]);



    for(jj=0; jj<16; jj++) {tcp->Read32(addr[0] + FTOF_DSC2_SCALER_BASE + jj*4,     &U1[jj]); refU1[jj]    = ref[0];}
    for(jj=0; jj<16; jj++) {tcp->Read32(addr[1] + FTOF_DSC2_SCALER_BASE + jj*4,     &U1[jj+16]); refU1[jj+16] = ref[1];}
    for(jj=0; jj<16; jj++) {tcp->Read32(addr[2] + FTOF_DSC2_SCALER_BASE + jj*4,     &U1[jj+32]); refU1[jj+32] = ref[2];}
    for(jj=0; jj<16; jj++) {tcp->Read32(addr[3] + FTOF_DSC2_SCALER_BASE + jj*4,     &U1[jj+48]); refU1[jj+48] = ref[3];}
    for(jj=0; jj<4; jj++)  {tcp->Read32(addr[4] + FTOF_DSC2_SCALER_BASE + jj*4,     &U1[jj+64]); refU1[jj+64] = ref[4];}

    for(jj=0; jj<12; jj++) {tcp->Read32(addr[4] + FTOF_DSC2_SCALER_BASE + (jj+4)*4, &V1[jj]); refV1[jj]    = ref[4];}
    for(jj=0; jj<16; jj++) {tcp->Read32(addr[5] + FTOF_DSC2_SCALER_BASE + jj*4,     &V1[jj+12]); refV1[jj+12] = ref[5];}
    for(jj=0; jj<16; jj++) {tcp->Read32(addr[6] + FTOF_DSC2_SCALER_BASE + jj*4,     &V1[jj+28]); refV1[jj+28] = ref[6];}
    for(jj=0; jj<16; jj++) {tcp->Read32(addr[7] + FTOF_DSC2_SCALER_BASE + jj*4,     &V1[jj+44]); refV1[jj+44] = ref[7];}
    for(jj=0; jj<2; jj++)  {tcp->Read32(addr[8] + FTOF_DSC2_SCALER_BASE + jj*4,     &V1[jj+60]); refV1[jj+60] = ref[8];}

    for(jj=0; jj<14; jj++) {tcp->Read32(addr[8] + FTOF_DSC2_SCALER_BASE + (jj+2)*4, &W1[jj]); refW1[jj]    = ref[8];}
    for(jj=0; jj<16; jj++) {tcp->Read32(addr[9] + FTOF_DSC2_SCALER_BASE + jj*4,     &W1[jj+14]); refW1[jj+14] = ref[9];}
    for(jj=0; jj<16; jj++) {tcp->Read32(addr[10]+ FTOF_DSC2_SCALER_BASE + jj*4,     &W1[jj+30]); refW1[jj+30] = ref[10];}
    for(jj=0; jj<16; jj++) {tcp->Read32(addr[11]+ FTOF_DSC2_SCALER_BASE + jj*4,     &W1[jj+46]); refW1[jj+46] = ref[11];}
	

	printf("Scalers: ref1=%d, U1=%d %d %d (address=0x%08x)\n",
      ref[0],U1[0],U1[1],U1[2],addr[0] + FTOF_DSC2_SCALER_BASE);

	// normalize
    if(ref[0]>0)
	{
      Float_t norm;
	  for(jj=0; jj<NU; jj++) norm = 125000000./((Float_t)refU1[jj]); U1[jj] = (Int_t)(((Float_t)U1[jj])*norm);
	  for(jj=0; jj<NV; jj++) norm = 125000000./((Float_t)refV1[jj]); V1[jj] = (Int_t)(((Float_t)V1[jj])*norm);
	  for(jj=0; jj<NW; jj++) norm = 125000000./((Float_t)refW1[jj]); W1[jj] = (Int_t)(((Float_t)W1[jj])*norm);
	  printf("Scalers(norm=%f): ref=%d, U1=%d %d %d\n",norm,ref[0],U1[0],U1[1],U1[2]);
	}

    UpdateGUI();
    FillHistos();
  }

}

void Dsc2Dlg::UpdateGUI()
{
   printf("Dsc2Dlg::UpdateGUI reached\n");
   Int_t jj;
   Char_t str[10];
   for(jj=0; jj<NU; jj++)
   {
     sprintf(str,"%8d",U1[jj]);
     tentU1[jj]->SetText(str);
   }
   for(jj=0; jj<NV; jj++)
   {
     sprintf(str,"%8d",V1[jj]);
     tentV1[jj]->SetText(str);
   }
   for(jj=0; jj<NW; jj++)
   {
     sprintf(str,"%8d",W1[jj]);
     tentW1[jj]->SetText(str);
   }

}









/***********************************/
/* DelaysDlg class implementation */

DelaysDlg::DelaysDlg(const TGWindow *p, GUIMainFrame *main,
					   UInt_t w, UInt_t h, UInt_t options) : TGTransientFrame(p, main, w, h, options)
{

  fMain = main; // remember mainframe

   // Create a dialog window. A dialog window pops up with respect to its
   // "main" window.

   // use hierarchical cleani
   SetCleanup(kDeepCleanup);

   fFrame1 = new TGHorizontalFrame(this, 60, 20, kFixedWidth);

   fOkButton = new TGTextButton(fFrame1, "&Ok", 1);
   fOkButton->Associate(this);
   fCancelButton = new TGTextButton(fFrame1, "&Cancel", 2);
   fCancelButton->Associate(this);

   fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,
                           2, 2, 2, 2);
   fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1);

   fFrame1->AddFrame(fOkButton, fL1);
   fFrame1->AddFrame(fCancelButton, fL1);

   fFrame1->Resize(150, fOkButton->GetDefaultHeight());
   AddFrame(fFrame1, fL2);

   //--------- create Tab widget and some composite frames for Tabs

   fTab = new TGTab(this, 300, 300);
   fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

   TGCompositeFrame *tf;


   /* delays */
   tf = fTab->AddTab("Delays");
   tf->SetLayoutManager(new TGHorizontalLayout(tf));

   fF6 = new TGGroupFrame(tf, "All values are in ticks (1 tick = 5 ns)", kVerticalFrame);
   fF6->SetTitlePos(TGGroupFrame::kLeft); // right aligned
   tf->AddFrame(fF6, fL3);

   // 14 columns, n rows
   fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 14, 1));

   TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;;

   char buff1[100];
   int jj;
   for(jj=0; jj<46; jj++)
   {
     // U1
     sprintf(buff1, "    U1[%2d] (ticks) ->", jj+1);
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
	 tnumU1[jj] = new TGNumberEntry(fF6, 0, 8, 10);
     tnumU1[jj]->SetLimits(lim,0,31 );
	 fF6->AddFrame(tnumU1[jj]);
   }
   for(jj=0; jj<46; jj++)
   {
     // V1
     sprintf(buff1, "    V1[%2d] (ticks) ->", jj+1);
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
	 tnumV1[jj] = new TGNumberEntry(fF6, 0, 8, 10);
     tnumV1[jj]->SetLimits(lim,0,31 );
	 fF6->AddFrame(tnumV1[jj]);
   }
   for(jj=0; jj<46; jj++)
   {
     // W1
     sprintf(buff1, "    W1[%2d] (ticks) ->", jj+1);
     fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff1)));
	 tnumW1[jj] = new TGNumberEntry(fF6, 0, 8, 10);
     tnumW1[jj]->SetLimits(lim,0,31 );
	 fF6->AddFrame(tnumW1[jj]);
   }

   fF6->Resize(); // resize to default size

   TGLayoutHints *fL5 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
                                          kLHintsExpandY, 2, 2, 5, 1);
   AddFrame(fTab, fL5);

   MapSubwindows();
   Resize();   // resize to default size

   // position relative to the parent's window
   CenterOnParent();

   SetWindowName("Dialog");

   MapWindow();
   //fClient->WaitFor(this);    // otherwise canvas contextmenu does not work

   {
     // read VME and update GUI for the first time
     ReadVME();
     UpdateGUI();
   }
}


DelaysDlg::~DelaysDlg()
{
   // Delete DelaysDlg widgets.

}


void DelaysDlg::CloseWindow()
{
   // Called when window is closed (via the window manager or not).
   // Add protection against double-clicks
   fOkButton->SetState(kButtonDisabled);
   fCancelButton->SetState(kButtonDisabled);
   // ... and close the Ged editor if it was activated.
   if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
      TVirtualPadEditor::Terminate();
   DeleteWindow();

   fMain->ClearDelaysDlg(); // clear pointer to ourself, so FainFrame will stop reading/writing VME

}


void DelaysDlg::ReadVME()
{
  printf("DelaysDlg::ReadVME reached\n");

  {
    Int_t jj;
    for(jj=0; jj<46; jj++) U1[jj] = 0;
    for(jj=0; jj<46; jj++) V1[jj] = 0;
    for(jj=0; jj<46; jj++) W1[jj] = 0;

	for(jj=0; jj<46; jj++) tcp->Read32(FTOF_BOARD_ADDRESS_1 + FTOF_U_DELAY_BASE + jj*4, &U1[jj]);
	for(jj=0; jj<46; jj++) tcp->Read32(FTOF_BOARD_ADDRESS_1 + FTOF_V_DELAY_BASE + jj*4, &V1[jj]);
	for(jj=0; jj<46; jj++) tcp->Read32(FTOF_BOARD_ADDRESS_1 + FTOF_W_DELAY_BASE + jj*4, &W1[jj]);

	//printf("Delays: %d %d %d\n",U1[0],U1[1],U1[2]);
  }

  WriteVME();
}

Bool_t DelaysDlg::ReadGUI()
{
   printf("DelaysDlg::ReadGUI reached\n");

   Int_t jj;
   for(jj=0; jj<46; jj++)
   {
     U1GUI[jj] = tnumU1[jj]->GetIntNumber();
   }
   for(jj=0; jj<46; jj++)
   {
     V1GUI[jj] = tnumV1[jj]->GetIntNumber();
   }
   for(jj=0; jj<46; jj++)
   {
     W1GUI[jj] = tnumW1[jj]->GetIntNumber();
   }
   return kTRUE;
}

void DelaysDlg::WriteVME()
{
  Int_t jj;

  {
	printf("DelaysDlg::WriteDelays reached\n");

    ReadGUI();

    for(jj=0; jj<46; jj++) if(U1[jj] != U1GUI[jj]) tcp->Write32(FTOF_BOARD_ADDRESS_1 + FTOF_U_DELAY_BASE + jj*4, &U1GUI[jj]);
    for(jj=0; jj<46; jj++) if(V1[jj] != V1GUI[jj]) tcp->Write32(FTOF_BOARD_ADDRESS_1 + FTOF_V_DELAY_BASE + jj*4, &V1GUI[jj]);
    for(jj=0; jj<46; jj++) if(W1[jj] != W1GUI[jj]) tcp->Write32(FTOF_BOARD_ADDRESS_1 + FTOF_W_DELAY_BASE + jj*4, &W1GUI[jj]);
  }

}


Bool_t DelaysDlg::UpdateGUI()
{
   printf("DelaysDlg::UpdateGUI reached\n");
   Int_t jj;
   for(jj=0; jj<46; jj++)
   {
     tnumU1[jj]->SetNumber(U1[jj]);
   }
   for(jj=0; jj<46; jj++)
   {
	 tnumV1[jj]->SetNumber(V1[jj]);
   }
   for(jj=0; jj<46; jj++)
   {
	 tnumW1[jj]->SetNumber(W1[jj]);
   }
   return kTRUE;
}


Bool_t DelaysDlg::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   // Process messages coming from widgets associated with the dialog.

   switch (GET_MSG(msg)) {
      case kC_COMMAND:

         switch (GET_SUBMSG(msg)) {
            case kCM_BUTTON:
               switch(parm1) {
                  case 1:
                  case 2:
                     printf("\nTerminating dialog: %s pressed\n",
                            (parm1 == 1) ? "OK" : "Cancel");
                     CloseWindow();
                     break;

                 default:
                     break;
               }
               break;
			   /*
            case kCM_RADIOBUTTON:
               switch (parm1) {
                  case 81:
                     fRad2->SetState(kButtonUp);
                     break;
                  case 82:
                     fRad1->SetState(kButtonUp);
                     break;
               }
               break;
			   */
			   /*
            case kCM_CHECKBUTTON:
               switch (parm1) {
                  case 92:
                     fListBox->SetMultipleSelections(fCheckMulti->GetState());
                     break;
                  default:
                     break;
               }
               break;
			   */
            case kCM_TAB:
               printf("Tab item %ld activated\n", parm1);
			   //if(parm1==5)
			   //{
               //  printf("Scalers !!!\n");
			   //}
               break;
            default:
               break;
         }
         break;

      default:
         break;
   }
   return kTRUE;
}


/*************************************/
/* RegistersDlg class implementation */

// TGNumberEntry widget
const char *const RegistersDlg::numlabel[13] = {
   "Integer",
   "One digit real",
   "Two digit real",
   "Three digit real",
   "Four digit real",
   "Real",
   "Degree.min.sec",
   "Min:sec",
   "Hour:min",
   "Hour:min:sec",
   "Day/month/year",
   "Month/day/year",
   "Hex"
};

const Double_t RegistersDlg::numinit[13] = {
   12345, 1.0, 1.00, 1.000, 1.0000, 1.2E-12,
   90 * 3600, 120 * 60, 12 * 60, 12 * 3600 + 15 * 60,
   19991121, 19991121, (Double_t) 0xDEADFACE
};


RegistersDlg::RegistersDlg(const TGWindow *p, const TGWindow *main)
 : TGTransientFrame(p, main, 10, 10, kHorizontalFrame)
{
   // build widgets

   // use hierarchical cleaning
   SetCleanup(kDeepCleanup);

   TGGC myGC = *fClient->GetResourcePool()->GetFrameGC();
   TGFont *myfont = fClient->GetFont("-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1");
   if (myfont) myGC.SetFont(myfont->GetFontHandle());

   fF1 = new TGVerticalFrame(this, 200, 300);
   fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2);
   AddFrame(fF1, fL1);
   fL2 = new TGLayoutHints(kLHintsCenterY | kLHintsRight, 2, 2, 2, 2);
   for (int i = 0; i < 13; i++) {
      fF[i] = new TGHorizontalFrame(fF1, 200, 30);
      fF1->AddFrame(fF[i], fL2);
      fNumericEntries[i] = new TGNumberEntry(fF[i], numinit[i], 12, i + 20,
                                             (TGNumberFormat::EStyle) i);
      fNumericEntries[i]->Associate(this);
      fF[i]->AddFrame(fNumericEntries[i], fL2);
      fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
      fF[i]->AddFrame(fLabel[i], fL2);
   }
   fF2 = new TGVerticalFrame(this, 200, 500);
   fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2);
   AddFrame(fF2, fL3);
   fLowerLimit = new TGCheckButton(fF2, "lower limit:", 4);
   fLowerLimit->Associate(this);
   fF2->AddFrame(fLowerLimit, fL3);
   fLimits[0] = new TGNumberEntry(fF2, 0, 12, 10);
   fLimits[0]->SetLogStep(kFALSE);
   fLimits[0]->Associate(this);
   fF2->AddFrame(fLimits[0], fL3);
   fUpperLimit = new TGCheckButton(fF2, "upper limit:", 5);
   fUpperLimit->Associate(this);
   fF2->AddFrame(fUpperLimit, fL3);
   fLimits[1] = new TGNumberEntry(fF2, 0, 12, 11);
   fLimits[1]->SetLogStep(kFALSE);
   fLimits[1]->Associate(this);
   fF2->AddFrame(fLimits[1], fL3);
   fPositive = new TGCheckButton(fF2, "Positive", 6);
   fPositive->Associate(this);
   fF2->AddFrame(fPositive, fL3);
   fNonNegative = new TGCheckButton(fF2, "Non negative", 7);
   fNonNegative->Associate(this);
   fF2->AddFrame(fNonNegative, fL3);
   fSetButton = new TGTextButton(fF2, " Set ", 2);
   fSetButton->Associate(this);
   fF2->AddFrame(fSetButton, fL3);
   fExitButton = new TGTextButton(fF2, " Close ", 1);
   fExitButton->Associate(this);
   fF2->AddFrame(fExitButton, fL3);

   // set dialog box title
   SetWindowName("Registers");
   SetIconName("Registers");
   SetClassHints("RegistersDlg", "RegistersDlg");
   // resize & move to center
   MapSubwindows();
   UInt_t width = GetDefaultWidth();
   UInt_t height = GetDefaultHeight();
   Resize(width, height);

   CenterOnParent();

   // make the message box non-resizable
   SetWMSize(width, height);
   SetWMSizeHints(width, height, width, height, 0, 0);
   SetMWMHints(kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize |
               kMWMDecorMinimize | kMWMDecorMenu,
               kMWMFuncAll | kMWMFuncResize | kMWMFuncMaximize |
               kMWMFuncMinimize, kMWMInputModeless);

   MapWindow();
   //fClient->WaitFor(this); ?????
}

RegistersDlg::~RegistersDlg()
{
   // dtor
}

void RegistersDlg::CloseWindow()
{
   DeleteWindow();
}

void RegistersDlg::SetLimits()
{
   Double_t min = fLimits[0]->GetNumber();
   Bool_t low = (fLowerLimit->GetState() == kButtonDown);
   Double_t max = fLimits[1]->GetNumber();
   Bool_t high = (fUpperLimit->GetState() == kButtonDown);
   TGNumberFormat::ELimit lim;
   if (low && high) {
      lim = TGNumberFormat::kNELLimitMinMax;
   } else if (low) {
      lim = TGNumberFormat::kNELLimitMin;
   } else if (high) {
      lim = TGNumberFormat::kNELLimitMax;
   } else {
      lim = TGNumberFormat::kNELNoLimits;
   }
   Bool_t pos = (fPositive->GetState() == kButtonDown);
   Bool_t nneg = (fNonNegative->GetState() == kButtonDown);
   TGNumberFormat::EAttribute attr;
   if (pos) {
      attr = TGNumberFormat::kNEAPositive;
   } else if (nneg) {
      attr = TGNumberFormat::kNEANonNegative;
   } else {
      attr = TGNumberFormat::kNEAAnyNumber;
   }
   for (int i = 0; i < 13; i++) {
      fNumericEntries[i]->SetFormat(fNumericEntries[i]->GetNumStyle(), attr);
      fNumericEntries[i]->SetLimits(lim, min, max);
   }
}

Bool_t RegistersDlg::ProcessMessage(Long_t msg, Long_t parm1, Long_t /*parm2*/)
{
   switch (GET_MSG(msg)) {
   case kC_COMMAND:
      {
         switch (GET_SUBMSG(msg)) {
         case kCM_BUTTON:
            {
               switch (parm1) {
                  // exit button
               case 1:
                  {
                     CloseWindow();
                     break;
                  }
                  // set button
               case 2:
                  {
                     SetLimits();
                     break;
                  }
               }
               break;
            }
         }
         break;
      }
   }
   return kTRUE;
}



/****************/
/* Main program */

int main(int argc, char **argv)
{
   TApplication theApp("App", &argc, argv);

   if (gROOT->IsBatch()) {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
      return(1);
   }

   if(argc != 2)
   {
     fprintf(stderr, "error: have to specify hostname\n");
     return(1);     
   }

   printf("Trying to connect to >%s<\n",argv[1]);

   GUIMainFrame mainWindow(gClient->GetRoot(), 400, 220, argv[1]);

   theApp.Run();

   return 0;
}
