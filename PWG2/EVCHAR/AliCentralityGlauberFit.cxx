/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
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

/*   Origin: Alberica Toia, CERN, Alberica.Toia@cern.ch                   */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  class to determine centrality percentiles from 1D distributions          // 
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <TNamed.h>
#include <TH1D.h>
#include <TString.h>
#include <TFile.h>
#include <TMath.h>
#include <TROOT.h>
#include <TH2F.h>
#include <TF1.h>
#include <TNtuple.h>
#include <TStyle.h>
#include <TGraphErrors.h>
#include <vector>
#include <TMinuit.h>
#include <TStopwatch.h>
#include <TRandom.h>
#include "AliCentralityGlauberFit.h"
#include "AliLog.h"

ClassImp(AliCentralityGlauberFit)  
 
//______________________________________________________________________________
AliCentralityGlauberFit::AliCentralityGlauberFit(const char *filename) : 
  fNmu(0),    
  fMulow(0),  
  fMuhigh(0), 
  fNk(0),     
  fKlow(0),   
  fKhigh(0),  
  fNalpha(0), 
  fAlphalow(0),   
  fAlphahigh(0),  
  fNeff(0),       
  fEfflow(0),     
  fEffhigh(0),    
  fRebinFactor(0),
  fMultmin(0),    
  fMultmax(0),    
  fGlauntuple(0), 
  fNpart(0),       
  fNcoll(0),       
  fB(0),           
  fTaa(0),         
  fEffi(0),        
  fhEffi(0),      
  fTempHist(0),   
  fGlauberHist(0), 
  fFastFit(0),      
  fNBD(0),         
  fUseChi2(kTRUE),      
  finrootfile(0),  
  foutrootfilename(0),
  fhistnames(), 
  fPercentXsec(0)
{
  // standard constructor
  // glauber file
  TFile *f = TFile::Open(filename);
  fGlauntuple = (TNtuple*) f->Get("nt_Pb_Pb");
  fGlauntuple->SetBranchAddress("Npart",&fNpart);
  fGlauntuple->SetBranchAddress("Ncoll",&fNcoll);
  fGlauntuple->SetBranchAddress("B",&fB);
  fGlauntuple->SetBranchAddress("tAA",&fTaa);
  
  fNBD = new TF1 ("fNBD", AliCentralityGlauberFit::NBDFunc, 0, 100,2);

}

//--------------------------------------------------------------------------------------------------
void AliCentralityGlauberFit::SetRangeToFit(Float_t fmultmin, Float_t fmultmax)
{
  // Set fit range.

  fMultmin=fmultmin;
  fMultmax=fmultmax;
}

//--------------------------------------------------------------------------------------------------
void AliCentralityGlauberFit::SetGlauberParam(
					      Int_t   Nmu,
					      Float_t mulow,
					      Float_t muhigh,
					      Int_t   Nk,
					      Float_t klow,
					      Float_t khigh,
					      Int_t   Nalpha,
					      Float_t alphalow,
					      Float_t alphahigh,
					      Int_t   Neff,
					      Float_t efflow,
					      Float_t effhigh)
{
  // Set Glauber parameters.

  fNmu=Nmu;
  fMulow=mulow;
  fMuhigh=muhigh;
  fNk=Nk;
  fKlow=klow;
  fKhigh=khigh;
  fNalpha=Nalpha;
  fAlphalow=alphalow;
  fAlphahigh=alphahigh;
  fNeff=Neff;
  fEfflow=efflow;
  fEffhigh=effhigh;
}

//--------------------------------------------------------------------------------------------------
void AliCentralityGlauberFit::MakeFits(TString infilename) 
{
  TH1D *hDATA;
  TH1D *thistGlau; 
  FILE* fTxt = fopen ("parameters.txt","w");
  TFile *outrootfile;
  
  // open inrootfile, outrootfile
  finrootfile  = new TFile(infilename);
  outrootfile = new TFile(foutrootfilename,"RECREATE");
  
  // loop over all distribution names  
  std::vector<TString>::const_iterator hni;
  for(hni=fhistnames.begin(); hni!=fhistnames.end(); hni++) {
    hDATA  = (TH1D*) (finrootfile->Get(*hni)); 
    //TList *list  = (TList*) (inrootfile->Get("coutput1")); 
    //hDATA  = (TH1D*) (list->FindObject(*hni)); 
    hDATA->Rebin(fRebinFactor);
    TH1D *hGLAU = new TH1D("hGLAU","hGLAU",hDATA->GetNbinsX(),0,hDATA->GetNbinsX()*hDATA->GetBinWidth(1));
    Float_t chi2min = 9999999.0;
    Float_t alpha_min=-1;
    Float_t mu_min=-1;
    Float_t k_min=-1;
    Float_t eff_min=-1;
    Float_t alpha, mu, k, eff, chi2;   

    for (int nalpha=0;nalpha<fNalpha;nalpha++) {
      alpha = fAlphalow   + ((float) nalpha ) * (fAlphahigh  - fAlphalow ) / fNalpha;
      mu=0.0;
      for (int nmu=0; nmu<fNmu; nmu++) {
	mu = (fMulow*(1-nalpha*0.05) )  + ((float) nmu ) * (fMuhigh  - fMulow ) / fNmu;
	//mu = mulow   + ((float) nmu ) * (muhigh  - mulow ) / Nmu;
	
	for (int nk=0; nk<fNk; nk++) {
	  k = fKlow  + ((float) nk ) * (fKhigh  - fKlow ) / fNk;
	  
	  for (int neff=0; neff<fNeff; neff++) {
	    eff = fEfflow + ((float) neff) * (fEffhigh - fEfflow) / fNeff;
	    
	    thistGlau = GlauberHisto(mu,k,eff,alpha,hDATA,kFALSE);
	    chi2 = CalculateChi2(hDATA,thistGlau,eff);
	    fprintf(fTxt, "%3.3f %3.3f %3.3f %3.3f %3.3f\n",(float) eff, (float) mu, (float) k, (float) alpha, chi2);

	    if ( chi2 < chi2min ) {
	      chi2min = chi2;
	      alpha_min=alpha;
	      mu_min=mu;
	      k_min=k;
	      eff_min=eff;
	    }
	  }
	}
      }
    }

    thistGlau = GlauberHisto(mu_min,k_min,eff_min,alpha_min,hDATA,kTRUE);
    hGLAU = (TH1D *) thistGlau->Clone("hGLAU");
    //hGLAU->SetName( ((TString)hDATA->GetName()).Append(Form("_GLAU_%d_%d_%d_%d",(int)(100*mu_min),(int)(100*k_min),(int)(100*alpha_min),(int)(100*eff_min))));
    hGLAU->SetName( ((TString)hDATA->GetName()).Append(Form("_GLAU")));
    hGLAU->SetTitle( ((TString)hDATA->GetName()).Append(Form("_GLAU_%.3f_%.3f_%.3f_%.3f",mu_min,k_min,alpha_min,eff_min)));

    float mcintegral = hGLAU->Integral(hDATA->FindBin(fMultmin),hGLAU->GetNbinsX());
    float scale = (hDATA->Integral(hDATA->FindBin(fMultmin),hDATA->GetNbinsX())/mcintegral) * ((float) eff_min);
    hGLAU->Scale(scale);

    fhEffi = GetTriggerEfficiencyFunction(hDATA, hGLAU);
    SaveHisto(hDATA,hGLAU,fhEffi,outrootfile);
    fclose (fTxt);

    std::cout << "chi2 min is " << chi2min << std::endl;
    std::cout << "fitted " << hGLAU->Integral(hGLAU->FindBin(fMultmin),
                                              hGLAU->FindBin(fMultmax))/hGLAU->Integral() 
              << " of the total cross section" << std::endl; 
    fTempHist=hDATA;
    fGlauberHist=hGLAU;
  }

  // close inrootfile, outrootfile
  finrootfile->Close();
  outrootfile->Close();
}

//--------------------------------------------------------------------------------------------------
void AliCentralityGlauberFit::MakeFitsMinuitNBD(TString infilename) 
{
  // Make fits using Minut.

  TH1D *hDATA;
  TH1D *thistGlau; 
  TFile *outrootfile;
  
  // open inrootfile, outrootfile
  finrootfile  = new TFile(infilename);
  outrootfile = new TFile(foutrootfilename,"RECREATE");
  
  // loop over all distribution names  
  std::vector<TString>::const_iterator hni;
  for(hni=fhistnames.begin(); hni!=fhistnames.end(); hni++) {
    fFastFit=0;
    fUseChi2 = 1;
    hDATA  = (TH1D*) (finrootfile->Get(*hni)); 
    hDATA->Rebin(fRebinFactor);
    fTempHist=hDATA;
    TH1D *hGLAU = new TH1D("hGLAU","hGLAU",hDATA->GetNbinsX(),0,hDATA->GetNbinsX()*hDATA->GetBinWidth(1));
    hGLAU->Sumw2();
    // Minimize here
    if(gMinuit) delete gMinuit;
    new TMinuit(3);
    gMinuit->mncler();
    gMinuit->SetFCN(AliCentralityGlauberFit::MinuitFcnNBD);
    gMinuit->SetObjectFit(this); 

    Double_t arglist[2]={0};
    Int_t ierflg;
    if (fUseChi2) arglist[0] = 1; // should be 1 if you want to minimize chi2
    else arglist[0] = 0.5; // should be 0.5 for ll
    gMinuit->mnexcm("SET ERR",arglist, 1, ierflg);

  // // Change verbosity
  // gMinuit->Command((TString("SET PRINTOUT ")+long(fVerbosity)).Data());

    // Set parameters
    // start from the middle of the allowed range, as a guess
    // gMinuit->mnparm(0,"alpha", (alphalow+alphahigh)/2, 1, alphalow, alphahigh, ierflg);
    // gMinuit->mnparm(1,"mu"   , (mulow   +muhigh   )/2, 1, mulow   , muhigh   , ierflg);
    // gMinuit->mnparm(2,"k"    , (klow    +khigh    )/2, 1, klow    , khigh    , ierflg);
    // gMinuit->mnparm(3,"eff"  , (efflow  +effhigh  )/2, 1, efflow  , effhigh   , ierflg);

    // NBD with limits
    // gMinuit->mnparm(0,"alpha", 1.104, 0.1, alphalow, alphahigh, ierflg);
    // gMinuit->mnparm(1,"mu"   , 65,  1,    mulow   , muhigh   , ierflg);
    // gMinuit->mnparm(2,"k"    , 0.41, 0.1, klow    , khigh    , ierflg);
    // //    gMinuit->mnparm(3,"eff"  , 1,   1, 0.9       , 1        , ierflg);
    // gMinuit->mnparm(3,"eff"  , 0.95, 0.1, efflow  , effhigh   , ierflg);

    // // NBD, no limits
    // gMinuit->mnparm(0,"alpha", 1.104, 0.1, 0,0, ierflg);
    // gMinuit->mnparm(1,"mu"   , 65,  1,     0,0, ierflg);
    // gMinuit->mnparm(2,"k"    , 0.41, 0.1,  0,0, ierflg);
    // gMinuit->mnparm(3,"eff"  , 0.95, 0.1,  0,0 , ierflg);

    if (!fFastFit) {
    // // NBD, Npart**alpha     
      //gMinuit->mnparm(0,"alpha", 1.16, 0.1, 0,0, ierflg);
      //gMinuit->mnparm(1,"mu"   , 27.55,   1,    0,0, ierflg);
      //gMinuit->mnparm(2,"k"    , 0.64, 0.1,  0,0, ierflg);
      //gMinuit->mnparm(3,"eff"  , 0.97, 0.1,  0,0 , ierflg);
      // V0 non rescaled
      // // NBD, alpha Npart * (1-alpha) Ncoll     
      gMinuit->mnparm(0,"alpha", 0.832, 0.1, 0,0, ierflg);
      gMinuit->mnparm(1,"mu"   , 29,   1,    0,0, ierflg);
      gMinuit->mnparm(2,"k"    , 1.4, 0.1,  0,0, ierflg);
      gMinuit->mnparm(3,"eff"  , 0.991, 0.1,  0,0 , ierflg);
      // SPD
      //gMinuit->mnparm(0,"alpha", 0.820, 0.1, 0.7,1, ierflg);
      //gMinuit->mnparm(1,"mu"   , 8.267,   1,  1,100, ierflg);
      //gMinuit->mnparm(2,"k"    , 0.530, 0.1,  0,0, ierflg);
      //gMinuit->mnparm(3,"eff"  , 0.990, 0.1,  0,0 , ierflg);      
    } else if (fFastFit == 2) {
      //gaussian
      //0.859136, 30.2057, 3.87565, 0.989747
      gMinuit->mnparm(0,"alpha", 0.59, 0.1, 0,1, ierflg);
      gMinuit->mnparm(1,"mu"   , 30.2,  1,     0,0, ierflg);
      gMinuit->mnparm(2,"k"    , 3.875, 0.1,  0,0, ierflg);
      //    gMinuit->mnparm(3,"eff"  , 1,   1, 0.9       , 1        , ierflg);
      gMinuit->mnparm(3,"eff"  , 0.98947, 0.1,  0,0, ierflg);
    }

    // Call migrad
    // arglist[0] = 100000; // max calls
    // arglist[1] = 0.1; // tolerance    
    // arglist[0] = 50; // max calls
    // arglist[1] = 1; // tolerance    
    // gMinuit->mnexcm("MIGrad",arglist, 2, ierflg);
    arglist[0] = 100; // max calls
    arglist[1] = 0.1; // tolerance    
    gMinuit->mnexcm("SIMPLEX",arglist, 2, ierflg);
    // //    fFastFit = 2;
    arglist[0] = 1000; // max calls
    arglist[1] = 0.1; // tolerance    
    gMinuit->mnexcm("MIGrad",arglist, 2, ierflg);
    //    gMinuit->mnexcm("IMPROVE",arglist, 0, ierflg);

    if (ierflg != 0) {
      AliWarning("Abnormal termination of minimization.");
      //    exit(0);
    }
    
    // if(opt.Contains("M")) {
    //   gMinuit->mnexcm("IMPROVE",arglist, 0, ierflg);
    // }
    // if(opt.Contains("E")) {
    // gMinuit->mnexcm("HESSE",arglist, 0, ierflg);
    // gMinuit->mnexcm("MINOS",arglist, 0, ierflg);
    // }

    // ______________________ Get chi2 and Fit Status __________
    Double_t amin,edm,errdef;
    int      nvpar, nparx, icstat;
    
    
    gMinuit->mnstat(amin,edm,errdef,nvpar,nparx,icstat);
    Double_t chi2min = amin;
    std::cout << "Fit status " << icstat << std::endl;

    Double_t alpha_min, mu_min, k_min, eff_min;
    Double_t alpha_mine, mu_mine, k_mine, eff_mine;
    gMinuit->GetParameter(0, alpha_min , alpha_mine );
    gMinuit->GetParameter(1, mu_min    , mu_mine    );
    gMinuit->GetParameter(2, k_min     , k_mine     );
    gMinuit->GetParameter(3, eff_min   , eff_mine   );

    // Print some infos
    std::cout << "chi2 min is " << chi2min << ", " << alpha_min << ", "<< mu_min<< ", "  << k_min << ", " <<  eff_min << std::endl;

    thistGlau = GlauberHisto(mu_min,k_min,eff_min,alpha_min,hDATA,kTRUE);
    hGLAU = (TH1D *) thistGlau->Clone("hGLAU");
    hGLAU->SetName( ((TString)hDATA->GetName()).Append(Form("_GLAU")));
    hGLAU->SetTitle( ((TString)hDATA->GetName()).Append(Form("_GLAU_%.3f_%.3f_%.3f_%.3f",mu_min,k_min,alpha_min,eff_min)));

    
    std::cout << "fitted " << hGLAU->Integral(hGLAU->FindBin(fMultmin),
                                              hGLAU->FindBin(fMultmax))/hGLAU->Integral() 
              << " of the total cross section" << std::endl; 
    //    eff_min=1;

    // Save histogram and ntuple
    //    fFastFit=kFALSE; // Use a better aprox to get the final histo

    // float mcintegral = hGLAU->Integral(hDATA->FindBin(multmin),hGLAU->GetNbinsX());
    // float scale = (hDATA->Integral(hDATA->FindBin(multmin),hDATA->GetNbinsX())/mcintegral) * ((float) eff_min);
    // hGLAU->Scale(scale);

    std::cout << "Chi2 final" << CalculateChi2(hDATA,hGLAU,eff_min) << std::endl;

    fhEffi = GetTriggerEfficiencyFunction(hDATA, hGLAU);
    SaveHisto(hDATA,hGLAU,fhEffi,outrootfile);
    fGlauberHist=hGLAU;
  }
  // close inrootfile, outrootfile
  finrootfile->Close();
  outrootfile->Close();
}

//--------------------------------------------------------------------------------------------------
TH1D *AliCentralityGlauberFit::GlauberHisto(Float_t mu, Float_t k, Float_t eff, Float_t alpha, 
                                             TH1D *hDATA, Bool_t save) 
{
  // Get Glauber histogram

  eff=eff;
  static TStopwatch sw;

  TH1D *hSample = fFastFit != 2 ? NBDhist(mu,k) : 0;
  //  fNBD->SetParameters(mu,k);
  static TH1D *h1 = (TH1D*)hDATA->Clone();
  h1->Reset();
  //  h1->SetName(Form("fit_%.3f_%.3f_%.3f_%.3f",mu,k,eff,alpha));
  TFile *outFile = NULL;
  TNtuple *ntuple = NULL;
 
  if (save) {
    outFile = TFile::Open("pippo.root", "RECREATE");
    ntuple = new TNtuple("gnt", "Glauber ntuple", "Npart:Ncoll:B:tAA:ntot");
  } 
  

  Int_t nents = 100000;//glau_ntuple->GetEntries();
  //if(fFastFit > 0 && fFastFit < 2) nents = 10000;
  //  Int_t nents = 100;//glau_ntuple->GetEntries();
  for (Int_t i=0;i<nents;++i) {
    fGlauntuple->GetEntry(i);
    //Int_t n = TMath::Nint(TMath::Power(Npart,alpha));
    //Int_t n = TMath::Nint(TMath::Power(Ncoll,alpha));
    Int_t n = alpha * fNpart + (1-alpha) * fNcoll;
    Int_t ntot=0;
    //    ntot=n*fNBD->GetRandom();    
    if (fFastFit == 1) {
      // NBD
      ntot = n*hSample->GetRandom();
    }
    else if (fFastFit == 2) {
      // Gaussian
      Double_t sigma = k*TMath::Sqrt(n*mu);  
      ntot = gRandom->Gaus(n*mu,sigma);
    }
    else {
      for(Int_t j = 0; j<n; ++j) 
	ntot+=hSample->GetRandom();
	//	ntot+=fNBD->GetRandom();
    }
    h1->Fill(ntot);
    
    if (save) 
      ntuple->Fill(fNpart,fNcoll,fB,fTaa,ntot);
   
  }

  if (save) {
    ntuple->Write();
    outFile->Close();
  }
  
  if (fFastFit != 2) delete hSample;
  return h1;
  
}

//--------------------------------------------------------------------------------------------------

Float_t AliCentralityGlauberFit::CalculateChi2(TH1D *hDATA, TH1D *thistGlau, Float_t eff) {
  // note that for different values of neff the mc distribution is identical, just re-scaled below
  // normalize the two histogram
  // scale MC up but leave data alone - that preserves error bars !!!!
  
  int lowchibin =   hDATA->FindBin(fMultmin);
  int highchibin =  hDATA->FindBin(fMultmax);

  float mcintegral = thistGlau->Integral(lowchibin,highchibin);
  //  float scale = (hDATA->Integral(lowchibin,highchibin)/mcintegral) * ((float) eff);
  float scale = (hDATA->Integral(lowchibin,highchibin)/mcintegral) * ((float) eff);
  thistGlau->Scale(scale);

  // FIXME: Kolmogorov
  //return hDATA->KolmogorovTest(thistGlau,"M");

  // // calculate the chi2 between MC and real data over some range ????
  if (fUseChi2) {
    float chi2 = 0.0;
    for (int i=lowchibin; i<=highchibin; i++) {
      if (hDATA->GetBinContent(i) < 1.0) continue;
      float diff = pow( (thistGlau->GetBinContent(i) - hDATA->GetBinContent(i)) , 2);
      diff = diff / (pow(hDATA->GetBinError(i) , 2)+pow(thistGlau->GetBinError(i) , 2)); // FIXME squared distance if commented
      chi2 += diff;
    }
    chi2 = chi2 / (highchibin - lowchibin + 1);
    return chi2;
  }
  // "-2 log likelihood ratio(mu;n) = 2[(mu - n) + n ln(n/mu)]"
  else {
    std::cout << "LL" << std::endl;
    

    float ll = 0.0;
    for (int i=lowchibin; i<=highchibin; i++) {
      //      if (thistGlau->GetBinContent(i) < 1) continue; 
      //      if (hDATA->GetBinContent(i) < 1) continue; 
      //    cout << ll << " " << thistGlau->GetBinContent(i) <<" " <<  hDATA->GetBinContent(i) << endl;
 
      Float_t data = hDATA    ->GetBinContent(i);
      Float_t mc   = thistGlau->GetBinContent(i);
      Int_t idata = TMath::Nint(data);
      if (mc < 1.e-9) mc = 1.e-9;
      Float_t fsub = - mc + idata * TMath::Log(mc);
      Float_t fobs = 0;
      //Int_t imc = TMath::Nint(mc);
      if (idata > 0) {
	for(Int_t istep = 0; istep < idata; istep++){
	  if (istep > 1) 
	    fobs += TMath::Log(istep);
	}
      }
      //      cout << mc << " " << data << " " << fsub << " " << fobs << endl;
      fsub -= fobs;
      ll -=  fsub ;
    }
    return 2*ll;
  }

}

//--------------------------------------------------------------------------------------------------

TH1D * AliCentralityGlauberFit::GetTriggerEfficiencyFunction(TH1D *hist1, TH1D *hist2) 
{
  // Get efficiency.

  fhEffi = (TH1D*)hist1->Clone("heffi");
  fhEffi->Divide(hist2);
  return fhEffi;
}

//--------------------------------------------------------------------------------------------------

Float_t AliCentralityGlauberFit::GetTriggerEfficiencyIntegral(TH1D *hist1, TH1D *hist2) 
{
  // Get eff integral.
  fEffi = hist1->Integral()/hist2->Integral();
  return fEffi;
}

//--------------------------------------------------------------------------------------------------

void AliCentralityGlauberFit::SaveHisto(TH1D *hist1, TH1D *hist2, TH1D *heffi, TFile *outrootfile) {
  outrootfile->cd();
  hist1->Write();
  hist2->Write();
  heffi->Write();
}

//--------------------------------------------------------------------------------------------------

Double_t AliCentralityGlauberFit::NBD(Int_t n, Double_t mu, Double_t k)
{
  // Compute NBD.
  Double_t ret = exp( lgamma(n+k) - lgamma(k) - lgamma(n+1) ) * TMath::Power(mu/(mu+k),n) * TMath::Power(1-mu/(mu+k),k);
  return ret;
}

//--------------------------------------------------------------------------------------------------

TH1D *AliCentralityGlauberFit::NBDhist(Double_t mu, Double_t k)
{
  TH1D *h = new TH1D("htemp","",100,-0.5,299.5);
  h->SetName(Form("nbd_%f_%f",mu,k));
  h->SetDirectory(0);
  for (Int_t i=0;i<300;++i) {
    Double_t val = NBD(i,mu,k);
    if (val>1e-20) h->Fill(i,val);
  }
  return h;
}

void AliCentralityGlauberFit::MinuitFcnNBD(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag)
{
  // FCN for minuit
  Double_t alpha = par[0];
  Double_t mu    = par[1];
  Double_t k     = par[2];
  Double_t eff   = par[3];
  //Double_t eff   = 1;//par[3];
  if (0) { //avoid warning
    gin=gin;
    npar=npar;
    iflag=iflag;
  }
  AliCentralityGlauberFit * obj = (AliCentralityGlauberFit *) gMinuit->GetObjectFit();

  // static TStopwatch sw;
  // sw.Start();
  TH1D * thistGlau = obj->GlauberHisto(mu,k,eff,alpha,obj->GetTempHist(),kFALSE);
  f = obj->CalculateChi2(obj->GetTempHist(),thistGlau,eff);
  // sw.Stop();
  // sw.Print();
  Printf("%f - %f - %f - %f - %f \n",f,alpha,mu,k,eff);
}

Double_t AliCentralityGlauberFit::NBDFunc(Double_t * x, Double_t *par) 
{
  // TF1  interface
  Double_t mu = par[0];
  Double_t k  = par[1];
  Double_t n  = x[0];
  Double_t ret = exp( lgamma(n+k) - lgamma(k) - lgamma(n+1) ) * TMath::Power(mu/(mu+k),n) * TMath::Power(1-mu/(mu+k),k);
  return ret;
}
