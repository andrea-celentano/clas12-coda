#ifndef TIDModule_H
#define TIDModule_H

#include "RootHeader.h"
#include "TID_Status.h"
#include "ModuleFrame.h"

class TIDModule	: public ModuleFrame
{
 public:
  TIDModule(const TGWindow *p, CrateMsgClient *pClient, unsigned int baseAddr) : ModuleFrame(p, pClient, baseAddr)
    {
      SetLayoutManager(new TGVerticalLayout(this));

      TGCompositeFrame *tFrame;
      AddFrame(pTIDTabs = new TGTab(this), 
	       new TGLayoutHints(kLHintsBottom | kLHintsRight | kLHintsExpandX | kLHintsExpandY));

      tFrame = pTIDTabs->AddTab("Status");
      tFrame->AddFrame(new TID_Status(tFrame, this), new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    }

    const char *GetModuleName() { return "TID"; }
    const char *GetModuleFullName() { return "Trigger Interface/Distributor"; }

 private:
    TGTab			*pTIDTabs;
};

#endif
