/**************************************************************************
 * Copyright(c) 1998-2013, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
#include <TClonesArray.h>
#include <THashList.h>
#include <THistManager.h>
#include <TObjArray.h>

#include "AliEmcalTriggerBitConfig.h"
#include "AliEmcalTriggerPatchInfo.h"
#include "AliEmcalTriggerMakerTask.h"
#include "AliEmcalTriggerMakerKernel.h"
#include "AliLog.h"

/// \cond CLASSIMP
ClassImp(AliEmcalTriggerMakerTask)
/// \endcond

/**
 * Dummy constructor
 */
AliEmcalTriggerMakerTask::AliEmcalTriggerMakerTask():
  AliAnalysisTaskEmcal(),
  fTriggerMaker(NULL),
  fCaloTriggersOutName("EmcalTriggers"),
  fCaloTriggerSetupOutName("EmcalTriggersSetup"),
  fV0InName("AliAODVZERO"),
  fV0(NULL),
  fUseTriggerBitConfig(kNewConfig),
  fTriggerBitConfig(NULL),
  fCaloTriggersOut(0),
  fDoQA(kFALSE),
  fQAHistos(NULL)
{

}

AliEmcalTriggerMakerTask::AliEmcalTriggerMakerTask(const char *name, Bool_t doQA):
  AliAnalysisTaskEmcal(),
  fTriggerMaker(NULL),
  fCaloTriggersOutName("EmcalTriggers"),
  fCaloTriggerSetupOutName("EmcalTriggersSetup"),
  fV0InName("AliAODVZERO"),
  fV0(NULL),
  fUseTriggerBitConfig(kNewConfig),
  fTriggerBitConfig(NULL),
  fCaloTriggersOut(NULL),
  fDoQA(doQA),
  fQAHistos(NULL)
{
  fTriggerMaker = new AliEmcalTriggerMakerKernel;
}

/**
 * Destructor
 */
AliEmcalTriggerMakerTask::~AliEmcalTriggerMakerTask() {
  if(fTriggerMaker) delete fTriggerMaker;
  if(fTriggerBitConfig) delete fTriggerBitConfig;
}

/**
 * Initialize output objets
 */
void AliEmcalTriggerMakerTask::UserCreateOutputObjects(){
  AliAnalysisTaskEmcal::UserCreateOutputObjects();

  if(fDoQA && fOutput){
    fQAHistos = new THistManager("TriggerQA");
    const char *patchtypes[2] = {"Online", "Offline"};

    for(int itype = 0; itype < 5; itype++){
      for(const char **patchtype = patchtypes; patchtype < patchtypes + 2; ++patchtype){
        fQAHistos->CreateTH2(Form("RCPos%s%s", AliEmcalTriggerMakerKernel::GetTriggerTypeName(itype).Data(), *patchtype), Form("Lower edge position of %s %s patches (col-row);iEta;iPhi", *patchtype, AliEmcalTriggerMakerKernel::GetTriggerTypeName(itype).Data()), 48, -0.5, 47.5, 104, -0.5, 103.5);
        fQAHistos->CreateTH2(Form("EPCentPos%s%s", AliEmcalTriggerMakerKernel::GetTriggerTypeName(itype).Data(), *patchtype), Form("Center position of the %s %s trigger patches;#eta;#phi", *patchtype, AliEmcalTriggerMakerKernel::GetTriggerTypeName(itype).Data()), 20, -0.8, 0.8, 700, 0., 7.);
        fQAHistos->CreateTH2(Form("PatchADCvsE%s%s", AliEmcalTriggerMakerKernel::GetTriggerTypeName(itype).Data(), *patchtype), Form("Patch ADC value for trigger type %s %s;Trigger ADC;FEE patch energy (GeV)", *patchtype, AliEmcalTriggerMakerKernel::GetTriggerTypeName(itype).Data()), 2000, 0., 2000, 200, 0., 200);
      }
    }
    fQAHistos->CreateTH1("triggerBitsAll", "Trigger bits for all incoming patches;bit nr", 64, -0.5, 63.5);
    fQAHistos->CreateTH1("triggerBitsSel", "Trigger bits for reconstructed patches;bit nr", 64, -0.5, 63.5);
    fOutput->Add(fQAHistos->GetListOfHistograms());
    PostData(1, fOutput);
  }
}

/**
 * Initializes the trigger maker kernel
 */
void AliEmcalTriggerMakerTask::ExecOnce(){
  AliAnalysisTaskEmcal::ExecOnce();

  if (!fInitialized)
    return;

  if(!fTriggerBitConfig){
    switch(fUseTriggerBitConfig){
    case kNewConfig:
      fTriggerBitConfig = new AliEmcalTriggerBitConfigNew();
      break;
    case kOldConfig:
      fTriggerBitConfig = new AliEmcalTriggerBitConfigOld();
      break;
    }
  }

  if (!fCaloTriggersOutName.IsNull()) {
    fCaloTriggersOut = new TClonesArray("AliEmcalTriggerPatchInfo");
    fCaloTriggersOut->SetName(fCaloTriggersOutName);

    if (!(InputEvent()->FindListObject(fCaloTriggersOutName))) {
      InputEvent()->AddObject(fCaloTriggersOut);
    }
    else {
      fInitialized = kFALSE;
      AliFatal(Form("%s: Container with same name %s already present. Aborting", GetName(), fCaloTriggersOutName.Data()));
      return;
    }
  }

  if ( ! fV0InName.IsNull()) {
    fV0 = (AliVVZERO*)InputEvent()->FindListObject(fV0InName);
  }

  fTriggerMaker->SetGeometry(fGeom);
  fTriggerMaker->SetRunNumber(InputEvent()->GetRunNumber());
  fTriggerMaker->SetMC(MCEvent() != NULL);
  fTriggerMaker->Init();
}

/**
 * Run the trigger maker
 * Move patches found by the trigger maker to the output clones array
 * Fill QA histograms if requested
 * @return True
 */
Bool_t AliEmcalTriggerMakerTask::Run(){
  fCaloTriggersOut->Clear();
  // prepare trigger maker
  fTriggerMaker->SetCaloCells(fCaloCells);
  fTriggerMaker->SetCaloTriggers(fCaloTriggers);
  fTriggerMaker->SetVZERO(fV0);
  TObjArray *patches = fTriggerMaker->CreateTriggerPatches(InputEvent());
  AliEmcalTriggerPatchInfo *recpatch = NULL;
  Int_t patchcounter = 0;
  for(TIter patchIter = TIter(patches).Begin(); patchIter != TIter::End(); ++patchIter){
    if(fDoQA){
      AliEmcalTriggerMakerKernel::TriggerMakerTriggerType_t type = AliEmcalTriggerMakerKernel::kTMUndefined;
      if(recpatch->IsJetHigh() || recpatch->IsJetLow() || recpatch->IsJetHighSimple() || recpatch->IsJetLowSimple()) type = AliEmcalTriggerMakerKernel::kTMEMCalJet;
      if(recpatch->IsGammaHigh() || recpatch->IsGammaLow() || recpatch->IsGammaHighSimple() || recpatch->IsGammaLowSimple()) type = AliEmcalTriggerMakerKernel::kTMEMCalGamma;
      if(recpatch->IsLevel0()) type = AliEmcalTriggerMakerKernel::kTMEMCalLevel0;
      if(recpatch->IsRecalcJet()) type = AliEmcalTriggerMakerKernel::kTMEMCalRecalcJet;
      if(recpatch->IsRecalcGamma()) type = AliEmcalTriggerMakerKernel::kTMEMCalRecalcGamma;
      TString patchtype = recpatch->IsOfflineSimple() ? "Offline" : "Online";
      fQAHistos->FillTH2(Form("RCPos%s%s", AliEmcalTriggerMakerKernel::GetTriggerTypeName(type).Data(), patchtype.Data()), recpatch->GetColStart(), recpatch->GetRowStart());
      fQAHistos->FillTH2(Form("EPCentPos%s%s", AliEmcalTriggerMakerKernel::GetTriggerTypeName(type).Data(), patchtype.Data()), recpatch->GetEtaGeo(), recpatch->GetPhiGeo());
      fQAHistos->FillTH2(Form("PatchADCvsE%s%s", AliEmcalTriggerMakerKernel::GetTriggerTypeName(type).Data(), patchtype.Data()), recpatch->IsOfflineSimple() ? recpatch->GetADCOfflineAmp() : recpatch->GetADCAmp(), recpatch->GetPatchE());
      // Redo checking of found trigger bits after masking of unwanted triggers
      int tBits = recpatch->GetTriggerBits();
      for(unsigned int ibit = 0; ibit < sizeof(tBits)*8; ibit++) {
        if(tBits & (1 << ibit)){
          fQAHistos->FillTH1("triggerBitsSel", ibit);
        }
      }
    }
    new((*fCaloTriggersOut)[patchcounter++]) AliEmcalTriggerPatchInfo(*recpatch);
  }
  return true;
}