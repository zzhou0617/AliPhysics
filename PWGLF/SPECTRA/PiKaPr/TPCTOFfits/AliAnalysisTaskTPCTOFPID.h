#ifndef ALIANALYSISTASKTPCTOFPID_H
#define ALIANALYSISTASKTPCTOFPID_H

#include "AliAnalysisTaskSE.h"
#include "AliPID.h"

class AliESDEvent;
class AliMCEvent;
class AliStack;
class AliPhysicsSelection;
class AliESDtrackCuts;
class AliESDpid;
class AliESDtrack;
class AliTOFcalib;
class AliTOFT0maker;
class TList;
class TH1F;
class TH2F;
class TObjArray;
class AliAnalysisPIDEvent;
class AliAnalysisPIDTrack;
class AliAnalysisPIDParticle;
class TClonesArray;
class AliCentrality;
class AliPIDResponse;
class AliPPVsMultUtils;
class AliESDtrackCuts;
class TTree;
class AliAnalysisTaskTPCTOFPID :
public AliAnalysisTaskSE
{

 public:

  AliAnalysisTaskTPCTOFPID(); // default constructor
  AliAnalysisTaskTPCTOFPID(Bool_t isMC); // default constructor
  virtual ~AliAnalysisTaskTPCTOFPID(); // default destructor

  virtual void UserCreateOutputObjects(); // user create output objects
  
  virtual void UserExec(Option_t *option); // user exec
  virtual void Terminate(Option_t *option); // terminate



  /* getters */
  AliESDpid *GetESDpid() const {return fESDpid;}; // get ESD PID
  AliTOFcalib *GetTOFcalib() const {return fTOFcalib;}; // getter
  AliTOFT0maker *GetTOFT0maker() const {return fTOFT0maker;}; // getter
  
  /* setters */
  void SetMCFlag(Bool_t value = kTRUE) {fMCFlag = value;}; // setter
  void SetMCTuneFlag(Bool_t value = kTRUE) {fMCTuneFlag = value;}; // setter
  void SetPbPbFlag(Bool_t value = kTRUE) {fPbPbFlag = value;}; // setter
  void SetVertexSelectionFlag(Bool_t value = kTRUE) {fVertexSelectionFlag = value;}; // setter
  void SetVertexCut(Double_t value) {fVertexCut = value;}; // setter
  void SetRapidityCut(Double_t value) {fRapidityCut = value;}; // setter
  void SetTimeResolution(Double_t value) {fTimeResolution = value;}; // setter

 protected:

  AliAnalysisTaskTPCTOFPID(const AliAnalysisTaskTPCTOFPID &); // copy constructor
  AliAnalysisTaskTPCTOFPID &operator=(const AliAnalysisTaskTPCTOFPID &); // operator=
 

  /* methods */
  Bool_t InitRun(); // init run
  Bool_t InitEvent(); // init event
  Bool_t HasPrimaryDCA(AliESDtrack *track); // has primary DCA
  Bool_t MakeTPCPID(AliESDtrack *track, Double_t *nsigma, Double_t *signal); // make TPC PID
  Bool_t MakeTOFPID(AliESDtrack *track, Double_t *nsigma, Double_t *signal); // make TOF PID

  /* flags */
  AliESDtrackCuts *fESDtrackCuts;
  Bool_t fInitFlag; // init flag
  Bool_t fMCFlag; // MC flag
  Bool_t fMCTuneFlag; // MC tune flag
  Bool_t fPbPbFlag; // PbPb flag
  Bool_t fVertexSelectionFlag; // vertex selection flag
  Bool_t fPrimaryDCASelectionFlag; // primary DCA selection flag
  TTree *fPIDTree;
  /* ESD analysis */
  AliPIDResponse *fPIDResponse; //! PID object
  AliPPVsMultUtils *fMultiUtils; //! V0M Multi utils
  Int_t fRunNumber; // run number
  UInt_t fStartTime; // start time
  UInt_t fEndTime; // end time
  AliESDEvent *fESDEvent; // ESD event
  AliMCEvent *fMCEvent; // MC event
  AliStack *fMCStack; // MC stack
  AliESDtrackCuts *fTrackCuts; //! track cuts
  AliESDpid *fESDpid; // ESD PID
  Bool_t fIsCollisionCandidate; // is collision candidate
  UInt_t fIsEventSelected; // is event selected
  Bool_t fIsPileupFromSPD; // is pile-up from SPD
  Bool_t fHasVertex; // has vertex
  Float_t fVertexZ; // vertex z
  Float_t fMCTimeZero; // MC time-zero
  AliCentrality *fCentrality; // centrality
  
  AliAnalysisPIDEvent *fAnalysisEvent; // analysis event
  TClonesArray *fAnalysisTrackArray; // analysis track array
  AliAnalysisPIDTrack *fAnalysisTrack; // analysis track
  TClonesArray *fAnalysisParticleArray; // analysis particle array
  AliAnalysisPIDParticle *fAnalysisParticle; // analysis particle

  /* TOF related */
  AliTOFcalib *fTOFcalib; // TOF calib
  AliTOFT0maker *fTOFT0maker; // TOF-T0 maker
  Float_t fTimeResolution; // time resolution

  /*** CUTS ***/

  /* vertex cut */
  Double_t fVertexCut; // vertex cut
  Double_t fRapidityCut; // rapidity cut

  /*** HISTOGRAMS ***/
  
  TH1F *V0MBinCount;

  /* histo lists */
  TList *fHistoList; // histo list
  TList *fMCHistoList; // MC histo list

  
  ClassDef(AliAnalysisTaskTPCTOFPID, 1);
};

#endif /* ALIANALYSISTASKTPCTOFPID_H */
