// Class for extended track information
// Author: Ionut-Cristian Arsene (iarsene@cern.ch)
// 

#ifndef ALIREDUCEDTRACKINFO_H
#define ALIREDUCEDTRACKINFO_H

#include <TMath.h>
#include "AliReducedBaseTrack.h"

//_____________________________________________________________________
class AliReducedTrackInfo : public AliReducedBaseTrack {

  friend class AliAnalysisTaskReducedTreeMaker;  // friend analysis task which fills the object
  
 public:
  AliReducedTrackInfo();
  virtual ~AliReducedTrackInfo();

  // getters
  UShort_t TrackId()                     const {return fTrackId;}
  ULong_t  Status()                      const {return fStatus;}
  Bool_t   CheckTrackStatus(UInt_t flag) const {return (flag<8*sizeof(ULong_t) ? (fStatus&(1<<flag)) : kFALSE);}
  Float_t  PxTPC()                       const {return fTPCPt*TMath::Cos(fTPCPhi);}
  Float_t  PyTPC()                       const {return fTPCPt*TMath::Sin(fTPCPhi);}
  Float_t  PzTPC()                       const {return fTPCPt*TMath::SinH(fTPCEta);}
  Float_t  PTPC()                        const {return fTPCPt*TMath::CosH(fTPCEta);};
  Float_t  PhiTPC()                      const {return fTPCPhi;}
  Float_t  PtTPC()                       const {return fTPCPt;}
  Float_t  EtaTPC()                      const {return fTPCEta;}
  Float_t  ThetaTPC()                    const {return TMath::ACos(TMath::TanH(fTPCEta));}
  Float_t  DCAxyTPC()                    const {return fTPCDCA[0];}
  Float_t  DCAzTPC()                     const {return fTPCDCA[1];}
  Float_t  Pin()                         const {return fMomentumInner;}
  Float_t  DCAxy()                       const {return fDCA[0];}
  Float_t  DCAz()                        const {return fDCA[1];}
  Float_t  TrackLength()                 const {return fTrackLength;}
  
  UShort_t ITSncls()                const;
  UChar_t  ITSclusterMap()          const {return fITSclusterMap;}
  Bool_t   ITSLayerHit(Int_t layer) const {return (layer>=0 && layer<6 ? (fITSclusterMap&(1<<layer)) : kFALSE);};
  Float_t  ITSsignal()              const {return fITSsignal;}
  Float_t  ITSnSig(Int_t specie)    const {return (specie>=0 && specie<=3 ? fITSnSig[specie] : -999.);}
  Float_t  ITSchi2()                const {return fITSchi2;}
  
  UChar_t TPCncls()                        const {return fTPCNcls;}
  UChar_t TPCFindableNcls()                const {return fTPCNclsF;}
  UChar_t TPCCrossedRows()                 const {return fTPCCrossedRows;}
  UChar_t TPCnclsShared()                  const {return fTPCNclsShared;}
  UChar_t TPCClusterMap()                  const {return fTPCClusterMap;}
  Int_t   TPCClusterMapBitsFired()         const;
  Bool_t  TPCClusterMapBitFired(Int_t bit) const {return (bit>=0 && bit<8 ? (fTPCClusterMap&(1<<bit)) : kFALSE);};
  Float_t TPCsignal()                      const {return fTPCsignal;}
  UChar_t TPCsignalN()                     const {return fTPCsignalN;}
  Float_t TPCnSig(Int_t specie)            const {return (specie>=0 && specie<=3 ? fTPCnSig[specie] : -999.);}
  Float_t TPCchi2()                        const {return fTPCchi2;}
  
  Float_t  TOFbeta()             const {return fTOFbeta;}
  Float_t  TOFtime()             const {return fTOFtime;}
  Float_t  TOFdx()               const {return fTOFdx;}
  Float_t  TOFdz()               const {return fTOFdz;}
  Float_t  TOFmismatchProbab()   const {return fTOFmismatchProbab;}
  Float_t  TOFchi2()             const {return fTOFchi2;}
  Float_t  TOFnSig(Int_t specie) const {return (specie>=0 && specie<=3 ? fTOFnSig[specie] : -999.);}
  Short_t  TOFdeltaBC()          const {return fTOFdeltaBC;}
  
  Int_t    TRDntracklets(Int_t type)  const {return (type==0 || type==1 ? fTRDntracklets[type] : -1);}
  Float_t  TRDpid(Int_t specie)       const {return (specie>=0 && specie<=1 ? fTRDpid[specie] : -999.);}
  Float_t  TRDpidLQ1D(Int_t specie)   const {return (specie>=0 && specie<=1 ? fTRDpid[specie] : -999.);}
  Float_t  TRDpidLQ2D(Int_t specie)   const {return (specie>=0 && specie<=1 ? fTRDpidLQ2D[specie] : -999.);}
  
  Int_t    CaloClusterId() const {return fCaloClusterId;}
     
  ULong_t GetQualityFlag()            const {return fQualityFlags;}
  Bool_t UsedForQvector()             const {return fQualityFlags&(UShort_t(1)<<0);}
  Bool_t TestFlag(UShort_t iflag)     const {return (iflag<8*sizeof(ULong_t) ? fQualityFlags&(ULong_t(1)<<iflag) : kFALSE);}
  Bool_t SetFlag(UShort_t iflag)            {if (iflag>=8*sizeof(ULong_t)) return kFALSE; fQualityFlags|=(ULong_t(1)<<iflag); return kTRUE;}
  Bool_t UnsetFlag(UShort_t iflag)          {if (iflag>=8*sizeof(ULong_t)) return kFALSE; fQualityFlags|=(ULong_t(0)<<iflag); return kTRUE;}
  Bool_t IsGammaLeg()                 const {return fQualityFlags&(ULong_t(1)<<1);}
  Bool_t IsPureGammaLeg()             const {return fQualityFlags&(ULong_t(1)<<8);}
  Bool_t IsK0sLeg()                   const {return fQualityFlags&(ULong_t(1)<<2);}
  Bool_t IsPureK0sLeg()               const {return fQualityFlags&(ULong_t(1)<<9);}
  Bool_t IsLambdaLeg()                const {return fQualityFlags&(ULong_t(1)<<3);}
  Bool_t IsPureLambdaLeg()            const {return fQualityFlags&(ULong_t(1)<<10);}
  Bool_t IsALambdaLeg()               const {return fQualityFlags&(ULong_t(1)<<4);}
  Bool_t IsPureALambdaLeg()           const {return fQualityFlags&(ULong_t(1)<<11);}
  Bool_t IsKink(Int_t i=0)            const {return (i>=0 && i<3 ? ((fQualityFlags&(ULong_t(1)<<(5+i))) || 
                                                                    (fFlags&(UShort_t(1)<<(12+i)))) : kFALSE);}
  Float_t GetBayesProb(Int_t specie)  const { return (fQualityFlags&(ULong_t(1)<<(15+specie)) ? (fQualityFlags&(ULong_t(1)<<21) ? 0.9 : (fQualityFlags&(ULong_t(1)<<20) ? 0.8 : (fQualityFlags&(ULong_t(1)<<19) ? 0.7 : 0.5)))   : 0.0);}




 protected:
  UShort_t fTrackId;            // track id 
  ULong_t fStatus;              // tracking status
  Float_t fTPCPhi;              // inner param phi
  Float_t fTPCPt;               // inner param pt  
  Float_t fTPCEta;              // inner param eta 
  Float_t fMomentumInner;       // inner param momentum (only the magnitude)
  Float_t fDCA[2];              // DCA xy,z
  Float_t fTPCDCA[2];           // TPConly DCA xy,z
  Float_t fTrackLength;         // track length
  
  // ITS
  UChar_t  fITSclusterMap;      // ITS cluster map
  Float_t  fITSsignal;          // ITS signal
  Float_t  fITSnSig[4];         // 0-electron; 1-pion; 2-kaon; 3-proton
  Float_t  fITSchi2;            // ITS chi2 / cls
  
  // TPC
  UChar_t fTPCNcls;            // TPC ncls                          
  UChar_t fTPCCrossedRows;     // TPC crossed rows                  
  UChar_t fTPCNclsF;           // TPC findable ncls                 
  UChar_t fTPCNclsShared;      // TPC number of shared clusters
  UChar_t fTPCClusterMap;      // TPC cluster distribution map
  Float_t fTPCsignal;          // TPC de/dx
  UChar_t fTPCsignalN;         // TPC no clusters de/dx
  Float_t fTPCnSig[4];         // 0-electron; 1-pion; 2-kaon; 3-proton
  Float_t fTPCchi2;            // TPC chi2 / cls
    
  // TOF
  Float_t fTOFbeta;             // TOF pid info
  Float_t fTOFtime;             // TOF flight time
  Float_t fTOFdx;               // TOF matching dx
  Float_t fTOFdz;               // TOF matching dz
  Float_t fTOFmismatchProbab;   // TOF mismatch probability
  Float_t fTOFchi2;             // TOF chi2
  Float_t fTOFnSig[4];          // TOF n-sigma deviation from expected signal
  Short_t fTOFdeltaBC;          // BC(event) - BC(track) estimated by TOF

  // TRD
  UChar_t fTRDntracklets[2];       // 0 - AliESDtrack::GetTRDntracklets(); 1 - AliESDtrack::GetTRDntrackletsPID()   TODO: use only 1 char
  Float_t fTRDpid[2];              // TRD pid 1D likelihoods, [0]-electron , [1]- pion
  Float_t fTRDpidLQ2D[2];          // TRD pid 2D likelihoods, [0]-electron , [1]- pion
  
  // EMCAL/PHOS
  Int_t  fCaloClusterId;          // ID for the calorimeter cluster (if any)
    
  ULong_t fQualityFlags;          // BIT0 toggled if track used for TPC event plane
                                  // BIT1 toggled if track belongs to a gamma conversion
                                  // BIT2 toggled if track belongs to a K0s
                                  // BIT3 toggled if track belongs to a Lambda
                                  // BIT4 toggled if track belongs to an Anti-Lambda
                                  // BIT5 toggled if the track has kink0 index > 0
                                  // BIT6 toggled if the track has kink1 index > 0
                                  // BIT7 toggled if the track has kink2 index > 0
                                  // BIT8 toggled if track belongs to a pure gamma conversion
                                  // BIT9 toggled if track belongs to a pure K0s
                                  // BIT10 toggled if track belongs to a pure Lambda
                                  // BIT11 toggled if track belongs to a pure ALambda
                                  // BIT12 toggled if the track has kink0 index < 0
                                  // BIT13 toggled if the track has kink1 index < 0
                                  // BIT14 toggled if the track has kink2 index < 0
                                  // BAYES TPC(||TOF)
                                  // BIT15 toggled if electron (prob>0.5)
                                  // BIT16 toggled if pion (prob>0.5)
                                  // BIT17 toggled if kaon (prob>0.5)
                                  // BIT18 toggled if proton (prob>0.5)
                                  // BIT19 toggled if bayes probability > 0.7
                                  // BIT20 toggled if bayes probability > 0.8
                                  // BIT21 toggled if bayes probability > 0.9
      
  AliReducedTrackInfo(const AliReducedTrackInfo &c);
  AliReducedTrackInfo& operator= (const AliReducedTrackInfo &c);

  ClassDef(AliReducedTrackInfo, 1);
};

//_______________________________________________________________________________
inline UShort_t AliReducedTrackInfo::ITSncls() const
{
  //
  // ITS number of clusters from the cluster map
  //
  UShort_t ncls=0;
  for(Int_t i=0; i<6; ++i) ncls += (ITSLayerHit(i) ? 1 : 0);
  return ncls;
}


//_______________________________________________________________________________
inline Int_t AliReducedTrackInfo::TPCClusterMapBitsFired()  const
{
  //
  // Count the number of bits fired in the TPC cluster map
  //
  Int_t nbits=0;
  for(Int_t i=0; i<8; ++i) nbits += (TPCClusterMapBitFired(i) ? 1 : 0);
  return nbits;
}

#endif