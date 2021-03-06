class CutHandlerConvDalitz{
  public:
    CutHandlerConvDalitz(Int_t nMax=10){
      nCuts=0; nMaxCuts=nMax; validCuts = true;
      eventCutArray = new TString[nMaxCuts]; photonCutArray = new TString[nMaxCuts]; mesonCutArray = new TString[nMaxCuts]; elecCutArray = new TString[nMaxCuts];
      for(Int_t i=0; i<nMaxCuts; i++) {eventCutArray[i] = ""; photonCutArray[i] = ""; mesonCutArray[i] = ""; elecCutArray[i] = "";}
    }

    void AddCut(TString eventCut, TString photonCut, TString elecCut, TString mesonCut){
      if(nCuts>=nMaxCuts) {cout << "ERROR in CutHandlerConvDalitz: Exceeded maximum number of cuts!" << endl; validCuts = false; return;}
      if( eventCut.Length()!=8 || photonCut.Length()!=26 || mesonCut.Length()!=16 || elecCut.Length()!=20 ) {cout << "ERROR in CutHandlerConvDalitz: Incorrect length of cut string!" << endl; validCuts = false; return;}
      eventCutArray[nCuts]=eventCut; photonCutArray[nCuts]=photonCut; mesonCutArray[nCuts]=mesonCut; elecCutArray[nCuts]=elecCut;
      nCuts++;
      return;
    }

    Bool_t AreValid(){return validCuts;}
    Int_t GetNCuts(){if(validCuts) return nCuts; else return 0;}
    TString GetEventCut(Int_t i){if(validCuts&&i<nMaxCuts&&i>=0) return eventCutArray[i]; else{cout << "ERROR in CutHandlerConvDalitz: GetEventCut wrong index i" << endl;return "";}}
    TString GetPhotonCut(Int_t i){if(validCuts&&i<nMaxCuts&&i>=0) return photonCutArray[i]; else {cout << "ERROR in CutHandlerConvDalitz: GetPhotonCut wrong index i" << endl;return "";}}
    TString GetMesonCut(Int_t i){if(validCuts&&i<nMaxCuts&&i>=0) return mesonCutArray[i]; else {cout << "ERROR in CutHandlerConvDalitz: GetMesonCut wrong index i" << endl;return "";}}
    TString GetElecCut(Int_t i){if(validCuts&&i<nMaxCuts&&i>=0) return elecCutArray[i]; else {cout << "ERROR in CutHandlerConvDalitz: GetElecCut wrong index i" << endl;return "";}}
  private:
    Bool_t validCuts;
    Int_t nCuts; Int_t nMaxCuts;
    TString* eventCutArray;
    TString* photonCutArray;
    TString* mesonCutArray;
    TString* elecCutArray;
};

void AddTask_GammaConvDalitzQAV1_pPb(  Int_t    trainConfig               = 1,
                                       Bool_t   isMC                      = kFALSE, //run MC 
                                       Int_t   enableQAMesonTask          = 0, //enable QA in AliAnalysisTaskGammaConvDalitzV1
                                       Bool_t   enableDoMesonChic         = kFALSE, // enable additional Chic analysis
                                       Bool_t   enableSetProdVtxVGamma    = kTRUE,
                                       TString  fileNameInputForWeighting = "MCSpectraInput.root", // path to file for weigting input
                                       Bool_t   doWeighting               = kFALSE,  //enable Weighting
                                       TString  generatorName             = "DPMJET",
                                       TString  cutnumberAODBranch        = "0000000060084001001500000",
                                       Bool_t   enableV0findingEffi       = kFALSE,
                                       Int_t   enableMatBudWeightsPi0          = 0,              // 1 = three radial bins, 2 = 10 radial bins
                                       TString filenameMatBudWeights           = "MCInputFileMaterialBudgetWeights.root"
				    ) {

  cout<<"*********Parameters*******"<<endl;
  cout<<"trainConfig: "<<trainConfig<<endl;
  cout<<"isMC: "<<isMC<<endl;
  cout<<"enableQAMesonTask: "<<enableQAMesonTask<<endl;
  cout<<"enableDoMesonChic: "<<enableDoMesonChic<<endl;
  cout<<"enableSetProdVtxVGamma: "<<enableSetProdVtxVGamma<<endl;
  cout<<"fileNameInputForWeighting: "<<fileNameInputForWeighting.Data()<<endl;
  cout<<"doWeighting: "<<doWeighting<<endl;
  cout<<"generatorName: "<<generatorName.Data()<<endl;
  cout<<"cutnumberAODBranch: "<<cutnumberAODBranch.Data()<<endl;
  
  Int_t isHeavyIon = 2;

  cout<<"Entro 0"<<endl;

  // ================== GetAnalysisManager ===============================
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
      Error(Form("AddTask_GammaConvDalitzV1_pPb_%i",trainConfig), "No analysis manager found.");
      return ;
  }

  // ================== GetInputEventHandler =============================
  AliVEventHandler *inputHandler=mgr->GetInputEventHandler();
  
  //========= Add PID Reponse to ANALYSIS manager ====
  if(!(AliPIDResponse*)mgr->GetTask("PIDResponseTask")){
      gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
      AddTaskPIDResponse(isMC);
  }
  
  //=========  Set Cutnumber for V0Reader ================================
  
  TString cutnumberEvent  = "80000103";
  TString cutnumberPhoton = "";
  
  if(trainConfig == 9 || trainConfig == 10 ){
    cutnumberPhoton = "16000008400100001500000000";   //Offline  V0 finder 
  } else if ( trainConfig >= 54 && trainConfig <= 60  ){   // Warning!!!   Those trains must run in stand-alone mode
    cutnumberPhoton = "00000070004000000500000000";  
  } else {
    cutnumberPhoton = "06000008000100007500000000";   //Online  V0 finder //change      
  }
  
  TString ElecCuts        = "30105400000003300000";            //Electron Cuts
  Bool_t doEtaShift       = kFALSE;
  AliAnalysisDataContainer *cinput = mgr->GetCommonInputContainer();
  
  //========= Add V0 Reader to  ANALYSIS manager if not yet existent =====
  //========= Add V0 Reader to  ANALYSIS manager if not yet existent =====
  TString V0ReaderName = Form("V0ReaderV1_%s_%s",cutnumberEvent.Data(),cutnumberPhoton.Data());
  if( !(AliV0ReaderV1*)mgr->GetTask(V0ReaderName.Data()) ){
    AliV0ReaderV1 *fV0ReaderV1 = new AliV0ReaderV1(V0ReaderName.Data());
    fV0ReaderV1->SetUseOwnXYZCalculation(kTRUE);
    fV0ReaderV1->SetCreateAODs(kFALSE);// AOD Output
    fV0ReaderV1->SetUseAODConversionPhoton(kTRUE);
    fV0ReaderV1->SetProduceV0FindingEfficiency(enableV0findingEffi);
    
    if (!mgr) {
      Error("AddTask_V0ReaderV1", "No analysis manager found.");
      return;
    }
  
    AliConvEventCuts *fEventCuts=NULL;
    if(cutnumberEvent!=""){
      fEventCuts = new AliConvEventCuts(cutnumberEvent.Data(),cutnumberEvent.Data());
      fEventCuts->SetPreSelectionCutFlag(kTRUE);
      fEventCuts->SetV0ReaderName(V0ReaderName);
      if(fEventCuts->InitializeCutsFromCutString(cutnumberEvent.Data())){
        fEventCuts->DoEtaShift(doEtaShift);
        fV0ReaderV1->SetEventCuts(fEventCuts);
        fEventCuts->SetFillCutHistograms("",kTRUE);
      }
    }

    // Set AnalysisCut Number
    AliConversionPhotonCuts *fCuts=NULL;
    if(cutnumberPhoton!=""){
      fCuts = new AliConversionPhotonCuts(cutnumberPhoton.Data(),cutnumberPhoton.Data());
      fCuts->SetPreSelectionCutFlag(kTRUE);
      fCuts->SetIsHeavyIon(isHeavyIon);
      fCuts->SetV0ReaderName(V0ReaderName);
      if(fCuts->InitializeCutsFromCutString(cutnumberPhoton.Data())){
        fV0ReaderV1->SetConversionCuts(fCuts);
        fCuts->SetFillCutHistograms("",kTRUE);
      }
    }
    if(inputHandler->IsA()==AliAODInputHandler::Class()){
    // AOD mode
      cout << "AOD handler: adding " << cutnumberAODBranch.Data() << " as conversion branch" << endl;
      fV0ReaderV1->SetDeltaAODBranchName(Form("GammaConv_%s_gamma",cutnumberAODBranch.Data()));
    }
    fV0ReaderV1->Init();
  
    AliLog::SetGlobalLogLevel(AliLog::kInfo);
  
    //connect input V0Reader
    mgr->AddTask(fV0ReaderV1);
    mgr->ConnectInput(fV0ReaderV1,0,cinput);
  
  }
  //================================================
  //========= Add Electron Selector ================


  if( !(AliDalitzElectronSelector*)mgr->GetTask("ElectronSelector") ){
    AliDalitzElectronSelector *fElectronSelector = new AliDalitzElectronSelector("ElectronSelector");
    // Set AnalysisCut Number
    AliDalitzElectronCuts *fElecCuts=0;
    //ElecCuts = "900054000000020000");
    if( ElecCuts!=""){
      fElecCuts= new AliDalitzElectronCuts(ElecCuts.Data(),ElecCuts.Data());
      fElecCuts->SetUseCrossedRows(kTRUE);
      if(fElecCuts->InitializeCutsFromCutString(ElecCuts.Data())){
        fElectronSelector->SetDalitzElectronCuts(fElecCuts);
        fElecCuts->SetFillCutHistograms("",kTRUE);
      }
    }

    fElectronSelector->Init();
    mgr->AddTask(fElectronSelector);
    
    AliAnalysisDataContainer *cinput1  = mgr->GetCommonInputContainer();
    //connect input V0Reader
    mgr->ConnectInput (fElectronSelector,0,cinput1);
  }



    cout<<"Entro"<<endl;
  //================================================
  //========= Add task to the ANALYSIS manager =====
  //================================================
  //            find input container
  
  

  AliAnalysisTaskGammaConvDalitzV1 *task = NULL;
  task = new AliAnalysisTaskGammaConvDalitzV1(Form("GammaConvDalitzV1_%i",trainConfig));
  task->SetIsHeavyIon(2);
  task->SetIsMC(isMC);
  task->SetV0ReaderName(V0ReaderName);


  CutHandlerConvDalitz cuts;

  // Cut Numbers to use in Analysis
  //Int_t numberOfCuts          = 1;
  
  Bool_t doEtaShiftIndCuts   = kFALSE;
  TString stringShift        = "";

  // Shifting in pPb direction
  doEtaShiftIndCuts          = kFALSE;
  stringShift                = "pPb";
  
  

  if( trainConfig == 1 ) {  // No eta shift |Y| < 0.8
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011
  }  else if( trainConfig == 2 ) {  // No eta shift |Y| < 0.8
    cuts.AddCut("80000113", "03200009360300007200000000", "20475400239102223710", "0263303500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 +  |Y| < 0.6 and |Gamma_eta| < 0.65 and |e+_eta| < 0.65 and |e-_eta| < 0.65 
  }  else if( trainConfig == 3 ) {  // No eta shift |Y| < 0.8
    cuts.AddCut("80000113", "04200009360300007200000000", "20475400235102223710", "0263203500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 +  |Y| < 0.7 and |Gamma_eta| < 0.75 and |e+_eta| < 0.75 and |e-_eta| < 0.75
  }  else if( trainConfig == 4 ) {  // No eta shift  |Y| < 0.8
    cuts.AddCut("80000113", "01200009360300007200000000", "20475400236102223710", "0263403500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 +  |Y| < 0.5 and |Gamma_eta| < 0.60 and |e+_eta| < 0.60 and |e-_eta| < 0.60  
  } else if ( trainConfig == 5 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233102223310", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011
  } else if ( trainConfig == 6 ) {  // No eta shift |Y| < 0.8
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233102243710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011
  } else if ( trainConfig == 7 ) {
    cuts.AddCut("80000123", "00200009360300007200000000", "20475400233102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011
  } else if ( trainConfig == 8 ) {  // No eta shift |Y| < 0.8
    cuts.AddCut("80000123", "00200009360300007200000000", "20475400233102243710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011
  } else if ( trainConfig == 9  ) {
    cuts.AddCut("80000113", "10200009360300007200000000", "20475400233102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011
  } else if ( trainConfig == 10 ) {
    cuts.AddCut("80000123", "10200009360300007200000000", "20475400233102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011
  } else if ( trainConfig == 11  ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233102223010", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011
  } else if ( trainConfig == 12 ) {
    cuts.AddCut("80000123", "00200009360300007200000000", "20475400233102223010", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011
  }  else if ( trainConfig == 13 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400533102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011  + 4 ITScls
  }  else if ( trainConfig == 14 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400733102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011  + 4 ITScls no Any
  }  else if ( trainConfig == 15 ) {
    cuts.AddCut("80000123", "00200009360300007200000000", "20475400533102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011  + 4 ITScls
  }  else if ( trainConfig == 16 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400833002223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth  +  No psipair
  }  else if ( trainConfig == 17 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400833102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth
  } else if ( trainConfig  == 18 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400933102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth + 4ITS cls
  } else if ( trainConfig  == 19 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400133102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kFirts
  } else if ( trainConfig  == 20 ) {
    cuts.AddCut("80000113", "00200009217000008260400000", "20475400233102223710", "0162103500900000"); // standard cut Annika analysis:
  } else if ( trainConfig  == 21 ) {
    cuts.AddCut("80000113", "00200009217000008260400000", "20475400133102223710", "0162103500900000"); // standard cut Annika analysis: + kFirst
  } else if ( trainConfig  == 22 ){
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400153102221710", "0263103500900000"); //standard cut Pi0 pPb 00-100 + Old Standard 2010 + kFirtst
  } else if ( trainConfig  == 23 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400853102221710", "0263103500900000"); //standard cut Pi0 pPb 00-100 + Old Standard 2010 + kBoth
  } else if ( trainConfig  == 24 ){
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400153102221700", "0263103500900000"); //standard cut Pi0 pPb 00-100 + Old Standard 2010 + kFirtst No weights
  } else if ( trainConfig  == 25 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400853102221700", "0263103500900000"); //standard cut Pi0 pPb 00-100 + Old Standard 2010 + kBoth No weights
  } else if ( trainConfig  == 26 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400133102223700", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kFirts + No weights
  } else if ( trainConfig  == 27 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400833102223700", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth  +  No weights
  } else if ( trainConfig  == 28 ) {
    cuts.AddCut("80000113", "00200049360300007200000000", "20475400233102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + Pt > 0.075
  } else if ( trainConfig  == 29 ) {
    cuts.AddCut("80000113", "00200019360300007200000000", "20475400233102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + Pt > 0.100
  } else if ( trainConfig  == 30 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233102233710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + Pt{e} > 0.150
  } else if ( trainConfig  == 31 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233102253710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + Pt{e} > 0.175
  } else if ( trainConfig  == 32  ) {
    cuts.AddCut("80000113", "00700009360300007200000000", "20475400233102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011  + Photon R > 35 cm
  } else if ( trainConfig  == 33  ) {
    cuts.AddCut("80000113", "00700009360300007200000000", "20475400833102223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth + Photon R > 35 cm 
  } else if ( trainConfig  == 34  ) {
    cuts.AddCut("80000113", "00700009360300007200000000", "20475400833102223700", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth + Photon R > 35 cm + No weights 
  } else if ( trainConfig  == 35 ) {						
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400833002223700", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth  + NoPsiPair + No weights
  } else if ( trainConfig  == 36 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233102223700", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny no Weights
  } else if ( trainConfig  == 37 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400833102223711", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth + smearing photon virtual
  } else if ( trainConfig  == 38 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400133102223711", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kFirts + smearing photon virtual
  } else if( trainConfig   == 39 ) {  
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233102223711", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011  + kAny  + smearing photon virtual 
  } else if ( trainConfig  == 40 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400833102223712", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth + smearing photon virtual  electrons
  } else if ( trainConfig  == 41 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400133102223712", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kFirts + smearing photon virtual electrons
  } else if( trainConfig   == 42 ) {  
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233102223712", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011  + kAny  + smearing photon virtual electrons 
  } else if( trainConfig  == 43 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400833202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 +  kBoth + New psi pair cut  fPsiPairCut = 0.60;    fDeltaPhiCutMin = 0.0; fDeltaPhiCutMax = 0.12;
  } else if( trainConfig  == 44 ) {
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400833502223712", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kBoth + New psi pair cut  fPsiPairCut = 0.60;     fDeltaPhiCutMin = 0.0; fDeltaPhiCutMax = 0.12; +  Electron Smearing
  } else if( trainConfig  == 45 ) {  
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011  + kAny  + New psi pair cut + New psi pair cut  fPsiPairCut = 0.60    fDeltaPhiCutMin = 0.0 fDeltaPhiCutMax = 0.12
  } else if( trainConfig == 46 ) {  
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233202223712", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011  + kAny  + New psi pair cut + New psi pair cut  fPsiPairCut = 0.60;    fDeltaPhiCutMin = 0.0; fDeltaPhiCutMax = 0.12; + photon virtual electrons 
  } else if( trainConfig == 47 ) {
    cuts.AddCut("80000113", "00500009360300007200000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny + new psiPair Cut + gammaR >  10cm
  } else if( trainConfig == 48 ){
    cuts.AddCut("80000113", "00800009360300007200000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny + new psiPair Cut + gammaR >  12.5cm
  } else if( trainConfig == 49 ){
    cuts.AddCut("80000113", "00600009360300007200000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny + new psiPair Cut + gammaR >  20 cm 
  } else if( trainConfig == 50 ){
    cuts.AddCut("80000113", "00700009360300007200000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny + new psiPair Cut + gammaR >  35 cm
  } else if( trainConfig == 51 ){
    cuts.AddCut("80000113", "00900009360300007200000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny + new psiPair Cut + gammaR >  7.5 cm
  } else if( trainConfig == 52 ){
    cuts.AddCut("80000113", "00000009360300007200000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny + new psiPair Cut + gammaR >  0 cm
  } else if( trainConfig == 53 ) {  
    cuts.AddCut("80000113", "00200009360300007200000000", "20475400233202223700", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011  + kAny  + New psi pair cut + New psi pair cut  fPsiPairCut = 0.60    fDeltaPhiCutMin = 0.0 fDeltaPhiCutMax = 0.12
  }  else if( trainConfig == 54 ) {
    cuts.AddCut("80000113", "00000000004000000500000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny    + new psiPair Cut    0.60, 0.0 0.12   + 0   cm < Rconv  < 180 cm   //open cuts
  } else if( trainConfig == 55 ) {
    cuts.AddCut("80000113", "00200000004000000500000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny  + new psiPair Cut    0.60, 0.0 0.12 + Rconv > 5 cm
  } else if( trainConfig == 56 ) {
    cuts.AddCut("80000113", "00900000004000000500000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny  + new psiPair Cut    0.60, 0.0 0.12 + Rconv > 7.5 cm
  } else if( trainConfig == 57 ) {
    cuts.AddCut("80000113", "00500000004000000500000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny  + new psiPair Cut    0.60, 0.0 0.12 + Rconv > 10 cm
  } else if( trainConfig == 58 ) {
    cuts.AddCut("80000113", "00800000004000000500000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny  + new psiPair Cut    0.60, 0.0 0.12 + Rconv > 12.5 cm
  } else if( trainConfig == 59 ) {
    cuts.AddCut("80000113", "00600000004000000500000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny  + new psiPair Cut    0.60, 0.0 0.12 + Rconv > 20 cm
  } else if( trainConfig  == 60 ) {
    cuts.AddCut("80000113", "00700000004000000500000000", "20475400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny  + new psiPair Cut    0.60, 0.0 0.12 + Rconv > 35 cm
  } else if( trainConfig  == 61 ) {  
    cuts.AddCut("80000113", "00200009360300007200000000", "20405400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny   + new psiPair Cut    0.60, 0.0 0.12 + pion rejection low 0 -2 weights
  } else if( trainConfig  == 62 ) {  
    cuts.AddCut("80000113", "00200009360300007200000000", "20405400233202223700", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny   + new psiPair Cut    0.60, 0.0 0.12 + pion rejection low 0 -2 No weights
  } else if( trainConfig  == 63 ) {  
    cuts.AddCut("80000113", "00200009360300007200000000", "20405400233202224710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny   + new psiPair Cut    0.60, 0.0 0.12 + pion rejection low 0 -2 weights	
  } else if( trainConfig  == 64 ) {  
    cuts.AddCut("80000113", "00200009360300007200000000", "20405400233202221710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny   + new psiPair Cut    0.60, 0.0 0.12 + pion rejection low 0 -2 weights	
  } else if( trainConfig  == 65 ) {  
    cuts.AddCut("80000113", "00200079360300007200000000", "20405400233202221710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny   + new psiPair Cut    0.60, 0.0 0.12 + pion rejection low 0 -2 weights	
  } else if( trainConfig  == 66 ) {
    cuts.AddCut("80000113", "00200009360300007200004000", "20405400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny   + new psiPair Cut    0.60, 0.0 0.12 + pion rejection low 0 -2 weights	
  } else if( trainConfig  == 67 ) {
    cuts.AddCut("80000113", "00200009360300007200004000", "20405400233202223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny   + new psiPair Cut + double counting rejection	
  } else if( trainConfig  == 68 ) {
    cuts.AddCut("80000113", "00200009360300007200004000", "20405400233002223710", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny   + new psiPair Cut + double counting rejection	
  } else if( trainConfig  == 69 ) {
    cuts.AddCut("80000113", "00200009360300007200004000", "20405400233202223010", "0263103500900000"); //standard cut Pi0 pPb 00-100  //Tracks 2011 + kAny   + new psiPair Cut + double counting rejection	New standard
  }

  if(!cuts.AreValid()){
    cout << "\n\n****************************************************" << endl;
    cout << "ERROR: No valid cuts stored in CutHandlerConvDalitz! Returning..." << endl;
    cout << "****************************************************\n\n" << endl;
    return;
  }
  
  Int_t numberOfCuts		= 	cuts.GetNCuts();	 
    
  TList *EventCutList = new TList();
  TList *ConvCutList  = new TList();
  TList *MesonCutList = new TList();
  TList *ElecCutList  = new TList();
  
  TList *HeaderList = new TList();
  TObjString *Header1 = new TObjString("pi0_1");
  HeaderList->Add(Header1);
  TObjString *Header3 = new TObjString("eta_2");
  HeaderList->Add(Header3);
  
  EventCutList->SetOwner(kTRUE);
  AliConvEventCuts **analysisEventCuts = new AliConvEventCuts*[numberOfCuts];
  ConvCutList->SetOwner(kTRUE);
  AliConversionPhotonCuts **analysisCuts = new AliConversionPhotonCuts*[numberOfCuts];
  MesonCutList->SetOwner(kTRUE);
  AliConversionMesonCuts **analysisMesonCuts   = new AliConversionMesonCuts*[numberOfCuts];
  ElecCutList->SetOwner(kTRUE);
  AliDalitzElectronCuts **analysisElecCuts     = new AliDalitzElectronCuts*[numberOfCuts];
  
  Bool_t initializedMatBudWeigths_existing    = kFALSE;

  for(Int_t i = 0; i<numberOfCuts; i++){
    analysisEventCuts[i] = new AliConvEventCuts();
    
    if (  ( trainConfig >= 1 && trainConfig <= 6 ) || trainConfig == 9  ||  trainConfig == 11  || trainConfig == 13 || trainConfig == 14 || trainConfig == 16 || trainConfig == 17 || trainConfig == 18 || trainConfig == 19 || trainConfig == 20 || trainConfig == 21 || trainConfig == 22 || trainConfig == 23 ||
        trainConfig == 28 || trainConfig == 29 || trainConfig == 30 ||  trainConfig == 31  || trainConfig == 32 || trainConfig == 33 || trainConfig == 37 || trainConfig == 38 || trainConfig == 39 || trainConfig == 40 || trainConfig == 41 || trainConfig == 41 || trainConfig == 43 || trainConfig == 44 ||
        trainConfig == 45 || trainConfig == 46 || trainConfig == 47 ||  trainConfig == 48  || trainConfig == 49 || trainConfig == 50 || trainConfig == 51 || trainConfig == 52 || trainConfig == 54 || trainConfig == 55 || trainConfig == 56 || trainConfig == 57 || trainConfig == 58 || trainConfig == 59 || 
        trainConfig == 60 || trainConfig == 61 || trainConfig == 63 ||  trainConfig == 64  || trainConfig == 65 || trainConfig == 66 || trainConfig == 67 || trainConfig == 68 || trainConfig == 69 ) {
      
      if (doWeighting){
        if (generatorName.CompareTo("DPMJET")==0){
          analysisEventCuts[i]->SetUseReweightingWithHistogramFromFile(kTRUE, kTRUE, kFALSE, fileNameInputForWeighting, "Pi0_DPMJET_LHC13b2_efix_pPb_5023GeV_MBV0A", "Eta_DPMJET_LHC13b2_efix_pPb_5023GeV_MBV0A", "","Pi0_Fit_Data_pPb_5023GeV_MBV0A","Eta_Fit_Data_pPb_5023GeV_MBV0A");
        } else if (generatorName.CompareTo("HIJING")==0){   
          analysisEventCuts[i]->SetUseReweightingWithHistogramFromFile(kTRUE, kTRUE, kFALSE, fileNameInputForWeighting, "Pi0_Hijing_LHC13e7_pPb_5023GeV_MBV0A", "Eta_Hijing_LHC13e7_pPb_5023GeV_MBV0A", "","Pi0_Fit_Data_pPb_5023GeV_MBV0A","Eta_Fit_Data_pPb_5023GeV_MBV0A");
        }
      }
    } else if ( trainConfig == 7 || trainConfig == 8 || trainConfig == 10 || trainConfig == 12  || trainConfig == 15 ){
      if (doWeighting){
        analysisEventCuts[i]->SetUseReweightingWithHistogramFromFile(kTRUE, kTRUE, kFALSE, fileNameInputForWeighting, "Pi0_Hijing_LHC13e7_addSig_pPb_5023GeV_MBV0A", "Eta_Hijing_LHC13e7_addSig_pPb_5023GeV_MBV0A", "","Pi0_Fit_Data_pPb_5023GeV_MBV0A","Eta_Fit_Data_pPb_5023GeV_MBV0A");
      }
    }
    analysisEventCuts[i]->SetV0ReaderName(V0ReaderName);    
    if( ! analysisEventCuts[i]->InitializeCutsFromCutString((cuts.GetEventCut(i)).Data() )){
      cout<<"ERROR: analysisEventCuts [ " << i <<" ] "<<endl;
      return 0;
    }
      
    if (doEtaShiftIndCuts) {
      analysisEventCuts[i]->DoEtaShift(doEtaShiftIndCuts);
      analysisEventCuts[i]->SetEtaShift(stringShift);
    }
    EventCutList->Add(analysisEventCuts[i]);
    analysisEventCuts[i]->SetFillCutHistograms("",kFALSE);	
    analysisEventCuts[i]->SetAcceptedHeader(HeaderList);
    
      
    
    analysisCuts[i] = new AliConversionPhotonCuts();
    if (enableMatBudWeightsPi0 > 0){
        if (isMC > 0){
            if (analysisCuts[i]->InitializeMaterialBudgetWeights(enableMatBudWeightsPi0,filenameMatBudWeights)){
                initializedMatBudWeigths_existing = kTRUE;}
            else {cout << "ERROR The initialization of the materialBudgetWeights did not work out." << endl;}
        }
        else {cout << "ERROR 'enableMatBudWeightsPi0'-flag was set > 0 even though this is not a MC task. It was automatically reset to 0." << endl;}
    }
    
    
    analysisCuts[i]->SetV0ReaderName(V0ReaderName);
    if( ! analysisCuts[i]->InitializeCutsFromCutString((cuts.GetPhotonCut(i)).Data()) ) {
      cout<<"ERROR: analysisCuts [ " << i <<" ] "<<endl;
      return 0;
    }
    analysisCuts[i]->SetIsHeavyIon(isHeavyIon);
    ConvCutList->Add(analysisCuts[i]);
    analysisCuts[i]->SetFillCutHistograms("",kFALSE);	   

    analysisMesonCuts[i] = new AliConversionMesonCuts();		
    if( ! analysisMesonCuts[i]->InitializeCutsFromCutString((cuts.GetMesonCut(i)).Data()) ) {
      cout<<"ERROR: analysisMesonCuts [ " <<i<<" ] "<<endl;
      return 0;
    }
    MesonCutList->Add(analysisMesonCuts[i]);
    analysisMesonCuts[i]->SetFillCutHistograms("");

    //TString cutName( Form("%s_%s_%s_%s",eventCutArray[i].Data(), photonCutArray[i].Data(),ElecCutarray[i].Data(),MesonCutarray[i].Data() ) );
    analysisElecCuts[i] = new AliDalitzElectronCuts();
    analysisElecCuts[i]->SetUseCrossedRows(kTRUE);
   
    if( !analysisElecCuts[i]->InitializeCutsFromCutString((cuts.GetElecCut(i)).Data())) {
      cout<< "ERROR:  analysisElecCuts [ " <<i<<" ] "<<endl;
      return 0;
    }
    ElecCutList->Add(analysisElecCuts[i]);
    analysisElecCuts[i]->SetFillCutHistograms("",kFALSE,(cuts.GetElecCut(i)).Data()); 
  }
  
  task->SetEventCutList(numberOfCuts,EventCutList);
  task->SetConversionCutList(numberOfCuts,ConvCutList);
  task->SetMesonCutList(MesonCutList);
  task->SetElectronCutList(ElecCutList);

  task->SetMoveParticleAccordingToVertex(kTRUE);
  
  if(enableSetProdVtxVGamma) task->SetProductionVertextoVGamma(kTRUE);
  task->SetDoMesonQA(enableQAMesonTask);
  if(enableDoMesonChic) task->SetDoChicAnalysis(kTRUE);
  
  if (initializedMatBudWeigths_existing) {
      task->SetDoMaterialBudgetWeightingOfGammasForTrueMesons(kTRUE);
  }

  //connect containers
  AliAnalysisDataContainer *coutput =
  mgr->CreateContainer(Form("GammaConvDalitzV1_%i",trainConfig), TList::Class(),
              AliAnalysisManager::kOutputContainer,Form("GammaConvV1Dalitz_%i.root",trainConfig));

  mgr->AddTask(task);
  mgr->ConnectInput(task,0,cinput);
  mgr->ConnectOutput(task,1,coutput);

  return;

}
