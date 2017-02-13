/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
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


/* $Id$ */

/////////////////////////////////////////////////////////////
//
// AliAnalysisTaskSE for D0 candidates invariant mass histogram
// and comparison with the MC truth and cut variables distributions.
//
// Authors: A.Dainese, andrea.dainese@lnl.infn.it
// Chiara Bianchin, chiara.bianchin@pd.infn.it (invariant mass)
// Carmelo Di Giglio, carmelo.digiglio@ba.infn.it (like sign)
// Jeremy Wilkinson, jwilkinson@physi.uni-heidelberg.de (weighted Bayesian
////////////////////////////////////////////////////////////

#include <Riostream.h>
#include <TClonesArray.h>
#include <TCanvas.h>
#include <TNtuple.h>
#include <TTree.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TDatabasePDG.h>
#include <THnSparse.h>
#include "AliVertexingHFUtils.h"
#include <TVector3.h>
#include <TVector2.h>
#include <AliAnalysisDataSlot.h>
#include <AliAnalysisDataContainer.h>
#include "AliAnalysisManager.h"
#include "AliESDtrack.h"
#include "AliVertexerTracks.h"
#include "AliAODHandler.h"
#include "AliAODEvent.h"
#include "AliAODVertex.h"
#include "AliAODTrack.h"
#include "AliAODMCHeader.h"
#include "AliAODMCParticle.h"
#include "AliAODRecoDecayHF2Prong.h"
#include "AliAODRecoCascadeHF.h"
#include "AliAnalysisVertexingHF.h"
#include "AliAnalysisTaskSE.h"
#include "AliAnalysisTaskSED0MassNonPromptFraction.h"
#include "AliNormalizationCounter.h"

using std::cout;
using std::endl;

/// \cond CLASSIMP
ClassImp(AliAnalysisTaskSED0MassNonPromptFraction);
/// \endcond

//________________________________________________________________________
AliAnalysisTaskSED0MassNonPromptFraction::AliAnalysisTaskSED0MassNonPromptFraction():
AliAnalysisTaskSE(),
  fOutputMass(0),
  fOutputMassPt(0),
  fOutputMassY(0),
  fDistr(0),
  fNentries(0), 
  fCuts(0),
  fArray(0),
  fReadMC(0),
  fCutOnDistr(0),
  fUsePid4Distr(0),
  fCounter(0),
  fNPtBins(1),
  fLsNormalization(1.),
  fFillOnlyD0D0bar(0),
  fDaughterTracks(),
  fIsSelectedCandidate(0),
  fFillVarHists(kTRUE),
  fFill(0),
  fSys(0),
  fIsRejectSDDClusters(0),
  fFillPtHist(kTRUE),
  fFillYHist(kFALSE),
  fFillImpParHist(kFALSE),
  fUseSelectionBit(kTRUE),
  fAODProtection(1),
  fFillAfterCuts(0),
  fWriteVariableTree(kFALSE),
  fVariablesTree(0),
  fCandidateVariables(),
  fPIDCheck(kFALSE),
  fDrawDetSignal(kFALSE),
  fUseQuarkTagInKine(kTRUE),
  fDetSignal(0),
  fhMultVZEROTPCoutTrackCorrNoCut(0x0),
  fhMultVZEROTPCoutTrackCorr(0x0),
  fEnablePileupRejVZEROTPCout(kFALSE)
{
  /// Default constructor
  for(Int_t ih=0; ih<5; ih++) fHistMassPtImpParTC[ih]=0x0;

}

//________________________________________________________________________
AliAnalysisTaskSED0MassNonPromptFraction::AliAnalysisTaskSED0MassNonPromptFraction(const char *name,AliRDHFCutsD0toKpi* cuts):
  AliAnalysisTaskSE(name),
  fOutputMass(0),
  fOutputMassPt(0),
  fOutputMassY(0),
  fDistr(0),
  fNentries(0),
  fCuts(0),
  fArray(0),
  fReadMC(0),
  fCutOnDistr(0),
  fUsePid4Distr(0),
  fCounter(0),
  fNPtBins(1),
  fLsNormalization(1.),
  fFillOnlyD0D0bar(0),
  fDaughterTracks(),
  fIsSelectedCandidate(0),
  fFillVarHists(kTRUE),
  fFill(0),
  fSys(0),
  fIsRejectSDDClusters(0),
  fFillPtHist(kTRUE),
  fFillYHist(kFALSE),
  fFillImpParHist(kFALSE),
  fUseSelectionBit(kTRUE),
  fAODProtection(1),
  fFillAfterCuts(0),
  fWriteVariableTree(kFALSE),
  fVariablesTree(0),
  fCandidateVariables(),
  fPIDCheck(kFALSE),
  fDrawDetSignal(kFALSE),
  fUseQuarkTagInKine(kTRUE),
  fDetSignal(0),
  fhMultVZEROTPCoutTrackCorrNoCut(0x0),
  fhMultVZEROTPCoutTrackCorr(0x0),
  fEnablePileupRejVZEROTPCout(kFALSE)
{
  /// Default constructor

  fNPtBins=cuts->GetNPtBins();
    
  fCuts=cuts;
  for(Int_t ih=0; ih<5; ih++) fHistMassPtImpParTC[ih]=0x0;

  // Output slot #1 writes into a TList container (mass with cuts)
  DefineOutput(1,TList::Class());  //My private output
  // Output slot #2 writes into a TList container (distributions)
  DefineOutput(2,TList::Class());  //My private output
  // Output slot #3 writes into a TH1F container (number of events)
  DefineOutput(3,TH1F::Class());  //My private output
  // Output slot #4 writes into a TList container (cuts)
  DefineOutput(4,AliRDHFCutsD0toKpi::Class());  //My private output
  // Output slot #5 writes Normalization Counter 
  DefineOutput(5,AliNormalizationCounter::Class());
  // Output slot #6 stores the mass vs pt and impact parameter distributions
  DefineOutput(6,TList::Class());  //My private output
  // Output slot #7 keeps a tree of the candidate variables after track selection
  DefineOutput(7,TTree::Class());  //My private outpu
  // Output slot #8 writes into a TList container (Detector signals)
  DefineOutput(8, TList::Class()); //My private output
  // Output slot #9 stores the mass vs rapidity (y) distributions
  DefineOutput(9, TList::Class()); //My private output
}

//________________________________________________________________________
AliAnalysisTaskSED0MassNonPromptFraction::~AliAnalysisTaskSED0MassNonPromptFraction()
{
  if (fOutputMass) {
    delete fOutputMass;
    fOutputMass = 0;
  }
  if (fOutputMassPt) {
    delete fOutputMassPt;
    fOutputMassPt = 0;
  }
  if (fOutputMassY) {
    delete fOutputMassY;
    fOutputMassY = 0;
  }
  if (fDistr) {
    delete fDistr;
    fDistr = 0;
  }
  if (fCuts) {
    delete fCuts;
    fCuts = 0;
  }
  for(Int_t i=0; i<5; i++){
    if(fHistMassPtImpParTC[i]) delete fHistMassPtImpParTC[i];
  }
  if (fNentries){
    delete fNentries;
    fNentries = 0;
  }
  if(fCounter){
    delete fCounter;
    fCounter=0;
  }
  if(fVariablesTree){
    delete fVariablesTree;
    fVariablesTree = 0;
  }
  if (fDetSignal) {
    delete fDetSignal;
    fDetSignal = 0;
  }
    
    if(fhMultVZEROTPCoutTrackCorrNoCut){
        delete fhMultVZEROTPCoutTrackCorrNoCut;
        fhMultVZEROTPCoutTrackCorrNoCut = 0;
    }
    if (fhMultVZEROTPCoutTrackCorr) {
        delete fhMultVZEROTPCoutTrackCorr;
        fhMultVZEROTPCoutTrackCorr = 0;
    }
 
}  

//________________________________________________________________________
void AliAnalysisTaskSED0MassNonPromptFraction::Init()
{
  /// Initialization

  if(fDebug > 1) printf("AnalysisTaskSED0Mass::Init() \n");

  
  AliRDHFCutsD0toKpi* copyfCuts=new AliRDHFCutsD0toKpi(*fCuts);
  const char* nameoutput=GetOutputSlot(4)->GetContainer()->GetName();
  copyfCuts->SetName(nameoutput);
  // Post the data
  PostData(4,copyfCuts);


  return;
}

//________________________________________________________________________
void AliAnalysisTaskSED0MassNonPromptFraction::UserCreateOutputObjects()
{

  /// Create the output container
  //
  if(fDebug > 1) printf("AnalysisTaskSED0Mass::UserCreateOutputObjects() \n");

  // Several histograms are more conveniently managed in a TList
  fOutputMass = new TList();
  fOutputMass->SetOwner();
  fOutputMass->SetName("listMass");

  fOutputMassPt = new TList();
  fOutputMassPt->SetOwner();
  fOutputMassPt->SetName("listMassPt");

  fOutputMassY = new TList();
  fOutputMassY->SetOwner();
  fOutputMassY->SetName("listMassY");

  fDistr = new TList();
  fDistr->SetOwner();
  fDistr->SetName("distributionslist");

  fDetSignal = new TList();
  fDetSignal->SetOwner();
  fDetSignal->SetName("detectorsignals");

  TString nameMass=" ",nameSgn27=" ",nameSgn=" ", nameBkg=" ", nameRfl=" ",nameMassNocutsS =" ",nameMassNocutsB =" ", namedistr=" ";
  TString nameMassPt="", nameSgnPt="", nameBkgPt="", nameRflPt="";
  TString nameMassY="", nameSgnY="", nameBkgY="", nameRflY="";
  Int_t nbins2dPt=60; Float_t binInPt=0., binFinPt=30.;
  Int_t nbins2dY=60; Float_t binInY=-1.5, binFinY=1.5;

  for(Int_t i=0;i<fCuts->GetNPtBins();i++){

    nameMass="histMass_";
    nameMass+=i;
    nameSgn27="histSgn27_";
    nameSgn27+=i;
    nameSgn="histSgn_";
    nameSgn+=i;
    
    nameBkg="histBkg_";
       nameBkg+=i;
    
    nameRfl="histRfl_";
    nameRfl+=i;
    nameMassNocutsS="hMassS_";
    nameMassNocutsS+=i;
    nameMassNocutsB="hMassB_";
    nameMassNocutsB+=i;

    //histograms of cut variable distributions
    if(fFillVarHists){
      if(fReadMC){

	namedistr="hNclsD0vsptS_";
	namedistr+=i;
	TH2F *hNclsD0vsptS = new TH2F(namedistr.Data(),"N cls distrubution [S];p_{T} [GeV/c];N cls",200,0.,20.,100,0.,200.);
	namedistr="hNclsD0barvsptS_";
	namedistr+=i;
	TH2F *hNclsD0barvsptS = new TH2F(namedistr.Data(),"N cls distrubution [S];p_{T} [GeV/c];N cls",200,0.,20.,100,0.,200.);
	
	namedistr="hNITSpointsD0vsptS_";
	namedistr+=i;
	TH2F *hNITSpointsD0vsptS = new TH2F(namedistr.Data(),"N ITS points distrubution [S];p_{T} [GeV/c];N points",200,0.,20.,7,0.,7.);
	
	namedistr="hNSPDpointsD0S_";
	namedistr+=i;
	TH1I *hNSPDpointsD0S = new TH1I(namedistr.Data(),"N SPD points distrubution [S]; ;N tracks",4,0,4);
	hNSPDpointsD0S->GetXaxis()->SetBinLabel(1, "no SPD");
	hNSPDpointsD0S->GetXaxis()->SetBinLabel(2, "kOnlyFirst");
	hNSPDpointsD0S->GetXaxis()->SetBinLabel(3, "kOnlySecond");
	hNSPDpointsD0S->GetXaxis()->SetBinLabel(4, "kBoth");
      
	namedistr="hptD0S_";
	namedistr+=i;
	TH1F *hptD0S = new TH1F(namedistr.Data(), "p_{T} distribution [S];p_{T} [GeV/c]",200,0.,20.);
	namedistr="hptD0barS_";
	namedistr+=i;
	TH1F *hptD0barS = new TH1F(namedistr.Data(), "p_{T} distribution [S];p_{T} [GeV/c]",200,0.,20.);
      
	namedistr="hphiD0S_";
	namedistr+=i;
	TH1F *hphiD0S = new TH1F(namedistr.Data(), "#phi distribution [S];#phi [rad]",100,0.,2*TMath::Pi());
	namedistr="hphiD0barS_";
	namedistr+=i;
	TH1F *hphiD0barS = new TH1F(namedistr.Data(), "#phi distribution [S];#phi [rad]",100,0.,2*TMath::Pi());


	namedistr="hetaphiD0candidateS_";
	namedistr+=i;
	TH2F *hetaphiD0candidateS = new TH2F(namedistr.Data(), "D^{0} candidates #eta #phi distribution [S];#eta;#phi [rad]",100, -1.5, 1.5, 100, 0.,2*TMath::Pi());
	namedistr="hetaphiD0barcandidateS_";
	namedistr+=i;
	TH2F *hetaphiD0barcandidateS = new TH2F(namedistr.Data(), "anti-D^{0} candidates #eta #phi distribution [S];#eta;#phi [rad]",100, -1.5, 1.5, 100, 0.,2*TMath::Pi());

	namedistr="hetaphiD0candidatesignalregionS_";
	namedistr+=i;
	TH2F *hetaphiD0candidatesignalregionS = new TH2F(namedistr.Data(), "D^{0} candidates #eta #phi distribution [S] [mass cut];#eta;#phi [rad]",100, -1.5, 1.5, 100, 0.,2*TMath::Pi());
	namedistr="hetaphiD0barcandidatesignalregionS_";
	namedistr+=i;
	TH2F *hetaphiD0barcandidatesignalregionS = new TH2F(namedistr.Data(), "anti-D^{0} candidates #eta #phi distribution [S] [mass cut];#eta;#phi [rad]",100, -1.5, 1.5, 100, 0.,2*TMath::Pi());

	//  dca
	namedistr="hdcaS_";
	namedistr+=i;
	TH1F *hdcaS = new TH1F(namedistr.Data(), "DCA distribution;dca [cm]",200,0.,0.1);
	// impact parameter
	namedistr="hd0piS_";
	namedistr+=i;
	TH1F *hd0piS = new TH1F(namedistr.Data(), "Impact parameter distribution (pions);d0(#pi) [cm]",200,-0.1,0.1);

	namedistr="hd0KS_";
	namedistr+=i;
	TH1F *hd0KS = new TH1F(namedistr.Data(), "Impact parameter distribution (kaons);d0(K) [cm]",200,-0.1,0.1);
          
	namedistr="hd0d0S_";
	namedistr+=i;
	TH1F *hd0d0S = new TH1F(namedistr.Data(), "d_{0}#timesd_{0} distribution;d_{0}#timesd_{0} [cm^{2}]",200,-0.001,0.001);
 
	//decay lenght
	namedistr="hdeclS_";
	namedistr+=i;
	TH1F *hdeclengthS = new TH1F(namedistr.Data(), "Decay Length^{2} distribution;Decay Length^{2} [cm]",200,0,0.015);

	namedistr="hnormdeclS_";
	namedistr+=i;
	TH1F *hnormdeclengthS = new TH1F(namedistr.Data(), "Normalized Decay Length^{2} distribution;(Decay Length/Err)^{2} ",200,0,12.);

	namedistr="hdeclxyS_";
	namedistr+=i;
	TH1F* hdeclxyS=new TH1F(namedistr.Data(),"Decay Length XY distribution;Decay Length XY [cm]",200,0,0.15);
          
	namedistr="hnormdeclxyS_";
	namedistr+=i;
	TH1F* hnormdeclxyS=new TH1F(namedistr.Data(),"Normalized decay Length XY distribution;Decay Length XY/Err",200,0,6.);
    
          
          namedistr="hCorrCosX_";
          namedistr+=i;
          TH2F* hCorrCosPpdX =new TH2F(namedistr.Data(), "Correlation CosPointAngle vs Pseudo-proper decay-length",200, -1, 1, 1000,-1.5,1.5);
          
          
          
          namedistr="hCorrCosXtrue_";
          namedistr+=i;
          TH2F* hCorrCosPpdXtrue =new TH2F(namedistr.Data(),"Correlation CosPointAngle vs Pseudo-proper decay-length",200, -1, 1, 1000,-1.5,1.5);
          
          namedistr="hCorrCosX_prim_";
          namedistr+=i;
          TH2F* hCorrCosPpdX_prim =new TH2F(namedistr.Data(),"Correlation CosPointAngle vs Pseudo-proper decay-length",200, -1, 1, 1000,-1.5,1.5);
          
          namedistr="hCorrCosX_sec_";
          namedistr+=i;
          TH2F* hCorrCosPpdX_sec =new TH2F(namedistr.Data(),"Correlation CosPointAngle vs Pseudo-proper decay-length",200, -1, 1, 1000,-1.5,1.5);
          
          
          
          
          namedistr="hCorrDistX_";
          namedistr+=i;
          TH2F* hCorrDistPpdX =new TH2F(namedistr.Data(),"Distance vs pseudo-proper decay-length correlation",1000, 0, 1.5, 1000,-1.5,1.5);
          
          
          namedistr="hCorrDistXtrue_";
          namedistr+=i;
          TH2F* hCorrDistPpdXtrue =new TH2F(namedistr.Data(),"Distance vs pseudo-proper decay-length correlation",1000, 0, 1.5, 1000,-1.5,1.5);
          
          
          namedistr="hCorrDistX_prim_";
          namedistr+=i;
          TH2F* hCorrDistPpdX_prim =new TH2F(namedistr.Data(),"Distance vs pseudo-proper decay-length correlation",1000, 0, 1.5, 1000,-1.5,1.5);
          
          
          namedistr="hCorrDistX_sec_";
          namedistr+=i;
          TH2F* hCorrDistPpdX_sec =new TH2F(namedistr.Data(),"Distance vs pseudo-proper decay-length correlation",1000, 0, 1.5, 1000,-1.5,1.5);
          
          namedistr="hCorrNormLxyXtrue_";
          namedistr+=i;
          TH2F* hcorrnormLxyXtrue =new TH2F(namedistr.Data(),"Normalized Lxy vs pseudo-proper decay-length correlation",1000, 0, 1.5, 1000,-1.5,1.5);
          
          
          namedistr="hCorrNormLxyX_prim_";
          namedistr+=i;
          TH2F* hcorrnormLxyX_prim =new TH2F(namedistr.Data(),"Normalized Lxy vs pseudo-proper decay-length correlatio",1000, 0, 1.5, 1000,-1.5,1.5);
          
          
          namedistr="hCorrNormLxyX_sec_";
          namedistr+=i;
          TH2F* hcorrnormLxyX_sec =new TH2F(namedistr.Data(),"Normalized Lxy vs pseudo-proper decay-length correlatio",1000, 0, 1.5, 1000,-1.5,1.5);
          

          
          
          namedistr="hpseudoProperDLRec_";
          namedistr+=i;
          TH1F* hpseudoProperDecLen =new TH1F(namedistr.Data(),"Pseudo-proper decay-length (reconstructed)",1000,-0.1,0.5);
          
          
          namedistr="hpseudoProperGENERATE_";
          namedistr+=i;
          TH1F* hpseudoProperGENERATED  =new TH1F(namedistr.Data(),"Pseudo proper decay length (MC-generated)",1000,-1.5,1.5);
          

          namedistr="hpseudoProperPrimGenerate_";
          namedistr+=i;
          TH1F* hXPrimGenerate  =new TH1F(namedistr.Data(),"Pseudo proper decay length (MC-generated)",1500,-1.5,1.5);
          
          namedistr="hpseudoProperSecGenerate_";
          namedistr+=i;
          TH1F* hXSecGenerate  =new TH1F(namedistr.Data(),"Pseudo proper decay length (MC-generated)",1000,-1.5,1.5);

          
          
          namedistr="hGenMatchRec_";
          namedistr+=i;
          TH1F* hGenReconstructed  =new TH1F(namedistr.Data(),"Pseudo proper decay length (MC-generated matched reconstructed)",1000,-1.5,1.5);
          
          
          namedistr="hGenMatchRecPrim_";
          namedistr+=i;
          TH1F* hGenRecPrimaries  =new TH1F(namedistr.Data(),"Pseudo proper decay length (MC-generated matched reconstructed)",2000,-1.5,1.5);
          
          namedistr="hGenMatchRecSec_";
          namedistr+=i;
          TH1F* hGenRecSecondaries  =new TH1F(namedistr.Data(),"Pseudo proper decay length (MC-generated matched reconstructed)",1000,-1.5,1.5);
          
          namedistr="hresidualX_";
          namedistr+=i;
          TH1F* hresidualX =new TH1F(namedistr.Data(),"Residual x - x_{gen}",1000,-1.5,1.5);


          
          namedistr="hpseudoProperDLRecSec_";
          namedistr+=i;
          TH1F* hpseudoProperDecLen2 =new TH1F(namedistr.Data(),"Pseudo proper decay length (reconstructed)",1000,-0.1,0.5);
          
          namedistr="hresidualXSec_";
          namedistr+=i;
          TH1F* hresidualpseudoXsec =new TH1F(namedistr.Data(),"Residual x - x_{gen}",1000,-1.5,1.5);
          
          
          namedistr="hresidual_Primary_";
          namedistr+=i;
          TH1F* hresidualXprim =new TH1F(namedistr.Data(),"Residual x - x_{gen}",1000,-1.5,1.5);
          
          namedistr="hpseudoProperDLRecPrim_";
          namedistr+=i;
          TH1F* hpseudoProperDecLenprim =new TH1F(namedistr.Data(),"Pseudo proper decay length (reconstructed)",1000,-0.1,0.5);

          
          
          namedistr="hctauS_";
          namedistr+=i;
          TH1F* hdctauS = new TH1F(namedistr.Data(),"Proper Decay time distribution (zeta component); ctau [cm]",200,0,0.5);
          

          
          
          namedistr="hctauD0_";
          namedistr+=i;
          TH1F* hctauD0meson1=new TH1F(namedistr.Data(),"CtauD0 [cm]",200,0,0.5);
          
          namedistr="hctauD0S_";
          namedistr+=i;
          TH1F* hctauD0meson1S=new TH1F(namedistr.Data(),"CtauD0 [cm]",200,0,0.5);
          

          
          
          namedistr="hctauXYctauZ_";
          namedistr+=i;
          TH2F* hctauXYvsZ=new TH2F(namedistr.Data(),"ctauD0-XY plane vs Ctau-Z [cm]",200,0,0.5,200,0,0.05);
 
          
          namedistr="hctauXYctauZS_";
          namedistr+=i;
          TH2F* hctauXYvsZS=new TH2F(namedistr.Data(),"ctauD0-XY plane vs Ctau-Z [cm]",200,0,0.5,200,0,0.05);
          


          
          namedistr="hctauD0prim_";
          namedistr+=i;
          TH1F* hctauD0primary=new TH1F(namedistr.Data(),"ctauD0 [cm]",200,0,0.5);
          
          

          
          namedistr="hctauD0sec_";
          namedistr+=i;
          TH1F* hctauD0secondary =new TH1F(namedistr.Data(),"ctauD0 [cm]",200,0,0.5);
        
          


	     namedistr="hdeclxyd0d0S_";
	     namedistr+=i;
	     TH2F* hdeclxyd0d0S=new TH2F(namedistr.Data(),"Correlation decay Length XY [cm] vs  d_{0}#times d_{0} [cm^{2}]",200,0,0.15,200,-0.001,0.001);

         namedistr="hnormdeclxyd0d0S_";
	     namedistr+=i;
	     TH2F* hnormdeclxyd0d0S=new TH2F(namedistr.Data(),"Correlation normalized decay Length XY - d_{0}#times d_{0};Decay Length XY/Err;d_{0}#times d_{0}[cm^{2}]",200,0,6,200,-0.001,0.001);
          
         
          
          //correlation ctau XY vs d0 D0
          namedistr="hCorrXd0D0_";
          namedistr+=i;
          TH2F* hcorrXd0D0 = new TH2F(namedistr.Data(),"Correlation Proper Decay time XY - D0 d_{0} ; Proper decay time XY [cm]; d_{0}[cm]",200,-1.5, 1.5, 1000,-1000,1000);
          
          
          namedistr="hCorrXd0D0true_";
          namedistr+=i;
          TH2F* hcorrXd0D0true = new TH2F(namedistr.Data(),"Correlation Proper Decay time XY - D0 d_{0} ; Proper decay time XY [cm]; d_{0}[cm]",200,-1.5, 1.5, 1000,-1000,1000);
          

          namedistr="hCorrXd0D0_sec_";
          namedistr+=i;
          TH2F* hcorrXd0D0sec = new TH2F(namedistr.Data(),"Correlation Proper Decay time XY - D0 d_{0} ; Proper decay time XY [cm]; d_{0}[cm]",200,-1.5, 1.5, 1000,-1000,1000);
          

          namedistr="hCorrXd0D0_prim_";
          namedistr+=i;
          TH2F* hcorrXd0D0prim = new TH2F(namedistr.Data(),"Correlation Proper Decay time XY - D0 d_{0} ; Proper decay time XY [cm]; d_{0}[cm]",200,-1.5, 1.5, 1000,-1000,1000);
          
          
          
          namedistr="hImpParXYtrue_";
          namedistr+=i;
          TH1F* hImpactParXYtrue = new TH1F(namedistr.Data()," D0 d_{0} ; Proper decay time XY [cm]; d_{0}[cm]", 1000,-1000,1000);
          
          
          namedistr="hImpParXY_sec_";
          namedistr+=i;
          TH1F* hImpParXYsec = new TH1F(namedistr.Data(),"D0 d_{0} ; Proper decay time XY [cm]; d_{0}[cm]",1000,-1000,1000);
          
          
          namedistr="hImpParXY_prim_";
          namedistr+=i;
          TH1F* hImpParXYprim = new TH1F(namedistr.Data(),"D0 d_{0} ; Proper decay time XY [cm]; d_{0}[cm]",1000,-1000,1000);
          

         

	//  costhetapoint
	namedistr="hcosthetapointS_";
	namedistr+=i;
	TH1F *hcosthetapointS = new TH1F(namedistr.Data(), "cos#theta_{Point} distribution;cos#theta_{Point}",200,0,1.);

	namedistr="hcosthetapointxyS_";
	namedistr+=i;
	TH1F *hcosthetapointxyS = new TH1F(namedistr.Data(), "cos#theta_{Point} XYdistribution;cos#theta_{Point}",300,0.,1.);

	TH1F* tmpMS = new TH1F(nameMassNocutsS.Data(),"D^{0} invariant mass; M [GeV]; Entries",300,1.5648,2.1648); //range (MD0-300MeV, mD0 + 300MeV)
	tmpMS->Sumw2();

	fDistr->Add(hNclsD0vsptS);
	fDistr->Add(hNclsD0barvsptS);
	fDistr->Add(hNITSpointsD0vsptS);
	fDistr->Add(hNSPDpointsD0S);
	fDistr->Add(hptD0S);
	fDistr->Add(hphiD0S);
	fDistr->Add(hptD0barS);
	fDistr->Add(hphiD0barS);
	fDistr->Add(hetaphiD0candidateS);
	fDistr->Add(hetaphiD0candidatesignalregionS);
	fDistr->Add(hetaphiD0barcandidateS);
	fDistr->Add(hetaphiD0barcandidatesignalregionS);
	fDistr->Add(hdcaS);
	fDistr->Add(hd0piS);
	fDistr->Add(hd0KS);
	fDistr->Add(hd0d0S);
	fDistr->Add(hcosthetapointS);
	fDistr->Add(hcosthetapointxyS);
	fDistr->Add(hdeclengthS);
	fDistr->Add(hnormdeclengthS);
	fDistr->Add(hdeclxyS);
    fDistr->Add(hdctauS);
    fDistr->Add(hctauD0meson1);
    fDistr->Add(hctauD0meson1S);
    fDistr->Add(hctauXYvsZ);
    fDistr->Add(hctauXYvsZS);
    fDistr->Add(hctauD0primary);
    fDistr->Add(hctauD0secondary);
    fDistr->Add(hnormdeclxyS);
    fDistr->Add(hCorrCosPpdX);
    fDistr->Add(hCorrCosPpdXtrue);
    fDistr->Add(hCorrCosPpdX_prim);
    fDistr->Add(hCorrCosPpdX_sec);
    fDistr->Add(hCorrDistPpdX);
    fDistr->Add(hCorrDistPpdXtrue);
    fDistr->Add(hCorrDistPpdX_prim);
    fDistr->Add(hCorrDistPpdX_sec);
    fDistr->Add(hcorrnormLxyXtrue);
    fDistr->Add(hcorrnormLxyX_prim);
    fDistr->Add(hcorrnormLxyX_sec);
    fDistr->Add(hpseudoProperDecLen);
    fDistr->Add(hpseudoProperGENERATED);
    fDistr->Add(hXSecGenerate);
    fDistr->Add(hXPrimGenerate);
    fDistr->Add(hGenReconstructed);
    fDistr->Add(hGenRecPrimaries);
    fDistr->Add(hGenRecSecondaries);
    fDistr->Add(hpseudoProperDecLen2);
    fDistr->Add(hresidualX);
    fDistr->Add(hresidualpseudoXsec);
    fDistr->Add(hresidualXprim);
    fDistr->Add(hpseudoProperDecLenprim);
    fDistr->Add(hdeclxyd0d0S);
    fDistr->Add(hcorrXd0D0);
    fDistr->Add(hcorrXd0D0true);
    fDistr->Add(hcorrXd0D0sec);
    fDistr->Add(hcorrXd0D0prim);
    fDistr->Add(hImpactParXYtrue);
    fDistr->Add(hImpParXYsec);
    fDistr->Add(hImpParXYprim);
    fDistr->Add(hnormdeclxyd0d0S);
    fDistr->Add(tmpMS);
      }


      //Ncls, phi, pt distrubutions

      namedistr="hNclsD0vsptB_";
      namedistr+=i;
      TH2F *hNclsD0vsptB = new TH2F(namedistr.Data(),"N cls distrubution [B];p_{T} [GeV/c];N cls",200,0.,20.,100,0.,200.);
      namedistr="hNclsD0barvsptB_";
      namedistr+=i;
      TH2F *hNclsD0barvsptB = new TH2F(namedistr.Data(),"N cls distrubution [B];p_{T} [GeV/c];N cls",200,0.,20.,100,0.,200.);

      namedistr="hNITSpointsD0vsptB_";
      namedistr+=i;
      TH2F *hNITSpointsD0vsptB = new TH2F(namedistr.Data(),"N ITS points distrubution [B];p_{T} [GeV/c];N points",200,0.,20.,7,0.,7.);

      namedistr="hNSPDpointsD0B_";
      namedistr+=i;
      TH1I *hNSPDpointsD0B = new TH1I(namedistr.Data(),"N SPD points distrubution [B]; ;N tracks",4,0,4);
      hNSPDpointsD0B->GetXaxis()->SetBinLabel(1, "no SPD");
      hNSPDpointsD0B->GetXaxis()->SetBinLabel(2, "kOnlyFirst");
      hNSPDpointsD0B->GetXaxis()->SetBinLabel(3, "kOnlySecond");
      hNSPDpointsD0B->GetXaxis()->SetBinLabel(4, "kBoth");

      namedistr="hptD0B_";
      namedistr+=i;
      TH1F *hptD0B = new TH1F(namedistr.Data(), "p_{T} distribution [B];p_{T} [GeV/c]",200,0.,20.);
      namedistr="hptD0barB_";
      namedistr+=i;
      TH1F *hptD0barB = new TH1F(namedistr.Data(), "p_{T} distribution [B];p_{T} [GeV/c]",200,0.,20.);
      
      namedistr="hphiD0B_";
      namedistr+=i;
      TH1F *hphiD0B = new TH1F(namedistr.Data(), "#phi distribution [B];#phi [rad]",100,0.,2*TMath::Pi());
      namedistr="hphiD0barB_";
      namedistr+=i;
      TH1F *hphiD0barB = new TH1F(namedistr.Data(), "#phi distribution [B];#phi [rad]",100,0.,2*TMath::Pi());

      namedistr="hetaphiD0candidateB_";
      namedistr+=i;
      TH2F *hetaphiD0candidateB = new TH2F(namedistr.Data(), "D^{0} candidates #eta #phi distribution [B];#eta;#phi [rad]",100, -1.5, 1.5, 100, 0.,2*TMath::Pi());
      namedistr="hetaphiD0barcandidateB_";
      namedistr+=i;
      TH2F *hetaphiD0barcandidateB = new TH2F(namedistr.Data(), "anti-D^{0} candidates #eta #phi distribution [B];#eta;#phi [rad]",100, -1.5, 1.5, 100, 0.,2*TMath::Pi());
      
      namedistr="hetaphiD0candidatesignalregionB_";
      namedistr+=i;
      TH2F *hetaphiD0candidatesignalregionB = new TH2F(namedistr.Data(), "D^{0} candidates #eta #phi distribution [B] [mass cut];#eta;#phi [rad]",100, -1.5, 1.5, 100, 0.,2*TMath::Pi());
      namedistr="hetaphiD0barcandidatesignalregionB_";
      namedistr+=i;
      TH2F *hetaphiD0barcandidatesignalregionB = new TH2F(namedistr.Data(), "anti-D^{0} candidates #eta #phi distribution [B] [mass cut];#eta;#phi [rad]",100, -1.5, 1.5, 100, 0.,2*TMath::Pi());
      
      //  dca
      namedistr="hdcaB_";
      namedistr+=i;
      TH1F *hdcaB = new TH1F(namedistr.Data(), "DCA distribution;dca [cm]",200,0.,0.1);

      // impact parameter
      namedistr="hd0B_";
      namedistr+=i;
      TH1F *hd0B = new TH1F(namedistr.Data(), "Impact parameter distribution (both);d0 [cm]",200,-0.1,0.1);
 
      namedistr="hd0d0B_";
      namedistr+=i;
      TH1F *hd0d0B = new TH1F(namedistr.Data(), "d_{0}#timesd_{0} distribution;d_{0}#timesd_{0} [cm^{2}]",200,-0.001,0.001);

      //decay lenght
      namedistr="hdeclB_";
      namedistr+=i;
      TH1F *hdeclengthB = new TH1F(namedistr.Data(), "Decay Length^{2} distribution;Decay Length^{2} [cm^{2}]",200,0,0.015);

      namedistr="hnormdeclB_";
      namedistr+=i;
      TH1F *hnormdeclengthB = new TH1F(namedistr.Data(), "Normalized Decay Length distribution;(Decay Length/Err)^{2} ",200,0,12.);
        
        
      namedistr="hctau_";
      namedistr+=i;
      TH1F *hctauD = new TH1F(namedistr.Data(), "Ctau D meson;(ctau);[cm] ",200,0,0.15);

    



      namedistr="hdeclxyB_";
      namedistr+=i;
      TH1F* hdeclxyB=new TH1F(namedistr.Data(),"Decay Length XY distribution;Decay Length XY [cm]",200,0,0.15);
      namedistr="hnormdeclxyB_";
      namedistr+=i;
      TH1F* hnormdeclxyB=new TH1F(namedistr.Data(),"Normalized decay Length XY distribution;Decay Length XY/Err",200,0,6.); 

      namedistr="hdeclxyd0d0B_";
      namedistr+=i;
      TH2F* hdeclxyd0d0B=new TH2F(namedistr.Data(),"Correlation decay Length XY - d_{0}#times d_{0};Decay Length XY [cm];d_{0}#times d_{0}[cm^{2}]",200,0,0.15,200,-0.001,0.001);

      namedistr="hnormdeclxyd0d0B_";
      namedistr+=i;
      TH2F* hnormdeclxyd0d0B=new TH2F(namedistr.Data(),"Correlation normalized decay Length XY - d_{0}#times d_{0};Decay Length XY/Err;d_{0}#times d_{0}[cm^{2}]",200,0,6,200,-0.001,0.001);

      //  costhetapoint
      namedistr="hcosthetapointB_";
      namedistr+=i;
      TH1F *hcosthetapointB = new TH1F(namedistr.Data(), "cos#theta_{Point} distribution;cos#theta_{Point}",200,0,1.);

      namedistr="hcosthetapointxyB_";
      namedistr+=i;
      TH1F *hcosthetapointxyB = new TH1F(namedistr.Data(), "cos#theta_{Point} XY distribution;cos#theta_{Point} XY",300,0.,1.);

      TH1F* tmpMB = new TH1F(nameMassNocutsB.Data(),"D^{0} invariant mass; M [GeV]; Entries",300,1.5648,2.1648); //range (MD0-300MeV, mD0 + 300MeV)
      tmpMB->Sumw2();


      fDistr->Add(hNclsD0vsptB);
      fDistr->Add(hNclsD0barvsptB);
      fDistr->Add(hNITSpointsD0vsptB);
      fDistr->Add(hNSPDpointsD0B);
      fDistr->Add(hptD0B);
      fDistr->Add(hphiD0B);
      fDistr->Add(hptD0barB);
      fDistr->Add(hphiD0barB);
      fDistr->Add(hetaphiD0candidateB);
      fDistr->Add(hetaphiD0candidatesignalregionB);
      fDistr->Add(hetaphiD0barcandidateB);
      fDistr->Add(hetaphiD0barcandidatesignalregionB);
      fDistr->Add(hdcaB);
      fDistr->Add(hd0B);
      fDistr->Add(hd0d0B);
      fDistr->Add(hcosthetapointB);
      fDistr->Add(hcosthetapointxyB);
      fDistr->Add(hdeclengthB);
      fDistr->Add(hnormdeclengthB);
      fDistr->Add(hctauD);
      fDistr->Add(hdeclxyB);
      fDistr->Add(hnormdeclxyB);
      fDistr->Add(hdeclxyd0d0B);
      fDistr->Add(hnormdeclxyd0d0B);
      fDistr->Add(tmpMB);

      //histograms filled only when the secondary vertex is recalculated w/o the daughter tracks (as requested in the cut object)

      if(fCuts->GetIsPrimaryWithoutDaughters()){
    if(fReadMC){
	  namedistr="hd0vpiS_";
	  namedistr+=i;
	  TH1F *hd0vpiS = new TH1F(namedistr.Data(), "Impact parameter distribution (pions)(vtx w/o these tracks);d0(#pi) [cm]",200,-0.1,0.1);
	  
	  namedistr="hd0vKS_";
	  namedistr+=i;
	  TH1F *hd0vKS = new TH1F(namedistr.Data(), "Impact parameter distribution (kaons) (vtx w/o these tracks);d0(K) [cm]",200,-0.1,0.1);

	  namedistr="hd0d0vS_";
	  namedistr+=i;
	  TH1F *hd0d0vS = new TH1F(namedistr.Data(), "d_{0}#timesd_{0} distribution (vtx w/o these tracks);d_{0}#timesd_{0} [cm^{2}]",200,-0.001,0.001);
 
	  namedistr="hdeclvS_";
	  namedistr+=i;
	  TH1F *hdeclengthvS = new TH1F(namedistr.Data(), "Decay Length distribution (vtx w/o tracks);Decay Length [cm]",200,0,0.6);

	  namedistr="hnormdeclvS_";
	  namedistr+=i;
	  TH1F *hnormdeclengthvS = new TH1F(namedistr.Data(), "Normalized Decay Length distribution (vtx w/o tracks);Decay Length/Err ",200,0,10.);

	  fDistr->Add(hd0vpiS);
	  fDistr->Add(hd0vKS);
	  fDistr->Add(hd0d0vS);
	  fDistr->Add(hdeclengthvS);
	  fDistr->Add(hnormdeclengthvS);

	}

	namedistr="hd0vmoresB_";
	namedistr+=i;
	TH1F *hd0vmoresB = new TH1F(namedistr.Data(), "Impact parameter distribution (both);d0 [cm]",200,-0.1,0.1);

	namedistr="hd0d0vmoresB_";
	namedistr+=i;
	TH1F *hd0d0vmoresB = new TH1F(namedistr.Data(), "Impact parameter distribution (prong +);d0 [cm]",200,-0.001,0.001);


	namedistr="hd0vB_";
	namedistr+=i;
	TH1F *hd0vB = new TH1F(namedistr.Data(), "Impact parameter distribution (vtx w/o these tracks);d0 [cm]",200,-0.1,0.1);

	namedistr="hd0vp0B_";
	namedistr+=i;
	TH1F *hd0vp0B = new TH1F(namedistr.Data(), "Impact parameter distribution (prong + ** vtx w/o these tracks);d0 [cm]",200,-0.1,0.1);

	namedistr="hd0vp1B_";
	namedistr+=i;
	TH1F *hd0vp1B = new TH1F(namedistr.Data(), "Impact parameter distribution (prong - ** vtx w/o these tracks);d0 [cm]",200,-0.1,0.1);

	namedistr="hd0d0vB_";
	namedistr+=i;
	TH1F *hd0d0vB = new TH1F(namedistr.Data(), "d_{0}#timesd_{0} distribution (vtx w/o these tracks);d_{0}#timesd_{0} [cm^{2}]",200,-0.001,0.001);

	namedistr="hdeclvB_";
	namedistr+=i;
	TH1F *hdeclengthvB = new TH1F(namedistr.Data(), "Decay Length distribution (vtx w/o tracks);Decay Length [cm]",200,0,0.6);

	namedistr="hnormdeclvB_";
	namedistr+=i;
	TH1F *hnormdeclengthvB = new TH1F(namedistr.Data(), "Normalized Decay Length distribution (vtx w/o tracks);Decay Length/Err ",200,0,10.);

	fDistr->Add(hd0vB);
	fDistr->Add(hd0vp0B);
	fDistr->Add(hd0vp1B);
	fDistr->Add(hd0vmoresB);

	fDistr->Add(hd0d0vB);
	fDistr->Add(hd0d0vmoresB);

	fDistr->Add(hdeclengthvB);

	fDistr->Add(hnormdeclengthvB);
      }


    }
    //histograms of invariant mass distributions


    //MC signal
    if(fReadMC){
      TH1F* tmpSt = new TH1F(nameSgn.Data(), "D^{0} invariant mass - MC; M [GeV]; Entries",600,1.5648,2.1648);

      TH1F *tmpSl=(TH1F*)tmpSt->Clone();
      tmpSt->Sumw2();
      tmpSl->Sumw2();

      //Reflection: histo filled with D0Mass which pass the cut (also) as D0bar and with D0bar which pass (also) the cut as D0
      TH1F* tmpRt = new TH1F(nameRfl.Data(), "Reflected signal invariant mass - MC; M [GeV]; Entries",600,1.5648,2.1648);
      //TH1F *tmpRl=(TH1F*)tmpRt->Clone();
      TH1F* tmpBt = new TH1F(nameBkg.Data(), "Background invariant mass - MC; M [GeV]; Entries",600,1.5648,2.1648);
      //TH1F *tmpBl=(TH1F*)tmpBt->Clone();
      tmpBt->Sumw2();
      //tmpBl->Sumw2();
      tmpRt->Sumw2();
      //tmpRl->Sumw2();

      fOutputMass->Add(tmpSt);
      fOutputMass->Add(tmpRt);
      fOutputMass->Add(tmpBt);

    }

    //mass
    TH1F* tmpMt = new TH1F(nameMass.Data(),"D^{0} invariant mass; M [GeV]; Entries",600,1.5648,2.1648);
    //TH1F *tmpMl=(TH1F*)tmpMt->Clone();
    tmpMt->Sumw2();
    //tmpMl->Sumw2();
    //distribution w/o cuts large range
    // TH1F* tmpMS = new TH1F(nameMassNocutsS.Data(),"D^{0} invariant mass; M [GeV]; Entries",300,0.7,3.);

    fOutputMass->Add(tmpMt);

    if(fSys==0){ //histograms filled only in pp to save time in PbPb
      if(fFillVarHists){

	if(fReadMC){
	  //  pT
	  namedistr="hptpiS_";
	  namedistr+=i;
	  TH1F *hptpiS = new TH1F(namedistr.Data(), "P_{T} distribution (pions);p_{T} [GeV/c]",200,0.,8.);
	
	  namedistr="hptKS_";
	  namedistr+=i;
	  TH1F *hptKS = new TH1F(namedistr.Data(), "P_{T} distribution (kaons);p_{T} [GeV/c]",200,0.,8.);

	  //  costhetastar
	  namedistr="hcosthetastarS_";
	  namedistr+=i;
	  TH1F *hcosthetastarS = new TH1F(namedistr.Data(), "cos#theta* distribution;cos#theta*",200,-1.,1.);

	  //pT no mass cut

	  namedistr="hptpiSnoMcut_";
	  namedistr+=i;
	  TH1F *hptpiSnoMcut = new TH1F(namedistr.Data(), "P_{T} distribution (pions);p_{T} [GeV/c]",200,0.,8.);

	  namedistr="hptKSnoMcut_";
	  namedistr+=i;
	  TH1F *hptKSnoMcut = new TH1F(namedistr.Data(), "P_{T} distribution (kaons);p_{T} [GeV/c]",200,0.,8.);

	  fDistr->Add(hptpiS);
	  fDistr->Add(hptKS);
	  fDistr->Add(hcosthetastarS);

	  fDistr->Add(hptpiSnoMcut);
	  fDistr->Add(hptKSnoMcut);

	  //  costhetapoint vs d0 or d0d0
	  namedistr="hcosthpointd0S_";
	  namedistr+=i;
	  TH2F *hcosthpointd0S= new TH2F(namedistr.Data(),"Correlation cos#theta_{Point}-d_{0};cos#theta_{Point};d_{0} [cm^{2}]",200,0,1.,200,-0.001,0.001);
	  namedistr="hcosthpointd0d0S_";
	  namedistr+=i;
	  TH2F *hcosthpointd0d0S= new TH2F(namedistr.Data(),"Correlation cos#theta_{Point}-d_{0}#timesd_{0};cos#theta_{Point};d_{0}#timesd_{0} [cm^{2}]",200,0,1.,200,-0.001,0.001);

	  fDistr->Add(hcosthpointd0S);
	  fDistr->Add(hcosthpointd0d0S);

	  //to compare with AliAnalysisTaskCharmFraction
	  TH1F* tmpS27t = new TH1F(nameSgn27.Data(),"D^{0} invariant mass in M(D^{0}) +/- 27 MeV - MC; M [GeV]; Entries",600,1.5648,2.1648);
	  TH1F *tmpS27l=(TH1F*)tmpS27t->Clone();
	  tmpS27t->Sumw2();
	  tmpS27l->Sumw2();
 
	  fOutputMass->Add(tmpS27t);
	  fOutputMass->Add(tmpS27l);

	}

	//  pT
	namedistr="hptB_";
	namedistr+=i;
	TH1F *hptB = new TH1F(namedistr.Data(), "P_{T} distribution;p_{T} [GeV/c]",200,0.,8.);

	//  costhetastar
	namedistr="hcosthetastarB_";
	namedistr+=i;
	TH1F *hcosthetastarB = new TH1F(namedistr.Data(), "cos#theta* distribution;cos#theta*",200,-1.,1.);

	//pT no mass cut
	namedistr="hptB1prongnoMcut_";
	namedistr+=i;
	TH1F *hptB1pnoMcut = new TH1F(namedistr.Data(), "P_{T} distribution;p_{T} [GeV/c]",200,0.,8.);

	namedistr="hptB2prongsnoMcut_";
	namedistr+=i;
	TH1F *hptB2pnoMcut = new TH1F(namedistr.Data(), "P_{T} distribution;p_{T} [GeV/c]",200,0.,8.);
    
	fDistr->Add(hptB);
	fDistr->Add(hcosthetastarB);

	fDistr->Add(hptB1pnoMcut);
	fDistr->Add(hptB2pnoMcut);

	//impact parameter of negative/positive track
	namedistr="hd0p0B_";
	namedistr+=i;
	TH1F *hd0p0B = new TH1F(namedistr.Data(), "Impact parameter distribution (prong +);d0 [cm]",200,-0.1,0.1);

	namedistr="hd0p1B_";
	namedistr+=i;
	TH1F *hd0p1B = new TH1F(namedistr.Data(), "Impact parameter distribution (prong -);d0 [cm]",200,-0.1,0.1);

	//impact parameter corrected for strangeness
	namedistr="hd0moresB_";
	namedistr+=i;
	TH1F *hd0moresB = new TH1F(namedistr.Data(), "Impact parameter distribution (both);d0 [cm]",200,-0.1,0.1);

	namedistr="hd0d0moresB_";
	namedistr+=i;
	TH1F *hd0d0moresB = new TH1F(namedistr.Data(), "Impact parameter distribution (prong +);d0 [cm]",200,-0.001,0.001);

    
	namedistr="hcosthetapointmoresB_";
	namedistr+=i;
	TH1F *hcosthetapointmoresB = new TH1F(namedistr.Data(), "cos#theta_{Point} distribution;cos#theta_{Point}",200,0,1.);

	//  costhetapoint vs d0 or d0d0
	namedistr="hcosthpointd0B_";
	namedistr+=i;
	TH2F *hcosthpointd0B= new TH2F(namedistr.Data(),"Correlation cos#theta_{Point}-d_{0};cos#theta_{Point};d_{0} [cm^{2}]",200,0,1.,200,-0.001,0.001);

	namedistr="hcosthpointd0d0B_";
	namedistr+=i;
	TH2F *hcosthpointd0d0B= new TH2F(namedistr.Data(),"Correlation cos#theta_{Point}-d_{0}#timesd_{0};cos#theta_{Point};d_{0}#timesd_{0} [cm^{2}]",200,0,1.,200,-0.001,0.001);

	fDistr->Add(hd0p0B);
	fDistr->Add(hd0p1B);

	fDistr->Add(hd0moresB);
	fDistr->Add(hd0d0moresB);
	fDistr->Add(hcosthetapointmoresB);


	fDistr->Add(hcosthpointd0B);


	fDistr->Add(hcosthpointd0d0B);
      }

    } //end pp histo
  }


  //for Like sign analysis

  if(fArray==1){
    namedistr="hpospair";
    TH1F* hpospair=new TH1F(namedistr.Data(),"Number of positive pairs",fCuts->GetNPtBins(),-0.5,fCuts->GetNPtBins()-0.5);
    namedistr="hnegpair";
    TH1F* hnegpair=new TH1F(namedistr.Data(),"Number of negative pairs",fCuts->GetNPtBins(),-0.5,fCuts->GetNPtBins()-0.5);
    fOutputMass->Add(hpospair);
    fOutputMass->Add(hnegpair);
  }


  // 2D Pt distributions and impact parameter histograms
  if(fFillPtHist) {

    nameMassPt="histMassPt";
    nameSgnPt="histSgnPt";
    nameBkgPt="histBkgPt";
    nameRflPt="histRflPt";

    //MC signal
    if(fReadMC){
      TH2F* tmpStPt = new TH2F(nameSgnPt.Data(), "D^{0} invariant mass - MC; M [GeV]; Entries; Pt[GeV/c]",600,1.5648,2.16484,nbins2dPt,binInPt,binFinPt);
      TH2F *tmpSlPt=(TH2F*)tmpStPt->Clone();
      tmpStPt->Sumw2();
      tmpSlPt->Sumw2();
      
      //Reflection: histo filled with D0MassV1 which pass the cut (also) as D0bar and with D0bar which pass (also) the cut as D0
      TH2F* tmpRtPt = new TH2F(nameRflPt.Data(), "Reflected signal invariant mass - MC; M [GeV]; Entries; Pt[GeV/c]",600,1.5648,2.1648,nbins2dPt,binInPt,binFinPt);
      TH2F* tmpBtPt = new TH2F(nameBkgPt.Data(), "Background invariant mass - MC; M [GeV]; Entries; Pt[GeV/c]",600,1.5648,2.1648,nbins2dPt,binInPt,binFinPt);
      tmpBtPt->Sumw2();
      tmpRtPt->Sumw2();
      
      fOutputMassPt->Add(tmpStPt);
      fOutputMassPt->Add(tmpRtPt);
      fOutputMassPt->Add(tmpBtPt);

      //       cout<<endl<<endl<<"***************************************"<<endl;
      //       cout << " created and added to the list "<< nameSgnPt.Data() <<" "<< tmpStPt <<
      // 	", "<<nameRflPt.Data() <<" " << tmpRtPt<<", "<<nameBkgPt.Data()<< " " << tmpBtPt <<endl;
      //       cout<<"***************************************"<<endl<<endl;
    }

    TH2F* tmpMtPt = new TH2F(nameMassPt.Data(),"D^{0} invariant mass; M [GeV]; Entries; Pt[GeV/c]",600,1.5648,2.1648,nbins2dPt,binInPt,binFinPt);
    tmpMtPt->Sumw2();      

    fOutputMassPt->Add(tmpMtPt);
  }

  if(fFillImpParHist) CreateImpactParameterHistos();
  
  // 2D Y distributions
  
  if(fFillYHist) {
    for(Int_t i=0;i<fCuts->GetNPtBins();i++){
      nameMassY="histMassY_";
      nameMassY+=i;
      nameSgnY="histSgnY_";
      nameSgnY+=i;
      nameBkgY="histBkgY_";
      nameBkgY+=i;
      nameRflY="histRflY_";
      nameRflY+=i;
      //MC signal
      if(fReadMC){
	TH2F* tmpStY = new TH2F(nameSgnY.Data(), "D^{0} invariant mass - MC; M [GeV]; Entries; y",600,1.5648,2.16484,nbins2dY,binInY,binFinY);
	tmpStY->Sumw2();
	//Reflection: histo filled with D0MassV1 which pass the cut (also) as D0bar and with D0bar which pass (also) the cut as D0
	TH2F* tmpRtY = new TH2F(nameRflY.Data(), "Reflected signal invariant mass - MC; M [GeV]; Entries; y",600,1.5648,2.1648,nbins2dY,binInY,binFinY);
	TH2F* tmpBtY = new TH2F(nameBkgY.Data(), "Background invariant mass - MC; M [GeV]; Entries; y",600,1.5648,2.1648,nbins2dY,binInY,binFinY);
	tmpBtY->Sumw2();
	tmpRtY->Sumw2();
      
	fOutputMassY->Add(tmpStY);
	fOutputMassY->Add(tmpRtY);
	fOutputMassY->Add(tmpBtY);
      }
      TH2F* tmpMtY = new TH2F(nameMassY.Data(),"D^{0} invariant mass; M [GeV]; Entries; y",600,1.5648,2.1648,nbins2dY,binInY,binFinY);
      tmpMtY->Sumw2();      
      fOutputMassY->Add(tmpMtY);
    }
  }


  const char* nameoutput=GetOutputSlot(3)->GetContainer()->GetName();

  fNentries=new TH1F(nameoutput, "Integral(1,2) = number of AODs *** Integral(2,3) = number of candidates selected with cuts *** Integral(3,4) = number of D0 selected with cuts *** Integral(4,5) = events with good vertex ***  Integral(5,6) = pt out of bounds", 19,-0.5,18.5);

  fNentries->GetXaxis()->SetBinLabel(1,"nEventsAnal");
  fNentries->GetXaxis()->SetBinLabel(2,"nCandSel(Cuts)");
  if(fReadMC) fNentries->GetXaxis()->SetBinLabel(3,"nD0Selected");
  else fNentries->GetXaxis()->SetBinLabel(3,"Dstar<-D0");
  fNentries->GetXaxis()->SetBinLabel(4,"nEventsGoodVtxS");
  fNentries->GetXaxis()->SetBinLabel(5,"ptbin = -1");
  fNentries->GetXaxis()->SetBinLabel(6,"no daughter");
  if(fSys==0) fNentries->GetXaxis()->SetBinLabel(7,"nCandSel(Tr)");
  if(fFillVarHists || fPIDCheck){
    fNentries->GetXaxis()->SetBinLabel(8,"PID=0");
    fNentries->GetXaxis()->SetBinLabel(9,"PID=1");
    fNentries->GetXaxis()->SetBinLabel(10,"PID=2");
    fNentries->GetXaxis()->SetBinLabel(11,"PID=3");
  }
  if(fReadMC && fSys==0){
    fNentries->GetXaxis()->SetBinLabel(12,"K");
    fNentries->GetXaxis()->SetBinLabel(13,"Lambda");
  }
  fNentries->GetXaxis()->SetBinLabel(14,"Pile-up Rej");
  fNentries->GetXaxis()->SetBinLabel(15,"N. of 0SMH");
  if(fSys==1) fNentries->GetXaxis()->SetBinLabel(16,"Nev in centr");
  if(fIsRejectSDDClusters) fNentries->GetXaxis()->SetBinLabel(17,"SDD-Cls Rej");
  fNentries->GetXaxis()->SetBinLabel(18,"Phys.Sel.Rej");
  fNentries->GetXaxis()->SetBinLabel(19,"D0 failed to be filled");
  fNentries->GetXaxis()->SetNdivisions(1,kFALSE);

  fCounter = new AliNormalizationCounter(Form("%s",GetOutputSlot(5)->GetContainer()->GetName()));
  fCounter->Init();

  //
  // Output slot 7 : tree of the candidate variables after track selection
  //
  nameoutput = GetOutputSlot(7)->GetContainer()->GetName();
  fVariablesTree = new TTree(nameoutput,"Candidates variables tree");
  Int_t nVar = 15;
  fCandidateVariables = new Double_t [nVar];
  TString * fCandidateVariableNames = new TString[nVar];
  fCandidateVariableNames[0] = "massD0";
  fCandidateVariableNames[1] = "massD0bar";
  fCandidateVariableNames[2] = "pt";
  //fCandidateVariableNames[3] = "dca";
  //fCandidateVariableNames[4] = "costhsD0";
  //fCandidateVariableNames[5] = "costhsD0bar";
  //fCandidateVariableNames[6] = "ptk";
  //fCandidateVariableNames[7] = "ptpi";
  //fCandidateVariableNames[8] = "d0k";
  //fCandidateVariableNames[9] = "d0pi";
  //fCandidateVariableNames[10] = "d0xd0";
  //fCandidateVariableNames[11] = "costhp";
  //fCandidateVariableNames[12] = "costhpxy";
  //fCandidateVariableNames[13] = "lxy";
  //fCandidateVariableNames[14] = "specialcuts"; 
    fCandidateVariableNames[3] = "ctau";
    fCandidateVariableNames[4] = "PseudoProperDecayLength";
    fCandidateVariableNames[5] = "IsSelected";
    
  for(Int_t ivar=0; ivar<nVar; ivar++){
    fVariablesTree->Branch(fCandidateVariableNames[ivar].Data(),&fCandidateVariables[ivar],Form("%s/d",fCandidateVariableNames[ivar].Data()));
  }

  //
  // Output slot 8 : List for detector response histograms
  //
  if (fDrawDetSignal) {
    TH2F *TOFSigBefPID = new TH2F("TOFSigBefPID", "TOF signal of daughters before PID;p(daught)(GeV/c);Signal", 500, 0, 10, 5000, 0, 50e3);
    TH2F *TOFSigAftPID = new TH2F("TOFSigAftPID", "TOF signal after PID;p(daught)(GeV/c);Signal", 500, 0, 10, 5000, 0, 50e3);

    TH2F *TPCSigBefPID = new TH2F("TPCSigBefPID", "TPC dE/dx before PID;p(daught)(GeV/c);dE/dx (arb. units)", 1000, 0, 10, 1000, 0, 500);
    TH2F *TPCSigAftPID = new TH2F("TPCSigAftPID", "TPC dE/dx after PID;p(daught)(GeV/c);dE/dx (arb. units)", 1000, 0, 10, 1000, 0, 500);

    fDetSignal->Add(TOFSigBefPID);
    fDetSignal->Add(TOFSigAftPID);
    fDetSignal->Add(TPCSigBefPID);
    fDetSignal->Add(TPCSigAftPID);
  }
    
    
    
    fhMultVZEROTPCoutTrackCorrNoCut = new TH2F("hMultVZEROTPCoutTrackCorrNoCut", ";Tracks with kTPCout on;VZERO multiplicity", 1000, 0., 30000., 1000, 0., 40000.);
    fhMultVZEROTPCoutTrackCorr = new TH2F("hMultVZEROTPCoutTrackCorr", ";Tracks with kTPCout on;VZERO multiplicity", 1000, 0., 30000., 1000, 0., 40000.);
    fDistr->Add(fhMultVZEROTPCoutTrackCorrNoCut);
    fDistr->Add(fhMultVZEROTPCoutTrackCorr);

  // Post the data
  PostData(1,fOutputMass);
  PostData(2,fDistr);
  PostData(3,fNentries);
  PostData(5,fCounter);  
  PostData(6,fOutputMassPt);
  PostData(7,fVariablesTree);
  PostData(8,fDetSignal);
  PostData(9,fOutputMassY);

  return;
}

//________________________________________________________________________



void AliAnalysisTaskSED0MassNonPromptFraction::UserExec(Option_t */*option*/)
{
  /// Execute analysis for current event:
  /// heavy flavor candidates association to MC truth

  //cuts order
  //     printf("    |M-MD0| [GeV]    < %f\n",fD0toKpiCuts[0]);
  //     printf("    dca    [cm]  < %f\n",fD0toKpiCuts[1]);
  //     printf("    cosThetaStar     < %f\n",fD0toKpiCuts[2]);
  //     printf("    pTK     [GeV/c]    > %f\n",fD0toKpiCuts[3]);
  //     printf("    pTpi    [GeV/c]    > %f\n",fD0toKpiCuts[4]);
  //     printf("    |d0K|  [cm]  < %f\n",fD0toKpiCuts[5]);
  //     printf("    |d0pi| [cm]  < %f\n",fD0toKpiCuts[6]);
  //     printf("    d0d0  [cm^2] < %f\n",fD0toKpiCuts[7]);
  //     printf("    cosThetaPoint    > %f\n",fD0toKpiCuts[8]);
  
  AliAODEvent *aod = dynamic_cast<AliAODEvent*> (InputEvent());
    
    if(fAODProtection>=0){
        //   Protection against different number of events in the AOD and deltaAOD
        //   In case of discrepancy the event is rejected.
        Int_t matchingAODdeltaAODlevel = AliRDHFCuts::CheckMatchingAODdeltaAODevents();
        if (matchingAODdeltaAODlevel<0 || (matchingAODdeltaAODlevel==0 && fAODProtection==1)) {
            // AOD/deltaAOD trees have different number of entries || TProcessID do not match while it was required
            fNentries->Fill(21);
            return;
        }
        fNentries->Fill(22);
    }
    
  TString bname;
  if(fArray==0){ //D0 candidates
    // load D0->Kpi candidates
    //cout<<"D0 candidates"<<endl;
    bname="D0toKpi";
  } else { //LikeSign candidates
    // load like sign candidates
    //cout<<"LS candidates"<<endl;
    bname="LikeSign2Prong";
  }
  TClonesArray *inputArray=0;
  if(!aod && AODEvent() && IsStandardAOD()) {
    // In case there is an AOD handler writing a standard AOD, use the AOD 
    // event in memory rather than the input (ESD) event.    
    aod = dynamic_cast<AliAODEvent*> (AODEvent());
    // in this case the braches in the deltaAOD (AliAOD.VertexingHF.root)
    // have to taken from the AOD event hold by the AliAODExtension
    AliAODHandler* aodHandler = (AliAODHandler*) 
      ((AliAnalysisManager::GetAnalysisManager())->GetOutputEventHandler());
      
      
    

    if(aodHandler->GetExtensions()) {
      AliAODExtension *ext = (AliAODExtension*)aodHandler->GetExtensions()->FindObject("AliAOD.VertexingHF.root");
      AliAODEvent* aodFromExt = ext->GetAOD();
      inputArray=(TClonesArray*)aodFromExt->GetList()->FindObject(bname.Data());
    }
  } else if(aod) {
    inputArray=(TClonesArray*)aod->GetList()->FindObject(bname.Data());
  }

  if(!inputArray || !aod) {
    printf("AliAnalysisTaskSED0MassNonPromptFraction::UserExec: input branch not found!\n");
    return;
  }
  // fix for temporary bug in ESDfilter
  // the AODs with null vertex pointer didn't pass the PhysSel
  if(!aod->GetPrimaryVertex() || TMath::Abs(aod->GetMagneticField())<0.001) return;


  TClonesArray *mcArray = 0;
  AliAODMCHeader *mcHeader = 0;

  if(fReadMC) {
    // load MC particles
    mcArray = (TClonesArray*)aod->GetList()->FindObject(AliAODMCParticle::StdBranchName());
    if(!mcArray) {
      printf("AliAnalysisTaskSED0MassNonPromptFraction::UserExec: MC particles branch not found!\n");
      return;
    }
    
    // load MC header
    mcHeader = (AliAODMCHeader*)aod->GetList()->FindObject(AliAODMCHeader::StdBranchName());
    if(!mcHeader) {
      printf("AliAnalysisTaskSED0MassNonPromptFraction::UserExec: MC header branch not found!\n");
      return;
    }
  }

    Int_t nTPCout=0;
    Float_t mTotV0=0;
    AliAODVZERO* v0data=(AliAODVZERO*)((AliAODEvent*)aod)->GetVZEROData();
    Float_t mTotV0A=v0data->GetMTotV0A();
    Float_t mTotV0C=v0data->GetMTotV0C();
    mTotV0=mTotV0A+mTotV0C;
    Int_t ntracksEv = aod->GetNumberOfTracks();
    for(Int_t itrack=0; itrack<ntracksEv; itrack++) { // loop on tacks
        //    ... get the track
        AliAODTrack * track = dynamic_cast<AliAODTrack*>(aod->GetTrack(itrack));
        if(!track) {AliFatal("Not a standard AOD");}
        if(track->GetID()<0)continue;
        if((track->GetFlags())&(AliESDtrack::kTPCout)) nTPCout++;
        else continue;
    }
    if(fhMultVZEROTPCoutTrackCorrNoCut) fhMultVZEROTPCoutTrackCorrNoCut->Fill(nTPCout,mTotV0);
    Float_t mV0Cut=-2200.+(2.5*nTPCout)+(0.000012*nTPCout*nTPCout);
    if(fEnablePileupRejVZEROTPCout){
        if(mTotV0<mV0Cut) return;	
    }
    
    
    if(fhMultVZEROTPCoutTrackCorr)fhMultVZEROTPCoutTrackCorr->Fill(nTPCout,mTotV0);
  
  //printf("VERTEX Z %f %f\n",vtx1->GetZ(),mcHeader->GetVtxZ());
  
  //histogram filled with 1 for every AOD
  fNentries->Fill(0);
  fCounter->StoreEvent(aod,fCuts,fReadMC); 
  //fCounter->StoreEvent(aod,fReadMC); 

  // trigger class for PbPb C0SMH-B-NOPF-ALLNOTRD, C0SMH-B-NOPF-ALL
  TString trigclass=aod->GetFiredTriggerClasses();
  if(trigclass.Contains("C0SMH-B-NOPF-ALLNOTRD") || trigclass.Contains("C0SMH-B-NOPF-ALL")) fNentries->Fill(14);

  if(!fCuts->IsEventSelected(aod)) {
    if(fCuts->GetWhyRejection()==1) // rejected for pileup
      fNentries->Fill(13);
    if(fSys==1 && (fCuts->GetWhyRejection()==2 || fCuts->GetWhyRejection()==3)) fNentries->Fill(15);
    if(fCuts->GetWhyRejection()==7) fNentries->Fill(17);
    return;
  }

  // Check the Nb of SDD clusters
  if (fIsRejectSDDClusters) { 
    Bool_t skipEvent = kFALSE;
    Int_t ntracks = 0;
    if (aod) ntracks = aod->GetNumberOfTracks();
    for(Int_t itrack=0; itrack<ntracks; itrack++) { // loop on tacks
      //    ... get the track
      AliAODTrack * track = dynamic_cast<AliAODTrack*>(aod->GetTrack(itrack));
      if(!track) AliFatal("Not a standard AOD");
      if(TESTBIT(track->GetITSClusterMap(),2) || TESTBIT(track->GetITSClusterMap(),3) ){
	skipEvent=kTRUE;
	fNentries->Fill(16);
	break;
      }
    }
    if (skipEvent) return;
  }
  
  // AOD primary vertex
  AliAODVertex *vtx1 = (AliAODVertex*)aod->GetPrimaryVertex();

  Bool_t isGoodVtx=kFALSE;

  //vtx1->Print();
  TString primTitle = vtx1->GetTitle();
  if(primTitle.Contains("VertexerTracks") && vtx1->GetNContributors()>0) {
    isGoodVtx=kTRUE;
    fNentries->Fill(3);
  }

  // loop over candidates
  Int_t nInD0toKpi = inputArray->GetEntriesFast();
  if(fDebug>2) printf("Number of D0->Kpi: %d\n",nInD0toKpi);

  // FILE *f=fopen("4display.txt","a");
  // printf("Number of D0->Kpi: %d\n",nInD0toKpi);
  Int_t nSelectedloose=0,nSelectedtight=0;  

  // vHF object is needed to call the method that refills the missing info of the candidates
  // if they have been deleted in dAOD reconstruction phase
  // in order to reduce the size of the file
  AliAnalysisVertexingHF *vHF = new AliAnalysisVertexingHF();

  for (Int_t iD0toKpi = 0; iD0toKpi < nInD0toKpi; iD0toKpi++) {
    AliAODRecoDecayHF2Prong *d = (AliAODRecoDecayHF2Prong*)inputArray->UncheckedAt(iD0toKpi);
 
    if(fUseSelectionBit && d->GetSelectionMap()) if(!d->HasSelectionBit(AliRDHFCuts::kD0toKpiCuts)){
	fNentries->Fill(2);
	continue; //skip the D0 from Dstar
      }

    if(!(vHF->FillRecoCand(aod,d))) {//Fill the data members of the candidate only if they are empty.   
      fNentries->Fill(18); //monitor how often this fails 
      continue;
    }
    
    if ( fCuts->IsInFiducialAcceptance(d->Pt(),d->Y(421)) ) {
      nSelectedloose++;
      nSelectedtight++;      
      if(fSys==0){
	if(fCuts->IsSelected(d,AliRDHFCuts::kTracks,aod))fNentries->Fill(6);       
      }
      Int_t ptbin=fCuts->PtBin(d->Pt());
      if(ptbin==-1) {fNentries->Fill(4); continue;} //out of bounds
      fIsSelectedCandidate=fCuts->IsSelected(d,AliRDHFCuts::kAll,aod); //selected
      if(fFillVarHists) {
	//if(!fCutOnDistr || (fCutOnDistr && fIsSelectedCandidate)) {
	fDaughterTracks.AddAt((AliAODTrack*)d->GetDaughter(0),0);
	fDaughterTracks.AddAt((AliAODTrack*)d->GetDaughter(1),1);
	//check daughters
	if(!fDaughterTracks.UncheckedAt(0) || !fDaughterTracks.UncheckedAt(1)) {
	  AliDebug(1,"at least one daughter not found!");
	  fNentries->Fill(5);
	  fDaughterTracks.Clear();
	  continue;
	}
         
         // FillVarHists(aod,d,mcArray,fCuts,fDistr);
        
          
          if(!fFillAfterCuts) FillVarHists(aod,d,mcArray,fCuts,fDistr);
          if(fFillAfterCuts && fIsSelectedCandidate>=1)  FillVarHists(aod,d,mcArray,fCuts,fDistr);
              
          
          
                   }
        
      if (fDrawDetSignal) {
	DrawDetSignal(d, fDetSignal);
      }
    
        //filling invariant mass histograms
    FillMassHists(d,mcArray,mcHeader,fCuts,fOutputMass);
        
        
      if (fPIDCheck) {
	Int_t isSelectedPIDfill = 3;
	if (!fReadMC || (fReadMC && fUsePid4Distr)) isSelectedPIDfill = fCuts->IsSelectedPID(d); //0 rejected,1 D0,2 D0bar, 3 both

	if (isSelectedPIDfill == 0)fNentries->Fill(7);
	if (isSelectedPIDfill == 1)fNentries->Fill(8);
	if (isSelectedPIDfill == 2)fNentries->Fill(9);
	if (isSelectedPIDfill == 3)fNentries->Fill(10);
      }

    }
  
    fDaughterTracks.Clear();
    //if(unsetvtx) d->UnsetOwnPrimaryVtx();
  } //end for prongs
    
      	FillMCGenHists(aod, mcArray,fCuts,fDistr);
    
  fCounter->StoreCandidates(aod,nSelectedloose,kTRUE);  
  fCounter->StoreCandidates(aod,nSelectedtight,kFALSE);  
  delete vHF;
  // Post the data
  PostData(1,fOutputMass);
  PostData(2,fDistr);
  PostData(3,fNentries);
  PostData(5,fCounter);
  PostData(6,fOutputMassPt);
  PostData(7,fVariablesTree);
  PostData(8, fDetSignal);

  return;
}

//____________________________________________________________________________
void AliAnalysisTaskSED0MassNonPromptFraction::FillMCGenHists(AliAODEvent* aod, TClonesArray *arrMC, AliRDHFCutsD0toKpi *cuts, TList *listout){

    
      //  cout << "numero di entries MC      " << arrMC->GetEntries() << endl;
    
    
    
    for(Int_t i=0; i< arrMC->GetEntries(); i++){
        
        AliAODMCParticle *partD0 = (AliAODMCParticle*)arrMC->At(i);
        
        Int_t ptbin=cuts->PtBin(partD0->Pt());
        Double_t pt = partD0->Pt();
        
        TString fillthisGEN ="";
        Double_t mPDG=TDatabasePDG::Instance()->GetParticle(421)->Mass();
       
        if (TMath::Abs(((AliAODMCParticle*)partD0)->GetPdgCode() != 421)) continue;
        
        AliAODMCParticle *partDaughter = (AliAODMCParticle*)arrMC->At(partD0->GetDaughter(0));
        //if(!partDaughter) continue;
        
        if (!arrMC->At(partD0->GetDaughter(0))) continue;
        
        Double_t XvtxGen=0;
        Double_t YvtxGen=0;
        Double_t ZvtxGen=0;
        
        
        Double_t XpGen=0;
        Double_t YpGen=0;
        Double_t ZpGen=0;
        
        XvtxGen = partDaughter->Xv();
        YvtxGen = partDaughter->Yv();
        ZvtxGen = partDaughter->Zv();
        
     
        
        Double_t PvecGen[2]={-99,-99};
        
        PvecGen[0] = partD0->Px();
        PvecGen[1] = partD0->Py();
        
        
        Double_t ptravGen = partD0->Pt();
        
        AliAODMCHeader* mcHeader = (AliAODMCHeader*)aod->GetList()->FindObject(AliAODMCHeader::StdBranchName());
        if(!mcHeader) {
            printf("AliAnalysisTaskSED0MassNonPromptFraction::UserExec: MC header branch not found!\n");
            return;
        }
        
        if (mcHeader){
            XpGen = mcHeader->GetVtxX();
            YpGen = mcHeader->GetVtxY();
            ZpGen = mcHeader->GetVtxZ();
        }
        
    

        
        TVector3 LGen ;
        LGen.SetXYZ (XvtxGen-XpGen, YvtxGen-YpGen, ZvtxGen-ZpGen);
        
        
        TVector3 PtvecGen;
        
        PtvecGen.SetXYZ (partD0->Px(), partD0->Py(), 0.);
        
        
        
        Double_t LxyGen;
        LxyGen = (LGen.Dot(PtvecGen))/ptravGen;
        
        
        
        Double_t xgenerate;
        xgenerate= LxyGen*mPDG/ptravGen;

        
        
        fillthisGEN= "hpseudoProperGENERATE_";
        fillthisGEN+=ptbin;
        ((TH1F*)listout->FindObject(fillthisGEN))->Fill(xgenerate);

       
        //old function if(CheckOrigin(arrMC,partD0)==5){
        if(AliVertexingHFUtils::CheckOrigin(arrMC,partD0,fUseQuarkTagInKine)==5){ //isPrimary=kFALSE;
            //isPrimary=kFALSE;
            
            
            fillthisGEN= "hpseudoProperSecGenerate_";
            fillthisGEN+=ptbin;
            ((TH1F*)listout->FindObject(fillthisGEN))->Fill(xgenerate);

            
            
        }
        else{
            
            fillthisGEN= "hpseudoProperPrimGenerate_";
            fillthisGEN+=ptbin;
            ((TH1F*)listout->FindObject(fillthisGEN))->Fill(xgenerate);
        }
    
    

        
    }//end ciclo for

    
}//end void


void AliAnalysisTaskSED0MassNonPromptFraction::DrawDetSignal(AliAODRecoDecayHF2Prong *part, TList *ListDetSignal)
{
  //
  /// Function called in UserExec for drawing detector signal histograms:
  //
  fDaughterTracks.AddAt((AliAODTrack*)part->GetDaughter(0), 0);
  fDaughterTracks.AddAt((AliAODTrack*)part->GetDaughter(1), 1);

  AliESDtrack *esdtrack1 = new AliESDtrack((AliVTrack*)fDaughterTracks.UncheckedAt(0));
  AliESDtrack *esdtrack2 = new AliESDtrack((AliVTrack*)fDaughterTracks.UncheckedAt(1));


  //For filling detector histograms
  Int_t isSelectedPIDfill = 3;
  if (!fReadMC || (fReadMC && fUsePid4Distr)) isSelectedPIDfill = fCuts->IsSelectedPID(part); //0 rejected,1 D0,2 Dobar, 3 both


  //fill "before PID" histos with every daughter
  ((TH2F*)ListDetSignal->FindObject("TOFSigBefPID"))->Fill(esdtrack1->P(), esdtrack1->GetTOFsignal());
  ((TH2F*)ListDetSignal->FindObject("TOFSigBefPID"))->Fill(esdtrack2->P(), esdtrack2->GetTOFsignal());
  ((TH2F*)ListDetSignal->FindObject("TPCSigBefPID"))->Fill(esdtrack1->P(), esdtrack1->GetTPCsignal());
  ((TH2F*)ListDetSignal->FindObject("TPCSigBefPID"))->Fill(esdtrack2->P(), esdtrack2->GetTPCsignal());

  if (isSelectedPIDfill != 0)  { //fill "After PID" with everything that's not rejected
    ((TH2F*)ListDetSignal->FindObject("TOFSigAftPID"))->Fill(esdtrack1->P(), esdtrack1->GetTOFsignal());
    ((TH2F*)ListDetSignal->FindObject("TOFSigAftPID"))->Fill(esdtrack2->P(), esdtrack2->GetTOFsignal());
    ((TH2F*)ListDetSignal->FindObject("TPCSigAftPID"))->Fill(esdtrack1->P(), esdtrack1->GetTPCsignal());
    ((TH2F*)ListDetSignal->FindObject("TPCSigAftPID"))->Fill(esdtrack2->P(), esdtrack2->GetTPCsignal());

  }

  delete esdtrack1;
  delete esdtrack2;

  return;
}

//____________________________________________________________________________
void AliAnalysisTaskSED0MassNonPromptFraction::FillVarHists(AliAODEvent* aod,AliAODRecoDecayHF2Prong *part, TClonesArray *arrMC, AliRDHFCutsD0toKpi *cuts, TList *listout){
  //
  /// function used in UserExec to fill variable histograms:
  //


  Int_t pdgDgD0toKpi[2]={321,211};
  Int_t lab=-9999;
  if(fReadMC) lab=part->MatchToMC(421,arrMC,2,pdgDgD0toKpi); //return MC particle label if the array corresponds to a D0, -1 if not (cf. AliAODRecoDecay.cxx)
  //Double_t pt = d->Pt(); //mother pt
  Int_t isSelectedPID=3;
  if(!fReadMC || (fReadMC && fUsePid4Distr)) isSelectedPID=cuts->IsSelectedPID(part); //0 rejected,1 D0,2 Dobar, 3 both
  if (!fPIDCheck) {  //if fPIDCheck, this is already filled elsewhere
    if (isSelectedPID==0)fNentries->Fill(7);
    if (isSelectedPID==1)fNentries->Fill(8);
    if (isSelectedPID==2)fNentries->Fill(9);
    if (isSelectedPID==3)fNentries->Fill(10);
    //fNentries->Fill(8+isSelectedPID);
  }

  if(fCutOnDistr && !fIsSelectedCandidate) return; 
  //printf("\nif no cuts or cuts passed\n");


  //add distr here
  UInt_t pdgs[2];
    
    Double_t mPDG=TDatabasePDG::Instance()->GetParticle(421)->Mass();
  
    
    Double_t Xvtx=0;
    Double_t Yvtx=0;
    Double_t Zvtx=0;
   
    
    Xvtx = part->GetSecondaryVtx()->GetX();
    
    Yvtx = part->GetSecondaryVtx()->GetY();
    Zvtx = part->GetSecondaryVtx()->GetZ();
    
    AliAODVertex *vtx1 = (AliAODVertex*)aod->GetPrimaryVertex();

    Double_t Xp =0;
    Double_t Yp =0;
    Double_t Zp =0;
    
    Xp=aod->GetPrimaryVertex()->GetX();
    Yp=aod->GetPrimaryVertex()->GetY();
    Zp=aod->GetPrimaryVertex()->GetZ();
   
    
    
    Double_t CosinePointingAngle = part->CosPointingAngle();
    
    Double_t dSecToPrim=-99;
    dSecToPrim = TMath::Sqrt((Xvtx-Xp)*(Xvtx-Xp) +(Yvtx-Yp)*(Yvtx-Yp)+(Zvtx-Zp)*(Zvtx-Zp));
    
   
    
    Double_t Lvec[3]={-99,-99,-99};
    Double_t Pvec[2]={-99,-99};
    
    Pvec[0] = part->Px();
    Pvec[1] = part->Py();
    
    Double_t Pt = TMath::Sqrt(Pvec[0]*Pvec[0]+Pvec[1]*Pvec[1]);
    
    
    Double_t deltaX = Xp-Xvtx;
    
    Lvec[0]= Xp-Xvtx;
    Lvec[1]= Yp-Yvtx;
    Lvec[2]= Zp-Zvtx;
    
    TVector3 posMom;
    
    //posMom.SetXYZ(pTrack->Px(),pTrack->Py(),pTrack->Pz());
    
    TVector3 L ;
    L.SetXYZ (Xvtx-Xp, Yvtx-Yp, Zvtx-Zp);
    
    TVector3 L1 ;
    L1.SetXYZ (Xp-Xvtx, Yp-Yvtx, 0);
    
    TVector3 Ptvec;
    
    Ptvec.SetXYZ (part->Px(), part->Py(), 0);
 
    
 
    Double_t ptrav = part->Pt();
   
    
    Double_t Lxy;
    Lxy = (L.Dot(Ptvec))/ptrav;
    
    Double_t Lxy1;
    Lxy1 = (L1.Dot(Ptvec))/ptrav;
    
    Double_t x;
    x= Lxy*mPDG/ptrav;
    


    
    
    
    // Fill candidate variable Tree (track selection, no candidate selection)
    if( fWriteVariableTree && !part->HasBadDaughters() && fCuts->AreDaughtersSelected(part) && fCuts->IsSelectedPID(part) ){
        
        
        
        fCandidateVariables[0] = part->InvMassD0(); //ok
        fCandidateVariables[1] = part->InvMassD0bar(); //ok
        fCandidateVariables[2] = part->Pt(); //ok
        
        // fCandidateVariables[3] = part->Pt2Prong(0);
        // fCandidateVariables[4] = part->Pt2Prong(1);
        // fCandidateVariables[5] = part->Getd0Prong(0);
        // fCandidateVariables[6] = part->Getd0Prong(1);
        // fCandidateVariables[7] = part->Prodd0d0();
        // fCandidateVariables[8] = part->DecayLengthXY();
        
        fCandidateVariables[3] = part->CtD0();
        fCandidateVariables[4] = x;
        fCandidateVariables[5] = fIsSelectedCandidate;
        // fCandidateVariables[10]= part->ImpParXY()*10000.;
        // fCandidateVariables[14] = fCuts->IsSelectedSpecialCuts(part);
        
      
        
        fVariablesTree->Fill();
    }
    
   
    
    if (!fIsSelectedCandidate){
        //cout<<" cut " << cuts << " Rejected because "<<cuts->GetWhy()<<endl;
        return;
    }
    
    if(fDebug>2)  cout << "Candidate selected" << endl;

    
    Double_t CtauD0 = -1;

    Double_t C= 2.99792458e10;
    
    //CtauD0 = (((part->DecayLength())*mPDG*C)/(TMath::Sqrt(part->P()))); //confronta con il valore CtD0
    CtauD0 = part->CtD0();
    
  pdgs[0]=211;
  pdgs[1]=321;
  Double_t minvD0 = part->InvMassD0();
  pdgs[1]=211;
  pdgs[0]=321;
  Double_t minvD0bar = part->InvMassD0bar();
  //cout<<"inside mass cut"<<endl;
  Double_t  decayLengthxy = part->DecayLengthXY();
  Double_t normalizedDecayLengthxy=decayLengthxy/part->DecayLengthXYError();


  Double_t invmasscut=0.03;

  TString fillthispi="",fillthisK="",fillthis="", fillthispt="", fillthisetaphi="";
    
    

  Int_t ptbin=cuts->PtBin(part->Pt());
  Double_t pt = part->Pt();

  Double_t dz1[2],dz2[2],covar1[3],covar2[3];//,d0xd0proper,errd0xd0proper;
  dz1[0]=-99; dz2[0]=-99;
  Double_t d0[2];
  Double_t decl[2]={-99,-99};
  Bool_t recalcvtx=kFALSE;

    fillthis= "hCorrCosX_";
    fillthis+=ptbin;
    ((TH2F*)listout->FindObject(fillthis))->Fill(CosinePointingAngle, x);

    
    
    fillthis= "hCorrDistX_";
    fillthis+=ptbin;
    ((TH2F*)listout->FindObject(fillthis))->Fill(dSecToPrim, x);
    
    Double_t impparXY=part->ImpParXY()*10000.;
    
    fillthis= "hCorrXd0D0_";
    fillthis+=ptbin;
    ((TH2F*)listout->FindObject(fillthis))->Fill(x,impparXY);
    
    
    fillthis="hctauD0_";
    fillthis+=ptbin;
    ((TH1F*)listout->FindObject(fillthis))->Fill(CtauD0);
    
  if(fCuts->GetIsPrimaryWithoutDaughters()){
    recalcvtx=kTRUE;
    //recalculate vertex
    AliAODVertex *vtxProp=0x0;
    vtxProp=GetPrimaryVtxSkipped(aod);
    if(vtxProp) {
      part->SetOwnPrimaryVtx(vtxProp);
      //Bool_t unsetvtx=kTRUE;
      //Calculate d0 for daughter tracks with Vtx Skipped
      AliESDtrack *esdtrack1=new AliESDtrack((AliVTrack*)fDaughterTracks.UncheckedAt(0));
      AliESDtrack *esdtrack2=new AliESDtrack((AliVTrack*)fDaughterTracks.UncheckedAt(1));
      esdtrack1->PropagateToDCA(vtxProp,aod->GetMagneticField(),1.,dz1,covar1);
      esdtrack2->PropagateToDCA(vtxProp,aod->GetMagneticField(),1.,dz2,covar2);
      delete vtxProp; vtxProp=NULL;
      delete esdtrack1;
      delete esdtrack2;
    }

    d0[0]=dz1[0];
    d0[1]=dz2[0];
      

    decl[0]=part->DecayLength2();
    decl[1]=part->NormalizedDecayLength2();
    part->UnsetOwnPrimaryVtx();
  
  }
    
  Double_t cosThetaStarD0 = 99;
  Double_t cosThetaStarD0bar = 99;
  Double_t cosPointingAngle = 99;
  Double_t ctau = -1;
 // Double_t CtauD0 = -1;
  Double_t CtauD0z = -1;
  Double_t normalizedDecayLength2 = -1;
  Double_t decayLength2 = -1;
  Double_t ptProng[2]={-99,-99};
  Double_t pzprong0 = -99;
  Double_t pzprong1 = -99;
  Double_t d0Prong[2]={-99,-99};
  Double_t etaD = 99.;
  Double_t phiD = 99.;
  Double_t decayLengthz = -1;
    
  //disable the PID
  if(!fUsePid4Distr) isSelectedPID=0;
    
    Int_t labD0=-1;
    if (fReadMC) labD0 = part->MatchToMC(421,arrMC,2,pdgDgD0toKpi);
    AliAODMCParticle *partD0 = (AliAODMCParticle*)arrMC->At(labD0);
    //Int_t pdgD0 = partD0->GetPdgCode();
    if(fReadMC){
        
        
        if(labD0>=0) {
            
            fillthis= "hpseudoProperDLRec_";
            fillthis+=ptbin;
            ((TH1F*)listout->FindObject(fillthis))->Fill(x);

            
            fillthis= "hCorrCosXtrue_";
            fillthis+=ptbin;
            ((TH2F*)listout->FindObject(fillthis))->Fill(CosinePointingAngle, x);
            
            fillthis= "hCorrDistXtrue_";
            fillthis+=ptbin;
            ((TH2F*)listout->FindObject(fillthis))->Fill(dSecToPrim, x);
            
            
            fillthis= "hCorrXd0D0true_";
            fillthis+=ptbin;
            ((TH2F*)listout->FindObject(fillthis))->Fill(x,impparXY);
            
            
            fillthis= "hImpParXYtrue_";
            fillthis+=ptbin;
            ((TH1F*)listout->FindObject(fillthis))->Fill(impparXY);
            
            
            fillthis= "hCorrNormLxyXtrue_";
            fillthis+=ptbin;
            ((TH2F*)listout->FindObject(fillthis))->Fill(normalizedDecayLengthxy, x);
            
            
            fillthis="hctauD0S_";
            fillthis+=ptbin;
            ((TH1F*)listout->FindObject(fillthis))->Fill(CtauD0);
            
            
            Double_t XvtxGen=0;
            Double_t YvtxGen=0;
            Double_t ZvtxGen=0;
            
            
            Double_t XpGen=0;
            Double_t YpGen=0;
            Double_t ZpGen=0;
            
            
            AliAODMCParticle *partDaughter = (AliAODMCParticle*)arrMC->At(partD0->GetDaughter(0));
           
            if (!arrMC->At(partD0->GetDaughter(0))) return;
           
            
            XvtxGen = partDaughter->Xv();
            YvtxGen = partDaughter->Yv();
            ZvtxGen = partDaughter->Zv();
            

            Double_t PvecGen[2]={-99,-99};
            
            PvecGen[0] = partD0->Px();
            PvecGen[1] = partD0->Py();
            
            
            Double_t ptravGen = partD0->Pt();

            AliAODMCHeader* mcHeader = (AliAODMCHeader*)aod->GetList()->FindObject(AliAODMCHeader::StdBranchName());
            if(!mcHeader) {
                printf("alysisTaskSED0MassNonPromptFraction::UserExec: MC header branch not found!\n");
                return;
            }
            
            if (mcHeader){
            XpGen = mcHeader->GetVtxX();
            YpGen = mcHeader->GetVtxY();
            ZpGen = mcHeader->GetVtxZ();}
            
        
            
            TVector3 LGen ;
            LGen.SetXYZ (XvtxGen-XpGen, YvtxGen-YpGen, ZvtxGen-ZpGen);
            
            
            TVector3 PtvecGen;
            
            PtvecGen.SetXYZ (partD0->Px(), partD0->Py(), 0);
            
            
            Double_t LxyGen;
            LxyGen = (LGen.Dot(PtvecGen))/ptravGen;

            
            Double_t xgen;
            xgen= LxyGen*mPDG/ptravGen;
         

            Double_t Residual;
            Residual= xgen -x;
            
            fillthis= "hGenMatchRec_";
            fillthis+=ptbin;
            ((TH1F*)listout->FindObject(fillthis))->Fill(xgen);
            
            
            fillthis= "hresidualX_";
            fillthis+=ptbin;
            ((TH1F*)listout->FindObject(fillthis))->Fill(Residual);

            
            
            if(AliVertexingHFUtils::CheckOrigin(arrMC,partD0,fUseQuarkTagInKine)==5){ //isPrimary=kFALSE;
            
            //old function if(CheckOrigin(arrMC,partD0)==5){
     //isPrimary=kFALSE; //la funzione checkorigin restituisce 5 se la D deriva da B, ovvero se è secondaria!

 
    
        fillthis= "hpseudoProperDLRecSec_";
        fillthis+=ptbin;
        ((TH1F*)listout->FindObject(fillthis))->Fill(x);
                
                
                fillthis= "hCorrDistX_sec_";
                fillthis+=ptbin;
                ((TH2F*)listout->FindObject(fillthis))->Fill(dSecToPrim, x);
            
                fillthis= "hCorrNormLxyX_sec_";
                fillthis+=ptbin;
                ((TH2F*)listout->FindObject(fillthis))->Fill(normalizedDecayLengthxy, x);
                
                fillthis= "hCorrCosX_sec_";
                fillthis+=ptbin;
                ((TH2F*)listout->FindObject(fillthis))->Fill(CosinePointingAngle, x);
                
                fillthis= "hCorrXd0D0_sec_";
                fillthis+=ptbin;
                ((TH2F*)listout->FindObject(fillthis))->Fill(x,impparXY);

                fillthis= "hImpParXY_sec_";
                fillthis+=ptbin;
                ((TH1F*)listout->FindObject(fillthis))->Fill(impparXY);
                
                
                
                fillthis= "hresidualXSec_";
                fillthis+=ptbin;
                ((TH1F*)listout->FindObject(fillthis))->Fill(Residual);
                
                fillthis= "hGenMatchRecSec_";
                fillthis+=ptbin;
                ((TH1F*)listout->FindObject(fillthis))->Fill(xgen);
                
                
             
                
                fillthis="hctauD0sec_";
                fillthis+=ptbin;
                ((TH1F*)listout->FindObject(fillthis))->Fill(CtauD0);
                
                
      
    
    
    }
            else{
            
                fillthis= "hpseudoProperDLRecPrim_";
                fillthis+=ptbin;
                ((TH1F*)listout->FindObject(fillthis))->Fill(x);
                
                
                fillthis= "hresidual_Primary_";
                fillthis+=ptbin;
                ((TH1F*)listout->FindObject(fillthis))->Fill(Residual);
                
                fillthis= "hGenMatchRecPrim_";
                fillthis+=ptbin;
                ((TH1F*)listout->FindObject(fillthis))->Fill(xgen);
                
                fillthis= "hCorrNormLxyX_prim_";
                fillthis+=ptbin;
                ((TH2F*)listout->FindObject(fillthis))->Fill(normalizedDecayLengthxy, x);

                fillthis= "hCorrDistX_prim_";
                fillthis+=ptbin;
                ((TH2F*)listout->FindObject(fillthis))->Fill(dSecToPrim, x);
                
                fillthis= "hCorrCosX_prim_";
                fillthis+=ptbin;
                ((TH2F*)listout->FindObject(fillthis))->Fill(CosinePointingAngle, x);
                
                fillthis= "hCorrXd0D0_prim_";
                fillthis+=ptbin;
                ((TH2F*)listout->FindObject(fillthis))->Fill(x,impparXY);
                
                fillthis= "hImpParXY_prim_";
                fillthis+=ptbin;
                ((TH1F*)listout->FindObject(fillthis))->Fill(impparXY);
                
                
                fillthis="hctauD0prim_";
                fillthis+=ptbin;
                ((TH1F*)listout->FindObject(fillthis))->Fill(CtauD0);

            
            }
    
        }
    
          }
    
    
    
  if((lab>=0 && fReadMC) || (!fReadMC && isSelectedPID)){ //signal (from MC or PID)
      
   
   
    //check pdg of the prongs
    AliAODTrack *prong0=(AliAODTrack*)fDaughterTracks.UncheckedAt(0);
    AliAODTrack *prong1=(AliAODTrack*)fDaughterTracks.UncheckedAt(1);

    if(!prong0 || !prong1) {
      return;
    }
    Int_t labprong[2];
    if(fReadMC){
      labprong[0]=prong0->GetLabel();
      labprong[1]=prong1->GetLabel();
    }
    AliAODMCParticle *mcprong=0;
    Int_t pdgProng[2]={0,0};

    for (Int_t iprong=0;iprong<2;iprong++){
      if(fReadMC && labprong[iprong]>=0) {
	mcprong= (AliAODMCParticle*)arrMC->At(labprong[iprong]);
	pdgProng[iprong]=mcprong->GetPdgCode();
      }
    }
    
      
      
      
    if(fSys==0){
      //no mass cut ditributions: ptbis
	
      fillthispi="hptpiSnoMcut_";
      fillthispi+=ptbin;

      fillthisK="hptKSnoMcut_";
      fillthisK+=ptbin;

      if ((TMath::Abs(pdgProng[0]) == 211 && TMath::Abs(pdgProng[1]) == 321)
	  || (isSelectedPID==1 || isSelectedPID==3)){
	((TH1F*)listout->FindObject(fillthispi))->Fill(prong0->Pt());
	((TH1F*)listout->FindObject(fillthisK))->Fill(prong1->Pt());
      }

      if ((TMath::Abs(pdgProng[0]) == 321 && TMath::Abs(pdgProng[1]) == 211)
	  || (isSelectedPID==2 || isSelectedPID==3)){
	((TH1F*)listout->FindObject(fillthisK))->Fill(prong0->Pt());
	((TH1F*)listout->FindObject(fillthispi))->Fill(prong1->Pt());
      }
    }

    //no mass cut ditributions: mass

    etaD = part->Eta();
    phiD = part->Phi();


    fillthis="hMassS_";
    fillthis+=ptbin;
    fillthispt="histSgnPt";
      
    if ((fReadMC && ((AliAODMCParticle*)arrMC->At(lab))->GetPdgCode() == 421)
	|| (!fReadMC && (isSelectedPID==1 || isSelectedPID==3))){//D0
      ((TH1F*)listout->FindObject(fillthis))->Fill(minvD0);
      if(fFillPtHist && fReadMC) ((TH2F*)fOutputMassPt->FindObject(fillthispt))->Fill(minvD0,pt);
      
      fillthisetaphi="hetaphiD0candidateS_";	
      fillthisetaphi+=ptbin;
      ((TH2F*)listout->FindObject(fillthisetaphi))->Fill(etaD, phiD);
    
      if(TMath::Abs(minvD0-mPDG)<0.05){
	fillthisetaphi="hetaphiD0candidatesignalregionS_";	
	fillthisetaphi+=ptbin;
	((TH2F*)listout->FindObject(fillthisetaphi))->Fill(etaD, phiD);
      }

    }
    else { //D0bar
      if(fReadMC || (!fReadMC && isSelectedPID > 1)){
	((TH1F*)listout->FindObject(fillthis))->Fill(minvD0bar);
	if(fFillPtHist && fReadMC) ((TH2F*)fOutputMassPt->FindObject(fillthispt))->Fill(minvD0bar,pt);

	fillthisetaphi="hetaphiD0barcandidateS_";	
	fillthisetaphi+=ptbin;
	((TH2F*)listout->FindObject(fillthisetaphi))->Fill(etaD, phiD);
	
	if(TMath::Abs(minvD0bar-mPDG)<0.05){
	  fillthisetaphi="hetaphiD0barcandidatesignalregionS_";	
	  fillthisetaphi+=ptbin;
	  ((TH2F*)listout->FindObject(fillthisetaphi))->Fill(etaD, phiD);
	}

      }
    }

    //apply cut on invariant mass on the pair
   
      //invmasscut
      
      if(TMath::Abs(minvD0-mPDG)<invmasscut || TMath::Abs(minvD0bar-mPDG)<invmasscut){

      cosThetaStarD0 = part->CosThetaStarD0();
      cosThetaStarD0bar = part->CosThetaStarD0bar();
      cosPointingAngle = part->CosPointingAngle();
      normalizedDecayLength2 = part->NormalizedDecayLength2();
        ctau=part->CtD0();
     
      decayLength2 = part->DecayLength2();
        

      decayLengthxy = part->DecayLengthXY();
        
      normalizedDecayLengthxy=decayLengthxy/part->DecayLengthXYError();

      ptProng[0]=prong0->Pt(); ptProng[1]=prong1->Pt();
      d0Prong[0]=part->Getd0Prong(0); d0Prong[1]=part->Getd0Prong(1);
        decayLengthz = TMath::Sqrt(decayLength2-decayLengthxy*decayLengthxy);
        pzprong0 = prong0->Pz();
        pzprong1 = prong1->Pz();
        
  
      

      if(fArray==1) cout<<"LS signal: ERROR"<<endl;
      for (Int_t iprong=0; iprong<2; iprong++){
	AliAODTrack *prong=(AliAODTrack*)fDaughterTracks.UncheckedAt(iprong);
	if (fReadMC) labprong[iprong]=prong->GetLabel();
	  
	//cout<<"prong name = "<<prong->GetName()<<" label = "<<prong->GetLabel()<<endl;
	Int_t pdgprong=0;
	if(fReadMC && labprong[iprong]>=0) {
	  mcprong= (AliAODMCParticle*)arrMC->At(labprong[iprong]);
	  pdgprong=mcprong->GetPdgCode();
	}

	Bool_t isPionHere[2]={(isSelectedPID==1 || isSelectedPID==3) ? kTRUE : kFALSE,(isSelectedPID==2 || isSelectedPID==3) ? kTRUE : kFALSE};

	if(TMath::Abs(pdgprong)==211 || isPionHere[iprong]) {
	  //cout<<"pi"<<endl;
	   
	  if(fSys==0){ 
	    fillthispi="hptpiS_";
	    fillthispi+=ptbin;
	    ((TH1F*)listout->FindObject(fillthispi))->Fill(ptProng[iprong]);
	  
        
     
      }

	  fillthispi="hd0piS_";
	  fillthispi+=ptbin;
	  ((TH1F*)listout->FindObject(fillthispi))->Fill(d0Prong[iprong]);
	  if(recalcvtx) {

	    fillthispi="hd0vpiS_";
	    fillthispi+=ptbin;
	    ((TH1F*)listout->FindObject(fillthispi))->Fill(d0[iprong]);
	  }

	}
	  
	if(TMath::Abs(pdgprong)==321 || !isPionHere[iprong]) {
	  //cout<<"kappa"<<endl;
	  if(fSys==0){
	    fillthisK="hptKS_";
	    fillthisK+=ptbin;
	    ((TH1F*)listout->FindObject(fillthisK))->Fill(ptProng[iprong]);
	 
         
      }
      
      fillthisK="hd0KS_";
	  fillthisK+=ptbin;
	  ((TH1F*)listout->FindObject(fillthisK))->Fill(d0Prong[iprong]);
	  if (recalcvtx){
	    fillthisK="hd0vKS_";
	    fillthisK+=ptbin;
	    ((TH1F*)listout->FindObject(fillthisK))->Fill(d0[iprong]);
	  }
	}

	if(fSys==0){
	  fillthis="hcosthpointd0S_";
	  fillthis+=ptbin;	  
	  ((TH1F*)listout->FindObject(fillthis))->Fill(cosPointingAngle,d0Prong[iprong]);
	}
      } //end loop on prongs

      fillthis="hdcaS_";
      fillthis+=ptbin;	  
      ((TH1F*)listout->FindObject(fillthis))->Fill(part->GetDCA());

      fillthis="hcosthetapointS_";
      fillthis+=ptbin;	  
      ((TH1F*)listout->FindObject(fillthis))->Fill(cosPointingAngle);

      fillthis="hcosthetapointxyS_";
      fillthis+=ptbin;	  
      ((TH1F*)listout->FindObject(fillthis))->Fill(part->CosPointingAngleXY());


      fillthis="hd0d0S_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(part->Prodd0d0());

      fillthis="hdeclS_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(decayLength2);

      fillthis="hnormdeclS_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(normalizedDecayLength2);
      
        //ctau DzeroZ
        fillthis="hctauS_";
        fillthis+=ptbin;
        ((TH1F*)listout->FindObject(fillthis))->Fill(CtauD0z);
        
        
        
  
        
        
        //correlation ctauXY ctauZ
        fillthis="hctauXYctauZS_"  ;
        fillthis+=ptbin;
        ((TH2F*)listout->FindObject(fillthis))->Fill(CtauD0,CtauD0z);
        
        

       



      fillthis="hdeclxyS_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(decayLengthxy);

      fillthis="hnormdeclxyS_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(normalizedDecayLengthxy);

      fillthis="hdeclxyd0d0S_";
      fillthis+=ptbin;
      ((TH2F*)listout->FindObject(fillthis))->Fill(decayLengthxy,d0Prong[0]*d0Prong[1]);
        
        

      fillthis="hnormdeclxyd0d0S_";
      fillthis+=ptbin;
      ((TH2F*)listout->FindObject(fillthis))->Fill(normalizedDecayLengthxy,d0Prong[0]*d0Prong[1]);

      if(recalcvtx) {
	fillthis="hdeclvS_";
	fillthis+=ptbin;
	((TH1F*)listout->FindObject(fillthis))->Fill(decl[0]);

	fillthis="hnormdeclvS_";
	fillthis+=ptbin;
	((TH1F*)listout->FindObject(fillthis))->Fill(decl[1]);

	fillthis="hd0d0vS_";
	fillthis+=ptbin;
	((TH1F*)listout->FindObject(fillthis))->Fill(d0[0]*d0[1]);
      }

      if(fSys==0){
	fillthis="hcosthetastarS_";
	fillthis+=ptbin;
	if ((fReadMC && ((AliAODMCParticle*)arrMC->At(lab))->GetPdgCode() == 421)) ((TH1F*)listout->FindObject(fillthis))->Fill(cosThetaStarD0);
	else {
	  if (fReadMC || isSelectedPID>1)((TH1F*)listout->FindObject(fillthis))->Fill(cosThetaStarD0bar);
	  if(isSelectedPID==1 || isSelectedPID==3)((TH1F*)listout->FindObject(fillthis))->Fill(cosThetaStarD0);
	}

	fillthis="hcosthpointd0d0S_";
	fillthis+=ptbin;	  
	((TH2F*)listout->FindObject(fillthis))->Fill(cosPointingAngle,part->Prodd0d0());
      }

      if ((fReadMC && ((AliAODMCParticle*)arrMC->At(lab))->GetPdgCode() == 421)){
	for(Int_t it=0; it<2; it++){
	  fillthis="hptD0S_";
	  fillthis+=ptbin;
	  ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt());
	  fillthis="hphiD0S_";
	  fillthis+=ptbin;
	  ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Phi());
	  Int_t nPointsITS = 0;
	  for (Int_t il=0; il<6; il++){ 
	    if(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(il)) nPointsITS++;
	  }
	  fillthis="hNITSpointsD0vsptS_";
	  fillthis+=ptbin;
	  ((TH2F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt(),nPointsITS);
	  fillthis="hNSPDpointsD0S_";
	  fillthis+=ptbin;
	  if(!(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0)) && !(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(1))){ //no SPD points
	    ((TH1I*)listout->FindObject(fillthis))->Fill(0);
	  } 
	  if(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0) && !(((AliAODTrack*)(fDaughterTracks.UncheckedAt(it)))->HasPointOnITSLayer(1))){ //kOnlyFirst
	    ((TH1I*)listout->FindObject(fillthis))->Fill(1);
	  } 
	  if(!(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0)) && ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(1)){ //kOnlySecond
	    ((TH1I*)listout->FindObject(fillthis))->Fill(2);
	  }
	  if(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0) && ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(1)){ //kboth
	    ((TH1I*)listout->FindObject(fillthis))->Fill(3);
	  } 
	  fillthis="hNclsD0vsptS_";
	  fillthis+=ptbin;
	  Float_t mom = ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt();
	  Float_t ncls = (Float_t)((AliAODTrack*)fDaughterTracks.UncheckedAt(0))->GetTPCNcls();
	  ((TH2F*)listout->FindObject(fillthis))->Fill(mom, ncls);
	}
      }
      else {
	if (fReadMC || isSelectedPID>1){
	  for(Int_t it=0; it<2; it++){
	    fillthis="hptD0barS_";
	    fillthis+=ptbin;
	    ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt());
	    fillthis="hphiD0barS_";
	    fillthis+=ptbin;
	    ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Phi());
	    fillthis="hNclsD0barvsptS_";
	    fillthis+=ptbin;
	    Float_t mom = ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt();
	    Float_t ncls = (Float_t)((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->GetTPCNcls();
	    ((TH2F*)listout->FindObject(fillthis))->Fill(mom, ncls);
	  }	  
	}
	if(isSelectedPID==1 || isSelectedPID==3){
	  for(Int_t it=0; it<2; it++){
	    fillthis="hptD0S_";
	    fillthis+=ptbin;
	    ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt());
	    fillthis="hphiD0S_";
	    fillthis+=ptbin;
	    ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Phi());
	    Int_t nPointsITS = 0;
	    for (Int_t il=0; il<6; il++){ 
	      if(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(il)) nPointsITS++;
	    }
	    fillthis="hNITSpointsD0vsptS_";
	    fillthis+=ptbin;
	    ((TH2F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt(), nPointsITS);
	    fillthis="hNSPDpointsD0S_";
	    fillthis+=ptbin;
	    if(!(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0)) && !(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(1))){ //no SPD points
	      ((TH1I*)listout->FindObject(fillthis))->Fill(0);
	    } 
	    if(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0) && !(((AliAODTrack*)(fDaughterTracks.UncheckedAt(it)))->HasPointOnITSLayer(1))){ //kOnlyFirst
	      ((TH1I*)listout->FindObject(fillthis))->Fill(1);
	    } 
	    if(!(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0)) && ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(1)){ //kOnlySecond
	      ((TH1I*)listout->FindObject(fillthis))->Fill(2);
	    }
	    if(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0) && ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(1)){ //kboth
	      ((TH1I*)listout->FindObject(fillthis))->Fill(3);
	    } 
       	    fillthis="hNclsD0vsptS_";
	    fillthis+=ptbin;
	    Float_t mom = ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt();
	    Float_t ncls = (Float_t)((AliAODTrack*)fDaughterTracks.UncheckedAt(0))->GetTPCNcls();
	    ((TH2F*)listout->FindObject(fillthis))->Fill(mom, ncls);
	  }
	}	  
      }
      
      
      } //end mass cut
    
  } else{ //Background or LS
    //if(!fReadMC){
    //cout<<"is background"<<endl;

    etaD = part->Eta();
    phiD = part->Phi();
           
    //no mass cut distributions: mass, ptbis
    fillthis="hMassB_";
    fillthis+=ptbin;
    fillthispt="histBkgPt";

    if (!fCutOnDistr || (fCutOnDistr && (fIsSelectedCandidate==1 || fIsSelectedCandidate==3))) {
      ((TH1F*)listout->FindObject(fillthis))->Fill(minvD0);
      if(fFillPtHist && fReadMC) ((TH2F*)fOutputMassPt->FindObject(fillthispt))->Fill(minvD0,pt);
      
      fillthisetaphi="hetaphiD0candidateB_";
      fillthisetaphi+=ptbin;
      ((TH2F*)listout->FindObject(fillthisetaphi))->Fill(etaD, phiD);
    
      if(TMath::Abs(minvD0-mPDG)<0.05){
	fillthisetaphi="hetaphiD0candidatesignalregionB_";
	fillthisetaphi+=ptbin;
	((TH2F*)listout->FindObject(fillthisetaphi))->Fill(etaD, phiD);
      }
    }
    if (!fCutOnDistr || (fCutOnDistr && fIsSelectedCandidate>1)) {
      ((TH1F*)listout->FindObject(fillthis))->Fill(minvD0bar);
      if(fFillPtHist && fReadMC) ((TH2F*)fOutputMassPt->FindObject(fillthispt))->Fill(minvD0bar,pt);

      fillthisetaphi="hetaphiD0barcandidateB_";	
      fillthisetaphi+=ptbin;
      ((TH2F*)listout->FindObject(fillthisetaphi))->Fill(etaD, phiD);
      
      if(TMath::Abs(minvD0bar-mPDG)<0.05){
	fillthisetaphi="hetaphiD0barcandidatesignalregionB_";	
	fillthisetaphi+=ptbin;
	((TH2F*)listout->FindObject(fillthisetaphi))->Fill(etaD, phiD);
      }

    }
    if(fSys==0){
      fillthis="hptB1prongnoMcut_";
      fillthis+=ptbin;
      
      ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(0))->Pt());
      
      fillthis="hptB2prongsnoMcut_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(0))->Pt());
      ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(0))->Pt());
    }


      //apply cut on invariant mass on the pair
      if(TMath::Abs(minvD0-mPDG)<invmasscut || TMath::Abs(minvD0bar-mPDG)<invmasscut){
      if(fSys==0){
	ptProng[0]=((AliAODTrack*)fDaughterTracks.UncheckedAt(0))->Pt(); ptProng[1]=((AliAODTrack*)fDaughterTracks.UncheckedAt(0))->Pt();
	cosThetaStarD0 = part->CosThetaStarD0();
	cosThetaStarD0bar = part->CosThetaStarD0bar();
      }
//     AliAODTrack *prong0=(AliAODTrack*)fDaughterTracks.UncheckedAt(0);
        
//  AliAODTrack *prong1=(AliAODTrack*)fDaughterTracks.UncheckedAt(1);

      cosPointingAngle = part->CosPointingAngle();
      normalizedDecayLength2 = part->NormalizedDecayLength2();
          ctau = part->CtD0();
          Double_t pzprong0 = -99;
          Double_t pzprong1 = -99;
          
          decayLengthz = TMath::Sqrt(part->DecayLength2() - (part->DecayLengthXY())*(part->DecayLength2() - (part->DecayLengthXY())));
          
          
          
       
          Double_t C= 2.99792458e10;
          
          CtauD0z = ((decayLengthz*mPDG*C)/(TMath::Sqrt(part->Pz()* part->Pz())));


      decayLength2 = part->DecayLength2();
      decayLengthxy = part->DecayLengthXY();
      normalizedDecayLengthxy=decayLengthxy/part->DecayLengthXYError();
      d0Prong[0]=part->Getd0Prong(0); d0Prong[1]=part->Getd0Prong(1);
     

      AliAODTrack *prongg=(AliAODTrack*)fDaughterTracks.UncheckedAt(0);
      if(!prongg) {
	if(fDebug>2) cout<<"No daughter found";
	return;
      }
      else{
	if(fArray==1){
	  if(prongg->Charge()==1) {
	    //fTotPosPairs[ptbin]++;
	    ((TH1F*)fOutputMass->FindObject("hpospair"))->Fill(ptbin);
	  } else {
	    //fTotNegPairs[ptbin]++;
	    ((TH1F*)fOutputMass->FindObject("hnegpair"))->Fill(ptbin);
	  }
	}
      }

      //fill pt and phi distrib for prongs with M cut

      if (!fCutOnDistr || (fCutOnDistr && (fIsSelectedCandidate==1 || fIsSelectedCandidate==3))){
  	for(Int_t it=0; it<2; it++){
  	  fillthis="hptD0B_";
 	  fillthis+=ptbin;
 	  ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt());
 	  fillthis="hphiD0B_";
 	  fillthis+=ptbin;
 	  ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Phi());

 	  Int_t nPointsITS = 0;
 	  for (Int_t il=0; il<6; il++){ 
 	    if(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(il)) nPointsITS++;
 	  }
 	  fillthis="hNITSpointsD0vsptB_";
 	  fillthis+=ptbin;
	  ((TH2F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt(), nPointsITS);
	  fillthis="hNSPDpointsD0B_";
	  fillthis+=ptbin;
	  if(!(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0)) && !(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(1))){ //no SPD points
	    ((TH1I*)listout->FindObject(fillthis))->Fill(0);
	  } 
	  if(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0) && !(((AliAODTrack*)(fDaughterTracks.UncheckedAt(it)))->HasPointOnITSLayer(1))){ //kOnlyFirst
	    ((TH1I*)listout->FindObject(fillthis))->Fill(1);
	  } 
	  if(!(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0)) && ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(1)){ //kOnlySecond
	    ((TH1I*)listout->FindObject(fillthis))->Fill(2);
	  }
	  if(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(0) && ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->HasPointOnITSLayer(1)){ //kboth
	    ((TH1I*)listout->FindObject(fillthis))->Fill(3);
	  } 
	  fillthis="hNclsD0vsptB_";
	  fillthis+=ptbin;
	  Float_t mom = ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt();
	  Float_t ncls = (Float_t)((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->GetTPCNcls();
	  ((TH2F*)listout->FindObject(fillthis))->Fill(mom, ncls);	    
	}


         }

        if (!fCutOnDistr || (fCutOnDistr && fIsSelectedCandidate>1)) {
 	for(Int_t it=0; it<2; it++){
	  fillthis="hptD0barB_";
	  fillthis+=ptbin;
	  ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt());
	  fillthis="hphiD0barB_";
	  fillthis+=ptbin;
	  ((TH1F*)listout->FindObject(fillthis))->Fill(((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Phi());
	  fillthis="hNclsD0barvsptB_";
	  fillthis+=ptbin;
	  Float_t mom = ((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->Pt();
	  Float_t ncls = (Float_t)((AliAODTrack*)fDaughterTracks.UncheckedAt(it))->GetTPCNcls();
	  ((TH2F*)listout->FindObject(fillthis))->Fill(mom, ncls);
	}
      }
	
      fillthis="hd0B_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(d0Prong[0]);
      ((TH1F*)listout->FindObject(fillthis))->Fill(d0Prong[1]);

      if(fReadMC){
	Int_t pdgMother[2]={0,0};
	Double_t factor[2]={1,1};

	for(Int_t iprong=0;iprong<2;iprong++){
	  AliAODTrack *prong=(AliAODTrack*)fDaughterTracks.UncheckedAt(iprong);
	  lab=prong->GetLabel();
	  if(lab>=0){
	    AliAODMCParticle* mcprong=(AliAODMCParticle*)arrMC->At(lab);
	    if(mcprong){
	      Int_t labmom=mcprong->GetMother();
	      if(labmom>=0){
		AliAODMCParticle* mcmother=(AliAODMCParticle*)arrMC->At(labmom);
		if(mcmother) pdgMother[iprong]=mcmother->GetPdgCode();
	      }
	    }
	  }

	  if(fSys==0){

	    fillthis="hd0moresB_";
	    fillthis+=ptbin;
	  
	    if(TMath::Abs(pdgMother[iprong])==310 || TMath::Abs(pdgMother[iprong])==130 || TMath::Abs(pdgMother[iprong])==321){ //K^0_S, K^0_L, K^+-
	      if(ptProng[iprong]<=1)factor[iprong]=1./.7;
	      else factor[iprong]=1./.6;
	      fNentries->Fill(11);
	    }
	    
	    if(TMath::Abs(pdgMother[iprong])==3122) { //Lambda
	      factor[iprong]=1./0.25;
	      fNentries->Fill(12);
	    }
	    fillthis="hd0moresB_";
	    fillthis+=ptbin;

	    ((TH1F*)listout->FindObject(fillthis))->Fill(d0Prong[iprong],factor[iprong]);

	    if(recalcvtx){
	      fillthis="hd0vmoresB_";
	      fillthis+=ptbin;
	      ((TH1F*)listout->FindObject(fillthis))->Fill(d0[iprong],factor[iprong]);
	    }
	  }
	} //loop on prongs

	if(fSys==0){
	  fillthis="hd0d0moresB_";
	  fillthis+=ptbin;
	  ((TH1F*)listout->FindObject(fillthis))->Fill(part->Prodd0d0(),factor[0]*factor[1]);

	  fillthis="hcosthetapointmoresB_";
	  fillthis+=ptbin;
	  ((TH1F*)listout->FindObject(fillthis))->Fill(cosPointingAngle,factor[0]*factor[1]);

	  if(recalcvtx){
	    fillthis="hd0d0vmoresB_";
	    fillthis+=ptbin;
	    ((TH1F*)listout->FindObject(fillthis))->Fill(d0[0]*d0[1],factor[0]*factor[1]);
	  }
	}
          
          
          
          fillthis="hctau_";
          fillthis+=ptbin;
          ((TH1F*)listout->FindObject(fillthis))->Fill(CtauD0z);
          
          
          
          //correlation ctauXY ctauZ
          fillthis="hctauXYctauZ_"  ;
          fillthis+=ptbin;
          ((TH2F*)listout->FindObject(fillthis))->Fill(CtauD0,CtauD0z);
          
       
    
          
          Int_t pdgDgD0toKpi[2]={321,211};
          Int_t labD0=-1;
          Bool_t isPrimary=kTRUE;
         // Double_t impparXY=part->ImpParXY()*10000.;
          Double_t trueImpParXY=0.;
          Double_t invmassD0 = part->InvMassD0(), invmassD0bar = part->InvMassD0bar();
          Double_t arrayForSparse[3]={invmassD0,pt,impparXY};
          Double_t arrayForSparseTrue[3]={invmassD0,pt,trueImpParXY};
          if (fReadMC) labD0 = part->MatchToMC(421,arrMC,2,pdgDgD0toKpi); //return MC particle label if the array corresponds to a D0, -1 if not (cf. AliAODRecoDecay.cxx)
         // cout << "labD0         "<<labD0 <<endl;
          fNentries->Fill(1);
          //count true D0 selected by cuts
          if (fReadMC && labD0>=0) fNentries->Fill(2);
          
          if ((fIsSelectedCandidate==1 || fIsSelectedCandidate==3) && fFillOnlyD0D0bar<2) { //D0
              
            //  cout << "confronto ctau primarie e secondarie DOPO if" <<endl;
              
              //arrayForSparse[0]=invmassD0; arrayForSparse[2]=impparXY;
              if(fReadMC){
                  if(labD0>=0) {
                      if(fArray==1) cout<<"LS signal ERROR"<<endl;
                      
                      AliAODMCParticle *partD0 = (AliAODMCParticle*)arrMC->At(labD0);
                      Int_t pdgD0 = partD0->GetPdgCode();
                      
                   
            
                      
                  }
              }
              
          }
          
          
          Double_t weigD0=1.;
          Double_t weigD0bar=1.;
          if (fCuts->GetCombPID() && (fCuts->GetBayesianStrategy() == AliRDHFCutsD0toKpi::kBayesWeight || fCuts->GetBayesianStrategy() == AliRDHFCutsD0toKpi::kBayesWeightNoFilter)) {
              weigD0=fCuts->GetWeightsNegative()[AliPID::kKaon] * fCuts->GetWeightsPositive()[AliPID::kPion];
              weigD0bar=fCuts->GetWeightsPositive()[AliPID::kKaon] * fCuts->GetWeightsNegative()[AliPID::kPion];
              if (weigD0 > 1.0 || weigD0 < 0.) {weigD0 = 0.;}
              if (weigD0bar > 1.0 || weigD0bar < 0.) {weigD0bar = 0.;} //Prevents filling with weight > 1, or < 0
          }


          
          
          
          
      } //readMC

      if(fSys==0){	    
	//normalise pt distr to half afterwards
	fillthis="hptB_";
	fillthis+=ptbin;
	((TH1F*)listout->FindObject(fillthis))->Fill(ptProng[0]);
	((TH1F*)listout->FindObject(fillthis))->Fill(ptProng[1]);

	fillthis="hcosthetastarB_";
	fillthis+=ptbin;
	if (!fCutOnDistr || (fCutOnDistr && (fIsSelectedCandidate==1 || fIsSelectedCandidate==3)))((TH1F*)listout->FindObject(fillthis))->Fill(cosThetaStarD0);
	if (!fCutOnDistr || (fCutOnDistr && fIsSelectedCandidate>1))((TH1F*)listout->FindObject(fillthis))->Fill(cosThetaStarD0bar);	


	fillthis="hd0p0B_";
	fillthis+=ptbin;
	((TH1F*)listout->FindObject(fillthis))->Fill(d0Prong[0]);
	fillthis="hd0p1B_";
	fillthis+=ptbin;
	((TH1F*)listout->FindObject(fillthis))->Fill(d0Prong[1]);
	
	fillthis="hcosthpointd0d0B_";
	fillthis+=ptbin;
	((TH2F*)listout->FindObject(fillthis))->Fill(cosPointingAngle,part->Prodd0d0());
	
	fillthis="hcosthpointd0B_";
	fillthis+=ptbin;	  
	((TH1F*)listout->FindObject(fillthis))->Fill(cosPointingAngle,d0Prong[0]);
	((TH1F*)listout->FindObject(fillthis))->Fill(cosPointingAngle,d0Prong[1]);
	  

	if(recalcvtx){

	  fillthis="hd0vp0B_";
	  fillthis+=ptbin;
	  ((TH1F*)listout->FindObject(fillthis))->Fill(d0[0]);
	  fillthis="hd0vp1B_";
	  fillthis+=ptbin;
	  ((TH1F*)listout->FindObject(fillthis))->Fill(d0[1]);
	  
	  fillthis="hd0vB_";
	  fillthis+=ptbin;
	  ((TH1F*)listout->FindObject(fillthis))->Fill(d0[0]);
	  ((TH1F*)listout->FindObject(fillthis))->Fill(d0[1]);

	}

      }  

      fillthis="hdcaB_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(part->GetDCA());

      fillthis="hd0d0B_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(d0Prong[0]*d0Prong[1]);

      if(recalcvtx){
	fillthis="hd0d0vB_";
	fillthis+=ptbin;
	((TH1F*)listout->FindObject(fillthis))->Fill(d0[0]*d0[1]);
      }

      fillthis="hcosthetapointB_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(cosPointingAngle);

      fillthis="hcosthetapointxyB_";
      fillthis+=ptbin;	  
      ((TH1F*)listout->FindObject(fillthis))->Fill(part->CosPointingAngleXY());

      fillthis="hdeclB_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(decayLength2);

      fillthis="hnormdeclB_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(normalizedDecayLength2);
          
          
          
          
          
          
          
          /////////////////
      fillthis="hdeclxyB_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(decayLengthxy);

      fillthis="hnormdeclxyB_";
      fillthis+=ptbin;
      ((TH1F*)listout->FindObject(fillthis))->Fill(normalizedDecayLengthxy);

      fillthis="hdeclxyd0d0B_";
      fillthis+=ptbin;
      ((TH2F*)listout->FindObject(fillthis))->Fill(decayLengthxy,d0Prong[0]*d0Prong[1]);

      fillthis="hnormdeclxyd0d0B_";
      fillthis+=ptbin;
      ((TH2F*)listout->FindObject(fillthis))->Fill(normalizedDecayLengthxy,d0Prong[0]*d0Prong[1]);


      if(recalcvtx) {

	fillthis="hdeclvB_";
	fillthis+=ptbin;
	((TH1F*)listout->FindObject(fillthis))->Fill(decl[0]);

	fillthis="hnormdeclvB_";
	fillthis+=ptbin;
	((TH1F*)listout->FindObject(fillthis))->Fill(decl[1]);


      }
    }//mass cut	
  }//else (background)
  
  return;
}

//____________________________________________________________________________
void AliAnalysisTaskSED0MassNonPromptFraction::FillMassHists(AliAODRecoDecayHF2Prong *part, TClonesArray *arrMC, AliAODMCHeader *mcHeader, AliRDHFCutsD0toKpi* cuts, TList *listout){
  //
  /// function used in UserExec to fill mass histograms:
  //
    
Double_t mPDG=TDatabasePDG::Instance()->GetParticle(421)->Mass();

 /*

  //cout<<"is selected = "<<fIsSelectedCandidate<<endl;

  // Fill candidate variable Tree (track selection, no candidate selection)
  if( fWriteVariableTree && !part->HasBadDaughters()
      && fCuts->AreDaughtersSelected(part) && fCuts->IsSelectedPID(part) ){
    fCandidateVariables[0] = part->InvMassD0(); //ok
    fCandidateVariables[1] = part->InvMassD0bar(); //ok
    fCandidateVariables[2] = part->Pt(); //ok

   // fCandidateVariables[3] = part->Pt2Prong(0);
   // fCandidateVariables[4] = part->Pt2Prong(1);
   // fCandidateVariables[5] = part->Getd0Prong(0);
   // fCandidateVariables[6] = part->Getd0Prong(1);
   // fCandidateVariables[7] = part->Prodd0d0();

   //   fCandidateVariables[8] = part->DecayLengthXY();
     // Double_t PseudoProperDecLen =
      
      fCandidateVariables[3] = part->CtD0();
      fCandidateVariables[4] = PseudoProperDecLen;
   //   fCandidateVariables[10]= part->ImpParXY()*10000.;
   // fCandidateVariables[14] = fCuts->IsSelectedSpecialCuts(part);
    fVariablesTree->Fill();
  }
    
    */
    

  //cout<<"check cuts = "<<endl;
  //cuts->PrintAll();
  if (!fIsSelectedCandidate){
    //cout<<" cut " << cuts << " Rejected because "<<cuts->GetWhy()<<endl;
    return;
  }


  if(fDebug>2)  cout<<"Candidate selected"<<endl;

  Double_t invmassD0 = part->InvMassD0(), invmassD0bar = part->InvMassD0bar();
  //printf("SELECTED\n");
  Int_t ptbin=cuts->PtBin(part->Pt());
  Double_t pt = part->Pt();
  Double_t y = part->YD0();
  
  Double_t impparXY=part->ImpParXY()*10000.;
  Double_t trueImpParXY=0.;
  Double_t arrayForSparse[3]={invmassD0,pt,impparXY};
  Double_t arrayForSparseTrue[3]={invmassD0,pt,trueImpParXY};


 
  TString fillthis="", fillthispt="", fillthismasspt="", fillthismassy="";
  Int_t pdgDgD0toKpi[2]={321,211};
  Int_t labD0=-1;
    
    
    
    Double_t ctau = -1;
    Double_t ptProng[2]={-99,-99};
    AliAODTrack *prong0=(AliAODTrack*)fDaughterTracks.UncheckedAt(0);
    AliAODTrack *prong1=(AliAODTrack*)fDaughterTracks.UncheckedAt(1);
    Double_t CtauD0 = -1;
    
   
    
    
  Bool_t isPrimary=kTRUE;
  if (fReadMC) labD0 = part->MatchToMC(421,arrMC,2,pdgDgD0toKpi); //return MC particle label if the array corresponds to a D0, -1 if not (cf. AliAODRecoDecay.cxx)

  //Define weights for Bayesian (if appropriate)

  Double_t weigD0=1.;
  Double_t weigD0bar=1.;
  if (fCuts->GetCombPID() && (fCuts->GetBayesianStrategy() == AliRDHFCutsD0toKpi::kBayesWeight || fCuts->GetBayesianStrategy() == AliRDHFCutsD0toKpi::kBayesWeightNoFilter)) {
    weigD0=fCuts->GetWeightsNegative()[AliPID::kKaon] * fCuts->GetWeightsPositive()[AliPID::kPion];
    weigD0bar=fCuts->GetWeightsPositive()[AliPID::kKaon] * fCuts->GetWeightsNegative()[AliPID::kPion];
    if (weigD0 > 1.0 || weigD0 < 0.) {weigD0 = 0.;}    
    if (weigD0bar > 1.0 || weigD0bar < 0.) {weigD0bar = 0.;} //Prevents filling with weight > 1, or < 0
  }
  
  //count candidates selected by cuts
  fNentries->Fill(1);
  //count true D0 selected by cuts
  if (fReadMC && labD0>=0) fNentries->Fill(2);

  if ((fIsSelectedCandidate==1 || fIsSelectedCandidate==3) && fFillOnlyD0D0bar<2) { //D0

    arrayForSparse[0]=invmassD0; arrayForSparse[2]=impparXY;

    if(fReadMC){
      if(labD0>=0) {
	if(fArray==1) cout<<"LS signal ERROR"<<endl;

	AliAODMCParticle *partD0 = (AliAODMCParticle*)arrMC->At(labD0);
	Int_t pdgD0 = partD0->GetPdgCode();
	//	cout<<"pdg = "<<pdgD0<<endl;
       //   ctau=part->CtD0();
     

     
          // old function	if(CheckOrigin(arrMC,partD0)==5) isPrimary=kFALSE; //la funzione checkorigin restituisce 5 se la D deriva da B, ovvero se è secondaria!
          if(AliVertexingHFUtils::CheckOrigin(arrMC,partD0,fUseQuarkTagInKine)==5) isPrimary=kFALSE;

	
	if(!isPrimary)
        trueImpParXY=GetTrueImpactParameter(mcHeader,arrMC,partD0)*10000.;
        arrayForSparseTrue[0]=invmassD0; arrayForSparseTrue[2]=trueImpParXY;
 
          

	if (pdgD0==421){ //D0
	  //	  cout<<"Fill S with D0"<<endl;
	  fillthis="histSgn_";
	  fillthis+=ptbin;
	  ((TH1F*)(listout->FindObject(fillthis)))->Fill(invmassD0,weigD0);
        
        

	  if(fFillPtHist){ 
	    fillthismasspt="histSgnPt";
	    ((TH2F*)(fOutputMassPt->FindObject(fillthismasspt)))->Fill(invmassD0,pt,weigD0);
	  }
	  if(fFillImpParHist){ 
	    if(isPrimary) fHistMassPtImpParTC[1]->Fill(arrayForSparse,weigD0);
	    else {
	      fHistMassPtImpParTC[2]->Fill(arrayForSparse,weigD0);
	      fHistMassPtImpParTC[3]->Fill(arrayForSparseTrue,weigD0);
	    }
	  }

	  if(fFillYHist){ 
	    fillthismassy="histSgnY_";
	    fillthismassy+=ptbin;
	    ((TH2F*)(fOutputMassY->FindObject(fillthismassy)))->Fill(invmassD0,y,weigD0);
	  }

	  if(fSys==0){
	    if(TMath::Abs(invmassD0 - mPDG) < 0.027 && fFillVarHists){
	      fillthis="histSgn27_";
	      fillthis+=ptbin;
	      ((TH1F*)(listout->FindObject(fillthis)))->Fill(invmassD0,weigD0);
	    }
	  }
	} else{ //it was a D0bar
	  fillthis="histRfl_";
	  fillthis+=ptbin;
	  ((TH1F*)(listout->FindObject(fillthis)))->Fill(invmassD0,weigD0);

	  if(fFillPtHist){ 
	    fillthismasspt="histRflPt";
	    //	    cout << " Filling "<<fillthismasspt<<" D0bar"<<endl;	    
	    ((TH2F*)(fOutputMassPt->FindObject(fillthismasspt)))->Fill(invmassD0,pt,weigD0);
	  }

	  if(fFillYHist){ 
	    fillthismassy="histRflY_";
	    fillthismassy+=ptbin;
	    //	    cout << " Filling "<<fillthismassy<<" D0bar"<<endl;	    
	    ((TH2F*)(fOutputMassY->FindObject(fillthismassy)))->Fill(invmassD0,y,weigD0);
	  }

	}
      } else {//background
	fillthis="histBkg_";
	fillthis+=ptbin;
	((TH1F*)(listout->FindObject(fillthis)))->Fill(invmassD0,weigD0);
   
	if(fFillPtHist){ 
	  fillthismasspt="histBkgPt";
	  //	  cout << " Filling "<<fillthismasspt<<" D0bar"<<endl;
	  ((TH2F*)(fOutputMassPt->FindObject(fillthismasspt)))->Fill(invmassD0,pt,weigD0);
	}
	if(fFillImpParHist) fHistMassPtImpParTC[4]->Fill(arrayForSparse,weigD0);

	if(fFillYHist){ 
	  fillthismassy="histBkgY_";
	  fillthismassy+=ptbin;
	  //	  cout << " Filling "<<fillthismassy<<" D0bar"<<endl;
	  ((TH2F*)(fOutputMassY->FindObject(fillthismassy)))->Fill(invmassD0,y,weigD0);
	}

      }

    }else{
      fillthis="histMass_";
      fillthis+=ptbin;
      //      cout<<"Filling "<<fillthis<<endl;

      //      printf("Fill mass with D0");
      ((TH1F*)(listout->FindObject(fillthis)))->Fill(invmassD0,weigD0);
      

      if(fFillPtHist){ 
	fillthismasspt="histMassPt";
	//	cout<<"Filling "<<fillthismasspt<<endl;
	((TH2F*)(fOutputMassPt->FindObject(fillthismasspt)))->Fill(invmassD0,pt,weigD0);
      }
      if(fFillImpParHist) {
	//	cout << "Filling fHistMassPtImpParTC[0]"<<endl;
	fHistMassPtImpParTC[0]->Fill(arrayForSparse,weigD0);
      }

      if(fFillYHist){ 
	fillthismassy="histMassY_";
	fillthismassy+=ptbin;
	//	cout<<"Filling "<<fillthismassy<<endl;
	((TH2F*)(fOutputMassY->FindObject(fillthismassy)))->Fill(invmassD0,y,weigD0);
      }

    }
     
  }
  if (fIsSelectedCandidate>1 && (fFillOnlyD0D0bar==0 || fFillOnlyD0D0bar==2)) { //D0bar

    arrayForSparse[0]=invmassD0bar; arrayForSparse[2]=impparXY;

    if(fReadMC){
      if(labD0>=0) {
	if(fArray==1) cout<<"LS signal ERROR"<<endl;
	AliAODMCParticle *partD0 = (AliAODMCParticle*)arrMC->At(labD0);
	Int_t pdgD0 = partD0->GetPdgCode();
	//	cout<<" pdg = "<<pdgD0<<endl;

	
          //old function	if(CheckOrigin(arrMC,partD0)==5) isPrimary=kFALSE;
          if(AliVertexingHFUtils::CheckOrigin(arrMC,partD0,fUseQuarkTagInKine)==5) isPrimary=kFALSE;
	if(!isPrimary)
	  trueImpParXY=GetTrueImpactParameter(mcHeader,arrMC,partD0)*10000.;
	arrayForSparseTrue[0]=invmassD0bar; arrayForSparseTrue[2]=trueImpParXY;

	if (pdgD0==-421){ //D0bar
	  fillthis="histSgn_";
	  fillthis+=ptbin;
	  ((TH1F*)(listout->FindObject(fillthis)))->Fill(invmassD0bar,weigD0bar);
	 

	  if(fFillPtHist){ 
	    fillthismasspt="histSgnPt";
	    //	    cout<<" Filling "<< fillthismasspt << endl;
	    ((TH2F*)(fOutputMassPt->FindObject(fillthismasspt)))->Fill(invmassD0bar,pt,weigD0bar);
	  }
	  if(fFillImpParHist){ 
	    //	    cout << " Filling impact parameter thnsparse"<<endl;
	    if(isPrimary) fHistMassPtImpParTC[1]->Fill(arrayForSparse,weigD0bar);
	    else {
	      fHistMassPtImpParTC[2]->Fill(arrayForSparse,weigD0bar);
	      fHistMassPtImpParTC[3]->Fill(arrayForSparseTrue,weigD0bar);
	    }
	  }

	  if(fFillYHist){ 
	    fillthismassy="histSgnY_";
	    fillthismassy+=ptbin;
	    //	    cout<<" Filling "<< fillthismassy << endl;
	    ((TH2F*)(fOutputMassY->FindObject(fillthismassy)))->Fill(invmassD0bar,y,weigD0bar);
	  }
	  
	} else{
	  fillthis="histRfl_";
	  fillthis+=ptbin;
	  ((TH1F*)(listout->FindObject(fillthis)))->Fill(invmassD0bar,weigD0bar);
	  if(fFillPtHist){ 
	    fillthismasspt="histRflPt";
	    //	    cout << " Filling "<<fillthismasspt<<endl;
	    ((TH2F*)(fOutputMassPt->FindObject(fillthismasspt)))->Fill(invmassD0bar,pt,weigD0bar);
	  }
	  if(fFillYHist){ 
	    fillthismassy="histRflY_";
	    fillthismassy+=ptbin;
	    //	    cout << " Filling "<<fillthismassy<<endl;
	    ((TH2F*)(fOutputMassY->FindObject(fillthismassy)))->Fill(invmassD0bar,y,weigD0bar);
	  }
	}
      } else {//background or LS
	fillthis="histBkg_";
	fillthis+=ptbin;
	((TH1F*)(listout->FindObject(fillthis)))->Fill(invmassD0bar,weigD0bar);

	if(fFillPtHist){ 
	  fillthismasspt="histBkgPt";
	  //	  cout<<" Filling "<< fillthismasspt << endl;
	  ((TH2F*)(fOutputMassPt->FindObject(fillthismasspt)))->Fill(invmassD0bar,pt,weigD0bar);
	}
	if(fFillImpParHist) fHistMassPtImpParTC[4]->Fill(arrayForSparse,weigD0bar);
	if(fFillYHist){ 
	  fillthismassy="histBkgY_";
	  fillthismassy+=ptbin;
	  //	  cout<<" Filling "<< fillthismassy << endl;
	  ((TH2F*)(fOutputMassY->FindObject(fillthismassy)))->Fill(invmassD0bar,y,weigD0bar);
	}
      }
    }else{
      fillthis="histMass_";
      fillthis+=ptbin;
      //      printf("Fill mass with D0bar");

      ((TH1F*)listout->FindObject(fillthis))->Fill(invmassD0bar,weigD0bar);
      

      if(fFillPtHist){ 
	fillthismasspt="histMassPt";
	//	cout<<" Filling "<< fillthismasspt << endl;
	((TH2F*)(fOutputMassPt->FindObject(fillthismasspt)))->Fill(invmassD0bar,pt,weigD0bar);
      }
      if(fFillImpParHist) fHistMassPtImpParTC[0]->Fill(arrayForSparse,weigD0bar);
      if(fFillYHist){ 
	fillthismassy="histMassY_";
	fillthismassy+=ptbin;
	//	cout<<" Filling "<< fillthismassy << endl;
	((TH2F*)(fOutputMassY->FindObject(fillthismassy)))->Fill(invmassD0bar,y,weigD0bar);
      }
    }
  }

  return;
}

//__________________________________________________________________________
AliAODVertex* AliAnalysisTaskSED0MassNonPromptFraction::GetPrimaryVtxSkipped(AliAODEvent *aodev){
  /// Calculate the primary vertex w/o the daughter tracks of the candidate
  
  Int_t skipped[2];
  Int_t nTrksToSkip=2;
  AliAODTrack *dgTrack = (AliAODTrack*)fDaughterTracks.UncheckedAt(0);
  if(!dgTrack){
    AliDebug(2,"no daughter found!");
    return 0x0;
  }
  skipped[0]=dgTrack->GetID();
  dgTrack = (AliAODTrack*)fDaughterTracks.UncheckedAt(1);
  if(!dgTrack){
    AliDebug(2,"no daughter found!");
    return 0x0;
  }
  skipped[1]=dgTrack->GetID();

  AliESDVertex *vertexESD=0x0;
  AliAODVertex *vertexAOD=0x0;
  AliVertexerTracks *vertexer = new AliVertexerTracks(aodev->GetMagneticField());
  
  //
  vertexer->SetSkipTracks(nTrksToSkip,skipped);
  vertexer->SetMinClusters(4);  
  vertexESD = (AliESDVertex*)vertexer->FindPrimaryVertex(aodev); 
  if(!vertexESD) return vertexAOD;
  if(vertexESD->GetNContributors()<=0) { 
    AliDebug(2,"vertexing failed"); 
    delete vertexESD; vertexESD=NULL;
    return vertexAOD;
  }
  
  delete vertexer; vertexer=NULL;
  
  
  // convert to AliAODVertex
  Double_t pos[3],cov[6],chi2perNDF;
  vertexESD->GetXYZ(pos); // position
  vertexESD->GetCovMatrix(cov); //covariance matrix
  chi2perNDF = vertexESD->GetChi2toNDF();
  delete vertexESD; vertexESD=NULL;
  
  vertexAOD = new AliAODVertex(pos,cov,chi2perNDF);
  return vertexAOD;
  
}


//________________________________________________________________________
void AliAnalysisTaskSED0MassNonPromptFraction::Terminate(Option_t */*option*/)
{
  /// Terminate analysis
  //
  if(fDebug > 1) printf("AnalysisTaskSED0Mass: Terminate() \n");


  fOutputMass = dynamic_cast<TList*> (GetOutputData(1));
  if (!fOutputMass) {     
    printf("ERROR: fOutputMass not available\n");
    return;
  }
  fOutputMassPt = dynamic_cast<TList*> (GetOutputData(6));
  if ((fFillPtHist || fFillImpParHist) && !fOutputMassPt) {
    printf("ERROR: fOutputMass not available\n");
    return;
  }

  if(fFillVarHists){
    fDistr = dynamic_cast<TList*> (GetOutputData(2));
    if (!fDistr) {
      printf("ERROR: fDistr not available\n");
      return;
    }
  }

  fNentries = dynamic_cast<TH1F*>(GetOutputData(3));
  
  if(!fNentries){
    printf("ERROR: fNEntries not available\n");
    return;
  }
  fCuts = dynamic_cast<AliRDHFCutsD0toKpi*>(GetOutputData(4));
  if(!fCuts){
    printf("ERROR: fCuts not available\n");
    return;
  }
  fCounter = dynamic_cast<AliNormalizationCounter*>(GetOutputData(5));    
  if (!fCounter) {
    printf("ERROR: fCounter not available\n");
    return;
  }
  if (fDrawDetSignal) {
    fDetSignal = dynamic_cast<TList*>(GetOutputData(8));
    if (!fDetSignal) {
      printf("ERROR: fDetSignal not available\n");
      return;
    }
  }
  if(fFillYHist){
    fOutputMassY = dynamic_cast<TList*> (GetOutputData(9));
    if (fFillYHist && !fOutputMassY) {
      printf("ERROR: fOutputMassY not available\n");
      return;
    }
  }

  Int_t nptbins=fCuts->GetNPtBins();
  for(Int_t ipt=0;ipt<nptbins;ipt++){ 

    if(fArray==1 && fFillVarHists){ 
      fLsNormalization = 2.*TMath::Sqrt(((TH1F*)fOutputMass->FindObject("hpospair"))->Integral(nptbins+ipt+1,nptbins+ipt+2)*((TH1F*)fOutputMass->FindObject("hnegpair"))->Integral(nptbins+ipt+1,nptbins+ipt+2)); //after cuts


      if(fLsNormalization>1e-6) {
	
	TString massName="histMass_";
	massName+=ipt;
	((TH1F*)fOutputMass->FindObject(massName))->Scale((1/fLsNormalization)*((TH1F*)fOutputMass->FindObject(massName))->GetEntries());

      }
    

      fLsNormalization = 2.*TMath::Sqrt(((TH1F*)fOutputMass->FindObject("hpospair"))->Integral(ipt+1,ipt+2)*((TH1F*)fOutputMass->FindObject("hnegpair"))->Integral(ipt+1,ipt+2)); 
      //fLsNormalization = 2.*TMath::Sqrt(fTotPosPairs[4]*fTotNegPairs[4]);

      if(fLsNormalization>1e-6) {

	TString nameDistr="hdcaB_";
	nameDistr+=ipt;
	((TH1F*)fDistr->FindObject(nameDistr))->Scale((1/fLsNormalization)*((TH1F*)fDistr->FindObject(nameDistr))->GetEntries());
	nameDistr="hd0B_";
	nameDistr+=ipt;
	((TH1F*)fDistr->FindObject(nameDistr))->Scale((1/fLsNormalization)*((TH1F*)fDistr->FindObject(nameDistr))->GetEntries());
	nameDistr="hd0d0B_";
	nameDistr+=ipt;
	((TH1F*)fDistr->FindObject(nameDistr))->Scale((1/fLsNormalization)*((TH1F*)fDistr->FindObject(nameDistr))->GetEntries());
	nameDistr="hcosthetapointB_";
	nameDistr+=ipt;
	((TH1F*)fDistr->FindObject(nameDistr))->Scale((1/fLsNormalization)*((TH1F*)fDistr->FindObject(nameDistr))->GetEntries());
	if(fSys==0){
	  nameDistr="hptB_";
	  nameDistr+=ipt;
	  ((TH1F*)fDistr->FindObject(nameDistr))->Scale((1/fLsNormalization)*((TH1F*)fDistr->FindObject(nameDistr))->GetEntries());
	  nameDistr="hcosthetastarB_";
	  nameDistr+=ipt;
	  ((TH1F*)fDistr->FindObject(nameDistr))->Scale((1/fLsNormalization)*((TH1F*)fDistr->FindObject(nameDistr))->GetEntries());
	  nameDistr="hcosthpointd0d0B_";
	  nameDistr+=ipt;
	  ((TH2F*)fDistr->FindObject(nameDistr))->Scale((1/fLsNormalization)*((TH2F*)fDistr->FindObject(nameDistr))->GetEntries());
	}
      }
    }
  }
  TString cvname,cstname;

  if (fArray==0){
    cvname="D0invmass";
    cstname="cstat0";
  } else {
    cvname="LSinvmass";
    cstname="cstat1";
  }

  TCanvas *cMass=new TCanvas(cvname,cvname);
  cMass->cd();
  ((TH1F*)fOutputMass->FindObject("histMass_3"))->Draw();

  TCanvas* cStat=new TCanvas(cstname,Form("Stat%s",fArray ? "LS" : "D0"));
  cStat->cd();
  cStat->SetGridy();
  fNentries->Draw("htext0");

  // TCanvas *ccheck=new TCanvas(Form("cc%d",fArray),Form("cc%d",fArray));
  // ccheck->cd();

  return;
}


//________________________________________________________________________
void AliAnalysisTaskSED0MassNonPromptFraction::CreateImpactParameterHistos(){
  /// Histos for impact paramter study

  Int_t nmassbins=200; 
  Double_t fLowmasslimit=1.5648, fUpmasslimit=2.1648;
  Int_t fNImpParBins=400;
  Double_t fLowerImpPar=-2000., fHigherImpPar=2000.;
  Int_t nbins[3]={nmassbins,200,fNImpParBins};
  Double_t xmin[3]={fLowmasslimit,0.,fLowerImpPar};
  Double_t xmax[3]={fUpmasslimit,20.,fHigherImpPar};
  

  fHistMassPtImpParTC[0]=new THnSparseF("hMassPtImpParAll",
					"Mass vs. pt vs.imppar - All",
					3,nbins,xmin,xmax);
  fHistMassPtImpParTC[1]=new THnSparseF("hMassPtImpParPrompt",
					"Mass vs. pt vs.imppar - promptD",
					3,nbins,xmin,xmax);
  fHistMassPtImpParTC[2]=new THnSparseF("hMassPtImpParBfeed",
					"Mass vs. pt vs.imppar - DfromB",
					3,nbins,xmin,xmax);
  fHistMassPtImpParTC[3]=new THnSparseF("hMassPtImpParTrueBfeed",
					"Mass vs. pt vs.true imppar -DfromB",
					3,nbins,xmin,xmax);
  fHistMassPtImpParTC[4]=new THnSparseF("hMassPtImpParBkg",
				        "Mass vs. pt vs.imppar - backgr.",
					3,nbins,xmin,xmax);

  for(Int_t i=0; i<5;i++){
    fOutputMassPt->Add(fHistMassPtImpParTC[i]);
  }
}

//_________________________________________________________________________________________________
Float_t AliAnalysisTaskSED0MassNonPromptFraction::GetTrueImpactParameter(AliAODMCHeader *mcHeader, TClonesArray* arrayMC, AliAODMCParticle *partD0) const {
  /// true impact parameter calculation

  printf(" AliAnalysisTaskSED0MassNonPromptFractionV1::GetTrueImpactParameter() \n");

  Double_t vtxTrue[3];
  mcHeader->GetVertex(vtxTrue);
  Double_t origD[3];
  partD0->XvYvZv(origD);
  Short_t charge=partD0->Charge();
  Double_t pXdauTrue[2],pYdauTrue[2],pZdauTrue[2];
  for(Int_t iDau=0; iDau<2; iDau++){
    pXdauTrue[iDau]=0.;
    pYdauTrue[iDau]=0.;
    pZdauTrue[iDau]=0.;
  }

  //  Int_t nDau=partD0->GetNDaughters();
  Int_t labelFirstDau = partD0->GetDaughter(0); 

  for(Int_t iDau=0; iDau<2; iDau++){
    Int_t ind = labelFirstDau+iDau;
    AliAODMCParticle* part = dynamic_cast<AliAODMCParticle*>(arrayMC->At(ind));
    if(!part) continue;
    Int_t pdgCode = TMath::Abs( part->GetPdgCode() );
    if(!part){
      AliError("Daughter particle not found in MC array");
      return 99999.;
    } 
    if(pdgCode==211 || pdgCode==321){
      pXdauTrue[iDau]=part->Px();
      pYdauTrue[iDau]=part->Py();
      pZdauTrue[iDau]=part->Pz();
    }
  }
  
  Double_t d0dummy[2]={0.,0.};
  AliAODRecoDecayHF aodDzeroMC(vtxTrue,origD,2,charge,pXdauTrue,pYdauTrue,pZdauTrue,d0dummy);
  return aodDzeroMC.ImpParXY();

}

//_________________________________________________________________________________________________
Int_t AliAnalysisTaskSED0MassNonPromptFraction::CheckOrigin(TClonesArray* arrayMC, AliAODMCParticle *mcPartCandidate) const {		
  //
  /// checking whether the mother of the particles come from a charm or a bottom quark
  //
  //printf(" AliAnalysisTaskSED0MassNonPromptFraction V1::CheckOrigin() \n");
	
  Int_t pdgGranma = 0;
  Int_t mother = 0;
  mother = mcPartCandidate->GetMother();
  Int_t istep = 0;
  Int_t abspdgGranma =0;
  Bool_t isFromB=kFALSE;
  Bool_t isQuarkFound=kFALSE;
  while (mother >0 ){
    istep++;
    AliAODMCParticle* mcGranma = dynamic_cast<AliAODMCParticle*>(arrayMC->At(mother));
    if (mcGranma){
      pdgGranma = mcGranma->GetPdgCode();
      abspdgGranma = TMath::Abs(pdgGranma);
      if ((abspdgGranma > 500 && abspdgGranma < 600) || (abspdgGranma > 5000 && abspdgGranma < 6000)){
	isFromB=kTRUE;
      }
      if(abspdgGranma==4 || abspdgGranma==5) isQuarkFound=kTRUE;
      mother = mcGranma->GetMother();
    }else{
      AliError("Failed casting the mother particle!");
      break;
    }
  }
  
  if(isFromB) return 5;
  else return 4;
}