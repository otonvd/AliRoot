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
/* $Id$ */

//_________________________________________________________________________
// Base Class for PHOS description:
//   PHOS consists of a PbWO4 calorimeter (EMCA) and a gazeous charged 
//    particles detector (CPV or PPSD).
//   The only provided method here is CreateMaterials, 
//    which defines the materials common to all PHOS versions.   
// 
//*-- Author: Laurent Aphecetche & Yves Schutz (SUBATECH) 
//////////////////////////////////////////////////////////////////////////////


// --- ROOT system ---
class TFile;
#include "TROOT.h"
#include "TTree.h"
#include "TFolder.h" 

// --- Standard library ---

#include <strstream.h>
// --- AliRoot header files ---

#include "AliPHOS.h"
#include "AliMC.h"
#include "AliRun.h"
#include "AliMagF.h"
#include "AliPHOSGeometry.h"
#include "AliPHOSQAChecker.h" 

ClassImp(AliPHOS)
//____________________________________________________________________________
AliPHOS:: AliPHOS() : AliDetector()
{
  // Create folder and task hierarchy
  fName="PHOS";
  CreatePHOSWhiteBoard();

}

//____________________________________________________________________________
AliPHOS::AliPHOS(const char* name, const char* title): AliDetector(name, title) 
{
  // Create folder and task hierarchy
  CreatePHOSWhiteBoard(); 
}

//____________________________________________________________________________
void AliPHOS::CreatePHOSWhiteBoard()
{
  // create the ALICE TFolder
  // create the ALICE TTasks
  //  add the Alice QA TTAsks 
  // create the ALICE main TFolder
  //  add the Alice QA Alarms
  // this should be done of course by AliRun
  //==================== BEG TO BE DONE BY AliRUN ===========================
  TFolder *alice = gROOT->GetRootFolder()->AddFolder("YSAlice","Alice Folder") ;  
  gROOT->GetListOfBrowsables()->Add(alice, "YSAlice") ;

  TFolder * aliceF  = alice->AddFolder("WhiteBoard", "Alice memory Folder") ; 
  //  make it the owner of the objects that it contains
  aliceF->SetOwner() ;
  // geometry folder 
  TFolder * geomF = aliceF->AddFolder("Geometry", "Geometry objects") ; 
  // alarms folder
  TFolder * alarmsF = aliceF->AddFolder("QAAlarms", "Alarms raised by QA check") ; 
  // Hits folder
  TFolder * hitsF = aliceF->AddFolder("Hits", "Hits") ; 
  // SDigits folder
  TFolder * sdigitsF = aliceF->AddFolder("SDigits", "Summable Digits") ; 
  // Digits folder
  TFolder * digitsF = aliceF->AddFolder("Digits", "Digits") ; 
  // RecPoints folder
  TFolder * rpointsF = aliceF->AddFolder("RecPoints", "RecPoints") ; 
  // TrackSegments folder
  TFolder * tsF = aliceF->AddFolder("TrackSegments", "TrackSegments") ; 
  // RecParticles folder
  TFolder * rparticlesF = aliceF->AddFolder("RecParticles", "RecParticles") ; 
  //  make it the owner of the objects that it contains
  alarmsF->SetOwner() ;
  hitsF->SetOwner() ;
  sdigitsF->SetOwner() ;
  digitsF->SetOwner() ;
  rpointsF->SetOwner() ;
  tsF->SetOwner() ;
  rparticlesF->SetOwner() ;

  // Tasks folder
  TFolder * aliceT  = alice->AddFolder("tasks", "Alice tasks Folder") ; 
  //  make it the owner of the objects that it contains
  aliceT->SetOwner() ;

  TTask * aliceQA = new TTask("QA", "Alice QA tasks") ;
  aliceT->Add(aliceQA); 
 
  TTask * aliceSD = new TTask("SDigitizer", "Alice SDigitizer") ;
  aliceT->Add(aliceSD); 

  TTask * aliceDi = new TTask("Digitizer", "Alice Digitizer") ;
  aliceT->Add(aliceDi); 

  TTask * aliceRe = new TTask("Reconstructioner", "Alice Reconstructioner") ;
  aliceT->Add(aliceRe); 

  //==================== END TO BE DONE BY AliRUN ===========================

  // =================== Creating PHOS related folders
  char * tempo = new char[80] ; 

  // creates the PHOSQA (QAChecker knows how to add itself in the tasks list)
  sprintf(tempo, "%sCheckers container",GetName() ) ; 
  fQATask = new AliPHOSQAChecker(GetName(), tempo);  

  // creates the PHOS SDigitizer and adds it to alice main SDigitizer task 
  sprintf(tempo, "%sSDigitizers container",GetName() ) ; 
  TTask * sdT = new TTask(GetName(), tempo);   
  aliceSD->Add(sdT) ; 

  // creates the PHOS Digitizer and adds it to alice main Digitizer task 
  sprintf(tempo, "%sDigitizers container",GetName() ) ; 
  TTask * dT = new TTask(GetName(), tempo);   
  aliceDi->Add(dT) ; 

  // creates the PHOS reconstructioner and adds it to alice main Reconstructioner task 
  sprintf(tempo, "%s Reconstructioner container",GetName() ) ; 
  TTask * reT = new TTask(GetName(), tempo); 
  aliceRe->Add(reT) ; 

  // creates the PHOS clusterizer, tracksegment maker and PID and adds it to the PHOS Reconstructioner task

  delete tempo ;  

  // creates the PHOS geometry  folder
  geomF->AddFolder("PHOS", "Geometry for PHOS") ; 
  // creates the PHOSQA alarm folder
  alarmsF->AddFolder("PHOS", "QA alarms from PHOS") ; 
  // creates the PHOS Hits folder
  hitsF->AddFolder("PHOS", "Hits for PHOS") ; 
  // creates the PHOS Summable Digits folder
  sdigitsF->AddFolder("PHOS", "Summable Digits for PHOS") ; 
  // creates the PHOS Digits folder
  digitsF->AddFolder("PHOS", "Digits for PHOS") ; 
  // creates the PHOS RecPoints folder
  TFolder * prpF = rpointsF->AddFolder("PHOS", "RecPoints for PHOS") ;
  // creates the PHOS EMC RecPoints folder
  prpF->AddFolder("emc", "EMC RecPoints for PHOS") ;
  // creates the PHOS CPV RecPoints folder
  prpF->AddFolder("cpv", "CPV RecPoints for PHOS") ;
  
  // creates the PHOS TrackSegments folder
  tsF->AddFolder("PHOS", "Track Segments for PHOS") ; 
  // creates the PHOS RecParticles folder
  rparticlesF->AddFolder("PHOS", "RecParticles for PHOS") ;
 
}
//____________________________________________________________________________
AliPHOS::~AliPHOS() 
{  
  // remove the alice folder and alice QA task that PHOS creates instead of AliRun
  
  // remove and delete the PHOS QA tasks
  TTask * aliceQA = (TTask*)gROOT->FindObjectAny("YSAlice/tasks/QA") ; 
  fQATask->GetListOfTasks()->Delete() ;
  aliceQA->GetListOfTasks()->Remove(fQATask) ; 
  delete fQATask ;  
  
  // remove and delete aliceQA (should be done by AliRun) 
}

//____________________________________________________________________________
void AliPHOS::CreateMaterials()
{
  // Definitions of materials to build PHOS and associated tracking media.
  // media number in idtmed are 699 to 798.

  // --- The PbWO4 crystals ---
  Float_t aX[3] = {207.19, 183.85, 16.0} ;
  Float_t zX[3] = {82.0, 74.0, 8.0} ;
  Float_t wX[3] = {1.0, 1.0, 4.0} ;
  Float_t dX = 8.28 ;

  AliMixture(0, "PbWO4$", aX, zX, dX, -3, wX) ;


  // --- The polysterene scintillator (CH) ---
  Float_t aP[2] = {12.011, 1.00794} ;
  Float_t zP[2] = {6.0, 1.0} ;
  Float_t wP[2] = {1.0, 1.0} ;
  Float_t dP = 1.032 ;

  AliMixture(1, "Polystyrene$", aP, zP, dP, -2, wP) ;

  // --- Aluminium ---
  AliMaterial(2, "Al$", 26.98, 13., 2.7, 8.9, 999., 0, 0) ;
  // ---         Absorption length is ignored ^

 // --- Tyvek (CnH2n) ---
  Float_t aT[2] = {12.011, 1.00794} ;
  Float_t zT[2] = {6.0, 1.0} ;
  Float_t wT[2] = {1.0, 2.0} ;
  Float_t dT = 0.331 ;

  AliMixture(3, "Tyvek$", aT, zT, dT, -2, wT) ;

  // --- Polystyrene foam ---
  Float_t aF[2] = {12.011, 1.00794} ;
  Float_t zF[2] = {6.0, 1.0} ;
  Float_t wF[2] = {1.0, 1.0} ;
  Float_t dF = 0.12 ;

  AliMixture(4, "Foam$", aF, zF, dF, -2, wF) ;

 // --- Titanium ---
  Float_t aTIT[3] = {47.88, 26.98, 54.94} ;
  Float_t zTIT[3] = {22.0, 13.0, 25.0} ;
  Float_t wTIT[3] = {69.0, 6.0, 1.0} ;
  Float_t dTIT = 4.5 ;

  AliMixture(5, "Titanium$", aTIT, zTIT, dTIT, -3, wTIT);

 // --- Silicon ---
  AliMaterial(6, "Si$", 28.0855, 14., 2.33, 9.36, 42.3, 0, 0) ;



  // --- Foam thermo insulation ---
  Float_t aTI[2] = {12.011, 1.00794} ;
  Float_t zTI[2] = {6.0, 1.0} ;
  Float_t wTI[2] = {1.0, 1.0} ;
  Float_t dTI = 0.04 ;

  AliMixture(7, "Thermo Insul.$", aTI, zTI, dTI, -2, wTI) ;

  // --- Textolitn ---
  Float_t aTX[4] = {16.0, 28.09, 12.011, 1.00794} ;
  Float_t zTX[4] = {8.0, 14.0, 6.0, 1.0} ;
  Float_t wTX[4] = {292.0, 68.0, 462.0, 736.0} ;
  Float_t dTX    = 1.75 ;

  AliMixture(8, "Textolit$", aTX, zTX, dTX, -4, wTX) ;

  //--- FR4  ---
  Float_t aFR[3] = {28.0855, 15.9994, 17.749} ; 
  Float_t zFR[3] = {14., 8., 8.875} ; 
  Float_t wFR[3] = {.28, .32, .4} ;
  Float_t dFR = 1.8 ; 

  AliMixture(9, "FR4$", aFR, zFR, dFR, -3, wFR) ;

  // --- The Composite Material for  micromegas (so far polyetylene) ---                                       
  Float_t aCM[2] = {12.01, 1.} ; 
  Float_t zCM[2] = {6., 1.} ; 
  Float_t wCM[2] = {1., 2.} ; 
  Float_t dCM = 0.935 ; 

  AliMixture(10, "Compo Mat$", aCM, zCM, dCM, -2, wCM) ;

  // --- Copper ---                                                                    
  AliMaterial(11, "Cu$", 63.546, 29, 8.96, 1.43, 14.8, 0, 0) ;
 
  // --- G10 : Printed Circuit material ---                                                  
  Float_t aG10[4] = { 12., 1., 16., 28.} ;
  Float_t zG10[4] = { 6., 1., 8., 14.} ;
  Float_t wG10[4] = { .259, .288, .248, .205} ;
  Float_t dG10  = 1.7 ;
  
  AliMixture(12, "G10$", aG10, zG10, dG10, -4, wG10);

  // --- Lead ---                                                                     
  AliMaterial(13, "Pb$", 207.2, 82, 11.35, 0.56, 0., 0, 0) ;

 // --- The gas mixture ---                                                                
 // Co2
  Float_t aCO[2] = {12.0, 16.0} ; 
  Float_t zCO[2] = {6.0, 8.0} ; 
  Float_t wCO[2] = {1.0, 2.0} ; 
  Float_t dCO = 0.001977 ; 

  AliMixture(14, "CO2$", aCO, zCO, dCO, -2, wCO);

 // Ar
  Float_t dAr = 0.001782 ; 
  AliMaterial(15, "Ar$", 39.948, 18.0, dAr, 14.0, 0., 0, 0) ;   
 
 // ArCo2
  Char_t namate[21];
  Float_t aGM[2] ; 
  Float_t zGM[2] ; 
  Float_t wGM[2] ; 
  Float_t dGM ; 

  Float_t absL, radL, density ;
  Float_t buf[1] ;
  Int_t nbuf ;

  gMC->Gfmate((*fIdmate)[15], namate, aGM[0], zGM[0], density, radL, absL, buf, nbuf) ; // Get properties of Ar 
  gMC->Gfmate((*fIdmate)[14], namate, aGM[1], zGM[1], density, radL, absL, buf, nbuf) ; // Get properties of CO2 


  // Create gas mixture 

  Float_t arContent    = 0.80 ;  // Ar-content of the Ar/CO2-mixture (80% / 20%) 
 
  wGM[0] = arContent;
  wGM[1] = 1. - arContent ;
  dGM    = wGM[0] * dAr + wGM[1] * dCO;

  AliMixture(16, "ArCO2$", aGM, zGM, dGM,  2, wGM) ;

  // --- Stainless steel (let it be pure iron) ---
  AliMaterial(17, "Steel$", 55.845, 26, 7.87, 1.76, 0., 0, 0) ;


  // --- Fiberglass ---
  Float_t aFG[4] = {16.0, 28.09, 12.011, 1.00794} ;
  Float_t zFG[4] = {8.0, 14.0, 6.0, 1.0} ;
  Float_t wFG[4] = {292.0, 68.0, 462.0, 736.0} ;
  Float_t dFG    = 1.9 ;

  AliMixture(18, "Fibergla$", aFG, zFG, dFG, -4, wFG) ;

  // --- Cables in Air box  ---
  // SERVICES

  Float_t aCA[4] = { 1.,12.,55.8,63.5 };
  Float_t zCA[4] = { 1.,6.,26.,29. }; 
  Float_t wCA[4] = { .014,.086,.42,.48 };
  Float_t dCA    = 0.8 ;  //this density is raw estimation, if you know better - correct

  AliMixture(19, "Cables  $", aCA, zCA, dCA, -4, wCA) ;



 
  // --- Air ---
  AliMaterial(99, "Air$", 14.61, 7.3, 0.001205, 30420., 67500., 0, 0) ;
  
 
  // DEFINITION OF THE TRACKING MEDIA

  // for PHOS: idtmed[699->798] equivalent to fIdtmed[0->100]
  Int_t * idtmed = fIdtmed->GetArray() - 699 ; 
  Int_t   isxfld = gAlice->Field()->Integ() ;
  Float_t sxmgmx = gAlice->Field()->Max() ;

  // The scintillator of the calorimeter made of PBW04                              -> idtmed[699]
  AliMedium(0, "PHOS Xtal    $", 0, 1,
	    isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;

  // The scintillator of the CPV made of Polystyrene scintillator                   -> idtmed[700]
  AliMedium(1, "CPV scint.   $", 1, 1,
	    isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;

  // Various Aluminium parts made of Al                                             -> idtmed[701]
  AliMedium(2, "Al parts     $", 2, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.001, 0.001, 0, 0) ;

  // The Tywek which wraps the calorimeter crystals                                 -> idtmed[702]
  AliMedium(3, "Tyvek wrapper$", 3, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.001, 0.001, 0, 0) ;

  // The Polystyrene foam around the calorimeter module                             -> idtmed[703]
  AliMedium(4, "Polyst. foam $", 4, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;

  // The Titanium around the calorimeter crystal                                    -> idtmed[704]
  AliMedium(5, "Titan. cover $", 5, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.0001, 0.0001, 0, 0) ;

  // The Silicon of the pin diode to read out the calorimeter crystal               -> idtmed[705] 
 AliMedium(6, "Si PIN       $", 6, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.01, 0.01, 0, 0) ;

 // The thermo insulating material of the box which contains the calorimeter module -> idtmed[706]
  AliMedium(7, "Thermo Insul.$", 7, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;

  // The Textolit which makes up the box which contains the calorimeter module      -> idtmed[707]
  AliMedium(8, "Textolit     $", 8, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;

  // FR4: The Plastic which makes up the frame of micromegas                        -> idtmed[708]
  AliMedium(9, "FR4 $", 9, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.0001, 0, 0) ; 


  // The Composite Material for  micromegas                                         -> idtmed[709]
  AliMedium(10, "CompoMat   $", 10, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;

  // Copper                                                                         -> idtmed[710]
  AliMedium(11, "Copper     $", 11, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.0001, 0, 0) ;

  // G10: Printed Circuit material                                                  -> idtmed[711]
 
  AliMedium(12, "G10        $", 12, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.01, 0, 0) ;

  // The Lead                                                                       -> idtmed[712]
 
  AliMedium(13, "Lead      $", 13, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;

  // The gas mixture: ArCo2                                                         -> idtmed[715]
 
  AliMedium(16, "ArCo2      $", 16, 1,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.01, 0, 0) ;
 
  // Stainless steel                                                                -> idtmed[716]
  AliMedium(17, "Steel     $", 17, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.0001, 0, 0) ;

  // Fibergalss                                                                     -> idtmed[717]
  AliMedium(18, "Fiberglass$", 18, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;

  // Cables in air                                                                  -> idtmed[718]
  AliMedium(19, "Cables    $", 19, 0,
	     isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;

  // Air                                                                            -> idtmed[798] 
  AliMedium(99, "Air          $", 99, 0,
	     isxfld, sxmgmx, 10.0, 1.0, 0.1, 0.1, 10.0, 0, 0) ;

  // --- Set decent energy thresholds for gamma and electron tracking

  // Tracking threshold for photons and electrons in the scintillator crystal 
  gMC->Gstpar(idtmed[699], "CUTGAM",0.5E-4) ; 
  gMC->Gstpar(idtmed[699], "CUTELE",1.0E-4) ;
 
  // --- Generate explicitly delta rays in the titan cover ---
  gMC->Gstpar(idtmed[704], "LOSS",3.) ;
  gMC->Gstpar(idtmed[704], "DRAY",1.) ;
  // --- and in aluminium parts ---
  gMC->Gstpar(idtmed[701], "LOSS",3.) ;
  gMC->Gstpar(idtmed[701], "DRAY",1.) ;
  // --- and in PIN diode
  gMC->Gstpar(idtmed[705], "LOSS",3) ;
  gMC->Gstpar(idtmed[705], "DRAY",1) ;
  // --- and in the passive convertor
  gMC->Gstpar(idtmed[712], "LOSS",3) ;
  gMC->Gstpar(idtmed[712], "DRAY",1) ;
  // Tracking threshold for photons and electrons in the gas ArC02 
  gMC->Gstpar(idtmed[715], "CUTGAM",1.E-5) ; 
  gMC->Gstpar(idtmed[715], "CUTELE",1.E-5) ;
  gMC->Gstpar(idtmed[715], "CUTNEU",1.E-5) ;
  gMC->Gstpar(idtmed[715], "CUTHAD",1.E-5) ;
  gMC->Gstpar(idtmed[715], "CUTMUO",1.E-5) ;
  gMC->Gstpar(idtmed[715], "BCUTE",1.E-5) ;
  gMC->Gstpar(idtmed[715], "BCUTM",1.E-5) ;
  gMC->Gstpar(idtmed[715], "DCUTE",1.E-5) ;
  gMC->Gstpar(idtmed[715], "DCUTM",1.E-5) ;
  gMC->Gstpar(idtmed[715], "PPCUTM",1.E-5) ;
  gMC->Gstpar(idtmed[715], "LOSS",2.) ;
  gMC->Gstpar(idtmed[715], "DRAY",0.) ;
  gMC->Gstpar(idtmed[715], "STRA",2.) ;

}
//____________________________________________________________________________
AliPHOSGeometry * AliPHOS::GetGeometry() const 
{  
  // gets the pointer to the AliPHOSGeometry unique instance from the folder
  
  AliPHOSGeometry * rv = 0 ; 
  
  TString path("YSAlice/WhiteBoard/Geometry/PHOS/") ; 
  path += GetTitle() ; 
  rv = (AliPHOSGeometry*)gROOT->FindObjectAny(path) ; 
  return rv ; 
}

//____________________________________________________________________________
void AliPHOS::SetTreeAddress()
{ 


  // TBranch *branch;
  //  AliDetector::SetTreeAddress();

  TBranch *branch;
  char branchname[20];
  sprintf(branchname,"%s",GetName());
  
  // Branch address for hit tree
  TTree *treeH = gAlice->TreeH();
  if (treeH && fHits) {
    branch = treeH->GetBranch(branchname);
    if (branch) branch->SetAddress(&fHits);
  }
}

//____________________________________________________________________________
void AliPHOS::WriteQA()
{

  // Make TreeQA in the output file. 

  if(fTreeQA == 0)
    fTreeQA = new TTree("TreeQA", "QA Alarms") ;    
  // Create Alarms branches
  Int_t bufferSize = 32000 ;    
  Int_t splitlevel = 0 ; 
  TFolder * alarmsF = (TFolder*)gROOT->FindObjectAny("YSAlice/WhiteBoard/QAAlarms/PHOS") ; 
  TString branchName(alarmsF->GetName());  
  TBranch * alarmsBranch = fTreeQA->Branch(branchName,"TFolder", &alarmsF, bufferSize, splitlevel);
  TString branchTitle = branchName + " QA alarms" ; 
  alarmsBranch->SetTitle(branchTitle);
  alarmsBranch->Fill() ; 

  // fTreeQA->Fill() ; 
}

