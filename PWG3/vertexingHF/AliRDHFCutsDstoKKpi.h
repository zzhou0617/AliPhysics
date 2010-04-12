#ifndef ALIRDHFCUTSDSTOKKPI_H
#define ALIRDHFCUTSDSTOKKPI_H
/* Copyright(c) 1998-2010, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//***********************************************************
// Class AliRDHFCutsDstoKKpi
// class for cuts on AOD reconstructed Ds->KKpi
// Author: A.Dainese, andrea.dainese@pd.infn.it
//***********************************************************

#include "AliRDHFCuts.h"

class AliRDHFCutsDstoKKpi : public AliRDHFCuts 
{
 public:

  AliRDHFCutsDstoKKpi();
  
  virtual ~AliRDHFCutsDstoKKpi(){}

  AliRDHFCutsDstoKKpi(const AliRDHFCutsDstoKKpi& source);
  AliRDHFCutsDstoKKpi& operator=(const AliRDHFCutsDstoKKpi& source); 
 
  virtual void GetCutVarsForOpt(AliAODRecoDecayHF *d,Float_t *vars,Int_t nvars,Int_t *pdgdaughters);

  using AliRDHFCuts::IsSelected;
  virtual Int_t IsSelected(TObject* obj,Int_t selectionLevel);
  
  Float_t GetMassCut(Int_t iPtBin=0) const { return (GetCuts() ? fCutsRD[GetGlobalIndex(0,iPtBin)] : 1.e6);}
  Float_t GetDCACut(Int_t iPtBin=0) const { return (GetCuts() ? fCutsRD[GetGlobalIndex(11,iPtBin)] : 1.e6);}

 protected:


  ClassDef(AliRDHFCutsDstoKKpi,1);  // class for cuts on AOD reconstructed Ds->KKpi
};

#endif
