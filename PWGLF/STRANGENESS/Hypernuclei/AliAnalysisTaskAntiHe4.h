#ifndef ALIANALYSISTASKANTIHE4_CXX
#define ALIANALYSISTASKANTIHE4_CXX

// anti-alpha analysis
// Authors: Alexander Kalweit and Nicole Martin

class TF1;
class TH1F;
class TH2F;
class AliESDEvent;
class AliESDtrackCuts;
class AliESDVertex;

#include "AliAnalysisTaskSE.h"
#include "THn.h"
#include "TH3F.h"
#include "TGraph.h"

class AliAnalysisTaskAntiHe4 : public AliAnalysisTaskSE {
 public:
  AliAnalysisTaskAntiHe4();
  AliAnalysisTaskAntiHe4(const char *name);
  virtual ~AliAnalysisTaskAntiHe4() {}
  
  virtual void   UserCreateOutputObjects();
  virtual void   UserExec(Option_t *option);
  virtual void   Terminate(const Option_t*);
  Int_t  Initialize();
  Int_t  SetupEvent();
  void   ResetEvent();

  
 private:
  AliInputEventHandler *fEventHandler;                              //  for ESDs or AODs
  AliESDEvent          *fESD;                                       //  ESD object
  TH1F                 *fHistCentralityClass10;                     //! histo to look at the centrality distribution    
  TH1F                 *fHistCentralityPercentile;                  //! histo to look at the centrality distribution 
  TH1F                 *fHistTriggerStat;                           //! Trigger statistics
  TH1F                 *fHistTriggerStatAfterEventSelection;        //! Trigger statistics
  TH3F                 *fHistDEDx;                                  //! final histograms for anti-alpha analysis
  TH3F                 *fHistTOF3D;                                 //! final histograms for anti-alpha analysis
  TH1F                 *fHistAlpha;                                 //! Alpha plot TOF mass
  TH1F                 *fHistAlphaSignal;                           //! Alpha plot TOF mass only signal candidates
  TGraph               *fGraphAlphaSignal;                          //! TGraph with alphas / dE/dx vs. mom
  Int_t                 fNCounter;                                  //! counts alphas

  TH2F                 *fHistDeDx;                                  //! histo for a dE/dx     
  TH3F                 *fHistDeDxRegion;                            //! histo for a dE/dx per Region 
  TH2F                 *fHistDeDxSharp;                             //! histo for a dE/dx with sharp cuts

  TH2F                 *fHistTOF2D;                                 //! histo for a TOF
  TH2F                 *fHistTOFnuclei;                             //! histo for a TOF nuclei  

  Int_t                 fNTriggers;                                 //! N Triggers used
  Double_t              fBBParametersLightParticles[5];             //! Bethe Bloch paramters for light paritcles
  Double_t              fBBParametersNuclei[5];                     //! Bethe Bloch paramters for nuclei
  Bool_t                fMCtrue;                                    //! flag if real data or MC is processed
  Bool_t                fTriggerFired[5];                           //! TriggerFired 0: MB | 1: CE | 2: SC | 3: EJE | 4: EGA
  //
  AliESDtrackCuts      *fESDtrackCuts;                              //  basic cut variables
  AliESDtrackCuts      *fESDtrackCutsSharp;                         //  sharp cut variables for final results
  AliESDpid            *fESDpid;                                    //  basic TPC object for n-sigma cuts
  THnF                 *fAntiAlpha;                                 //! histogram for particle ratios as a function of momentum: (0.) dca, (1.) sign, (2.) particle Type, (3.) p_tot

  void                  BinLogAxis(const THn *h, Int_t axisNumber); //  define function for log axis for search for Anti-Alpha candidates
  void                  BinLogAxis(const TH3 *h, Int_t axisNumber); 
  void                  BinLogAxis(const TH1 *h);
  Bool_t                IsTriggered();
  void                  SetBBParameters(Int_t runnumber);

  //
  // output containers
  //
  TTree *fTree;  
  TObjArray * fOutputContainer; // ! output data container
  //
  // tree variables
  //
  Char_t Name[1000];  
  Int_t  evnt, itrk;
  //
  Float_t TrkPtot[1000];
  Float_t TPCPtot[1000];
  Float_t DeDx[1000];
  Float_t DCAZ[1000];
  Float_t TPCNsignal[1000];
  Float_t ITSnCluster[1000];
  Float_t Sign[1000];
  Float_t DCAXY[1000];
  Float_t Mass[1000];
  Float_t ITSRefit[1000];
  Float_t TOFRefit[1000];
  Float_t TOFtime[1000];
  Float_t TOFout[1000];
  Float_t ITSsignal[1000];
  Float_t SharedClusters[1000];
  Char_t fFileName[1000]; 
  Int_t fEventNumber[1000];


  //
  //
  AliAnalysisTaskAntiHe4(const AliAnalysisTaskAntiHe4&); // not implemented
  AliAnalysisTaskAntiHe4& operator=(const AliAnalysisTaskAntiHe4&); // not implemented
  
  ClassDef(AliAnalysisTaskAntiHe4, 1); // example of analysis
};

#endif
