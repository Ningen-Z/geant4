//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: G4QTauNuclearCrossSection.cc,v 1.10 2006/12/04 10:44:22 mkossov Exp $
// GEANT4 tag $Name: geant4-09-00 $
//
//
// G4 Physics class: G4QTauNuclearCrossSection for gamma+A cross sections
// Created: M.V. Kossov, CERN/ITEP(Moscow), 10-OCT-01
// The last update: M.V. Kossov, CERN/ITEP (Moscow) 17-Oct-03
// 
// ****************************************************************************************
// ********** This CLASS is temporary moved from the photolepton_hadron directory *********
// ******* DO NOT MAKE ANY CHANGE! With time it'll move back to photolepton...(M.K.) ******
// ****************************************************************************************
//=========================================================================================

///#define debug
#define edebug
//#define pdebug
//#define ppdebug
//#define tdebug
//#define sdebug

#include "G4QTauNuclearCrossSection.hh"

// Initialization of the
G4bool    G4QTauNuclearCrossSection::onlyCS=true;// Flag to calculate only CS
G4double  G4QTauNuclearCrossSection::lastSig=0.;// Last calculated cross section
G4int     G4QTauNuclearCrossSection::lastL=0;   // Last used in cross section TheLastBin
G4double  G4QTauNuclearCrossSection::lastE=0.;  // Last used in cross section TheEnergy
G4int     G4QTauNuclearCrossSection::lastF=0;   // Last used in cross section TheFirstBin
G4double  G4QTauNuclearCrossSection::lastG=0.;  // Last used in cross section TheGamma
G4double  G4QTauNuclearCrossSection::lastH=0.;  // LastValue of theHighEnergy A-dependence
G4double* G4QTauNuclearCrossSection::lastJ1=0;  // Pointer to the LastArray of J1 function
G4double* G4QTauNuclearCrossSection::lastJ2=0;  // Pointer to the LastArray of J2 function
G4double* G4QTauNuclearCrossSection::lastJ3=0;  // Pointer to the LastArray of J3 function
G4int     G4QTauNuclearCrossSection::lastPDG=0; // The last PDG code of the projectile
G4int     G4QTauNuclearCrossSection::lastN=0;   // The last N of calculated nucleus
G4int     G4QTauNuclearCrossSection::lastZ=0;   // The last Z of calculated nucleus
G4double  G4QTauNuclearCrossSection::lastP=0.;  // Last used in cross section Momentum
G4double  G4QTauNuclearCrossSection::lastTH=0.; // Last threshold momentum
G4double  G4QTauNuclearCrossSection::lastCS=0.; // Last value of the Cross Section
G4int     G4QTauNuclearCrossSection::lastI=0;   // The last position in the DAMDB

// Returns Pointer to the G4VQCrossSection class
G4VQCrossSection* G4QTauNuclearCrossSection::GetPointer()
{
  static G4QTauNuclearCrossSection theCrossSection; //**Static body of the Cross Section**
  return &theCrossSection;
}

// The main member function giving the collision cross section (P is in IU, CS is in mb)
// Make pMom in independent units ! (Now it is MeV)
G4double G4QTauNuclearCrossSection::GetCrossSection(G4bool fCS, G4double pMom,
                                                    G4int tgZ, G4int tgN, G4int pPDG)
{
  static const G4double mtu=1777.;     // Mass of a tau lepton in MeV
  static const G4double mtu2=mtu*mtu;  // Squared Mass of a tau-lepton in MeV^2
  static G4int j;                      // A#0f records found in DB for this projectile
  static std::vector <G4int>    colPDG;// Vector of the projectile PDG code
  static std::vector <G4int>    colN;  // Vector of N for calculated nuclei (isotops)
  static std::vector <G4int>    colZ;  // Vector of Z for calculated nuclei (isotops)
  static std::vector <G4double> colP;  // Vector of last momenta for the reaction
  static std::vector <G4double> colTH; // Vector of energy thresholds for the reaction
  static std::vector <G4double> colCS; // Vector of last cross sections for the reaction
  // ***---*** End of the mandatory Static Definitions of the Associative Memory ***---***
  G4double pEn=std::sqrt(pMom*pMom+mtu2)-mtu; // ==> tau-/tau+ kinEnergy
#ifdef pdebug
  G4cout<<"G4QTNCS::GetCS:>>> f="<<fCS<<", p="<<pMom<<", Z="<<tgZ<<"("<<lastZ<<") ,N="<<tgN
        <<"("<<lastN<<"),PDG="<<pPDG<<"("<<lastPDG<<"), T="<<pEn<<"("<<lastTH<<")"<<",Sz="
        <<colN.size()<<G4endl;
		//CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
  if(std::abs(pPDG)!=15)
  {
#ifdef pdebug
    G4cout<<"G4QTNCS::GetCS: *** Found pPDG="<<pPDG<<" ====> CS=0"<<G4endl;
    //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
    return 0.;                         // projectile PDG=0 is a mistake (?!) @@
  }
  G4bool in=false;                     // By default the isotope must be found in the AMDB
  if(tgN!=lastN || tgZ!=lastZ || pPDG!=lastPDG)// The nucleus was not the last used isotope
  {
    in = false;                        // By default the isotope haven't be found in AMDB  
    lastP   = 0.;                      // New momentum history (nothing to compare with)
    lastPDG = pPDG;                    // The last PDG of the projectile
    lastN   = tgN;                     // The last N of the calculated nucleus
    lastZ   = tgZ;                     // The last Z of the calculated nucleus
    lastI   = colN.size();             // Size of the Associative Memory DB in the heap
    j  = 0;                            // A#0f records found in DB for this projectile
    if(lastI) for(G4int i=0; i<lastI; i++) if(colPDG[i]==pPDG) // The partType is found
	   {                                  // The nucleus with projPDG is found in AMDB
      if(colN[i]==tgN && colZ[i]==tgZ)
						{
        lastI=i;
        lastTH =colTH[i];                // Last THreshold (A-dependent)
#ifdef pdebug
        G4cout<<"G4QTNCS::GetCS:*Found* P="<<pMom<<",Threshold="<<lastTH<<",j="<<j<<G4endl;
        //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
        if(pEn<=lastTH)
        {
#ifdef pdebug
          G4cout<<"G4QTNCS::GetCS:Found T="<<pEn<<" < Threshold="<<lastTH<<",CS=0"<<G4endl;
          //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
          return 0.;                     // Energy is below the Threshold value
        }
        lastP  =colP [i];                // Last Momentum  (A-dependent)
        lastCS =colCS[i];                // Last CrossSect (A-dependent)
        if(std::fabs(lastP/pMom-1.)<tolerance)
        {
#ifdef pdebug
          G4cout<<"G4QTNCS::GetCS:P="<<pMom<<",CS="<<lastCS*millibarn<<G4endl;
#endif
          CalculateCrossSection(fCS,-1,j,lastPDG,lastZ,lastN,pMom); // Update param's only
          return lastCS*millibarn;     // Use theLastCS
        }
        in = true;                       // This is the case when the isotop is found in DB
        // Momentum pMom is in IU ! @@ Units
#ifdef pdebug
        G4cout<<"G4QTNCS::G:UpdatDB P="<<pMom<<",f="<<fCS<<",lI="<<lastI<<",j="<<j<<G4endl;
#endif
        lastCS=CalculateCrossSection(fCS,-1,j,lastPDG,lastZ,lastN,pMom); // read & update
#ifdef pdebug
        G4cout<<"G4QTNCS::GetCrosSec: *****> New (inDB) Calculated CS="<<lastCS<<G4endl;
        //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
        if(lastCS<=0. && pEn>lastTH)    // Correct the threshold
        {
#ifdef pdebug
          G4cout<<"G4QTNCS::GetCS: New T="<<pEn<<"(CS=0) > Threshold="<<lastTH<<G4endl;
#endif
          lastTH=pEn;
        }
        break;                           // Go out of the LOOP
      }
#ifdef pdebug
      G4cout<<"---G4QTNCrossSec::GetCrosSec:pPDG="<<pPDG<<",j="<<j<<",N="<<colN[i]
            <<",Z["<<i<<"]="<<colZ[i]<<",cPDG="<<colPDG[i]<<G4endl;
      //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
      j++;                             // Increment a#0f records found in DB for this pPDG
	   }
	   if(!in)                            // This nucleus has not been calculated previously
	   {
#ifdef pdebug
      G4cout<<"G4QTNCS::GetCrosSec:CalcNew P="<<pMom<<",f="<<fCS<<",lastI="<<lastI<<G4endl;
#endif
      //!!The slave functions must provide cross-sections in millibarns (mb) !! (not in IU)
      lastCS=CalculateCrossSection(fCS,0,j,lastPDG,lastZ,lastN,pMom); //calculate & create
      if(lastCS<=0.)
						{
        lastTH = ThresholdEnergy(tgZ, tgN); // The Threshold Energy which is now the last
#ifdef pdebug
        G4cout<<"G4QTNCrossSection::GetCrossSect: NewThresh="<<lastTH<<", T="<<pEn<<G4endl;
#endif
        if(pEn>lastTH)
        {
#ifdef pdebug
          G4cout<<"G4QTNCS::GetCS: First T="<<pEn<<"(CS=0) > Threshold="<<lastTH<<G4endl;
#endif
          lastTH=pEn;
        }
						}
#ifdef pdebug
      G4cout<<"G4QTNCS::GetCrosSec: New CS="<<lastCS<<",lZ="<<lastN<<",lN="<<lastZ<<G4endl;
      //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
      colN.push_back(tgN);
      colZ.push_back(tgZ);
      colPDG.push_back(pPDG);
      colP.push_back(pMom);
      colTH.push_back(lastTH);
      colCS.push_back(lastCS);
#ifdef pdebug
      G4cout<<"G4QTNCS::GetCS:1st,P="<<pMom<<"(MeV),CS="<<lastCS*millibarn<<"(mb)"<<G4endl;
      //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
      return lastCS*millibarn;
	   } // End of creation of the new set of parameters
    else
				{
#ifdef pdebug
      G4cout<<"G4QTNCS::GetCS: Update lastI="<<lastI<<",j="<<j<<G4endl;
#endif
      colP[lastI]=pMom;
      colPDG[lastI]=pPDG;
      colCS[lastI]=lastCS;
    }
  } // End of parameters udate
  else if(pEn<=lastTH)
  {
#ifdef pdebug
    G4cout<<"G4QTNCS::GetCS: Current T="<<pEn<<" < Threshold="<<lastTH<<", CS=0"<<G4endl;
    //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
    return 0.;                         // Momentum is below the Threshold Value -> CS=0
  }
  else if(std::fabs(lastP/pMom-1.)<tolerance)
  {
#ifdef pdebug
    G4cout<<"G4QTNCS::GetCS:OldCur P="<<pMom<<"="<<pMom<<", CS="<<lastCS*millibarn<<G4endl;
    //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
    return lastCS*millibarn;     // Use theLastCS
  }
  else
  {
#ifdef pdebug
    G4cout<<"G4QTNCS::GetCS:UpdatCur P="<<pMom<<",f="<<fCS<<",I="<<lastI<<",j="<<j<<G4endl;
#endif
    lastCS=CalculateCrossSection(fCS,1,j,lastPDG,lastZ,lastN,pMom); // Only UpdateDB
    lastP=pMom;
  }
#ifdef pdebug
  G4cout<<"G4QTNCS::GetCroSec:End,P="<<pMom<<"(MeV),CS="<<lastCS*millibarn<<"(mb)"<<G4endl;
  //CalculateCrossSection(fCS,-27,j,lastPDG,lastZ,lastN,pMom); // DUMMY TEST
#endif
  return lastCS*millibarn;
}

// Gives the threshold energy for different nuclei (min of p- and n-threshold)
// *******************************************************************************
// *** This function is the same for all lepto- & photo-nuclear reactions, for ***
// *** (nu,l) reactions the mass value of the final state lepton must be added ***
// ***@@ IT IS REASONABLE TO MAKE ADDITIONAL VIRTUAL CLASS FOR LEPTO-NUCLEAR @@***
// *******************************************************************************
G4double G4QTauNuclearCrossSection::ThresholdEnergy(G4int Z, G4int N, G4int)
{
  // CHIPS - Direct GEANT
  //static const G4double mNeut = G4QPDGCode(2112).GetMass();
  //static const G4double mProt = G4QPDGCode(2212).GetMass();
  static const G4double mNeut = G4NucleiProperties::GetNuclearMass(1,0)/MeV;
  static const G4double mProt = G4NucleiProperties::GetNuclearMass(1,1)/MeV;
  static const G4double mAlph = G4NucleiProperties::GetNuclearMass(4,2)/MeV;
  // ---------
  static const G4double infEn = 9.e27;

  G4int A=Z+N;
  if(A<1) return infEn;
  else if(A==1) return 1894114.; // Pi0 threshold in MeV for the proton: T>m+(m^2+2lm)/2M
  // CHIPS - Direct GEANT
  //G4double mT= G4QPDGCode(111).GetNuclMass(Z,N,0);
  G4double mT= 0.;
  if(G4NucleiPropertiesTable::IsInTable(Z,A))
                                            mT=G4NucleiProperties::GetNuclearMass(A,Z)/MeV;
  else return 0.;       // If it is not in the Table of Stable Nuclei, then the Threshold=0
  // --------- Splitting thresholds
  G4double mP= infEn;
  if(Z&&G4NucleiPropertiesTable::IsInTable(Z-1,A-1))
     	    mP=G4NucleiProperties::GetNuclearMass(A-1.,Z-1.)/MeV; // ResNucMass for a proton

  G4double mN= infEn;
  if(N&&G4NucleiPropertiesTable::IsInTable(Z,A-1))
    	    mN=G4NucleiProperties::GetNuclearMass(A-1.,Z-0.)/MeV;  // ResNucMass for a neutron

  G4double mA= infEn;
  if(N>1&&Z>1&&G4NucleiPropertiesTable::IsInTable(Z-2,A-4))
     	    mA=G4NucleiProperties::GetNuclearMass(A-4.,Z-2.)/MeV; // ResNucMass for an alpha

  G4double dP= mP +mProt - mT;
  G4double dN= mN +mNeut - mT;
  G4double dA= mA +mAlph - mT;
#ifdef pdebug
		G4cout<<"G4TauNucCS::ThreshEn: mP="<<mP<<",dP="<<dP<<",mN="<<mN<<",dN="<<dN<<",mA="
        <<mA<<",dA="<<dA<<",mT="<<mT<<",A="<<A<<",Z="<<Z<<G4endl;
#endif
  if(dP<dN)dN=dP;
  if(dA<dN)dN=dA;
  return dN;
}

// The main member function giving the gamma-A cross section (E_kin in MeV, CS in mb)
G4double G4QTauNuclearCrossSection::CalculateCrossSection(G4bool CS, G4int F, G4int I,
                                        G4int, G4int targZ, G4int targN, G4double Momentum)
{
  static const G4int nE=336;    // !! If change this, change it in GetFunctions() (*.hh) !!
  static const G4int mL=nE-1;
  static const G4double EMi=2.0612;          // Minimum tabulated KineticEnergy of Tau
  static const G4double EMa=50000.;          // Maximum tabulated Energy of the Tau 
  static const G4double lEMi=std::log(EMi);  // Minimum tabulatedLogarithmKinEnergy of Tau
  static const G4double lEMa=std::log(EMa);  // Maximum tabulatedLogarithmKinEnergy of Tau
  static const G4double dlnE=(lEMa-lEMi)/mL; // Logarithmic table step for TauKinEnergy
  static const G4double alop=1./137.036/3.14159265; //coeffitient for  E>50000 calculations
  static const G4double mtu=1777.;           // Mass of a tau lepton in MeV
  static const G4double mtu2=mtu*mtu;        // Squared Mass of a tau-lepton in MeV^2
  static const G4double lmtu=std::log(mtu);  // Log of the tau-lepton mass
  // *** Begin of the Associative memory for acceleration of the cross section calculations
  static std::vector <G4int> colF;     // Vector of LastStartPosition in Ji-function tables
  static std::vector <G4double> colH;  // Vector of HighEnergyCoefficients (functional)
  static std::vector <G4double*> J1;   // Vector of pointers to the J1 tabulated functions
  static std::vector <G4double*> J2;   // Vector of pointers to the J2 tabulated functions
  static std::vector <G4double*> J3;   // Vector of pointers to the J3 tabulated functions
#ifdef pdebug
		G4cout<<"G4QTauNucCrossSection::CalculateCrossSection: ***Called*** "<<J3.size();
  if(J3.size()) G4cout<<", p="<<J3[0];
  G4cout<<G4endl;
  //if(F==-27) return 0.;
#endif
  // *** End of Static Definitions (Associative Memory) ***
  //const G4double Energy = aPart->GetKineticEnergy()/MeV; // Energy of the Tau-lepton
  onlyCS=CS;                           // Flag to calculate only CS (not Si/Bi)
  G4double TotEnergy2=Momentum*Momentum+mtu2;
  G4double TotEnergy=std::sqrt(TotEnergy2); // Total energy of the muon
  lastE=TotEnergy-mtu;               // Kinetic energy of the muon
#ifdef pdebug
		G4cout<<"G4QElectronNucCS::CalcCS: P="<<Momentum<<", F="<<F<<", I="<<I<<", Z="<<targZ
        <<", N="<<targN<<", onlyCS="<<CS<<",E="<<lastE<<",th="<<EMi<<G4endl;
#endif
  G4double A=targN+targZ;            // New A (can be different from targetAtomicNumber)
  if(F<=0)                           // This isotope was not the last used isotop
  {
    if(F<0)                          // This isotope was found in DAMDB =========> RETRIEVE
				{                                // ...........................................========
      if (lastE<=EMi)                // Energy is below the minimum energy in the table
      {
        lastE=0.;
        lastG=0.;
        lastSig=0.;
#ifdef pdebug
				    G4cout<<"---> G4QTauNucCS::CalcCS: Old CS=0  as lastE="<<lastE<<" < "<<EMi<<G4endl;
#endif
        return 0.;
      }
      lastJ1 =J1[I];                 // Pointer to the prepared J1 function
      lastJ2 =J2[I];                 // Pointer to the prepared J2 function
      lastJ3 =J3[I];                 // Pointer to the prepared J3 function
      lastF  =colF[I];               // Last ZeroPosition in the J-functions
      lastH  =colH[I];               // Last High Energy Coefficient (A-dependent)
#ifdef pdebug
				G4cout<<"---> G4QTauNucCS::CalcCS: CS=0  as lastE="<<lastE<<" < "<<EMi<<G4endl;
#endif
	  }
	  else                              // This isotope wasn't calculated previously => CREATE
	  {
      lastJ1 = new G4double[nE];     // Allocate memory for the new J1 function
      lastJ2 = new G4double[nE];     // Allocate memory for the new J2 function
      lastJ3 = new G4double[nE];     // Allocate memory for the new J3 function
      lastF   = GetFunctions(A,lastJ1,lastJ2,lastJ3);//newZeroPos and J-functions filling
      lastH   = alop*A*(1.-.072*std::log(A)); // like lastSP of G4PhotonuclearCrossSection
#ifdef pdebug
      G4cout<<"==>G4QTaNCS::CalcCS:lJ1="<<lastJ1<<",lJ2="<<lastJ2<<",lJ3="<<lastJ3<<G4endl;
#endif
      // *** The synchronization check ***
      G4int sync=J1.size();
      if(sync!=I) G4cerr<<"***G4QTauNuclearCS::CalcCS: PDG=15, S="<<sync<<" # "<<I<<G4endl;
      J1.push_back(lastJ1);
      J2.push_back(lastJ2);
      J3.push_back(lastJ3);
      colF.push_back(lastF);
      colH.push_back(lastH);
	   } // End of creation of the new set of parameters
  } // End of parameters udate
  // ============================== NOW Calculate the Cross Section =====================
  if (lastE<=lastTH || lastE<=EMi)   // Check that muKiE is higher than ThreshE
  {
    lastE=0.;
    lastG=0.;
    lastSig=0.;
#ifdef pdebug
				G4cout<<"---> G4QTauNucCS::CalcCS:CS=0 as T="<<lastE<<"<"<<EMi<<" || "<<lastTH<<G4endl;
#endif
    return 0.;
  }
  G4double lE=std::log(lastE);            // log(muE) (it is necessary for the fit)
  lastG=lE-lmtu;                     // Gamma of the muon (used to recover log(muE))
  G4double dlg1=lastG+lastG-1.;
  G4double lgoe=lastG/lastE;
  if(lE<lEMa) // Log fit is made explicitly to fix the last bin for the randomization
  {
    G4double shift=(lE-lEMi)/dlnE;
    G4int    blast=static_cast<int>(shift);
#ifdef pdebug
    G4cout<<"-->G4QTauNuclearCS::CalcCrossSect:LOGfit b="<<blast<<",max="<<mL<<",lJ1="
				      <<lastJ1<<",lJ2="<<lastJ2<<",lJ3="<<lastJ3<<G4endl;
#endif
    if(blast<0)   blast=0;
    if(blast>=mL) blast=mL-1;
    shift-=blast;
    lastL=blast+1;
    G4double YNi=dlg1*lastJ1[blast]
                -lgoe*(lastJ2[blast]+lastJ2[blast]-lastJ3[blast]/lastE);
    G4double YNj=dlg1*lastJ1[lastL]
                -lgoe*(lastJ2[lastL]+lastJ2[lastL]-lastJ3[lastL]/lastE);
    lastSig= YNi+shift*(YNj-YNi);
    if(lastSig>YNj)lastSig=YNj;
#ifdef pdebug
    G4cout<<"G4QTauNCS::CalcCS:S="<<lastSig<<",E="<<lE<<",Yi="<<YNi<<",Yj="<<YNj<<",M="
          <<lEMa<<G4endl;
    G4cout<<"G4QTauNCS::CalcCS:s="<<shift<<",Jb="<<lastJ1[blast]<<",J="<<lastJ1[lastL]
          <<",b="<<blast<<G4endl;
#endif
  }
  else
  {
#ifdef pdebug
    G4cout<<"->G4QTauNucCS::CCS:LOGex="<<lastJ1<<",lJ2="<<lastJ2<<",lJ3="<<lastJ3<<G4endl;
#endif
    lastL=mL;
    G4double term1=lastJ1[mL]+lastH*HighEnergyJ1(lE);
    G4double term2=lastJ2[mL]+lastH*HighEnergyJ2(lE);
    G4double term3=lastJ3[mL]+lastH*HighEnergyJ3(lE);
    lastSig=dlg1*term1-lgoe*(term2+term2-term3/lastE);
#ifdef pdebug
    G4cout<<"G4QTauNucCS::CalculateCrossSection:S="<<lastSig<<",lE="<<lE<<",J1="
          <<lastH*HighEnergyJ1(lE)<<",Pm="<<lastJ1[mL]<<",Fm="<<lastJ2[mL]<<",Fh="
          <<lastH*HighEnergyJ2(lE)<<",EM="<<lEMa<<G4endl;
#endif
  }
  if(lastSig<0.) lastSig = 0.;
  return lastSig;
}

// Calculate the functions for the log(A)
// ****************************************************************************************
// *** This tables are the same for all lepto-nuclear reactions, only mass is different ***
// ***@@ IT'S REASONABLE TO MAKE ADDiTIONAL VIRTUAL CLASS FOR LEPTO-NUCLEAR WITH THIS@@ ***
// ****************************************************************************************
G4int G4QTauNuclearCrossSection::GetFunctions(G4double a,G4double*x,G4double*y,G4double*z)
{
  static const G4int nN=14;
  static const G4int nE=336; // !! If change this, change it in GetCrossSection() (*.cc) !!
  static G4int L[nN]={138, 2, 32, 75, 26, 41, 0, 67, 58, 46, 41, 38, 39, 36};
  // !! @@ Change it from ln(A) to A approximation !!
  static G4double A[nN]={1.,2.,3.,4.,6.,7.,9.,12.,16.,27.,63.546,118.71,207.2,238.472};
  static const G4double P00[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,9.508853e-10,
5.658793e-09,1.254527e-08,2.393031e-08,4.505061e-08,8.774895e-08,1.786164e-07,3.761182e-07,
8.030425e-07,1.694925e-06,3.434914e-06,6.495386e-06,1.124554e-05,1.778700e-05,2.601897e-05,
3.581584e-05,4.713652e-05,6.004450e-05,7.469108e-05,9.129458e-05,1.101221e-04,1.314694e-04,
1.556326e-04,1.828616e-04,2.132878e-04,2.468359e-04,2.831345e-04,3.214679e-04,3.608144e-04,
3.999890e-04,4.378523e-04,4.735003e-04,5.063667e-04,5.362207e-04,5.630982e-04,5.872062e-04,
6.088412e-04,6.283321e-04,6.460063e-04,6.621724e-04,6.771140e-04,6.910895e-04,7.043362e-04,
7.170748e-04,7.295161e-04,7.418666e-04,7.543331e-04,7.671270e-04,7.804653e-04,7.945670e-04,
8.096392e-04,8.258506e-04,8.432877e-04,8.619005e-04,8.814550e-04,9.015289e-04,9.215770e-04,
9.410623e-04,9.595795e-04,9.769243e-04,9.930963e-04,1.008264e-03,1.022722e-03,1.036854e-03,
1.051075e-03,1.065714e-03,1.080833e-03,1.096129e-03,1.111117e-03,1.125415e-03,1.138885e-03,
1.151571e-03,1.163605e-03,1.175135e-03,1.186298e-03,1.197206e-03,1.207945e-03,1.218575e-03,
1.229133e-03,1.239639e-03,1.250100e-03,1.260515e-03,1.270882e-03,1.281197e-03,1.291459e-03,
1.301668e-03,1.311824e-03,1.321929e-03,1.331985e-03,1.341993e-03,1.351954e-03,1.361871e-03,
1.371743e-03,1.381573e-03,1.391360e-03,1.401105e-03,1.410808e-03,1.420471e-03,1.430093e-03,
1.439674e-03,1.449216e-03,1.458718e-03,1.468180e-03,1.477604e-03,1.486990e-03,1.496338e-03,
1.505649e-03,1.514923e-03,1.524160e-03,1.533363e-03,1.542530e-03,1.551662e-03,1.560761e-03,
1.569826e-03,1.578859e-03,1.587860e-03,1.596829e-03,1.605768e-03,1.614676e-03,1.623555e-03,
1.632405e-03,1.641227e-03,1.650021e-03,1.658788e-03,1.667529e-03,1.676244e-03,1.684933e-03,
1.693598e-03,1.702238e-03,1.710855e-03,1.719449e-03,1.728020e-03,1.736570e-03,1.745097e-03,
1.753604e-03,1.762091e-03,1.770557e-03,1.779004e-03,1.787432e-03,1.795842e-03,1.804234e-03,
1.812608e-03,1.820965e-03,1.829305e-03,1.837630e-03,1.845938e-03,1.854231e-03,1.862510e-03,
1.870774e-03,1.879024e-03,1.887260e-03,1.895483e-03,1.903693e-03,1.911891e-03,1.920077e-03,
1.928252e-03,1.936415e-03,1.944567e-03,1.952709e-03,1.960841e-03,1.968963e-03,1.977075e-03,
1.985179e-03,1.993274e-03,2.001361e-03,2.009440e-03,2.017511e-03,2.025576e-03,2.033633e-03,
2.041684e-03,2.049728e-03,2.057767e-03,2.065800e-03,2.073828e-03,2.081851e-03,2.089870e-03,
2.097884e-03,2.105894e-03,2.113901e-03,2.121904e-03,2.129905e-03,2.137903e-03,2.145898e-03,
2.153892e-03,2.161884e-03,2.169874e-03,2.177863e-03,2.185852e-03,2.193839e-03,2.201827e-03,
2.209815e-03,2.217803e-03,2.225791e-03,2.233781e-03,2.241771e-03,2.249764e-03,2.257757e-03
  };
  static const G4double P10[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,1.283819e-07,
7.765990e-07,1.754155e-06,3.420430e-06,6.607329e-06,1.324941e-05,2.782040e-05,6.046430e-05,
1.331907e-04,2.897632e-04,6.045196e-04,1.174950e-03,2.087179e-03,3.381598e-03,5.060191e-03,
7.118943e-03,9.570755e-03,1.245202e-02,1.582161e-02,1.975854e-02,2.435970e-02,2.973661e-02,
3.600931e-02,4.329446e-02,5.168438e-02,6.121832e-02,7.184951e-02,8.342000e-02,9.565929e-02,
1.082176e-01,1.207266e-01,1.328639e-01,1.443966e-01,1.551929e-01,1.652103e-01,1.744709e-01,
1.830362e-01,1.909891e-01,1.984219e-01,2.054289e-01,2.121037e-01,2.185384e-01,2.248245e-01,
2.310550e-01,2.373266e-01,2.437434e-01,2.504191e-01,2.574803e-01,2.650677e-01,2.733352e-01,
2.824425e-01,2.925383e-01,3.037299e-01,3.160413e-01,3.293712e-01,3.434732e-01,3.579873e-01,
3.725249e-01,3.867626e-01,4.005068e-01,4.137140e-01,4.264807e-01,4.390234e-01,4.516594e-01,
4.647653e-01,4.786698e-01,4.934691e-01,5.089000e-01,5.244803e-01,5.397990e-01,5.546715e-01,
5.691075e-01,5.832202e-01,5.971567e-01,6.110630e-01,6.250681e-01,6.392783e-01,6.537749e-01,
6.686149e-01,6.838333e-01,6.994497e-01,7.154739e-01,7.319115e-01,7.487683e-01,7.660517e-01,
7.837715e-01,8.019397e-01,8.205697e-01,8.396760e-01,8.592735e-01,8.793773e-01,9.000027e-01,
9.211652e-01,9.428799e-01,9.651625e-01,9.880287e-01,1.011494e+00,1.035576e+00,1.060290e+00,
1.085653e+00,1.111684e+00,1.138400e+00,1.165821e+00,1.193964e+00,1.222851e+00,1.252503e+00,
1.282939e+00,1.314184e+00,1.346258e+00,1.379187e+00,1.412994e+00,1.447705e+00,1.483345e+00,
1.519941e+00,1.557522e+00,1.596116e+00,1.635752e+00,1.676460e+00,1.718273e+00,1.761223e+00,
1.805343e+00,1.850668e+00,1.897234e+00,1.945076e+00,1.994234e+00,2.044746e+00,2.096652e+00,
2.149995e+00,2.204816e+00,2.261160e+00,2.319074e+00,2.378603e+00,2.439796e+00,2.502703e+00,
2.567376e+00,2.633867e+00,2.702233e+00,2.772528e+00,2.844812e+00,2.919145e+00,2.995588e+00,
3.074206e+00,3.155064e+00,3.238230e+00,3.323776e+00,3.411772e+00,3.502294e+00,3.595419e+00,
3.691226e+00,3.789797e+00,3.891216e+00,3.995572e+00,4.102954e+00,4.213454e+00,4.327169e+00,
4.444197e+00,4.564641e+00,4.688605e+00,4.816199e+00,4.947534e+00,5.082726e+00,5.221895e+00,
5.365163e+00,5.512657e+00,5.664510e+00,5.820856e+00,5.981835e+00,6.147591e+00,6.318273e+00,
6.494036e+00,6.675037e+00,6.861440e+00,7.053415e+00,7.251136e+00,7.454784e+00,7.664543e+00,
7.880607e+00,8.103173e+00,8.332447e+00,8.568639e+00,8.811968e+00,9.062659e+00,9.320945e+00,
9.587066e+00,9.861271e+00,1.014381e+01,1.043496e+01,1.073499e+01,1.104417e+01,1.136281e+01,
1.169119e+01,1.202964e+01,1.237847e+01,1.273802e+01,1.310862e+01,1.349063e+01,1.388442e+01
  };
  static const G4double P20[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,1.733331e-05,
1.065910e-04,2.453685e-04,4.892572e-04,9.701734e-04,2.003481e-03,4.340178e-03,9.736092e-03,
2.212593e-02,4.961468e-02,1.065569e-01,2.128855e-01,3.880845e-01,6.442424e-01,9.865527e-01,
1.419220e+00,1.950269e+00,2.593461e+00,3.368727e+00,4.302302e+00,5.426842e+00,6.781267e+00,
8.409767e+00,1.035907e+01,1.267273e+01,1.538236e+01,1.849628e+01,2.198896e+01,2.579646e+01,
2.982262e+01,3.395557e+01,3.808836e+01,4.213540e+01,4.604002e+01,4.977393e+01,5.333143e+01,
5.672268e+01,5.996799e+01,6.309401e+01,6.613132e+01,6.911336e+01,7.207629e+01,7.505958e+01,
7.810711e+01,8.126885e+01,8.460302e+01,8.817811e+01,9.207556e+01,9.639190e+01,1.012394e+02,
1.067429e+02,1.130306e+02,1.202141e+02,1.283582e+02,1.374455e+02,1.473530e+02,1.578615e+02,
1.687086e+02,1.796566e+02,1.905484e+02,2.013353e+02,2.120818e+02,2.229636e+02,2.342629e+02,
2.463419e+02,2.595499e+02,2.740378e+02,2.896053e+02,3.058034e+02,3.222162e+02,3.386384e+02,
3.550669e+02,3.716193e+02,3.884660e+02,4.057911e+02,4.237738e+02,4.425787e+02,4.623501e+02,
4.832092e+02,5.052552e+02,5.285702e+02,5.532261e+02,5.792925e+02,6.068416e+02,6.359524e+02,
6.667118e+02,6.992147e+02,7.335640e+02,7.698696e+02,8.082482e+02,8.488235e+02,8.917255e+02,
9.370917e+02,9.850666e+02,1.035803e+03,1.089461e+03,1.146211e+03,1.206233e+03,1.269717e+03,
1.336863e+03,1.407884e+03,1.483006e+03,1.562468e+03,1.646521e+03,1.735436e+03,1.829495e+03,
1.929001e+03,2.034272e+03,2.145649e+03,2.263490e+03,2.388177e+03,2.520114e+03,2.659730e+03,
2.807480e+03,2.963847e+03,3.129341e+03,3.304506e+03,3.489917e+03,3.686187e+03,3.893961e+03,
4.113928e+03,4.346817e+03,4.593401e+03,4.854501e+03,5.130987e+03,5.423784e+03,5.733871e+03,
6.062289e+03,6.410140e+03,6.778595e+03,7.168898e+03,7.582366e+03,8.020398e+03,8.484480e+03,
8.976188e+03,9.497193e+03,1.004927e+04,1.063431e+04,1.125430e+04,1.191137e+04,1.260778e+04,
1.334591e+04,1.412831e+04,1.495767e+04,1.583685e+04,1.676890e+04,1.775703e+04,1.880469e+04,
1.991550e+04,2.109332e+04,2.234227e+04,2.366670e+04,2.507122e+04,2.656077e+04,2.814056e+04,
2.981612e+04,3.159335e+04,3.347850e+04,3.547822e+04,3.759955e+04,3.985000e+04,4.223753e+04,
4.477059e+04,4.745818e+04,5.030985e+04,5.333573e+04,5.654661e+04,5.995394e+04,6.356990e+04,
6.740741e+04,7.148022e+04,7.580293e+04,8.039107e+04,8.526113e+04,9.043063e+04,9.591822e+04,
1.017437e+05,1.079281e+05,1.144938e+05,1.214646e+05,1.288658e+05,1.367242e+05,1.450685e+05,
1.539289e+05,1.633378e+05,1.733295e+05,1.839405e+05,1.952096e+05,2.071781e+05,2.198898e+05,
2.333914e+05,2.477325e+05,2.629658e+05,2.791474e+05,2.963371e+05,3.145981e+05,3.339980e+05
  };
  static const G4double P01[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,2.954211e-05,9.596085e-05,1.664204e-04,2.410445e-04,
3.199604e-04,4.032996e-04,4.911973e-04,5.837930e-04,6.812306e-04,7.836580e-04,8.913164e-04,
1.004812e-03,1.124386e-03,1.250223e-03,1.382516e-03,1.521462e-03,1.667264e-03,1.820132e-03,
1.980284e-03,2.147940e-03,2.320852e-03,2.496690e-03,2.675545e-03,2.857507e-03,3.042673e-03,
3.231140e-03,3.423010e-03,3.617256e-03,3.809214e-03,3.998293e-03,4.184404e-03,4.367457e-03,
4.547357e-03,4.723746e-03,4.895195e-03,5.061371e-03,5.222113e-03,5.377254e-03,5.526693e-03,
5.670813e-03,5.809570e-03,5.942803e-03,6.070341e-03,6.192646e-03,6.310189e-03,6.422827e-03,
6.530413e-03,6.633483e-03,6.732384e-03,6.826989e-03,6.917342e-03,7.004039e-03,7.087050e-03,
7.166336e-03,7.242378e-03,7.315204e-03,7.384814e-03,7.451592e-03,7.515520e-03,7.576720e-03,
7.635427e-03,7.691607e-03,7.745512e-03,7.797147e-03,7.846671e-03,7.894156e-03,7.939695e-03,
7.983384e-03,8.025299e-03,8.065520e-03,8.104132e-03,8.141187e-03,8.176782e-03,8.210952e-03,
8.243782e-03,8.275321e-03,8.305622e-03,8.334750e-03,8.362747e-03,8.389662e-03,8.415545e-03,
8.440441e-03,8.464389e-03,8.487429e-03,8.509600e-03,8.530939e-03,8.551482e-03,8.571260e-03,
8.590305e-03,8.608648e-03,8.626317e-03,8.643340e-03,8.659743e-03,8.675552e-03,8.690789e-03,
8.705479e-03,8.719642e-03,8.733300e-03,8.746472e-03,8.759179e-03,8.771437e-03,8.783264e-03,
8.794677e-03,8.805692e-03,8.816325e-03,8.826589e-03,8.836500e-03,8.846069e-03,8.855311e-03,
8.864237e-03,8.872859e-03,8.881188e-03,8.889236e-03,8.897012e-03,8.904527e-03,8.911790e-03,
8.918810e-03,8.925597e-03,8.932158e-03,8.938501e-03,8.944635e-03,8.950567e-03,8.956303e-03,
8.961852e-03,8.967219e-03,8.972411e-03,8.977434e-03,8.982294e-03,8.986998e-03,8.991551e-03,
8.995958e-03,9.000224e-03,9.004354e-03,9.008353e-03,9.012227e-03,9.015984e-03,9.019633e-03,
9.023195e-03,9.026706e-03,9.030244e-03,9.033965e-03,9.038176e-03,9.043389e-03,9.050276e-03,
9.059416e-03,9.071006e-03,9.084867e-03,9.100704e-03,9.118316e-03,9.137649e-03,9.158778e-03,
9.181864e-03,9.207136e-03,9.234874e-03,9.265398e-03,9.299061e-03,9.336228e-03,9.377254e-03,
9.422441e-03,9.471977e-03,9.525862e-03,9.583820e-03,9.645241e-03,9.709165e-03,9.774343e-03,
9.839376e-03,9.902903e-03,9.963776e-03,1.002117e-02,1.007462e-02,1.012399e-02,1.016939e-02,
1.021107e-02,1.024941e-02,1.028482e-02,1.031771e-02,1.034850e-02,1.037759e-02,1.040535e-02,
1.043213e-02,1.045828e-02,1.048413e-02,1.051000e-02,1.053619e-02,1.056302e-02,1.059074e-02,
1.061960e-02,1.064973e-02,1.068119e-02,1.071387e-02,1.074750e-02,1.078164e-02,1.081576e-02,
1.084929e-02,1.088177e-02,1.091287e-02,1.094245e-02,1.097052e-02,1.099725e-02,1.102293e-02,
1.104790e-02,1.107250e-02,1.109694e-02,1.112120e-02,1.114507e-02,1.116831e-02,1.119086e-02,
1.121276e-02,1.123411e-02,1.125504e-02,1.127562e-02,1.129594e-02,1.131605e-02,1.133598e-02,
1.135576e-02,1.137542e-02,1.139497e-02,1.141444e-02,1.143383e-02,1.145315e-02,1.147240e-02,
1.149159e-02,1.151073e-02,1.152981e-02,1.154882e-02,1.156778e-02,1.158668e-02,1.160552e-02,
1.162429e-02,1.164299e-02,1.166162e-02,1.168018e-02,1.169867e-02,1.171708e-02,1.173542e-02,
1.175368e-02,1.177187e-02,1.178998e-02,1.180802e-02,1.182597e-02,1.184385e-02,1.186166e-02,
1.187939e-02,1.189704e-02,1.191463e-02,1.193214e-02,1.194957e-02,1.196694e-02,1.198424e-02,
1.200146e-02,1.201862e-02,1.203572e-02,1.205275e-02,1.206972e-02,1.208662e-02,1.210346e-02,
1.212025e-02,1.213697e-02,1.215364e-02,1.217025e-02,1.218681e-02,1.220331e-02,1.221976e-02,
1.223616e-02,1.225251e-02,1.226881e-02,1.228506e-02,1.230127e-02,1.231743e-02,1.233355e-02,
1.234962e-02,1.236566e-02,1.238165e-02,1.239760e-02,1.241351e-02,1.242939e-02,1.244523e-02,
1.246103e-02,1.247679e-02,1.249253e-02,1.250823e-02,1.252390e-02,1.253953e-02,1.255514e-02,
1.257072e-02,1.258627e-02,1.260179e-02,1.261728e-02,1.263275e-02,1.264819e-02,1.266361e-02,
1.267901e-02,1.269438e-02,1.270973e-02,1.272506e-02,1.274037e-02,1.275566e-02,1.277093e-02,
1.278618e-02,1.280141e-02,1.281663e-02,1.283183e-02,1.284702e-02,1.286219e-02,1.287735e-02,
1.289249e-02,1.290763e-02,1.292275e-02,1.293786e-02,1.295296e-02,1.296804e-02,1.298312e-02,
1.299819e-02,1.301326e-02,1.302831e-02,1.304336e-02,1.305840e-02,1.307344e-02,1.308847e-02,
1.310350e-02,1.311853e-02,1.313355e-02,1.314857e-02,1.316358e-02,1.317860e-02,1.319362e-02,
1.320863e-02,1.322365e-02,1.323866e-02,1.325368e-02,1.326870e-02,1.328372e-02,1.329875e-02
  };
  static const G4double P11[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,5.142599e-05,1.703905e-04,3.015318e-04,4.458151e-04,
6.042723e-04,7.780058e-04,9.681932e-04,1.176093e-03,1.403047e-03,1.650491e-03,1.920179e-03,
2.214927e-03,2.536789e-03,2.887800e-03,3.270133e-03,3.686105e-03,4.138189e-03,4.629026e-03,
5.161429e-03,5.738401e-03,6.354262e-03,7.002399e-03,7.684563e-03,8.402602e-03,9.158473e-03,
9.954245e-03,1.079211e-02,1.166922e-02,1.256544e-02,1.347811e-02,1.440680e-02,1.535097e-02,
1.631002e-02,1.728180e-02,1.825789e-02,1.923543e-02,2.021240e-02,2.118654e-02,2.215588e-02,
2.312152e-02,2.408180e-02,2.503409e-02,2.597551e-02,2.690781e-02,2.783303e-02,2.874848e-02,
2.965126e-02,3.054421e-02,3.142876e-02,3.230222e-02,3.316333e-02,3.401622e-02,3.485909e-02,
3.568998e-02,3.651243e-02,3.732532e-02,3.812714e-02,3.892091e-02,3.970501e-02,4.047958e-02,
4.124621e-02,4.200316e-02,4.275250e-02,4.349304e-02,4.422580e-02,4.495060e-02,4.566768e-02,
4.637734e-02,4.707965e-02,4.777482e-02,4.846319e-02,4.914460e-02,4.981971e-02,5.048815e-02,
5.115054e-02,5.180682e-02,5.245710e-02,5.310179e-02,5.374084e-02,5.437439e-02,5.500269e-02,
5.562592e-02,5.624410e-02,5.685739e-02,5.746597e-02,5.806996e-02,5.866950e-02,5.926469e-02,
5.985567e-02,6.044255e-02,6.102544e-02,6.160445e-02,6.217970e-02,6.275130e-02,6.331935e-02,
6.388395e-02,6.444520e-02,6.500318e-02,6.555801e-02,6.610977e-02,6.665854e-02,6.720441e-02,
6.774747e-02,6.828779e-02,6.882546e-02,6.936055e-02,6.989314e-02,7.042330e-02,7.095109e-02,
7.147659e-02,7.199987e-02,7.252099e-02,7.304001e-02,7.355699e-02,7.407199e-02,7.458508e-02,
7.509630e-02,7.560571e-02,7.611336e-02,7.661930e-02,7.712359e-02,7.762628e-02,7.812740e-02,
7.862701e-02,7.912515e-02,7.962186e-02,8.011718e-02,8.061117e-02,8.110391e-02,8.159563e-02,
8.208616e-02,8.257558e-02,8.306395e-02,8.355139e-02,8.403811e-02,8.452456e-02,8.501170e-02,
8.550172e-02,8.599961e-02,8.651664e-02,8.707738e-02,8.773149e-02,8.856633e-02,8.970318e-02,
9.125803e-02,9.328999e-02,9.579442e-02,9.874335e-02,1.021230e-01,1.059470e-01,1.102541e-01,
1.151048e-01,1.205775e-01,1.267685e-01,1.337904e-01,1.417717e-01,1.508542e-01,1.611870e-01,
1.729166e-01,1.861694e-01,2.010268e-01,2.174967e-01,2.354851e-01,2.547793e-01,2.750538e-01,
2.959020e-01,3.168904e-01,3.376171e-01,3.577581e-01,3.770900e-01,3.954917e-01,4.129288e-01,
4.294320e-01,4.450769e-01,4.599676e-01,4.742255e-01,4.879821e-01,5.013750e-01,5.145470e-01,
5.276464e-01,5.408286e-01,5.542584e-01,5.681099e-01,5.825682e-01,5.978263e-01,6.140796e-01,
6.315125e-01,6.502781e-01,6.704690e-01,6.920855e-01,7.150085e-01,7.389925e-01,7.636899e-01,
7.887092e-01,8.136834e-01,8.383295e-01,8.624834e-01,8.861115e-01,9.093041e-01,9.322622e-01,
9.552702e-01,9.786340e-01,1.002560e+00,1.027034e+00,1.051840e+00,1.076745e+00,1.101644e+00,
1.126566e+00,1.151612e+00,1.176902e+00,1.202547e+00,1.228636e+00,1.255240e+00,1.282420e+00,
1.310225e+00,1.338704e+00,1.367897e+00,1.397846e+00,1.428588e+00,1.460157e+00,1.492584e+00,
1.525899e+00,1.560130e+00,1.595301e+00,1.631438e+00,1.668565e+00,1.706703e+00,1.745877e+00,
1.786109e+00,1.827424e+00,1.869844e+00,1.913396e+00,1.958105e+00,2.003998e+00,2.051103e+00,
2.099449e+00,2.149066e+00,2.199987e+00,2.252243e+00,2.305870e+00,2.360904e+00,2.417381e+00,
2.475340e+00,2.534822e+00,2.595869e+00,2.658523e+00,2.722829e+00,2.788834e+00,2.856587e+00,
2.926136e+00,2.997533e+00,3.070832e+00,3.146088e+00,3.223356e+00,3.302697e+00,3.384170e+00,
3.467838e+00,3.553766e+00,3.642020e+00,3.732669e+00,3.825784e+00,3.921439e+00,4.019708e+00,
4.120669e+00,4.224403e+00,4.330993e+00,4.440524e+00,4.553084e+00,4.668764e+00,4.787659e+00,
4.909864e+00,5.035479e+00,5.164607e+00,5.297355e+00,5.433831e+00,5.574149e+00,5.718425e+00,
5.866778e+00,6.019333e+00,6.176218e+00,6.337563e+00,6.503506e+00,6.674186e+00,6.849749e+00,
7.030342e+00,7.216120e+00,7.407242e+00,7.603871e+00,7.806178e+00,8.014335e+00,8.228524e+00,
8.448929e+00,8.675743e+00,8.909163e+00,9.149393e+00,9.396644e+00,9.651133e+00,9.913084e+00,
1.018273e+01,1.046031e+01,1.074606e+01,1.104026e+01,1.134314e+01,1.165500e+01,1.197610e+01,
1.230674e+01,1.264721e+01,1.299783e+01,1.335890e+01,1.373077e+01,1.411376e+01,1.450823e+01,
1.491454e+01,1.533306e+01,1.576417e+01,1.620828e+01,1.666579e+01,1.713713e+01,1.762273e+01,
1.812306e+01,1.863856e+01,1.916973e+01,1.971706e+01,2.028107e+01,2.086229e+01,2.146127e+01,
2.207856e+01,2.271477e+01,2.337048e+01,2.404633e+01,2.474296e+01,2.546103e+01,2.620123e+01
  };
  static const G4double P21[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,8.952320e-05,3.026305e-04,5.467439e-04,8.257448e-04,
1.143952e-03,1.506169e-03,1.917732e-03,2.384568e-03,2.913257e-03,3.511097e-03,4.186748e-03,
4.952288e-03,5.818752e-03,6.797967e-03,7.903042e-03,9.148496e-03,1.055041e-02,1.212656e-02,
1.389665e-02,1.588243e-02,1.807616e-02,2.046542e-02,2.306750e-02,2.590121e-02,2.898706e-02,
3.234740e-02,3.600656e-02,3.996753e-02,4.415218e-02,4.855802e-02,5.319252e-02,5.806289e-02,
6.317608e-02,6.853047e-02,7.408791e-02,7.983893e-02,8.577731e-02,9.189460e-02,9.818275e-02,
1.046534e-01,1.112996e-01,1.181068e-01,1.250565e-01,1.321638e-01,1.394471e-01,1.468879e-01,
1.544640e-01,1.622007e-01,1.701127e-01,1.781777e-01,1.863853e-01,1.947763e-01,2.033353e-01,
2.120436e-01,2.209398e-01,2.300138e-01,2.392506e-01,2.486866e-01,2.583048e-01,2.681087e-01,
2.781207e-01,2.883204e-01,2.987380e-01,3.093594e-01,3.202022e-01,3.312664e-01,3.425587e-01,
3.540870e-01,3.658557e-01,3.778718e-01,3.901450e-01,4.026762e-01,4.154819e-01,4.285591e-01,
4.419246e-01,4.555823e-01,4.695390e-01,4.838088e-01,4.983965e-01,5.133109e-01,5.285640e-01,
5.441662e-01,5.601251e-01,5.764518e-01,5.931578e-01,6.102545e-01,6.277536e-01,6.456668e-01,
6.640062e-01,6.827845e-01,7.020148e-01,7.217106e-01,7.418860e-01,7.625554e-01,7.837337e-01,
8.054363e-01,8.276787e-01,8.504769e-01,8.738478e-01,8.978095e-01,9.223790e-01,9.475746e-01,
9.734160e-01,9.999218e-01,1.027113e+00,1.055010e+00,1.083634e+00,1.113008e+00,1.143153e+00,
1.174094e+00,1.205855e+00,1.238460e+00,1.271935e+00,1.306308e+00,1.341604e+00,1.377853e+00,
1.415083e+00,1.453324e+00,1.492606e+00,1.532963e+00,1.574426e+00,1.617029e+00,1.660807e+00,
1.705796e+00,1.752032e+00,1.799555e+00,1.848402e+00,1.898615e+00,1.950243e+00,2.003346e+00,
2.057951e+00,2.114105e+00,2.171862e+00,2.231280e+00,2.292433e+00,2.355430e+00,2.420453e+00,
2.487871e+00,2.558479e+00,2.634058e+00,2.718555e+00,2.820170e+00,2.953872e+00,3.141548e+00,
3.406090e+00,3.762360e+00,4.214883e+00,4.764020e+00,5.412646e+00,6.169038e+00,7.047144e+00,
8.066375e+00,9.251616e+00,1.063353e+01,1.224899e+01,1.414150e+01,1.636113e+01,1.896376e+01,
2.200875e+01,2.555457e+01,2.965149e+01,3.433211e+01,3.960073e+01,4.542475e+01,5.173186e+01,
5.841585e+01,6.535063e+01,7.240850e+01,7.947682e+01,8.646893e+01,9.332839e+01,1.000274e+02,
1.065618e+02,1.129463e+02,1.192092e+02,1.253898e+02,1.315359e+02,1.377029e+02,1.439541e+02,
1.503616e+02,1.570072e+02,1.639851e+02,1.714029e+02,1.793829e+02,1.880626e+02,1.975917e+02,
2.081256e+02,2.198120e+02,2.327710e+02,2.470695e+02,2.626961e+02,2.795460e+02,2.974276e+02,
3.160962e+02,3.353012e+02,3.548335e+02,3.745617e+02,3.944512e+02,4.145723e+02,4.351002e+02,
4.563033e+02,4.784941e+02,5.019153e+02,5.266048e+02,5.523951e+02,5.790813e+02,6.065771e+02,
6.349401e+02,6.643178e+02,6.948910e+02,7.268416e+02,7.603404e+02,7.955471e+02,8.326159e+02,
8.716998e+02,9.129543e+02,9.565398e+02,1.002622e+03,1.051371e+03,1.102964e+03,1.157582e+03,
1.215413e+03,1.276652e+03,1.341501e+03,1.410169e+03,1.482875e+03,1.559849e+03,1.641333e+03,
1.727580e+03,1.818857e+03,1.915447e+03,2.017647e+03,2.125773e+03,2.240159e+03,2.361159e+03,
2.489147e+03,2.624520e+03,2.767701e+03,2.919136e+03,3.079298e+03,3.248691e+03,3.427846e+03,
3.617331e+03,3.817744e+03,4.029722e+03,4.253941e+03,4.491115e+03,4.742005e+03,5.007417e+03,
5.288205e+03,5.585276e+03,5.899591e+03,6.232171e+03,6.584097e+03,6.956519e+03,7.350654e+03,
7.767793e+03,8.209307e+03,8.676651e+03,9.171366e+03,9.695090e+03,1.024956e+04,1.083661e+04,
1.145821e+04,1.211642e+04,1.281345e+04,1.355162e+04,1.433343e+04,1.516149e+04,1.603861e+04,
1.696773e+04,1.795200e+04,1.899477e+04,2.009956e+04,2.127014e+04,2.251050e+04,2.382486e+04,
2.521774e+04,2.669389e+04,2.825838e+04,2.991659e+04,3.167424e+04,3.353738e+04,3.551246e+04,
3.760630e+04,3.982616e+04,4.217976e+04,4.467528e+04,4.732140e+04,5.012737e+04,5.310298e+04,
5.625866e+04,5.960545e+04,6.315512e+04,6.692013e+04,7.091374e+04,7.515004e+04,7.964399e+04,
8.441148e+04,8.946939e+04,9.483567e+04,1.005294e+05,1.065708e+05,1.129813e+05,1.197840e+05,
1.270030e+05,1.346642e+05,1.427950e+05,1.514246e+05,1.605840e+05,1.703061e+05,1.806259e+05,
1.915807e+05,2.032100e+05,2.155559e+05,2.286629e+05,2.425788e+05,2.573538e+05,2.730419e+05,
2.896999e+05,3.073887e+05,3.261726e+05,3.461204e+05,3.673050e+05,3.898039e+05,4.136996e+05,
4.390797e+05,4.660376e+05,4.946723e+05,5.250894e+05,5.574009e+05,5.917262e+05,6.281921e+05
  };
  static const G4double P02[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,4.420900e-05,1.361817e-04,
2.266269e-04,3.153714e-04,4.017279e-04,4.855357e-04,5.667169e-04,6.451910e-04,7.209150e-04,
7.941235e-04,8.648089e-04,9.328938e-04,9.982988e-04,1.061300e-03,1.122181e-03,1.180878e-03,
1.237327e-03,1.291871e-03,1.344747e-03,1.395901e-03,1.445399e-03,1.493678e-03,1.540753e-03,
1.586647e-03,1.631791e-03,1.676264e-03,1.720139e-03,1.763868e-03,1.807513e-03,1.851337e-03,
1.895742e-03,1.940822e-03,1.987146e-03,2.034882e-03,2.084594e-03,2.136652e-03,2.191607e-03,
2.250027e-03,2.312541e-03,2.379824e-03,2.452669e-03,2.531743e-03,2.617905e-03,2.711676e-03,
2.813598e-03,2.923687e-03,3.041745e-03,3.166730e-03,3.297379e-03,3.432040e-03,3.568683e-03,
3.705488e-03,3.840963e-03,3.973777e-03,4.102940e-03,4.227769e-03,4.347808e-03,4.462773e-03,
4.572513e-03,4.676977e-03,4.776189e-03,4.870234e-03,4.959240e-03,5.043366e-03,5.122795e-03,
5.197723e-03,5.268357e-03,5.334907e-03,5.397586e-03,5.456607e-03,5.512171e-03,5.564479e-03,
5.613724e-03,5.660086e-03,5.703745e-03,5.744862e-03,5.783597e-03,5.820098e-03,5.854504e-03,
5.886946e-03,5.917549e-03,5.946428e-03,5.973693e-03,5.999444e-03,6.023779e-03,6.046787e-03,
6.068552e-03,6.089154e-03,6.108666e-03,6.127159e-03,6.144698e-03,6.161344e-03,6.177157e-03,
6.192190e-03,6.206496e-03,6.220122e-03,6.233117e-03,6.245523e-03,6.257148e-03,6.267814e-03,
6.278332e-03,6.288730e-03,6.299041e-03,6.309306e-03,6.319568e-03,6.329890e-03,6.340346e-03,
6.351039e-03,6.362103e-03,6.373708e-03,6.386067e-03,6.399432e-03,6.414085e-03,6.430316e-03,
6.448411e-03,6.468623e-03,6.491167e-03,6.516222e-03,6.543944e-03,6.574484e-03,6.608010e-03,
6.644722e-03,6.684861e-03,6.728714e-03,6.776610e-03,6.828910e-03,6.885984e-03,6.948178e-03,
7.015759e-03,7.088847e-03,7.167329e-03,7.250781e-03,7.338407e-03,7.429041e-03,7.521215e-03,
7.613307e-03,7.703728e-03,7.791106e-03,7.874414e-03,7.953011e-03,8.026631e-03,8.095312e-03,
8.159311e-03,8.219026e-03,8.274939e-03,8.327565e-03,8.377428e-03,8.425044e-03,8.470912e-03,
8.515509e-03,8.559288e-03,8.602682e-03,8.646088e-03,8.689867e-03,8.734324e-03,8.779691e-03,
8.826098e-03,8.873546e-03,8.921888e-03,8.970825e-03,9.019927e-03,9.068684e-03,9.116571e-03,
9.163124e-03,9.207992e-03,9.250957e-03,9.291937e-03,9.330961e-03,9.368142e-03,9.403647e-03,
9.437672e-03,9.470422e-03,9.502096e-03,9.532883e-03,9.562954e-03,9.592457e-03,9.621524e-03,
9.650264e-03,9.678770e-03,9.707115e-03,9.735359e-03,9.763547e-03,9.791714e-03,9.819881e-03,
9.848065e-03,9.876273e-03,9.904506e-03,9.932763e-03,9.961037e-03,9.989320e-03,1.001760e-02,
1.004587e-02,1.007412e-02,1.010233e-02,1.013050e-02,1.015861e-02,1.018666e-02,1.021463e-02,
1.024251e-02,1.027031e-02,1.029800e-02,1.032560e-02,1.035308e-02,1.038046e-02,1.040771e-02,
1.043486e-02,1.046188e-02,1.048878e-02,1.051555e-02,1.054221e-02,1.056874e-02,1.059515e-02,
1.062144e-02,1.064761e-02,1.067366e-02,1.069959e-02,1.072540e-02,1.075110e-02,1.077668e-02,
1.080215e-02,1.082752e-02,1.085277e-02,1.087792e-02,1.090296e-02,1.092791e-02,1.095275e-02,
1.097749e-02,1.100214e-02,1.102670e-02,1.105116e-02,1.107554e-02,1.109983e-02,1.112403e-02,
1.114815e-02,1.117219e-02,1.119615e-02,1.122003e-02,1.124383e-02,1.126757e-02,1.129123e-02,
1.131482e-02,1.133834e-02,1.136179e-02,1.138518e-02,1.140851e-02,1.143178e-02,1.145498e-02,
1.147813e-02,1.150122e-02,1.152426e-02,1.154724e-02,1.157017e-02,1.159305e-02,1.161588e-02,
1.163867e-02,1.166140e-02,1.168409e-02,1.170674e-02,1.172935e-02,1.175192e-02,1.177444e-02,
1.179693e-02,1.181938e-02,1.184180e-02,1.186418e-02,1.188652e-02,1.190884e-02,1.193113e-02,
1.195338e-02,1.197561e-02,1.199781e-02,1.201998e-02,1.204213e-02,1.206425e-02,1.208635e-02,
1.210843e-02,1.213049e-02,1.215253e-02,1.217455e-02,1.219655e-02,1.221854e-02,1.224051e-02,
1.226246e-02,1.228440e-02,1.230633e-02,1.232825e-02,1.235016e-02,1.237205e-02,1.239394e-02,
1.241582e-02,1.243769e-02,1.245956e-02,1.248142e-02,1.250328e-02,1.252513e-02,1.254699e-02,
1.256884e-02,1.259069e-02,1.261254e-02,1.263439e-02,1.265624e-02,1.267809e-02,1.269995e-02
};
  static const G4double P12[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,2.225283e-04,6.969163e-04,
1.179082e-03,1.668007e-03,2.159647e-03,2.652656e-03,3.146065e-03,3.638813e-03,4.130000e-03,
4.620522e-03,5.109707e-03,5.596353e-03,6.079144e-03,6.559396e-03,7.038618e-03,7.515675e-03,
7.989358e-03,8.461919e-03,8.934839e-03,9.407147e-03,9.878911e-03,1.035387e-02,1.083189e-02,
1.131287e-02,1.180118e-02,1.229762e-02,1.280305e-02,1.332290e-02,1.385829e-02,1.441300e-02,
1.499297e-02,1.560045e-02,1.624454e-02,1.692928e-02,1.766499e-02,1.845978e-02,1.932536e-02,
2.027454e-02,2.132233e-02,2.248557e-02,2.378467e-02,2.523919e-02,2.687393e-02,2.870892e-02,
3.076594e-02,3.305745e-02,3.559175e-02,3.835869e-02,4.134135e-02,4.451165e-02,4.782902e-02,
5.125391e-02,5.475126e-02,5.828676e-02,6.183222e-02,6.536541e-02,6.886876e-02,7.232843e-02,
7.573356e-02,7.907571e-02,8.234849e-02,8.554717e-02,8.866842e-02,9.171009e-02,9.467100e-02,
9.755076e-02,1.003496e-01,1.030683e-01,1.057082e-01,1.082710e-01,1.107584e-01,1.131724e-01,
1.155154e-01,1.177894e-01,1.199970e-01,1.221403e-01,1.242219e-01,1.262440e-01,1.282088e-01,
1.301187e-01,1.319759e-01,1.337825e-01,1.355408e-01,1.372527e-01,1.389203e-01,1.405455e-01,
1.421304e-01,1.436768e-01,1.451865e-01,1.466613e-01,1.481032e-01,1.495139e-01,1.508951e-01,
1.522486e-01,1.535763e-01,1.548799e-01,1.561612e-01,1.574222e-01,1.586397e-01,1.597917e-01,
1.609624e-01,1.621554e-01,1.633749e-01,1.646261e-01,1.659155e-01,1.672522e-01,1.686479e-01,
1.701192e-01,1.716882e-01,1.733847e-01,1.752469e-01,1.773226e-01,1.796681e-01,1.823462e-01,
1.854234e-01,1.889661e-01,1.930388e-01,1.977039e-01,2.030239e-01,2.090644e-01,2.158990e-01,
2.236126e-01,2.323048e-01,2.420924e-01,2.531103e-01,2.655101e-01,2.794568e-01,2.951204e-01,
3.126625e-01,3.322153e-01,3.538544e-01,3.775683e-01,4.032308e-01,4.305865e-01,4.592584e-01,
4.887811e-01,5.186554e-01,5.484082e-01,5.776430e-01,6.060694e-01,6.335111e-01,6.598960e-01,
6.852353e-01,7.096034e-01,7.331188e-01,7.559302e-01,7.782067e-01,8.001315e-01,8.218986e-01,
8.437111e-01,8.657805e-01,8.883258e-01,9.115688e-01,9.357297e-01,9.610168e-01,9.876119e-01,
1.015650e+00,1.045194e+00,1.076217e+00,1.108583e+00,1.142051e+00,1.176301e+00,1.210968e+00,
1.245702e+00,1.280202e+00,1.314250e+00,1.347718e+00,1.380565e+00,1.412820e+00,1.444563e+00,
1.475916e+00,1.507016e+00,1.538017e+00,1.569073e+00,1.600334e+00,1.631945e+00,1.664042e+00,
1.696751e+00,1.730185e+00,1.764449e+00,1.799637e+00,1.835830e+00,1.873102e+00,1.911516e+00,
1.951130e+00,1.991990e+00,2.034139e+00,2.077614e+00,2.122447e+00,2.168668e+00,2.216301e+00,
2.265373e+00,2.315904e+00,2.367919e+00,2.421439e+00,2.476486e+00,2.533084e+00,2.591258e+00,
2.651032e+00,2.712435e+00,2.775494e+00,2.840241e+00,2.906709e+00,2.974931e+00,3.044946e+00,
3.116791e+00,3.190508e+00,3.266141e+00,3.343734e+00,3.423336e+00,3.504997e+00,3.588769e+00,
3.674707e+00,3.762868e+00,3.853312e+00,3.946099e+00,4.041295e+00,4.138965e+00,4.239180e+00,
4.342009e+00,4.447529e+00,4.555814e+00,4.666945e+00,4.781004e+00,4.898075e+00,5.018245e+00,
5.141606e+00,5.268252e+00,5.398277e+00,5.531782e+00,5.668870e+00,5.809647e+00,5.954223e+00,
6.102709e+00,6.255223e+00,6.411885e+00,6.572819e+00,6.738153e+00,6.908018e+00,7.082551e+00,
7.261893e+00,7.446187e+00,7.635583e+00,7.830235e+00,8.030303e+00,8.235949e+00,8.447343e+00,
8.664658e+00,8.888076e+00,9.117781e+00,9.353964e+00,9.596824e+00,9.846563e+00,1.010339e+01,
1.036753e+01,1.063919e+01,1.091862e+01,1.120604e+01,1.150170e+01,1.180587e+01,1.211879e+01,
1.244074e+01,1.277199e+01,1.311284e+01,1.346358e+01,1.382452e+01,1.419596e+01,1.457824e+01,
1.497170e+01,1.537667e+01,1.579352e+01,1.622262e+01,1.666435e+01,1.711910e+01,1.758728e+01,
1.806931e+01,1.856562e+01,1.907666e+01,1.960289e+01,2.014478e+01,2.070283e+01,2.127756e+01,
2.186947e+01,2.247911e+01,2.310704e+01,2.375385e+01,2.442012e+01,2.510646e+01,2.581353e+01,
2.654196e+01,2.729244e+01,2.806568e+01,2.886238e+01,2.968330e+01,3.052921e+01,3.140091e+01,
3.229921e+01,3.322497e+01,3.417906e+01,3.516238e+01,3.617589e+01,3.722053e+01,3.829731e+01
  };
  static const G4double P22[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,1.120131e-03,3.567208e-03,
6.137882e-03,8.831785e-03,1.163101e-02,1.453145e-02,1.753060e-02,2.062489e-02,2.381127e-02,
2.709822e-02,3.048398e-02,3.396264e-02,3.752670e-02,4.118793e-02,4.496040e-02,4.883801e-02,
5.281322e-02,5.690771e-02,6.113789e-02,6.549907e-02,6.999581e-02,7.466886e-02,7.952312e-02,
8.456445e-02,8.984669e-02,9.538880e-02,1.012118e-01,1.073922e-01,1.139603e-01,1.209824e-01,
1.285578e-01,1.367447e-01,1.457007e-01,1.555238e-01,1.664127e-01,1.785480e-01,1.921825e-01,
2.076059e-01,2.251690e-01,2.452818e-01,2.684512e-01,2.952087e-01,3.262268e-01,3.621378e-01,
4.036571e-01,4.513583e-01,5.057654e-01,5.670252e-01,6.351235e-01,7.097670e-01,7.903118e-01,
8.760602e-01,9.663525e-01,1.060475e+00,1.157804e+00,1.257815e+00,1.360070e+00,1.464190e+00,
1.569856e+00,1.676792e+00,1.784762e+00,1.893564e+00,2.003028e+00,2.113012e+00,2.223396e+00,
2.334084e+00,2.444997e+00,2.556071e+00,2.667265e+00,2.778554e+00,2.889911e+00,3.001327e+00,
3.112811e+00,3.224358e+00,3.335994e+00,3.447730e+00,3.559596e+00,3.671622e+00,3.783838e+00,
3.896283e+00,4.009001e+00,4.122034e+00,4.235431e+00,4.349243e+00,4.463526e+00,4.578342e+00,
4.693754e+00,4.809831e+00,4.926647e+00,5.044285e+00,5.162832e+00,5.282382e+00,5.403039e+00,
5.524919e+00,5.648145e+00,5.772858e+00,5.899215e+00,6.027391e+00,6.154925e+00,6.279335e+00,
6.409661e+00,6.546552e+00,6.690783e+00,6.843308e+00,7.005336e+00,7.178452e+00,7.364781e+00,
7.567228e+00,7.789765e+00,8.037769e+00,8.318377e+00,8.640763e+00,9.016247e+00,9.458163e+00,
9.981511e+00,1.060252e+01,1.133833e+01,1.220702e+01,1.322802e+01,1.442289e+01,1.581627e+01,
1.743708e+01,1.931953e+01,2.150424e+01,2.403895e+01,2.697907e+01,3.038737e+01,3.433257e+01,
3.888631e+01,4.411756e+01,5.008434e+01,5.682345e+01,6.433960e+01,7.259691e+01,8.151634e+01,
9.098150e+01,1.008525e+02,1.109841e+02,1.212442e+02,1.315260e+02,1.417556e+02,1.518925e+02,
1.619260e+02,1.718706e+02,1.817614e+02,1.916501e+02,2.016029e+02,2.116989e+02,2.220295e+02,
2.326990e+02,2.438250e+02,2.555393e+02,2.679864e+02,2.813215e+02,2.957058e+02,3.112976e+02,
3.282386e+02,3.466366e+02,3.665465e+02,3.879537e+02,4.107678e+02,4.348286e+02,4.599280e+02,
4.858444e+02,5.123742e+02,5.393579e+02,5.666943e+02,5.943443e+02,6.223268e+02,6.507098e+02,
6.796009e+02,7.091381e+02,7.394822e+02,7.708109e+02,8.033129e+02,8.371852e+02,8.726312e+02,
9.098582e+02,9.490768e+02,9.904994e+02,1.034340e+03,1.080815e+03,1.130139e+03,1.182532e+03,
1.238214e+03,1.297406e+03,1.360335e+03,1.427230e+03,1.498326e+03,1.573865e+03,1.654097e+03,
1.739279e+03,1.829682e+03,1.925585e+03,2.027284e+03,2.135086e+03,2.249318e+03,2.370323e+03,
2.498462e+03,2.634120e+03,2.777703e+03,2.929640e+03,3.090388e+03,3.260429e+03,3.440278e+03,
3.630477e+03,3.831604e+03,4.044271e+03,4.269129e+03,4.506868e+03,4.758219e+03,5.023960e+03,
5.304913e+03,5.601955e+03,5.916011e+03,6.248067e+03,6.599167e+03,6.970418e+03,7.362995e+03,
7.778144e+03,8.217189e+03,8.681530e+03,9.172656e+03,9.692145e+03,1.024167e+04,1.082301e+04,
1.143804e+04,1.208877e+04,1.277731e+04,1.350591e+04,1.427696e+04,1.509299e+04,1.595667e+04,
1.687087e+04,1.783860e+04,1.886306e+04,1.994767e+04,2.109602e+04,2.231195e+04,2.359952e+04,
2.496305e+04,2.640711e+04,2.793656e+04,2.955655e+04,3.127256e+04,3.309039e+04,3.501622e+04,
3.705657e+04,3.921839e+04,4.150907e+04,4.393642e+04,4.650876e+04,4.923490e+04,5.212422e+04,
5.518666e+04,5.843278e+04,6.187379e+04,6.552160e+04,6.938886e+04,7.348899e+04,7.783625e+04,
8.244579e+04,8.733369e+04,9.251705e+04,9.801401e+04,1.038438e+05,1.100270e+05,1.165853e+05,
1.235418e+05,1.309211e+05,1.387492e+05,1.470539e+05,1.558646e+05,1.652126e+05,1.751311e+05,
1.856555e+05,1.968232e+05,2.086742e+05,2.212509e+05,2.345983e+05,2.487643e+05,2.637997e+05,
2.797587e+05,2.966988e+05,3.146809e+05,3.337702e+05,3.540357e+05,3.755506e+05,3.983931e+05,
4.226461e+05,4.483978e+05,4.757419e+05,5.047781e+05,5.356124e+05,5.683574e+05,6.031331e+05,
6.400668e+05,6.792939e+05,7.209586e+05,7.652140e+05,8.122231e+05,8.621592e+05,9.152064e+05
  };
  static const G4double P03[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,1.409974e-04,
3.120096e-04,5.006502e-04,7.064650e-04,9.273072e-04,1.160607e-03,1.403015e-03,1.650417e-03,
1.899130e-03,2.146133e-03,2.388721e-03,2.624876e-03,2.853195e-03,3.072731e-03,3.282883e-03,
3.483317e-03,3.673901e-03,3.854657e-03,4.025724e-03,4.187333e-03,4.339776e-03,4.483396e-03,
4.618564e-03,4.745669e-03,4.865110e-03,4.977295e-03,5.082631e-03,5.181502e-03,5.274289e-03,
5.361367e-03,5.443076e-03,5.519761e-03,5.591731e-03,5.659291e-03,5.722723e-03,5.782293e-03,
5.838252e-03,5.890835e-03,5.940264e-03,5.986745e-03,6.030472e-03,6.071627e-03,6.110379e-03,
6.146889e-03,6.181306e-03,6.213770e-03,6.244412e-03,6.273357e-03,6.300718e-03,6.326607e-03,
6.351124e-03,6.374368e-03,6.396429e-03,6.417395e-03,6.437348e-03,6.456265e-03,6.474073e-03,
6.491149e-03,6.507570e-03,6.523412e-03,6.538764e-03,6.553724e-03,6.568408e-03,6.582959e-03,
6.597557e-03,6.612423e-03,6.627837e-03,6.644132e-03,6.661699e-03,6.680967e-03,6.702376e-03,
6.726353e-03,6.753276e-03,6.783463e-03,6.817177e-03,6.854643e-03,6.896074e-03,6.941701e-03,
6.991787e-03,7.046648e-03,7.106649e-03,7.172202e-03,7.243749e-03,7.321731e-03,7.406542e-03,
7.498466e-03,7.597591e-03,7.703710e-03,7.816238e-03,7.934150e-03,8.055987e-03,8.179938e-03,
8.304010e-03,8.426239e-03,8.544896e-03,8.658642e-03,8.766594e-03,8.868315e-03,8.963752e-03,
9.053136e-03,9.136896e-03,9.215579e-03,9.289794e-03,9.360172e-03,9.427339e-03,9.491904e-03,
9.554447e-03,9.615520e-03,9.675636e-03,9.735266e-03,9.794830e-03,9.854684e-03,9.915103e-03,
9.976259e-03,1.003820e-02,1.010084e-02,1.016395e-02,1.022716e-02,1.029004e-02,1.035209e-02,
1.041285e-02,1.047195e-02,1.052910e-02,1.058414e-02,1.063704e-02,1.068785e-02,1.073670e-02,
1.078376e-02,1.082922e-02,1.087331e-02,1.091623e-02,1.095817e-02,1.099930e-02,1.103978e-02,
1.107975e-02,1.111933e-02,1.115860e-02,1.119765e-02,1.123654e-02,1.127531e-02,1.131400e-02,
1.135262e-02,1.139120e-02,1.142974e-02,1.146824e-02,1.150669e-02,1.154509e-02,1.158343e-02,
1.162169e-02,1.165987e-02,1.169795e-02,1.173592e-02,1.177377e-02,1.181149e-02,1.184906e-02,
1.188649e-02,1.192375e-02,1.196086e-02,1.199779e-02,1.203454e-02,1.207112e-02,1.210751e-02,
1.214372e-02,1.217975e-02,1.221559e-02,1.225124e-02,1.228671e-02,1.232200e-02,1.235710e-02,
1.239202e-02,1.242676e-02,1.246133e-02,1.249572e-02,1.252994e-02,1.256399e-02,1.259787e-02,
1.263159e-02,1.266515e-02,1.269855e-02,1.273181e-02,1.276490e-02,1.279786e-02,1.283067e-02,
1.286333e-02,1.289586e-02,1.292826e-02,1.296053e-02,1.299266e-02,1.302467e-02,1.305656e-02,
1.308833e-02,1.311999e-02,1.315153e-02,1.318296e-02,1.321428e-02,1.324550e-02,1.327661e-02,
1.330763e-02,1.333855e-02,1.336937e-02,1.340010e-02,1.343074e-02,1.346130e-02,1.349177e-02,
1.352215e-02,1.355246e-02,1.358269e-02,1.361284e-02,1.364292e-02,1.367293e-02,1.370287e-02,
1.373274e-02,1.376254e-02,1.379228e-02,1.382196e-02,1.385158e-02,1.388114e-02,1.391065e-02,
1.394010e-02,1.396950e-02,1.399885e-02,1.402815e-02,1.405740e-02,1.408661e-02,1.411577e-02,
1.414489e-02,1.417397e-02,1.420301e-02,1.423201e-02,1.426098e-02,1.428992e-02,1.431882e-02,
1.434768e-02,1.437652e-02,1.440533e-02,1.443412e-02,1.446287e-02,1.449161e-02,1.452032e-02,
1.454901e-02,1.457768e-02,1.460633e-02,1.463496e-02,1.466358e-02,1.469218e-02,1.472077e-02,
1.474934e-02,1.477791e-02,1.480646e-02,1.483501e-02,1.486355e-02,1.489208e-02,1.492061e-02,
1.494914e-02,1.497766e-02,1.500618e-02,1.503470e-02,1.506322e-02,1.509175e-02,1.512027e-02
  };
  static const G4double P13[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,2.762798e-03,
6.214464e-03,1.014119e-02,1.455950e-02,1.944870e-02,2.477496e-02,3.048202e-02,3.648846e-02,
4.271502e-02,4.909153e-02,5.554923e-02,6.203158e-02,6.849397e-02,7.490114e-02,8.122529e-02,
8.744455e-02,9.354199e-02,9.950466e-02,1.053230e-01,1.109903e-01,1.165020e-01,1.218558e-01,
1.270507e-01,1.320872e-01,1.369666e-01,1.416915e-01,1.462653e-01,1.506913e-01,1.549735e-01,
1.591164e-01,1.631241e-01,1.670017e-01,1.707532e-01,1.743837e-01,1.778977e-01,1.812996e-01,
1.845939e-01,1.877850e-01,1.908772e-01,1.938747e-01,1.967815e-01,1.996017e-01,2.023391e-01,
2.049976e-01,2.075809e-01,2.100927e-01,2.125365e-01,2.149160e-01,2.172346e-01,2.194959e-01,
2.217034e-01,2.238606e-01,2.259710e-01,2.280383e-01,2.300664e-01,2.320480e-01,2.339710e-01,
2.358717e-01,2.377555e-01,2.396290e-01,2.415002e-01,2.433797e-01,2.452813e-01,2.472236e-01,
2.492319e-01,2.513402e-01,2.535933e-01,2.560486e-01,2.587768e-01,2.618611e-01,2.653936e-01,
2.694712e-01,2.741902e-01,2.796437e-01,2.859211e-01,2.931111e-01,3.013060e-01,3.106073e-01,
3.211310e-01,3.330113e-01,3.464030e-01,3.614827e-01,3.784459e-01,3.975016e-01,4.188614e-01,
4.427220e-01,4.692400e-01,4.984989e-01,5.304750e-01,5.650069e-01,6.017804e-01,6.403371e-01,
6.801126e-01,7.204962e-01,7.608999e-01,8.008170e-01,8.398607e-01,8.777776e-01,9.144416e-01,
9.498326e-01,9.840127e-01,1.017105e+00,1.049275e+00,1.080716e+00,1.111643e+00,1.142283e+00,
1.172873e+00,1.203659e+00,1.234892e+00,1.266822e+00,1.299693e+00,1.333737e+00,1.369155e+00,
1.406103e+00,1.444672e+00,1.484870e+00,1.526605e+00,1.569693e+00,1.613861e+00,1.658784e+00,
1.704122e+00,1.749563e+00,1.794852e+00,1.839808e+00,1.884335e+00,1.928413e+00,1.972085e+00,
2.015447e+00,2.058627e+00,2.101780e+00,2.145070e+00,2.188667e+00,2.232736e+00,2.277440e+00,
2.322931e+00,2.369348e+00,2.416823e+00,2.465473e+00,2.515403e+00,2.566707e+00,2.619468e+00,
2.673759e+00,2.729644e+00,2.787177e+00,2.846409e+00,2.907381e+00,2.970134e+00,3.034703e+00,
3.101119e+00,3.169415e+00,3.239621e+00,3.311767e+00,3.385883e+00,3.462001e+00,3.540153e+00,
3.620375e+00,3.702702e+00,3.787172e+00,3.873828e+00,3.962711e+00,4.053869e+00,4.147350e+00,
4.243205e+00,4.341489e+00,4.442258e+00,4.545574e+00,4.651497e+00,4.760095e+00,4.871437e+00,
4.985593e+00,5.102639e+00,5.222653e+00,5.345715e+00,5.471910e+00,5.601324e+00,5.734049e+00,
5.870177e+00,6.009805e+00,6.153034e+00,6.299967e+00,6.450712e+00,6.605379e+00,6.764082e+00,
6.926941e+00,7.094076e+00,7.265614e+00,7.441685e+00,7.622423e+00,7.807966e+00,7.998458e+00,
8.194045e+00,8.394879e+00,8.601118e+00,8.812923e+00,9.030461e+00,9.253904e+00,9.483429e+00,
9.719220e+00,9.961464e+00,1.021036e+01,1.046610e+01,1.072890e+01,1.099897e+01,1.127653e+01,
1.156181e+01,1.185504e+01,1.215646e+01,1.246633e+01,1.278490e+01,1.311243e+01,1.344920e+01,
1.379550e+01,1.415161e+01,1.451783e+01,1.489447e+01,1.528186e+01,1.568033e+01,1.609020e+01,
1.651185e+01,1.694562e+01,1.739189e+01,1.785105e+01,1.832350e+01,1.880965e+01,1.930992e+01,
1.982475e+01,2.035460e+01,2.089992e+01,2.146121e+01,2.203895e+01,2.263366e+01,2.324587e+01,
2.387613e+01,2.452501e+01,2.519307e+01,2.588093e+01,2.658921e+01,2.731854e+01,2.806960e+01,
2.884305e+01,2.963961e+01,3.046001e+01,3.130500e+01,3.217535e+01,3.307186e+01,3.399537e+01,
3.494673e+01,3.592682e+01,3.693656e+01,3.797687e+01,3.904875e+01,4.015319e+01,4.129122e+01,
4.246392e+01,4.367240e+01,4.491780e+01,4.620129e+01,4.752410e+01,4.888749e+01,5.029275e+01
  };
  static const G4double P23[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,5.413975e-02,
1.238128e-01,2.055575e-01,3.004149e-01,4.086648e-01,5.302738e-01,6.646463e-01,8.104828e-01,
9.663775e-01,1.131003e+00,1.302922e+00,1.480873e+00,1.663800e+00,1.850809e+00,2.041137e+00,
2.234130e+00,2.429223e+00,2.625932e+00,2.823840e+00,3.022593e+00,3.221891e+00,3.421481e+00,
3.621155e+00,3.820738e+00,4.020088e+00,4.219105e+00,4.417721e+00,4.615865e+00,4.813503e+00,
5.010628e+00,5.207217e+00,5.403298e+00,5.598873e+00,5.793981e+00,5.988660e+00,6.182946e+00,
6.376896e+00,6.570570e+00,6.764031e+00,6.957349e+00,7.150601e+00,7.343872e+00,7.537254e+00,
7.730848e+00,7.924762e+00,8.119112e+00,8.314031e+00,8.509661e+00,8.706158e+00,8.903694e+00,
9.102464e+00,9.302681e+00,9.504585e+00,9.708450e+00,9.914589e+00,1.012220e+01,1.032986e+01,
1.054144e+01,1.075759e+01,1.097915e+01,1.120725e+01,1.144340e+01,1.168966e+01,1.194894e+01,
1.222528e+01,1.252429e+01,1.285366e+01,1.322363e+01,1.364736e+01,1.414112e+01,1.472402e+01,
1.541751e+01,1.624472e+01,1.723002e+01,1.839895e+01,1.977888e+01,2.139989e+01,2.329619e+01,
2.550749e+01,2.808038e+01,3.106957e+01,3.453874e+01,3.856086e+01,4.321766e+01,4.859753e+01,
5.479148e+01,6.188616e+01,6.995395e+01,7.904100e+01,8.915485e+01,1.002549e+02,1.122494e+02,
1.250017e+02,1.383453e+02,1.521040e+02,1.661133e+02,1.802356e+02,1.943703e+02,2.084566e+02,
2.224704e+02,2.364195e+02,2.503384e+02,2.642840e+02,2.783315e+02,2.925726e+02,3.071140e+02,
3.220770e+02,3.375972e+02,3.538252e+02,3.709238e+02,3.890662e+02,4.084313e+02,4.291951e+02,
4.515193e+02,4.755366e+02,5.013340e+02,5.289387e+02,5.583096e+02,5.893384e+02,6.218631e+02,
6.556933e+02,6.906377e+02,7.265303e+02,7.632504e+02,8.007334e+02,8.389740e+02,8.780231e+02,
9.179815e+02,9.589917e+02,1.001230e+03,1.044901e+03,1.090228e+03,1.137450e+03,1.186818e+03,
1.238592e+03,1.293040e+03,1.350433e+03,1.411047e+03,1.475160e+03,1.543054e+03,1.615014e+03,
1.691327e+03,1.772284e+03,1.858181e+03,1.949319e+03,2.046009e+03,2.148567e+03,2.257322e+03,
2.372614e+03,2.494796e+03,2.624240e+03,2.761331e+03,2.906477e+03,3.060105e+03,3.222666e+03,
3.394638e+03,3.576525e+03,3.768859e+03,3.972206e+03,4.187166e+03,4.414372e+03,4.654499e+03,
4.908260e+03,5.176412e+03,5.459761e+03,5.759158e+03,6.075508e+03,6.409771e+03,6.762966e+03,
7.136172e+03,7.530536e+03,7.947272e+03,8.387671e+03,8.853101e+03,9.345011e+03,9.864941e+03,
1.041452e+04,1.099549e+04,1.160967e+04,1.225902e+04,1.294560e+04,1.367160e+04,1.443934e+04,
1.525129e+04,1.611006e+04,1.701843e+04,1.797933e+04,1.899589e+04,2.007140e+04,2.120939e+04,
2.241358e+04,2.368790e+04,2.503657e+04,2.646401e+04,2.797495e+04,2.957440e+04,3.126766e+04,
3.306037e+04,3.495851e+04,3.696842e+04,3.909684e+04,4.135092e+04,4.373823e+04,4.626684e+04,
4.894528e+04,5.178262e+04,5.478849e+04,5.797312e+04,6.134735e+04,6.492269e+04,6.871139e+04,
7.272641e+04,7.698154e+04,8.149143e+04,8.627160e+04,9.133857e+04,9.670985e+04,1.024041e+05,
1.084410e+05,1.148416e+05,1.216281e+05,1.288244e+05,1.364554e+05,1.445480e+05,1.531304e+05,
1.622330e+05,1.718876e+05,1.821283e+05,1.929912e+05,2.045148e+05,2.167399e+05,2.297098e+05,
2.434706e+05,2.580712e+05,2.735637e+05,2.900034e+05,3.074489e+05,3.259628e+05,3.456113e+05,
3.664650e+05,3.885990e+05,4.120927e+05,4.370310e+05,4.635039e+05,4.916070e+05,5.214420e+05,
5.531172e+05,5.867475e+05,6.224550e+05,6.603699e+05,7.006301e+05,7.433826e+05,7.887837e+05,
8.369994e+05,8.882063e+05,9.425923e+05,1.000357e+06,1.061713e+06,1.126885e+06,1.196115e+06
  };
  static const G4double P04[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,4.442863e-06,
1.519363e-05,2.703219e-05,4.014211e-05,5.519168e-05,7.230975e-05,9.155962e-05,1.130065e-04,
1.367177e-04,1.629335e-04,1.925910e-04,2.259139e-04,2.630146e-04,3.040085e-04,3.491052e-04,
3.991538e-04,4.544632e-04,5.151945e-04,5.815134e-04,6.535550e-04,7.314583e-04,8.154027e-04,
9.055657e-04,1.001169e-03,1.101697e-03,1.207300e-03,1.317649e-03,1.430881e-03,1.546863e-03,
1.665438e-03,1.784771e-03,1.904473e-03,2.024325e-03,2.143171e-03,2.260820e-03,2.377119e-03,
2.491853e-03,2.605055e-03,2.717349e-03,2.828874e-03,2.940853e-03,3.054038e-03,3.169999e-03,
3.290343e-03,3.417114e-03,3.552511e-03,3.699143e-03,3.859120e-03,4.034680e-03,4.226785e-03,
4.435314e-03,4.658182e-03,4.892305e-03,5.131845e-03,5.371783e-03,5.606701e-03,5.831779e-03,
6.043927e-03,6.241290e-03,6.422941e-03,6.588882e-03,6.739685e-03,6.876237e-03,6.999584e-03,
7.110833e-03,7.211089e-03,7.301407e-03,7.382776e-03,7.456105e-03,7.522221e-03,7.581872e-03,
7.635730e-03,7.684395e-03,7.728406e-03,7.768250e-03,7.804367e-03,7.837137e-03,7.866907e-03,
7.893992e-03,7.918663e-03,7.941175e-03,7.961745e-03,7.980577e-03,7.997848e-03,8.013718e-03,
8.028333e-03,8.041823e-03,8.054305e-03,8.065884e-03,8.076657e-03,8.086709e-03,8.096119e-03,
8.104959e-03,8.113293e-03,8.121181e-03,8.128676e-03,8.135829e-03,8.142686e-03,8.149290e-03,
8.155681e-03,8.161895e-03,8.167971e-03,8.173941e-03,8.179842e-03,8.185532e-03,8.190791e-03,
8.196173e-03,8.201721e-03,8.207490e-03,8.213549e-03,8.219997e-03,8.226961e-03,8.234619e-03,
8.243208e-03,8.253043e-03,8.264530e-03,8.278167e-03,8.294543e-03,8.314306e-03,8.338122e-03,
8.366621e-03,8.400357e-03,8.439779e-03,8.485245e-03,8.537046e-03,8.595453e-03,8.660753e-03,
8.733284e-03,8.813450e-03,8.901722e-03,8.998630e-03,9.104733e-03,9.220575e-03,9.346623e-03,
9.483175e-03,9.630255e-03,9.787493e-03,9.954018e-03,1.012840e-02,1.030865e-02,1.049233e-02,
1.067675e-02,1.085922e-02,1.103733e-02,1.120911e-02,1.137317e-02,1.152871e-02,1.167547e-02,
1.181358e-02,1.194350e-02,1.206587e-02,1.218146e-02,1.229109e-02,1.239560e-02,1.249580e-02,
1.259247e-02,1.268636e-02,1.277816e-02,1.286849e-02,1.295790e-02,1.304686e-02,1.313576e-02,
1.322486e-02,1.331430e-02,1.340408e-02,1.349406e-02,1.358398e-02,1.367348e-02,1.376212e-02,
1.384946e-02,1.393508e-02,1.401867e-02,1.409996e-02,1.417885e-02,1.425528e-02,1.432934e-02,
1.440114e-02,1.447087e-02,1.453873e-02,1.460494e-02,1.466972e-02,1.473329e-02,1.479584e-02,
1.485754e-02,1.491855e-02,1.497899e-02,1.503897e-02,1.509858e-02,1.515789e-02,1.521695e-02,
1.527579e-02,1.533444e-02,1.539291e-02,1.545122e-02,1.550935e-02,1.556730e-02,1.562507e-02,
1.568263e-02,1.573999e-02,1.579713e-02,1.585402e-02,1.591067e-02,1.596706e-02,1.602317e-02,
1.607900e-02,1.613455e-02,1.618979e-02,1.624474e-02,1.629938e-02,1.635372e-02,1.640774e-02,
1.646145e-02,1.651485e-02,1.656795e-02,1.662073e-02,1.667321e-02,1.672539e-02,1.677727e-02,
1.682885e-02,1.688015e-02,1.693115e-02,1.698188e-02,1.703233e-02,1.708251e-02,1.713242e-02,
1.718208e-02,1.723147e-02,1.728062e-02,1.732953e-02,1.737819e-02,1.742662e-02,1.747483e-02,
1.752281e-02,1.757058e-02,1.761813e-02,1.766548e-02,1.771262e-02,1.775957e-02,1.780632e-02,
1.785289e-02,1.789928e-02,1.794549e-02,1.799152e-02,1.803739e-02,1.808309e-02,1.812863e-02,
1.817402e-02,1.821925e-02,1.826434e-02,1.830928e-02,1.835409e-02,1.839875e-02,1.844329e-02,
1.848769e-02,1.853197e-02,1.857613e-02,1.862017e-02,1.866409e-02,1.870791e-02,1.875161e-02,
1.879521e-02,1.883871e-02,1.888210e-02,1.892541e-02,1.896862e-02,1.901173e-02,1.905476e-02,
1.909771e-02,1.914058e-02,1.918336e-02,1.922607e-02,1.926871e-02,1.931127e-02,1.935377e-02,
1.939619e-02,1.943856e-02,1.948086e-02,1.952311e-02,1.956530e-02,1.960743e-02,1.964952e-02,
1.969155e-02,1.973354e-02,1.977548e-02,1.981737e-02,1.985923e-02,1.990105e-02,1.994283e-02,
1.998458e-02,2.002629e-02,2.006798e-02,2.010963e-02,2.015126e-02,2.019287e-02,2.023445e-02,
2.027601e-02,2.031756e-02,2.035908e-02,2.040059e-02,2.044209e-02,2.048358e-02,2.052506e-02,
2.056653e-02,2.060799e-02,2.064945e-02,2.069091e-02,2.073236e-02,2.077382e-02,2.081528e-02
  };
  static const G4double P14[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,1.830684e-05,
6.371064e-05,1.154186e-04,1.746380e-04,2.449308e-04,3.275905e-04,4.236803e-04,5.343383e-04,
6.607827e-04,8.052700e-04,9.741837e-04,1.170286e-03,1.395863e-03,1.653360e-03,1.945987e-03,
2.281454e-03,2.664366e-03,3.098601e-03,3.588306e-03,4.137638e-03,4.751029e-03,5.433489e-03,
6.190306e-03,7.018768e-03,7.918090e-03,8.893328e-03,9.945232e-03,1.105935e-02,1.223722e-02,
1.348004e-02,1.477087e-02,1.610715e-02,1.748784e-02,1.890063e-02,2.034378e-02,2.181580e-02,
2.331419e-02,2.483956e-02,2.640072e-02,2.800033e-02,2.965740e-02,3.138528e-02,3.321162e-02,
3.516681e-02,3.729155e-02,3.963237e-02,4.224740e-02,4.519013e-02,4.852109e-02,5.228032e-02,
5.648894e-02,6.112768e-02,6.615316e-02,7.145565e-02,7.693264e-02,8.246248e-02,8.792602e-02,
9.323627e-02,9.833048e-02,1.031653e-01,1.077195e-01,1.119872e-01,1.159719e-01,1.196832e-01,
1.231347e-01,1.263418e-01,1.293208e-01,1.320879e-01,1.346591e-01,1.370493e-01,1.392727e-01,
1.413425e-01,1.432706e-01,1.450684e-01,1.467463e-01,1.483144e-01,1.497813e-01,1.511550e-01,
1.524436e-01,1.536536e-01,1.547918e-01,1.558640e-01,1.568759e-01,1.578326e-01,1.587389e-01,
1.595992e-01,1.604178e-01,1.611987e-01,1.619454e-01,1.626615e-01,1.633503e-01,1.640150e-01,
1.646587e-01,1.652843e-01,1.658945e-01,1.664923e-01,1.670804e-01,1.676615e-01,1.682383e-01,
1.688138e-01,1.693906e-01,1.699718e-01,1.705606e-01,1.711603e-01,1.717563e-01,1.723243e-01,
1.729235e-01,1.735601e-01,1.742424e-01,1.749812e-01,1.757914e-01,1.766935e-01,1.777159e-01,
1.788979e-01,1.802932e-01,1.819727e-01,1.840281e-01,1.865720e-01,1.897363e-01,1.936664e-01,
1.985139e-01,2.044278e-01,2.115504e-01,2.200166e-01,2.299583e-01,2.415112e-01,2.548236e-01,
2.700636e-01,2.874240e-01,3.071261e-01,3.294187e-01,3.545749e-01,3.828823e-01,4.146274e-01,
4.500720e-01,4.894190e-01,5.327720e-01,5.800920e-01,6.311614e-01,6.855655e-01,7.427020e-01,
8.018245e-01,8.621139e-01,9.227611e-01,9.830442e-01,1.042383e+00,1.100363e+00,1.156744e+00,
1.211429e+00,1.264444e+00,1.315911e+00,1.366015e+00,1.414994e+00,1.463113e+00,1.510663e+00,
1.557947e+00,1.605276e+00,1.652968e+00,1.701334e+00,1.750675e+00,1.801276e+00,1.853388e+00,
1.907216e+00,1.962904e+00,2.020515e+00,2.080026e+00,2.141319e+00,2.204187e+00,2.268359e+00,
2.333525e+00,2.399370e+00,2.465608e+00,2.532008e+00,2.598408e+00,2.664720e+00,2.730931e+00,
2.797093e+00,2.863310e+00,2.929727e+00,2.996517e+00,3.063869e+00,3.131981e+00,3.201051e+00,
3.271270e+00,3.342824e+00,3.415884e+00,3.490610e+00,3.567148e+00,3.645628e+00,3.726167e+00,
3.808870e+00,3.893828e+00,3.981122e+00,4.070824e+00,4.163000e+00,4.257706e+00,4.354996e+00,
4.454920e+00,4.557525e+00,4.662858e+00,4.770965e+00,4.881891e+00,4.995685e+00,5.112397e+00,
5.232077e+00,5.354782e+00,5.480568e+00,5.609497e+00,5.741634e+00,5.877047e+00,6.015807e+00,
6.157992e+00,6.303681e+00,6.452959e+00,6.605913e+00,6.762637e+00,6.923226e+00,7.087782e+00,
7.256411e+00,7.429221e+00,7.606326e+00,7.787844e+00,7.973899e+00,8.164617e+00,8.360131e+00,
8.560577e+00,8.766096e+00,8.976834e+00,9.192943e+00,9.414578e+00,9.641901e+00,9.875078e+00,
1.011428e+01,1.035969e+01,1.061148e+01,1.086986e+01,1.113500e+01,1.140711e+01,1.168641e+01,
1.197310e+01,1.226741e+01,1.256956e+01,1.287978e+01,1.319833e+01,1.352545e+01,1.386140e+01,
1.420644e+01,1.456086e+01,1.492492e+01,1.529892e+01,1.568317e+01,1.607798e+01,1.648366e+01,
1.690054e+01,1.732896e+01,1.776928e+01,1.822186e+01,1.868707e+01,1.916530e+01,1.965693e+01,
2.016239e+01,2.068210e+01,2.121649e+01,2.176601e+01,2.233113e+01,2.291232e+01,2.351008e+01,
2.412492e+01,2.475737e+01,2.540797e+01,2.607728e+01,2.676588e+01,2.747436e+01,2.820334e+01,
2.895345e+01,2.972536e+01,3.051974e+01,3.133728e+01,3.217872e+01,3.304478e+01,3.393625e+01,
3.485392e+01,3.579859e+01,3.677113e+01,3.777240e+01,3.880330e+01,3.986476e+01,4.095774e+01,
4.208324e+01,4.324227e+01,4.443590e+01,4.566522e+01,4.693134e+01,4.823544e+01,4.957871e+01,
5.096240e+01,5.238778e+01,5.385619e+01,5.536898e+01,5.692756e+01,5.853340e+01,6.018799e+01,
6.189289e+01,6.364971e+01,6.546010e+01,6.732578e+01,6.924851e+01,7.123011e+01,7.327248e+01
  };
  static const G4double P24[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,7.543486e-05,
2.672072e-04,4.930762e-04,7.606037e-04,1.088954e-03,1.488138e-03,1.967834e-03,2.538841e-03,
3.213192e-03,4.009598e-03,4.971730e-03,6.125875e-03,7.497533e-03,9.115105e-03,1.101410e-02,
1.326287e-02,1.591403e-02,1.901913e-02,2.263546e-02,2.682459e-02,3.165470e-02,3.720349e-02,
4.355667e-02,5.073638e-02,5.878240e-02,6.778949e-02,7.781758e-02,8.878053e-02,1.007435e-01,
1.137711e-01,1.277352e-01,1.426538e-01,1.585606e-01,1.753566e-01,1.930607e-01,2.116938e-01,
2.312639e-01,2.518194e-01,2.735251e-01,2.964705e-01,3.209937e-01,3.473739e-01,3.761401e-01,
4.079082e-01,4.435227e-01,4.839951e-01,5.306350e-01,5.847703e-01,6.479743e-01,7.215434e-01,
8.064902e-01,9.030479e-01,1.010929e+00,1.128315e+00,1.253346e+00,1.383526e+00,1.516159e+00,
1.649089e+00,1.780588e+00,1.909280e+00,2.034281e+00,2.155065e+00,2.271349e+00,2.383027e+00,
2.490116e+00,2.592716e+00,2.690981e+00,2.785094e+00,2.875255e+00,2.961673e+00,3.044553e+00,
3.124097e+00,3.200497e+00,3.273938e+00,3.344609e+00,3.412697e+00,3.478359e+00,3.541758e+00,
3.603063e+00,3.662412e+00,3.719967e+00,3.775859e+00,3.830236e+00,3.883236e+00,3.934991e+00,
3.985641e+00,4.035323e+00,4.084172e+00,4.132329e+00,4.179936e+00,4.227142e+00,4.274100e+00,
4.320973e+00,4.367929e+00,4.415151e+00,4.462832e+00,4.511183e+00,4.560430e+00,4.610824e+00,
4.662638e+00,4.716179e+00,4.771788e+00,4.829854e+00,4.890825e+00,4.953249e+00,5.014598e+00,
5.081309e+00,5.154369e+00,5.235071e+00,5.325147e+00,5.426966e+00,5.543825e+00,5.680343e+00,
5.843034e+00,6.040974e+00,6.286575e+00,6.596368e+00,6.991576e+00,7.498259e+00,8.146893e+00,
8.971439e+00,1.000824e+01,1.129521e+01,1.287183e+01,1.477996e+01,1.706532e+01,1.977947e+01,
2.298186e+01,2.674168e+01,3.113945e+01,3.626805e+01,4.223285e+01,4.915057e+01,5.714619e+01,
6.634717e+01,7.687414e+01,8.882816e+01,1.022756e+02,1.172331e+02,1.336549e+02,1.514294e+02,
1.703848e+02,1.903058e+02,2.109585e+02,2.321157e+02,2.535790e+02,2.751935e+02,2.968551e+02,
3.185090e+02,3.401451e+02,3.617925e+02,3.835128e+02,4.053955e+02,4.275534e+02,4.501201e+02,
4.732483e+02,4.971083e+02,5.218876e+02,5.477871e+02,5.750188e+02,6.038010e+02,6.343508e+02,
6.668734e+02,7.015497e+02,7.385227e+02,7.778839e+02,8.196641e+02,8.638306e+02,9.102926e+02,
9.589179e+02,1.009554e+03,1.062052e+03,1.116289e+03,1.172185e+03,1.229716e+03,1.288919e+03,
1.349889e+03,1.412779e+03,1.477789e+03,1.545167e+03,1.615193e+03,1.688177e+03,1.764452e+03,
1.844372e+03,1.928304e+03,2.016627e+03,2.109729e+03,2.208008e+03,2.311865e+03,2.421710e+03,
2.537958e+03,2.661032e+03,2.791360e+03,2.929384e+03,3.075553e+03,3.230332e+03,3.394201e+03,
3.567658e+03,3.751220e+03,3.945429e+03,4.150853e+03,4.368085e+03,4.597754e+03,4.840519e+03,
5.097080e+03,5.368173e+03,5.654580e+03,5.957127e+03,6.276691e+03,6.614199e+03,6.970636e+03,
7.347046e+03,7.744537e+03,8.164282e+03,8.607528e+03,9.075596e+03,9.569887e+03,1.009189e+04,
1.064318e+04,1.122542e+04,1.184040e+04,1.249000e+04,1.317620e+04,1.390113e+04,1.466703e+04,
1.547628e+04,1.633140e+04,1.723507e+04,1.819012e+04,1.919958e+04,2.026662e+04,2.139464e+04,
2.258721e+04,2.384817e+04,2.518152e+04,2.659158e+04,2.808286e+04,2.966020e+04,3.132870e+04,
3.309379e+04,3.496121e+04,3.693706e+04,3.902782e+04,4.124035e+04,4.358193e+04,4.606030e+04,
4.868365e+04,5.146069e+04,5.440064e+04,5.751330e+04,6.080907e+04,6.429899e+04,6.799476e+04,
7.190880e+04,7.605431e+04,8.044530e+04,8.509662e+04,9.002404e+04,9.524433e+04,1.007752e+05,
1.066357e+05,1.128456e+05,1.194264e+05,1.264006e+05,1.337923e+05,1.416267e+05,1.499311e+05,
1.587342e+05,1.680664e+05,1.779602e+05,1.884499e+05,1.995721e+05,2.113657e+05,2.238718e+05,
2.371343e+05,2.511997e+05,2.661175e+05,2.819400e+05,2.987232e+05,3.165263e+05,3.354123e+05,
3.554481e+05,3.767048e+05,3.992580e+05,4.231879e+05,4.485800e+05,4.755247e+05,5.041186e+05,
5.344641e+05,5.666699e+05,6.008519e+05,6.371329e+05,6.756438e+05,7.165233e+05,7.599193e+05,
8.059888e+05,8.548986e+05,9.068263e+05,9.619605e+05,1.020502e+06,1.082664e+06,1.148672e+06,
1.218769e+06,1.293211e+06,1.372270e+06,1.456236e+06,1.545418e+06,1.640143e+06,1.740759e+06
  };
  static const G4double P05[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
5.861537e-07,3.061765e-06,5.963955e-06,9.305785e-06,1.342109e-05,1.865336e-05,2.503676e-05,
3.261650e-05,4.273714e-05,5.639991e-05,7.371320e-05,9.607336e-05,1.289866e-04,1.733628e-04,
2.302289e-04,3.053102e-04,4.003118e-04,5.138222e-04,6.363068e-04,7.666567e-04,9.046405e-04,
1.049782e-03,1.203029e-03,1.370490e-03,1.553883e-03,1.754746e-03,1.974232e-03,2.209369e-03,
2.457122e-03,2.712762e-03,2.971183e-03,3.228184e-03,3.480597e-03,3.726495e-03,3.965194e-03,
4.197027e-03,4.422883e-03,4.643939e-03,4.861858e-03,5.077910e-03,5.293416e-03,5.509553e-03,
5.727123e-03,5.946622e-03,6.168285e-03,6.392006e-03,6.617371e-03,6.843719e-03,7.070191e-03,
7.295793e-03,7.519457e-03,7.740099e-03,7.956683e-03,8.168260e-03,8.374004e-03,8.573230e-03,
8.765403e-03,8.950129e-03,9.127152e-03,9.296334e-03,9.457647e-03,9.611145e-03,9.756957e-03,
9.895275e-03,1.002632e-02,1.015036e-02,1.026766e-02,1.037853e-02,1.048327e-02,1.058218e-02,
1.067556e-02,1.076370e-02,1.084690e-02,1.092544e-02,1.099957e-02,1.106956e-02,1.113566e-02,
1.119809e-02,1.125710e-02,1.131288e-02,1.136563e-02,1.141557e-02,1.146285e-02,1.150766e-02,
1.155017e-02,1.159052e-02,1.162888e-02,1.166537e-02,1.170014e-02,1.173328e-02,1.176483e-02,
1.179510e-02,1.182421e-02,1.185232e-02,1.187957e-02,1.190617e-02,1.193232e-02,1.195832e-02,
1.198450e-02,1.201129e-02,1.203924e-02,1.206900e-02,1.210130e-02,1.213697e-02,1.217683e-02,
1.222166e-02,1.227215e-02,1.232884e-02,1.239218e-02,1.246252e-02,1.254018e-02,1.262551e-02,
1.271890e-02,1.282083e-02,1.293183e-02,1.305251e-02,1.318351e-02,1.332543e-02,1.347880e-02,
1.364395e-02,1.382089e-02,1.400920e-02,1.420789e-02,1.441539e-02,1.462947e-02,1.484742e-02,
1.506624e-02,1.528290e-02,1.549464e-02,1.569920e-02,1.589495e-02,1.608089e-02,1.625664e-02,
1.642228e-02,1.657827e-02,1.672530e-02,1.686422e-02,1.699595e-02,1.712141e-02,1.724154e-02,
1.735724e-02,1.746933e-02,1.757862e-02,1.768580e-02,1.779151e-02,1.789629e-02,1.800058e-02,
1.810471e-02,1.820886e-02,1.831308e-02,1.841729e-02,1.852126e-02,1.862466e-02,1.872709e-02,
1.882813e-02,1.892735e-02,1.902442e-02,1.911908e-02,1.921116e-02,1.930062e-02,1.938748e-02,
1.947187e-02,1.955394e-02,1.963391e-02,1.971200e-02,1.978844e-02,1.986346e-02,1.993725e-02,
2.001002e-02,2.008193e-02,2.015313e-02,2.022374e-02,2.029385e-02,2.036355e-02,2.043289e-02,
2.050193e-02,2.057069e-02,2.063918e-02,2.070743e-02,2.077542e-02,2.084316e-02,2.091063e-02,
2.097784e-02,2.104476e-02,2.111138e-02,2.117769e-02,2.124368e-02,2.130933e-02,2.137464e-02,
2.143959e-02,2.150419e-02,2.156841e-02,2.163227e-02,2.169575e-02,2.175885e-02,2.182157e-02,
2.188391e-02,2.194588e-02,2.200747e-02,2.206869e-02,2.212954e-02,2.219003e-02,2.225016e-02,
2.230994e-02,2.236936e-02,2.242845e-02,2.248720e-02,2.254562e-02,2.260371e-02,2.266149e-02,
2.271896e-02,2.277613e-02,2.283300e-02,2.288957e-02,2.294587e-02,2.300189e-02,2.305764e-02,
2.311312e-02,2.316835e-02,2.322333e-02,2.327806e-02,2.333255e-02,2.338681e-02,2.344084e-02,
2.349466e-02,2.354825e-02,2.360164e-02,2.365482e-02,2.370780e-02,2.376059e-02,2.381319e-02,
2.386561e-02,2.391784e-02,2.396991e-02,2.402180e-02,2.407353e-02,2.412509e-02,2.417651e-02,
2.422776e-02,2.427888e-02,2.432984e-02,2.438067e-02,2.443137e-02,2.448193e-02,2.453236e-02,
2.458267e-02,2.463286e-02,2.468294e-02,2.473290e-02,2.478275e-02,2.483249e-02,2.488213e-02,
2.493168e-02,2.498112e-02,2.503047e-02,2.507974e-02,2.512891e-02,2.517800e-02,2.522701e-02,
2.527595e-02,2.532480e-02,2.537359e-02,2.542230e-02,2.547095e-02,2.551954e-02,2.556806e-02,
2.561653e-02,2.566494e-02,2.571329e-02,2.576160e-02,2.580985e-02,2.585807e-02,2.590623e-02,
2.595436e-02,2.600245e-02,2.605050e-02,2.609852e-02,2.614650e-02,2.619446e-02,2.624239e-02,
2.629030e-02,2.633818e-02,2.638604e-02,2.643389e-02,2.648172e-02,2.652953e-02,2.657733e-02,
2.662513e-02,2.667291e-02,2.672069e-02,2.676847e-02,2.681624e-02,2.686402e-02,2.691179e-02
  };
  static const G4double P15[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
3.974782e-06,2.111703e-05,4.187126e-05,6.655079e-05,9.794658e-05,1.391596e-04,1.910712e-04,
2.547110e-04,3.424807e-04,4.647771e-04,6.247355e-04,8.380779e-04,1.162192e-03,1.613141e-03,
2.209563e-03,3.022272e-03,4.083389e-03,5.391437e-03,6.847763e-03,8.447014e-03,1.019379e-02,
1.208961e-02,1.415500e-02,1.648372e-02,1.911481e-02,2.208779e-02,2.543914e-02,2.914260e-02,
3.316793e-02,3.745194e-02,4.191903e-02,4.650110e-02,5.114289e-02,5.580689e-02,6.047654e-02,
6.515418e-02,6.985421e-02,7.459850e-02,7.942200e-02,8.435394e-02,8.942726e-02,9.467447e-02,
1.001215e-01,1.057883e-01,1.116895e-01,1.178312e-01,1.242106e-01,1.308174e-01,1.376334e-01,
1.446344e-01,1.517909e-01,1.590700e-01,1.664371e-01,1.738573e-01,1.812967e-01,1.887238e-01,
1.961101e-01,2.034302e-01,2.106622e-01,2.177880e-01,2.247927e-01,2.316643e-01,2.383937e-01,
2.449747e-01,2.514023e-01,2.576744e-01,2.637892e-01,2.697473e-01,2.755497e-01,2.811981e-01,
2.866954e-01,2.920447e-01,2.972497e-01,3.023141e-01,3.072423e-01,3.120386e-01,3.167076e-01,
3.212540e-01,3.256827e-01,3.299983e-01,3.342061e-01,3.383109e-01,3.423179e-01,3.462322e-01,
3.500593e-01,3.538045e-01,3.574734e-01,3.610719e-01,3.646061e-01,3.680773e-01,3.714851e-01,
3.748540e-01,3.781939e-01,3.815172e-01,3.848393e-01,3.881806e-01,3.915678e-01,3.950376e-01,
3.986393e-01,4.024394e-01,4.065252e-01,4.110083e-01,4.160253e-01,4.217350e-01,4.283120e-01,
4.359366e-01,4.447861e-01,4.550284e-01,4.668217e-01,4.803198e-01,4.956807e-01,5.130757e-01,
5.326983e-01,5.547703e-01,5.795448e-01,6.073059e-01,6.383637e-01,6.730439e-01,7.116699e-01,
7.545360e-01,8.018700e-01,8.537886e-01,9.102508e-01,9.710179e-01,1.035633e+00,1.103430e+00,
1.173580e+00,1.245163e+00,1.317264e+00,1.389054e+00,1.459853e+00,1.529164e+00,1.596682e+00,
1.662267e+00,1.725924e+00,1.787763e+00,1.847980e+00,1.906827e+00,1.964596e+00,2.021606e+00,
2.078191e+00,2.134696e+00,2.191472e+00,2.248861e+00,2.307198e+00,2.366794e+00,2.427929e+00,
2.490835e+00,2.555683e+00,2.622564e+00,2.691483e+00,2.762350e+00,2.834987e+00,2.909144e+00,
2.984527e+00,3.060829e+00,3.137758e+00,3.215070e+00,3.292583e+00,3.370189e+00,3.447853e+00,
3.525611e+00,3.603556e+00,3.681827e+00,3.760598e+00,3.840066e+00,3.920438e+00,4.001928e+00,
4.084743e+00,4.169087e+00,4.255151e+00,4.343112e+00,4.433133e+00,4.525363e+00,4.619934e+00,
4.716965e+00,4.816561e+00,4.918816e+00,5.023813e+00,5.131626e+00,5.242324e+00,5.355968e+00,
5.472618e+00,5.592327e+00,5.715152e+00,5.841146e+00,5.970363e+00,6.102860e+00,6.238695e+00,
6.377929e+00,6.520625e+00,6.666852e+00,6.816680e+00,6.970186e+00,7.127448e+00,7.288551e+00,
7.453583e+00,7.622638e+00,7.795812e+00,7.973209e+00,8.154935e+00,8.341103e+00,8.531830e+00,
8.727236e+00,8.927450e+00,9.132601e+00,9.342828e+00,9.558272e+00,9.779080e+00,1.000540e+01,
1.023740e+01,1.047524e+01,1.071908e+01,1.096910e+01,1.122548e+01,1.148841e+01,1.175808e+01,
1.203469e+01,1.231843e+01,1.260954e+01,1.290821e+01,1.321468e+01,1.352918e+01,1.385194e+01,
1.418323e+01,1.452328e+01,1.487236e+01,1.523075e+01,1.559872e+01,1.597656e+01,1.636457e+01,
1.676306e+01,1.717234e+01,1.759273e+01,1.802457e+01,1.846822e+01,1.892402e+01,1.939234e+01,
1.987358e+01,2.036810e+01,2.087633e+01,2.139868e+01,2.193558e+01,2.248747e+01,2.305481e+01,
2.363808e+01,2.423775e+01,2.485434e+01,2.548836e+01,2.614035e+01,2.681085e+01,2.750044e+01,
2.820970e+01,2.893925e+01,2.968970e+01,3.046170e+01,3.125593e+01,3.207305e+01,3.291380e+01,
3.377889e+01,3.466908e+01,3.558515e+01,3.652791e+01,3.749819e+01,3.849684e+01,3.952475e+01,
4.058284e+01,4.167204e+01,4.279334e+01,4.394772e+01,4.513625e+01,4.635997e+01,4.762000e+01,
4.891749e+01,5.025360e+01,5.162956e+01,5.304662e+01,5.450609e+01,5.600929e+01,5.755762e+01,
5.915250e+01,6.079540e+01,6.248786e+01,6.423144e+01,6.602777e+01,6.787852e+01,6.978543e+01,
7.175028e+01,7.377494e+01,7.586129e+01,7.801133e+01,8.022707e+01,8.251063e+01,8.486417e+01
  };
  static const G4double P25[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
2.695366e-05,1.456646e-04,2.940957e-04,4.763706e-04,7.159105e-04,1.040560e-03,1.462756e-03,
1.997124e-03,2.758357e-03,3.853132e-03,5.331120e-03,7.366831e-03,1.055881e-02,1.514171e-02,
2.139761e-02,3.019542e-02,4.204851e-02,5.712316e-02,7.444010e-02,9.406277e-02,1.161775e-01,
1.409425e-01,1.687811e-01,2.011671e-01,2.389176e-01,2.829245e-01,3.341005e-01,3.924354e-01,
4.578416e-01,5.296390e-01,6.068635e-01,6.885638e-01,7.739318e-01,8.624020e-01,9.537611e-01,
1.048148e+00,1.145963e+00,1.247793e+00,1.354566e+00,1.467159e+00,1.586602e+00,1.714000e+00,
1.850381e+00,1.996693e+00,2.153810e+00,2.322424e+00,2.503024e+00,2.695880e+00,2.901036e+00,
3.118309e+00,3.347312e+00,3.587475e+00,3.838086e+00,4.098335e+00,4.367355e+00,4.644260e+00,
4.928176e+00,5.218268e+00,5.513750e+00,5.813904e+00,6.118091e+00,6.425733e+00,6.736332e+00,
7.049470e+00,7.364768e+00,7.681941e+00,8.000723e+00,8.320924e+00,8.642387e+00,8.964984e+00,
9.288638e+00,9.613299e+00,9.938942e+00,1.026557e+01,1.059321e+01,1.092191e+01,1.125175e+01,
1.158282e+01,1.191526e+01,1.224919e+01,1.258480e+01,1.292228e+01,1.326186e+01,1.360381e+01,
1.394841e+01,1.429602e+01,1.464702e+01,1.500188e+01,1.536112e+01,1.572479e+01,1.609281e+01,
1.646782e+01,1.685103e+01,1.724405e+01,1.764902e+01,1.806883e+01,1.850751e+01,1.897069e+01,
1.946627e+01,2.000522e+01,2.060253e+01,2.127807e+01,2.205730e+01,2.297138e+01,2.405664e+01,
2.535341e+01,2.690469e+01,2.875518e+01,3.095123e+01,3.354183e+01,3.658031e+01,4.012667e+01,
4.424985e+01,4.902993e+01,5.455981e+01,6.094631e+01,6.831029e+01,7.678525e+01,8.651380e+01,
9.764114e+01,1.103048e+02,1.246205e+02,1.406659e+02,1.584635e+02,1.779675e+02,1.990585e+02,
2.215491e+02,2.452021e+02,2.697554e+02,2.949511e+02,3.205598e+02,3.463985e+02,3.723392e+02,
3.983095e+02,4.242881e+02,4.502985e+02,4.764027e+02,5.026947e+02,5.292959e+02,5.563519e+02,
5.840293e+02,6.125149e+02,6.420137e+02,6.727452e+02,7.049409e+02,7.388395e+02,7.746786e+02,
8.126858e+02,8.530658e+02,8.959874e+02,9.415706e+02,9.898771e+02,1.040906e+03,1.094598e+03,
1.150848e+03,1.209525e+03,1.270497e+03,1.333646e+03,1.398899e+03,1.466229e+03,1.535674e+03,
1.607330e+03,1.681358e+03,1.757972e+03,1.837437e+03,1.920058e+03,2.006179e+03,2.096171e+03,
2.190427e+03,2.289361e+03,2.393404e+03,2.502995e+03,2.618587e+03,2.740640e+03,2.869622e+03,
3.006011e+03,3.150290e+03,3.302954e+03,3.464511e+03,3.635478e+03,3.816392e+03,4.007807e+03,
4.210296e+03,4.424458e+03,4.650917e+03,4.890329e+03,5.143382e+03,5.410798e+03,5.693341e+03,
5.991817e+03,6.307078e+03,6.640026e+03,6.991616e+03,7.362858e+03,7.754825e+03,8.168654e+03,
8.605548e+03,9.066788e+03,9.553727e+03,1.006780e+04,1.061054e+04,1.118357e+04,1.178858e+04,
1.242741e+04,1.310199e+04,1.381436e+04,1.456669e+04,1.536129e+04,1.620059e+04,1.708718e+04,
1.802382e+04,1.901340e+04,2.005902e+04,2.116395e+04,2.233167e+04,2.356584e+04,2.487038e+04,
2.624943e+04,2.770738e+04,2.924888e+04,3.087888e+04,3.260261e+04,3.442562e+04,3.635382e+04,
3.839344e+04,4.055112e+04,4.283390e+04,4.524923e+04,4.780502e+04,5.050969e+04,5.337213e+04,
5.640180e+04,5.960874e+04,6.300357e+04,6.659759e+04,7.040280e+04,7.443190e+04,7.869840e+04,
8.321661e+04,8.800174e+04,9.306993e+04,9.843832e+04,1.041251e+05,1.101495e+05,1.165321e+05,
1.232946e+05,1.304601e+05,1.380532e+05,1.460998e+05,1.546276e+05,1.636660e+05,1.732461e+05,
1.834010e+05,1.941660e+05,2.055782e+05,2.176775e+05,2.305058e+05,2.441079e+05,2.585314e+05,
2.738267e+05,2.900474e+05,3.072504e+05,3.254964e+05,3.448495e+05,3.653781e+05,3.871546e+05,
4.102564e+05,4.347651e+05,4.607680e+05,4.883575e+05,5.176318e+05,5.486956e+05,5.816597e+05,
6.166422e+05,6.537685e+05,6.931719e+05,7.349941e+05,7.793856e+05,8.265066e+05,8.765271e+05,
9.296281e+05,9.860018e+05,1.045853e+06,1.109398e+06,1.176869e+06,1.248512e+06,1.324586e+06,
1.405371e+06,1.491162e+06,1.582272e+06,1.679036e+06,1.781809e+06,1.890967e+06,2.006914e+06
  };
  static const G4double P06[nE]={
1.177941e-08,2.588247e-08,4.232501e-08,6.117863e-08,8.251710e-08,1.064164e-07,1.329550e-07,
1.622136e-07,1.942754e-07,2.292262e-07,2.671543e-07,3.081509e-07,3.523099e-07,4.005143e-07,
4.568609e-07,5.221504e-07,5.966563e-07,6.806607e-07,7.744541e-07,8.783361e-07,9.926154e-07,
1.117610e-06,1.253648e-06,1.407254e-06,1.585177e-06,1.788160e-06,2.016972e-06,2.272402e-06,
2.555265e-06,2.866400e-06,3.210941e-06,3.607791e-06,4.060525e-06,4.570854e-06,5.140538e-06,
5.771395e-06,6.471521e-06,7.274318e-06,8.187243e-06,9.213663e-06,1.035705e-05,1.162722e-05,
1.307801e-05,1.472581e-05,1.657663e-05,1.863670e-05,2.096335e-05,2.361557e-05,2.660333e-05,
2.993756e-05,3.371561e-05,3.801233e-05,4.284356e-05,4.826338e-05,5.444679e-05,6.143438e-05,
6.927898e-05,7.822982e-05,8.836866e-05,9.979567e-05,1.128693e-04,1.276837e-04,1.445433e-04,
1.638882e-04,1.858924e-04,2.113879e-04,2.406623e-04,2.749277e-04,3.149839e-04,3.626925e-04,
4.199591e-04,4.903319e-04,5.776505e-04,6.896213e-04,8.327844e-04,1.019672e-03,1.256072e-03,
1.545358e-03,1.876495e-03,2.234266e-03,2.599517e-03,2.965057e-03,3.327538e-03,3.688496e-03,
4.049256e-03,4.410120e-03,4.770247e-03,5.127797e-03,5.480662e-03,5.826902e-03,6.164910e-03,
6.493445e-03,6.811580e-03,7.118638e-03,7.414125e-03,7.697697e-03,7.969130e-03,8.228314e-03,
8.475244e-03,8.710011e-03,8.932790e-03,9.143846e-03,9.343514e-03,9.532163e-03,9.710217e-03,
9.878136e-03,1.003638e-02,1.018543e-02,1.032577e-02,1.045788e-02,1.058222e-02,1.069925e-02,
1.080940e-02,1.091309e-02,1.101074e-02,1.110271e-02,1.118937e-02,1.127107e-02,1.134814e-02,
1.142087e-02,1.148956e-02,1.155449e-02,1.161591e-02,1.167407e-02,1.172919e-02,1.178149e-02,
1.183119e-02,1.187846e-02,1.192350e-02,1.196648e-02,1.200757e-02,1.204672e-02,1.208376e-02,
1.211950e-02,1.215412e-02,1.218777e-02,1.222066e-02,1.225302e-02,1.228514e-02,1.231736e-02,
1.235014e-02,1.238405e-02,1.241978e-02,1.245819e-02,1.250024e-02,1.254701e-02,1.259955e-02,
1.265887e-02,1.272581e-02,1.280106e-02,1.288513e-02,1.297844e-02,1.308135e-02,1.319425e-02,
1.331759e-02,1.345191e-02,1.359785e-02,1.375610e-02,1.392741e-02,1.411249e-02,1.431192e-02,
1.452607e-02,1.475493e-02,1.499797e-02,1.525402e-02,1.552116e-02,1.579678e-02,1.607760e-02,
1.636002e-02,1.664033e-02,1.691510e-02,1.718143e-02,1.743714e-02,1.768081e-02,1.791177e-02,
1.812995e-02,1.833578e-02,1.853000e-02,1.871357e-02,1.888757e-02,1.905313e-02,1.921139e-02,
1.936343e-02,1.951030e-02,1.965297e-02,1.979231e-02,1.992912e-02,2.006408e-02,2.019774e-02,
2.033054e-02,2.046278e-02,2.059460e-02,2.072598e-02,2.085680e-02,2.098677e-02,2.111556e-02,
2.124276e-02,2.136797e-02,2.149083e-02,2.161106e-02,2.172847e-02,2.184294e-02,2.195448e-02,
2.206318e-02,2.216917e-02,2.227266e-02,2.237386e-02,2.247304e-02,2.257041e-02,2.266623e-02,
2.276069e-02,2.285400e-02,2.294633e-02,2.303782e-02,2.312859e-02,2.321873e-02,2.330833e-02,
2.339744e-02,2.348610e-02,2.357434e-02,2.366216e-02,2.374958e-02,2.383660e-02,2.392321e-02,
2.400939e-02,2.409515e-02,2.418046e-02,2.426532e-02,2.434971e-02,2.443362e-02,2.451703e-02,
2.459996e-02,2.468237e-02,2.476428e-02,2.484568e-02,2.492656e-02,2.500692e-02,2.508678e-02,
2.516612e-02,2.524495e-02,2.532328e-02,2.540111e-02,2.547844e-02,2.555530e-02,2.563167e-02,
2.570757e-02,2.578302e-02,2.585800e-02,2.593255e-02,2.600666e-02,2.608034e-02,2.615360e-02,
2.622645e-02,2.629891e-02,2.637098e-02,2.644266e-02,2.651397e-02,2.658492e-02,2.665552e-02,
2.672577e-02,2.679569e-02,2.686527e-02,2.693454e-02,2.700349e-02,2.707214e-02,2.714049e-02,
2.720856e-02,2.727634e-02,2.734385e-02,2.741110e-02,2.747808e-02,2.754481e-02,2.761130e-02,
2.767754e-02,2.774355e-02,2.780934e-02,2.787490e-02,2.794025e-02,2.800540e-02,2.807034e-02,
2.813508e-02,2.819963e-02,2.826399e-02,2.832817e-02,2.839218e-02,2.845601e-02,2.851968e-02,
2.858319e-02,2.864654e-02,2.870974e-02,2.877279e-02,2.883570e-02,2.889848e-02,2.896112e-02,
2.902362e-02,2.908601e-02,2.914827e-02,2.921042e-02,2.927245e-02,2.933437e-02,2.939619e-02,
2.945791e-02,2.951952e-02,2.958105e-02,2.964248e-02,2.970383e-02,2.976509e-02,2.982628e-02,
2.988738e-02,2.994842e-02,3.000938e-02,3.007028e-02,3.013111e-02,3.019189e-02,3.025260e-02,
3.031327e-02,3.037388e-02,3.043444e-02,3.049496e-02,3.055544e-02,3.061588e-02,3.067628e-02,
3.073665e-02,3.079699e-02,3.085730e-02,3.091759e-02,3.097785e-02,3.103809e-02,3.109832e-02,
3.115854e-02,3.121874e-02,3.127894e-02,3.133912e-02,3.139931e-02,3.145949e-02,3.151968e-02
  };
  static const G4double P16[nE]={
1.804258e-08,4.051712e-08,6.777154e-08,1.002656e-07,1.384927e-07,1.829820e-07,2.343007e-07,
2.930569e-07,3.599020e-07,4.355335e-07,5.206986e-07,6.161967e-07,7.228834e-07,8.436657e-07,
9.900430e-07,1.165832e-06,1.373705e-06,1.616528e-06,1.897372e-06,2.219523e-06,2.586504e-06,
3.002083e-06,3.470293e-06,4.017609e-06,4.673657e-06,5.448091e-06,6.351250e-06,7.394207e-06,
8.588820e-06,9.947777e-06,1.150413e-05,1.335773e-05,1.554391e-05,1.809134e-05,2.103071e-05,
2.439490e-05,2.825371e-05,3.282620e-05,3.819883e-05,4.443975e-05,5.162192e-05,5.986434e-05,
6.958951e-05,8.099809e-05,9.423242e-05,1.094449e-04,1.271888e-04,1.480744e-04,1.723673e-04,
2.003571e-04,2.331035e-04,2.715493e-04,3.161734e-04,3.678507e-04,4.287074e-04,4.996878e-04,
5.819333e-04,6.787875e-04,7.920050e-04,9.236902e-04,1.079162e-03,1.260946e-03,1.474428e-03,
1.727161e-03,2.023776e-03,2.378366e-03,2.798406e-03,3.305670e-03,3.917386e-03,4.669086e-03,
5.599801e-03,6.779798e-03,8.289933e-03,1.028773e-02,1.292223e-02,1.646960e-02,2.109720e-02,
2.693730e-02,3.383013e-02,4.151002e-02,4.959508e-02,5.793931e-02,6.647249e-02,7.523542e-02,
8.426714e-02,9.358336e-02,1.031704e-01,1.129855e-01,1.229736e-01,1.330792e-01,1.432514e-01,
1.534460e-01,1.636247e-01,1.737542e-01,1.838047e-01,1.937493e-01,2.035636e-01,2.132257e-01,
2.227163e-01,2.320190e-01,2.411202e-01,2.500095e-01,2.586795e-01,2.671245e-01,2.753418e-01,
2.833311e-01,2.910927e-01,2.986297e-01,3.059453e-01,3.130445e-01,3.199327e-01,3.266157e-01,
3.331003e-01,3.393933e-01,3.455017e-01,3.514328e-01,3.571940e-01,3.627927e-01,3.682364e-01,
3.735326e-01,3.786887e-01,3.837123e-01,3.886108e-01,3.933918e-01,3.980628e-01,4.026315e-01,
4.071056e-01,4.114928e-01,4.158014e-01,4.200395e-01,4.242160e-01,4.283169e-01,4.323171e-01,
4.362959e-01,4.402669e-01,4.442464e-01,4.482555e-01,4.523215e-01,4.564807e-01,4.607821e-01,
4.652924e-01,4.701007e-01,4.753240e-01,4.811110e-01,4.876429e-01,4.951294e-01,5.037993e-01,
5.138871e-01,5.256210e-01,5.392151e-01,5.548691e-01,5.727754e-01,5.931301e-01,6.161452e-01,
6.420599e-01,6.711473e-01,7.037187e-01,7.401224e-01,7.807376e-01,8.259614e-01,8.761869e-01,
9.317722e-01,9.929958e-01,1.060005e+00,1.132763e+00,1.211000e+00,1.294186e+00,1.381542e+00,
1.472082e+00,1.564697e+00,1.658261e+00,1.751727e+00,1.844213e+00,1.935045e+00,2.023775e+00,
2.110166e+00,2.194160e+00,2.275844e+00,2.355414e+00,2.433149e+00,2.509381e+00,2.584481e+00,
2.658844e+00,2.732878e+00,2.806997e+00,2.881608e+00,2.957106e+00,3.033862e+00,3.112213e+00,
3.192444e+00,3.274781e+00,3.359367e+00,3.446260e+00,3.535423e+00,3.626726e+00,3.719962e+00,
3.814868e+00,3.911152e+00,4.008524e+00,4.106726e+00,4.205551e+00,4.304862e+00,4.404592e+00,
4.504750e+00,4.605408e+00,4.706697e+00,4.808789e+00,4.911889e+00,5.016222e+00,5.122024e+00,
5.229531e+00,5.338975e+00,5.450581e+00,5.564558e+00,5.681101e+00,5.800388e+00,5.922581e+00,
6.047825e+00,6.176250e+00,6.307973e+00,6.443097e+00,6.581718e+00,6.723919e+00,6.869782e+00,
7.019380e+00,7.172786e+00,7.330069e+00,7.491299e+00,7.656547e+00,7.825885e+00,7.999388e+00,
8.177134e+00,8.359207e+00,8.545692e+00,8.736681e+00,8.932270e+00,9.132563e+00,9.337664e+00,
9.547688e+00,9.762753e+00,9.982984e+00,1.020851e+01,1.043947e+01,1.067600e+01,1.091825e+01,
1.116638e+01,1.142055e+01,1.168091e+01,1.194766e+01,1.222096e+01,1.250100e+01,1.278798e+01,
1.308209e+01,1.338354e+01,1.369254e+01,1.400932e+01,1.433410e+01,1.466711e+01,1.500860e+01,
1.535881e+01,1.571802e+01,1.608647e+01,1.646446e+01,1.685226e+01,1.725016e+01,1.765847e+01,
1.807750e+01,1.850756e+01,1.894900e+01,1.940215e+01,1.986736e+01,2.034499e+01,2.083543e+01,
2.133906e+01,2.185627e+01,2.238747e+01,2.293309e+01,2.349357e+01,2.406936e+01,2.466091e+01,
2.526871e+01,2.589326e+01,2.653505e+01,2.719463e+01,2.787252e+01,2.856929e+01,2.928551e+01,
3.002178e+01,3.077871e+01,3.155693e+01,3.235711e+01,3.317990e+01,3.402601e+01,3.489614e+01,
3.579106e+01,3.671150e+01,3.765827e+01,3.863217e+01,3.963405e+01,4.066476e+01,4.172521e+01,
4.281631e+01,4.393901e+01,4.509429e+01,4.628318e+01,4.750671e+01,4.876596e+01,5.006205e+01,
5.139612e+01,5.276937e+01,5.418302e+01,5.563833e+01,5.713661e+01,5.867921e+01,6.026751e+01,
6.190297e+01,6.358705e+01,6.532129e+01,6.710728e+01,6.894664e+01,7.084106e+01,7.279228e+01,
7.480211e+01,7.687239e+01,7.900504e+01,8.120204e+01,8.346544e+01,8.579735e+01,8.819995e+01,
9.067549e+01,9.322629e+01,9.585477e+01,9.856339e+01,1.013547e+02,1.042314e+02,1.071962e+02
};
  static const G4double P26[nE]={
2.763953e-08,6.345949e-08,1.086410e-07,1.646514e-07,2.331424e-07,3.159704e-07,4.152194e-07,
5.332258e-07,6.726060e-07,8.362873e-07,1.027541e-06,1.250021e-06,1.507800e-06,1.810470e-06,
2.190769e-06,2.664122e-06,3.244155e-06,3.946134e-06,4.787143e-06,5.786279e-06,6.964869e-06,
8.346712e-06,9.958338e-06,1.190868e-05,1.432794e-05,1.728288e-05,2.084815e-05,2.510710e-05,
3.015277e-05,3.608891e-05,4.311988e-05,5.177849e-05,6.233615e-05,7.505340e-05,9.022094e-05,
1.081628e-04,1.294330e-04,1.554788e-04,1.870999e-04,2.250498e-04,2.701684e-04,3.236600e-04,
3.888571e-04,4.678518e-04,5.624921e-04,6.748370e-04,8.101707e-04,9.746542e-04,1.172191e-03,
1.407177e-03,1.691031e-03,2.035063e-03,2.447271e-03,2.940049e-03,3.539047e-03,4.260131e-03,
5.122493e-03,6.170607e-03,7.434979e-03,8.952649e-03,1.080167e-02,1.303248e-02,1.573588e-02,
1.903802e-02,2.303667e-02,2.796869e-02,3.399608e-02,4.150621e-02,5.084875e-02,6.269354e-02,
7.782102e-02,9.760857e-02,1.237277e-01,1.593755e-01,2.078596e-01,2.751986e-01,3.657927e-01,
4.837016e-01,6.271917e-01,7.920609e-01,9.710429e-01,1.161533e+00,1.362429e+00,1.575182e+00,
1.801311e+00,2.041842e+00,2.297083e+00,2.566537e+00,2.849279e+00,3.144251e+00,3.450405e+00,
3.766771e+00,4.092464e+00,4.426651e+00,4.768529e+00,5.117305e+00,5.472190e+00,5.832406e+00,
6.197200e+00,6.565854e+00,6.937695e+00,7.312125e+00,7.688622e+00,8.066700e+00,8.445965e+00,
8.826109e+00,9.206837e+00,9.587977e+00,9.969349e+00,1.035087e+01,1.073249e+01,1.111417e+01,
1.149595e+01,1.187788e+01,1.226005e+01,1.264257e+01,1.302559e+01,1.340927e+01,1.379384e+01,
1.417951e+01,1.456656e+01,1.495527e+01,1.534597e+01,1.573905e+01,1.613491e+01,1.653401e+01,
1.693687e+01,1.734407e+01,1.775627e+01,1.817421e+01,1.859874e+01,1.902838e+01,1.946037e+01,
1.990328e+01,2.035890e+01,2.082955e+01,2.131826e+01,2.182915e+01,2.236781e+01,2.294202e+01,
2.356264e+01,2.424460e+01,2.500821e+01,2.588024e+01,2.689479e+01,2.809335e+01,2.952398e+01,
3.123968e+01,3.329659e+01,3.575267e+01,3.866763e+01,4.210424e+01,4.613052e+01,5.082266e+01,
5.626792e+01,6.256728e+01,6.983744e+01,7.821214e+01,8.784217e+01,9.889362e+01,1.115436e+02,
1.259725e+02,1.423521e+02,1.608289e+02,1.815051e+02,2.044192e+02,2.295289e+02,2.567044e+02,
2.857326e+02,3.163352e+02,3.481975e+02,3.810013e+02,4.144549e+02,4.483161e+02,4.824071e+02,
5.166161e+02,5.508948e+02,5.852521e+02,6.197460e+02,6.544766e+02,6.895795e+02,7.252210e+02,
7.615940e+02,7.989157e+02,8.374251e+02,8.773781e+02,9.190446e+02,9.627033e+02,1.008634e+03,
1.057108e+03,1.108378e+03,1.162661e+03,1.220133e+03,1.280910e+03,1.345053e+03,1.412558e+03,
1.483376e+03,1.557421e+03,1.634595e+03,1.714809e+03,1.798003e+03,1.884166e+03,1.973341e+03,
2.065641e+03,2.161241e+03,2.260386e+03,2.363376e+03,2.470569e+03,2.582365e+03,2.699205e+03,
2.821563e+03,2.949940e+03,3.084860e+03,3.226865e+03,3.376512e+03,3.534371e+03,3.701025e+03,
3.877070e+03,4.063111e+03,4.259770e+03,4.467683e+03,4.687502e+03,4.919903e+03,5.165583e+03,
5.425268e+03,5.699711e+03,5.989703e+03,6.296071e+03,6.619683e+03,6.961453e+03,7.322347e+03,
7.703383e+03,8.105638e+03,8.530251e+03,8.978428e+03,9.451447e+03,9.950663e+03,1.047751e+04,
1.103351e+04,1.162029e+04,1.223954e+04,1.289309e+04,1.358286e+04,1.431090e+04,1.507936e+04,
1.589056e+04,1.674691e+04,1.765102e+04,1.860561e+04,1.961359e+04,2.067805e+04,2.180224e+04,
2.298963e+04,2.424390e+04,2.556894e+04,2.696888e+04,2.844810e+04,3.001123e+04,3.166322e+04,
3.340927e+04,3.525492e+04,3.720606e+04,3.926890e+04,4.145005e+04,4.375653e+04,4.619574e+04,
4.877559e+04,5.150442e+04,5.439111e+04,5.744506e+04,6.067625e+04,6.409526e+04,6.771333e+04,
7.154238e+04,7.559504e+04,7.988472e+04,8.442567e+04,8.923299e+04,9.432270e+04,9.971181e+04,
1.054184e+05,1.114616e+05,1.178617e+05,1.246404e+05,1.318205e+05,1.394264e+05,1.474839e+05,
1.560204e+05,1.650649e+05,1.746485e+05,1.848038e+05,1.955658e+05,2.069713e+05,2.190597e+05,
2.318726e+05,2.454544e+05,2.598521e+05,2.751156e+05,2.912980e+05,3.084555e+05,3.266481e+05,
3.459394e+05,3.663968e+05,3.880920e+05,4.111014e+05,4.355058e+05,4.613913e+05,4.888493e+05,
5.179768e+05,5.488770e+05,5.816596e+05,6.164410e+05,6.533451e+05,6.925033e+05,7.340555e+05,
7.781503e+05,8.249455e+05,8.746091e+05,9.273193e+05,9.832659e+05,1.042650e+06,1.105687e+06,
1.172603e+06,1.243642e+06,1.319059e+06,1.399130e+06,1.484145e+06,1.574413e+06,1.670262e+06,
1.772044e+06,1.880129e+06,1.994914e+06,2.116817e+06,2.246287e+06,2.383799e+06,2.529858e+06
  };
  static const G4double P07[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,5.615850e-06,7.317061e-05,
1.569824e-04,2.639676e-04,4.016252e-04,5.863944e-04,8.328386e-04,1.177938e-03,1.649296e-03,
2.302400e-03,3.161734e-03,4.237816e-03,5.456960e-03,6.730797e-03,7.960891e-03,9.085925e-03,
1.009134e-02,1.098015e-02,1.176593e-02,1.246461e-02,1.309051e-02,1.365573e-02,1.417019e-02,
1.464189e-02,1.507718e-02,1.548101e-02,1.585724e-02,1.620889e-02,1.653832e-02,1.684741e-02,
1.713770e-02,1.741048e-02,1.766689e-02,1.790792e-02,1.813454e-02,1.834756e-02,1.854780e-02,
1.873604e-02,1.891300e-02,1.907937e-02,1.923580e-02,1.938293e-02,1.952135e-02,1.965163e-02,
1.977430e-02,1.988986e-02,1.999879e-02,2.010154e-02,2.019852e-02,2.029015e-02,2.037679e-02,
2.045879e-02,2.053649e-02,2.061020e-02,2.068021e-02,2.074680e-02,2.081022e-02,2.087073e-02,
2.092857e-02,2.098394e-02,2.103706e-02,2.108813e-02,2.113735e-02,2.118465e-02,2.122987e-02,
2.127393e-02,2.131701e-02,2.135933e-02,2.140114e-02,2.144273e-02,2.148447e-02,2.152683e-02,
2.157041e-02,2.161596e-02,2.166444e-02,2.171699e-02,2.177491e-02,2.183961e-02,2.191251e-02,
2.199488e-02,2.208782e-02,2.219215e-02,2.230849e-02,2.243729e-02,2.257897e-02,2.273395e-02,
2.290275e-02,2.308599e-02,2.328443e-02,2.349891e-02,2.373031e-02,2.397948e-02,2.424715e-02,
2.453375e-02,2.483929e-02,2.516314e-02,2.550390e-02,2.585931e-02,2.622620e-02,2.660061e-02,
2.697807e-02,2.735392e-02,2.772372e-02,2.808361e-02,2.843053e-02,2.876237e-02,2.907794e-02,
2.937688e-02,2.965945e-02,2.992642e-02,3.017888e-02,3.041812e-02,3.064554e-02,3.086254e-02,
3.107052e-02,3.127081e-02,3.146466e-02,3.165321e-02,3.183749e-02,3.201839e-02,3.219668e-02,
3.237296e-02,3.254769e-02,3.272115e-02,3.289347e-02,3.306460e-02,3.323440e-02,3.340257e-02,
3.356878e-02,3.373266e-02,3.389385e-02,3.405206e-02,3.420704e-02,3.435868e-02,3.450692e-02,
3.465181e-02,3.479348e-02,3.493210e-02,3.506790e-02,3.520114e-02,3.533207e-02,3.546095e-02,
3.558802e-02,3.571351e-02,3.583761e-02,3.596050e-02,3.608232e-02,3.620319e-02,3.632322e-02,
3.644246e-02,3.656098e-02,3.667881e-02,3.679597e-02,3.691248e-02,3.702833e-02,3.714354e-02,
3.725808e-02,3.737196e-02,3.748515e-02,3.759766e-02,3.770946e-02,3.782055e-02,3.793092e-02,
3.804056e-02,3.814948e-02,3.825765e-02,3.836510e-02,3.847181e-02,3.857779e-02,3.868304e-02,
3.878758e-02,3.889140e-02,3.899452e-02,3.909694e-02,3.919867e-02,3.929974e-02,3.940014e-02,
3.949989e-02,3.959900e-02,3.969749e-02,3.979536e-02,3.989264e-02,3.998933e-02,4.008545e-02,
4.018101e-02,4.027603e-02,4.037051e-02,4.046447e-02,4.055793e-02,4.065089e-02,4.074337e-02,
4.083538e-02,4.092693e-02,4.101803e-02,4.110870e-02,4.119895e-02,4.128879e-02,4.137822e-02,
4.146726e-02,4.155592e-02,4.164422e-02,4.173215e-02,4.181972e-02,4.190696e-02,4.199387e-02,
4.208045e-02,4.216671e-02,4.225267e-02,4.233834e-02,4.242371e-02,4.250880e-02,4.259361e-02,
4.267816e-02,4.276246e-02,4.284650e-02,4.293029e-02,4.301385e-02,4.309718e-02,4.318028e-02,
4.326316e-02,4.334584e-02,4.342831e-02,4.351058e-02,4.359266e-02,4.367456e-02,4.375627e-02,
4.383781e-02,4.391918e-02,4.400038e-02,4.408143e-02,4.416233e-02,4.424307e-02,4.432368e-02,
4.440415e-02,4.448448e-02,4.456469e-02,4.464477e-02,4.472474e-02,4.480460e-02,4.488434e-02,
4.496398e-02,4.504353e-02,4.512297e-02,4.520233e-02,4.528160e-02,4.536079e-02,4.543990e-02,
4.551894e-02,4.559790e-02,4.567681e-02,4.575564e-02,4.583443e-02,4.591315e-02,4.599183e-02,
4.607046e-02,4.614905e-02,4.622760e-02,4.630612e-02,4.638460e-02,4.646306e-02,4.654149e-02,
4.661990e-02,4.669829e-02,4.677667e-02,4.685504e-02,4.693340e-02,4.701176e-02,4.709012e-02
  };
  static const G4double P17[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,8.694188e-05,1.151450e-03,
2.513681e-03,4.307796e-03,6.688706e-03,9.985798e-03,1.452144e-02,2.107299e-02,3.030290e-02,
4.349128e-02,6.138684e-02,8.449316e-02,1.114863e-01,1.405633e-01,1.695167e-01,1.968221e-01,
2.219850e-01,2.449239e-01,2.658362e-01,2.850105e-01,3.027228e-01,3.192162e-01,3.346962e-01,
3.493314e-01,3.632567e-01,3.765775e-01,3.893736e-01,4.017050e-01,4.136155e-01,4.251374e-01,
4.362941e-01,4.471031e-01,4.575777e-01,4.677296e-01,4.775694e-01,4.871054e-01,4.963468e-01,
5.053029e-01,5.139821e-01,5.223946e-01,5.305490e-01,5.384555e-01,5.461239e-01,5.535637e-01,
5.607851e-01,5.677982e-01,5.746128e-01,5.812389e-01,5.876864e-01,5.939653e-01,6.000854e-01,
6.060565e-01,6.118886e-01,6.175915e-01,6.231750e-01,6.286492e-01,6.340241e-01,6.393099e-01,
6.445170e-01,6.496560e-01,6.547378e-01,6.597740e-01,6.647766e-01,6.697321e-01,6.746154e-01,
6.795190e-01,6.844617e-01,6.894666e-01,6.945629e-01,6.997888e-01,7.051949e-01,7.108496e-01,
7.168454e-01,7.233059e-01,7.303926e-01,7.383100e-01,7.473057e-01,7.576639e-01,7.696919e-01,
7.837012e-01,7.999913e-01,8.188394e-01,8.405009e-01,8.652194e-01,8.932419e-01,9.248353e-01,
9.603004e-01,9.999814e-01,1.044271e+00,1.093607e+00,1.148468e+00,1.209355e+00,1.276765e+00,
1.351156e+00,1.432891e+00,1.522179e+00,1.619010e+00,1.723096e+00,1.833831e+00,1.950298e+00,
2.071307e+00,2.195490e+00,2.321415e+00,2.447717e+00,2.573196e+00,2.696895e+00,2.818133e+00,
2.936495e+00,3.051807e+00,3.164089e+00,3.273524e+00,3.380405e+00,3.485116e+00,3.588094e+00,
3.689814e+00,3.790775e+00,3.891483e+00,3.992439e+00,4.094131e+00,4.197019e+00,4.301527e+00,
4.408024e+00,4.516815e+00,4.628125e+00,4.742085e+00,4.858732e+00,4.978006e+00,5.099759e+00,
5.223773e+00,5.349792e+00,5.477539e+00,5.606756e+00,5.737222e+00,5.868772e+00,6.001315e+00,
6.134829e+00,6.269367e+00,6.405046e+00,6.542037e+00,6.680554e+00,6.820840e+00,6.963154e+00,
7.107768e+00,7.254950e+00,7.404963e+00,7.558058e+00,7.714470e+00,7.874417e+00,8.038099e+00,
8.205696e+00,8.377371e+00,8.553273e+00,8.733533e+00,8.918273e+00,9.107605e+00,9.301632e+00,
9.500452e+00,9.704160e+00,9.912848e+00,1.012661e+01,1.034554e+01,1.056973e+01,1.079929e+01,
1.103432e+01,1.127492e+01,1.152121e+01,1.177333e+01,1.203138e+01,1.229551e+01,1.256585e+01,
1.284257e+01,1.312581e+01,1.341573e+01,1.371252e+01,1.401634e+01,1.432738e+01,1.464584e+01,
1.497192e+01,1.530584e+01,1.564780e+01,1.599804e+01,1.635679e+01,1.672429e+01,1.710079e+01,
1.748656e+01,1.788187e+01,1.828699e+01,1.870221e+01,1.912783e+01,1.956415e+01,2.001149e+01,
2.047018e+01,2.094055e+01,2.142296e+01,2.191775e+01,2.242530e+01,2.294600e+01,2.348023e+01,
2.402840e+01,2.459093e+01,2.516825e+01,2.576081e+01,2.636905e+01,2.699347e+01,2.763453e+01,
2.829276e+01,2.896865e+01,2.966276e+01,3.037563e+01,3.110782e+01,3.185993e+01,3.263256e+01,
3.342632e+01,3.424188e+01,3.507988e+01,3.594100e+01,3.682597e+01,3.773550e+01,3.867034e+01,
3.963126e+01,4.061907e+01,4.163459e+01,4.267867e+01,4.375217e+01,4.485602e+01,4.599113e+01,
4.715847e+01,4.835904e+01,4.959386e+01,5.086399e+01,5.217051e+01,5.351455e+01,5.489727e+01,
5.631988e+01,5.778360e+01,5.928972e+01,6.083956e+01,6.243446e+01,6.407585e+01,6.576516e+01,
6.750389e+01,6.929358e+01,7.113583e+01,7.303229e+01,7.498465e+01,7.699466e+01,7.906413e+01,
8.119494e+01,8.338901e+01,8.564834e+01,8.797497e+01,9.037104e+01,9.283874e+01,9.538033e+01,
9.799814e+01,1.006946e+02,1.034722e+02,1.063335e+02,1.092812e+02,1.123179e+02,1.154467e+02,
1.186703e+02,1.219918e+02,1.254143e+02,1.289411e+02,1.325755e+02,1.363208e+02,1.401808e+02
  };
  static const G4double P27[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,1.345993e-03,1.812154e-02,
4.026430e-02,7.035354e-02,1.115368e-01,1.703760e-01,2.538581e-01,3.782461e-01,5.589961e-01,
8.253354e-01,1.198038e+00,1.694231e+00,2.291935e+00,2.955705e+00,3.637254e+00,4.300026e+00,
4.929838e+00,5.521904e+00,6.078501e+00,6.604750e+00,7.106030e+00,7.587357e+00,8.053179e+00,
8.507290e+00,8.952817e+00,9.392248e+00,9.827489e+00,1.025995e+01,1.069061e+01,1.112015e+01,
1.154897e+01,1.197730e+01,1.240524e+01,1.283285e+01,1.326014e+01,1.368706e+01,1.411358e+01,
1.453973e+01,1.496547e+01,1.539087e+01,1.581597e+01,1.624088e+01,1.666572e+01,1.709063e+01,
1.751579e+01,1.794142e+01,1.836777e+01,1.879512e+01,1.922377e+01,1.965407e+01,2.008642e+01,
2.052125e+01,2.095904e+01,2.140031e+01,2.184566e+01,2.229574e+01,2.275125e+01,2.321300e+01,
2.368188e+01,2.415886e+01,2.464505e+01,2.514170e+01,2.565021e+01,2.616939e+01,2.669677e+01,
2.724263e+01,2.780976e+01,2.840169e+01,2.902295e+01,2.967960e+01,3.037978e+01,3.113466e+01,
3.195970e+01,3.287602e+01,3.391207e+01,3.510515e+01,3.650240e+01,3.816072e+01,4.014549e+01,
4.252816e+01,4.538374e+01,4.878905e+01,5.282265e+01,5.756664e+01,6.310963e+01,6.955058e+01,
7.700256e+01,8.559606e+01,9.548163e+01,1.068314e+02,1.198391e+02,1.347180e+02,1.516961e+02,
1.710065e+02,1.928734e+02,2.174931e+02,2.450103e+02,2.754950e+02,3.089205e+02,3.451524e+02,
3.839498e+02,4.249834e+02,4.678667e+02,5.121950e+02,5.575832e+02,6.036977e+02,6.502788e+02,
6.971481e+02,7.442078e+02,7.914355e+02,8.388753e+02,8.866283e+02,9.348447e+02,9.837162e+02,
1.033470e+03,1.084366e+03,1.136689e+03,1.190749e+03,1.246871e+03,1.305393e+03,1.366657e+03,
1.430999e+03,1.498741e+03,1.570174e+03,1.645547e+03,1.725059e+03,1.808851e+03,1.897003e+03,
1.989542e+03,2.086454e+03,2.187703e+03,2.293252e+03,2.403083e+03,2.517218e+03,2.635733e+03,
2.758773e+03,2.886552e+03,3.019360e+03,3.157558e+03,3.301573e+03,3.451893e+03,3.609055e+03,
3.773646e+03,3.946289e+03,4.127639e+03,4.318381e+03,4.519221e+03,4.730887e+03,4.954126e+03,
5.189702e+03,5.438396e+03,5.701012e+03,5.978373e+03,6.271328e+03,6.580753e+03,6.907557e+03,
7.252683e+03,7.617117e+03,8.001887e+03,8.408074e+03,8.836812e+03,9.289295e+03,9.766784e+03,
1.027061e+04,1.080218e+04,1.136297e+04,1.195458e+04,1.257866e+04,1.323698e+04,1.393143e+04,
1.466398e+04,1.543675e+04,1.625197e+04,1.711202e+04,1.801940e+04,1.897679e+04,1.998700e+04,
2.105305e+04,2.217810e+04,2.336552e+04,2.461890e+04,2.594203e+04,2.733891e+04,2.881382e+04,
3.037128e+04,3.201606e+04,3.375326e+04,3.558825e+04,3.752676e+04,3.957482e+04,4.173887e+04,
4.402572e+04,4.644258e+04,4.899711e+04,5.169743e+04,5.455215e+04,5.757040e+04,6.076187e+04,
6.413682e+04,6.770616e+04,7.148143e+04,7.547490e+04,7.969957e+04,8.416924e+04,8.889853e+04,
9.390298e+04,9.919906e+04,1.048042e+05,1.107370e+05,1.170172e+05,1.236655e+05,1.307042e+05,
1.381568e+05,1.460482e+05,1.544049e+05,1.632551e+05,1.726285e+05,1.825569e+05,1.930738e+05,
2.042150e+05,2.160183e+05,2.285240e+05,2.417748e+05,2.558161e+05,2.706959e+05,2.864654e+05,
3.031790e+05,3.208942e+05,3.396723e+05,3.595783e+05,3.806813e+05,4.030546e+05,4.267761e+05,
4.519286e+05,4.786000e+05,5.068836e+05,5.368787e+05,5.686906e+05,6.024313e+05,6.382197e+05,
6.761823e+05,7.164532e+05,7.591751e+05,8.044997e+05,8.525881e+05,9.036114e+05,9.577516e+05,
1.015202e+06,1.076168e+06,1.140869e+06,1.209535e+06,1.282415e+06,1.359770e+06,1.441879e+06,
1.529038e+06,1.621562e+06,1.719787e+06,1.824068e+06,1.934784e+06,2.052338e+06,2.177156e+06,
2.309695e+06,2.450437e+06,2.599897e+06,2.758623e+06,2.927195e+06,3.106232e+06,3.296392e+06
  };
  static const G4double P08[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,5.850394e-06,3.813802e-05,7.678856e-05,1.235890e-04,
1.811834e-04,2.505496e-04,3.370214e-04,4.423119e-04,5.737849e-04,7.362470e-04,9.394324e-04,
1.193306e-03,1.512903e-03,1.914433e-03,2.424835e-03,3.067238e-03,3.887084e-03,4.914388e-03,
6.198077e-03,7.742119e-03,9.530727e-03,1.145750e-02,1.342370e-02,1.532690e-02,1.710499e-02,
1.873750e-02,2.022106e-02,2.156321e-02,2.277662e-02,2.387485e-02,2.487089e-02,2.577662e-02,
2.660257e-02,2.735786e-02,2.805033e-02,2.868661e-02,2.927238e-02,2.981248e-02,3.031106e-02,
3.077178e-02,3.119783e-02,3.159205e-02,3.195707e-02,3.229527e-02,3.260875e-02,3.289948e-02,
3.316932e-02,3.341990e-02,3.365279e-02,3.386940e-02,3.407105e-02,3.425896e-02,3.443423e-02,
3.459791e-02,3.475094e-02,3.489419e-02,3.502849e-02,3.515456e-02,3.527311e-02,3.538476e-02,
3.549011e-02,3.558968e-02,3.568400e-02,3.577351e-02,3.585865e-02,3.593982e-02,3.601738e-02,
3.609170e-02,3.616308e-02,3.623182e-02,3.629822e-02,3.636254e-02,3.642405e-02,3.648154e-02,
3.653815e-02,3.659408e-02,3.664961e-02,3.670503e-02,3.676072e-02,3.681714e-02,3.687490e-02,
3.693479e-02,3.699783e-02,3.706528e-02,3.713867e-02,3.721973e-02,3.731032e-02,3.741226e-02,
3.752719e-02,3.765644e-02,3.780102e-02,3.796161e-02,3.813871e-02,3.833276e-02,3.854422e-02,
3.877367e-02,3.902186e-02,3.928968e-02,3.957815e-02,3.988836e-02,4.022139e-02,4.057812e-02,
4.095916e-02,4.136456e-02,4.179369e-02,4.224495e-02,4.271572e-02,4.320224e-02,4.369977e-02,
4.420281e-02,4.470553e-02,4.520218e-02,4.568761e-02,4.615755e-02,4.660887e-02,4.703959e-02,
4.744879e-02,4.783646e-02,4.820329e-02,4.855044e-02,4.887942e-02,4.919191e-02,4.948968e-02,
4.977449e-02,5.004805e-02,5.031198e-02,5.056777e-02,5.081677e-02,5.106018e-02,5.129904e-02,
5.153419e-02,5.176632e-02,5.199590e-02,5.222324e-02,5.244845e-02,5.267151e-02,5.289225e-02,
5.311040e-02,5.332567e-02,5.353771e-02,5.374625e-02,5.395103e-02,5.415190e-02,5.434879e-02,
5.454172e-02,5.473078e-02,5.491615e-02,5.509804e-02,5.527672e-02,5.545245e-02,5.562552e-02,
5.579619e-02,5.596472e-02,5.613134e-02,5.629624e-02,5.645961e-02,5.662158e-02,5.678228e-02,
5.694179e-02,5.710018e-02,5.725750e-02,5.741379e-02,5.756906e-02,5.772332e-02,5.787658e-02,
5.802884e-02,5.818008e-02,5.833029e-02,5.847948e-02,5.862764e-02,5.877475e-02,5.892081e-02,
5.906581e-02,5.920977e-02,5.935267e-02,5.949453e-02,5.963535e-02,5.977514e-02,5.991390e-02,
6.005165e-02,6.018840e-02,6.032417e-02,6.045897e-02,6.059282e-02,6.072574e-02,6.085774e-02,
6.098884e-02,6.111906e-02,6.124841e-02,6.137693e-02,6.150463e-02,6.163152e-02,6.175763e-02,
6.188297e-02,6.200757e-02,6.213144e-02,6.225460e-02,6.237706e-02,6.249886e-02,6.261999e-02,
6.274049e-02,6.286037e-02,6.297964e-02,6.309831e-02,6.321642e-02,6.333396e-02,6.345096e-02,
6.356743e-02,6.368338e-02,6.379883e-02,6.391380e-02,6.402828e-02,6.414231e-02,6.425588e-02,
6.436902e-02,6.448173e-02,6.459402e-02,6.470592e-02,6.481742e-02,6.492854e-02,6.503929e-02,
6.514969e-02,6.525973e-02,6.536943e-02,6.547880e-02,6.558786e-02,6.569660e-02,6.580504e-02,
6.591318e-02,6.602104e-02,6.612863e-02,6.623595e-02,6.634300e-02,6.644981e-02,6.655637e-02,
6.666270e-02,6.676880e-02,6.687467e-02,6.698034e-02,6.708579e-02,6.719105e-02,6.729611e-02,
6.740099e-02,6.750569e-02,6.761022e-02,6.771458e-02,6.781878e-02,6.792283e-02,6.802673e-02,
6.813049e-02,6.823412e-02,6.833761e-02,6.844098e-02,6.854423e-02,6.864738e-02,6.875041e-02,
6.885334e-02,6.895618e-02,6.905893e-02,6.916159e-02,6.926418e-02,6.936669e-02,6.946912e-02,
6.957150e-02,6.967381e-02,6.977607e-02,6.987829e-02,6.998045e-02,7.008258e-02,7.018467e-02,
7.028673e-02,7.038876e-02,7.049077e-02,7.059277e-02,7.069475e-02,7.079672e-02,7.089869e-02
};
  static const G4double P18[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,6.824362e-05,4.522706e-04,9.266015e-04,1.519331e-03,
2.271897e-03,3.207104e-03,4.409961e-03,5.920914e-03,7.867568e-03,1.034890e-02,1.355071e-02,
1.767709e-02,2.303624e-02,2.998068e-02,3.908692e-02,5.090796e-02,6.646885e-02,8.657934e-02,
1.124957e-01,1.446424e-01,1.830426e-01,2.256984e-01,2.705794e-01,3.153773e-01,3.585357e-01,
3.993962e-01,4.376870e-01,4.734083e-01,5.067098e-01,5.377891e-01,5.668544e-01,5.941073e-01,
6.197329e-01,6.438952e-01,6.667360e-01,6.883758e-01,7.089162e-01,7.284426e-01,7.470277e-01,
7.647338e-01,7.816149e-01,7.977194e-01,8.130925e-01,8.277769e-01,8.418093e-01,8.552264e-01,
8.680643e-01,8.803545e-01,8.921305e-01,9.034214e-01,9.142574e-01,9.246668e-01,9.346759e-01,
9.443113e-01,9.535980e-01,9.625599e-01,9.712203e-01,9.796014e-01,9.877249e-01,9.956119e-01,
1.003283e+00,1.010757e+00,1.018054e+00,1.025193e+00,1.032192e+00,1.039071e+00,1.045846e+00,
1.052537e+00,1.059162e+00,1.065739e+00,1.072287e+00,1.078824e+00,1.085267e+00,1.091476e+00,
1.097776e+00,1.104194e+00,1.110761e+00,1.117517e+00,1.124514e+00,1.131820e+00,1.139531e+00,
1.147772e+00,1.156712e+00,1.166573e+00,1.177631e+00,1.190221e+00,1.204723e+00,1.221543e+00,
1.241088e+00,1.263744e+00,1.289862e+00,1.319764e+00,1.353751e+00,1.392131e+00,1.435238e+00,
1.483446e+00,1.537190e+00,1.596963e+00,1.663320e+00,1.736867e+00,1.818241e+00,1.908081e+00,
2.006982e+00,2.115434e+00,2.233748e+00,2.361978e+00,2.499847e+00,2.646693e+00,2.801459e+00,
2.962731e+00,3.128831e+00,3.297953e+00,3.468314e+00,3.638292e+00,3.806530e+00,3.972005e+00,
4.134030e+00,4.292233e+00,4.446513e+00,4.596991e+00,4.743962e+00,4.887846e+00,5.029151e+00,
5.168447e+00,5.306340e+00,5.443454e+00,5.580411e+00,5.717819e+00,5.856258e+00,5.996267e+00,
6.138329e+00,6.282856e+00,6.430177e+00,6.580525e+00,6.734031e+00,6.890722e+00,7.050529e+00,
7.213302e+00,7.378833e+00,7.546883e+00,7.717208e+00,7.889591e+00,8.063859e+00,8.239901e+00,
8.417680e+00,8.597232e+00,8.778664e+00,8.962146e+00,9.147902e+00,9.336191e+00,9.527301e+00,
9.721535e+00,9.919203e+00,1.012061e+01,1.032605e+01,1.053580e+01,1.075013e+01,1.096928e+01,
1.119347e+01,1.142290e+01,1.165776e+01,1.189822e+01,1.214442e+01,1.239652e+01,1.265463e+01,
1.291891e+01,1.318945e+01,1.346640e+01,1.374986e+01,1.403997e+01,1.433686e+01,1.464064e+01,
1.495148e+01,1.526949e+01,1.559485e+01,1.592771e+01,1.626825e+01,1.661662e+01,1.697304e+01,
1.733768e+01,1.771076e+01,1.809249e+01,1.848310e+01,1.888282e+01,1.929190e+01,1.971059e+01,
2.013915e+01,2.057787e+01,2.102702e+01,2.148690e+01,2.195783e+01,2.244012e+01,2.293409e+01,
2.344009e+01,2.395847e+01,2.448959e+01,2.503383e+01,2.559158e+01,2.616323e+01,2.674919e+01,
2.734990e+01,2.796580e+01,2.859733e+01,2.924496e+01,2.990918e+01,3.059048e+01,3.128937e+01,
3.200638e+01,3.274207e+01,3.349698e+01,3.427170e+01,3.506682e+01,3.588297e+01,3.672077e+01,
3.758087e+01,3.846397e+01,3.937074e+01,4.030190e+01,4.125820e+01,4.224040e+01,4.324928e+01,
4.428566e+01,4.535037e+01,4.644426e+01,4.756824e+01,4.872322e+01,4.991015e+01,5.112999e+01,
5.238376e+01,5.367249e+01,5.499726e+01,5.635916e+01,5.775935e+01,5.919898e+01,6.067929e+01,
6.220151e+01,6.376694e+01,6.537690e+01,6.703278e+01,6.873600e+01,7.048801e+01,7.229032e+01,
7.414450e+01,7.605215e+01,7.801493e+01,8.003455e+01,8.211279e+01,8.425146e+01,8.645245e+01,
8.871771e+01,9.104924e+01,9.344911e+01,9.591946e+01,9.846249e+01,1.010805e+02,1.037758e+02,
1.065509e+02,1.094082e+02,1.123504e+02,1.153801e+02,1.185000e+02,1.217131e+02,1.250223e+02,
1.284306e+02,1.319411e+02,1.355571e+02,1.392819e+02,1.431190e+02,1.470720e+02,1.511445e+02,
1.553403e+02,1.596635e+02,1.641179e+02,1.687079e+02,1.734378e+02,1.783119e+02,1.833351e+02
  };
  static const G4double P28[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,7.960502e-04,5.364017e-03,1.118562e-02,1.869316e-02,
2.852749e-02,4.113714e-02,5.787072e-02,7.955515e-02,1.083806e-01,1.462818e-01,1.967401e-01,
2.638146e-01,3.536865e-01,4.737998e-01,6.362798e-01,8.538194e-01,1.149192e+00,1.542907e+00,
2.066174e+00,2.735516e+00,3.560003e+00,4.504415e+00,5.528958e+00,6.583511e+00,7.631145e+00,
8.653933e+00,9.642298e+00,1.059309e+01,1.150711e+01,1.238671e+01,1.323493e+01,1.405501e+01,
1.485012e+01,1.562315e+01,1.637661e+01,1.711263e+01,1.783294e+01,1.853896e+01,1.923178e+01,
1.991230e+01,2.058123e+01,2.123916e+01,2.188666e+01,2.252430e+01,2.315250e+01,2.377173e+01,
2.438256e+01,2.498540e+01,2.558088e+01,2.616948e+01,2.675181e+01,2.732849e+01,2.790012e+01,
2.846739e+01,2.903101e+01,2.959170e+01,3.015023e+01,3.070742e+01,3.126414e+01,3.182130e+01,
3.237989e+01,3.294094e+01,3.350557e+01,3.407498e+01,3.465046e+01,3.523340e+01,3.582530e+01,
3.642781e+01,3.704271e+01,3.767193e+01,3.831763e+01,3.898218e+01,3.965715e+01,4.032768e+01,
4.102906e+01,4.176548e+01,4.254216e+01,4.336573e+01,4.424489e+01,4.519125e+01,4.622060e+01,
4.735458e+01,4.862267e+01,5.006425e+01,5.173064e+01,5.368618e+01,5.600789e+01,5.878339e+01,
6.210756e+01,6.607900e+01,7.079781e+01,7.636571e+01,8.288853e+01,9.048030e+01,9.926829e+01,
1.093979e+02,1.210367e+02,1.343782e+02,1.496435e+02,1.670814e+02,1.869668e+02,2.095939e+02,
2.352665e+02,2.642811e+02,2.969039e+02,3.333440e+02,3.737232e+02,4.180489e+02,4.661957e+02,
5.179023e+02,5.727873e+02,6.303818e+02,6.901743e+02,7.516590e+02,8.143781e+02,8.779563e+02,
9.421156e+02,1.006680e+03,1.071573e+03,1.136805e+03,1.202469e+03,1.268723e+03,1.335784e+03,
1.403917e+03,1.473430e+03,1.544668e+03,1.618005e+03,1.693837e+03,1.772579e+03,1.854654e+03,
1.940482e+03,2.030475e+03,2.125017e+03,2.224456e+03,2.329092e+03,2.439170e+03,2.554874e+03,
2.676334e+03,2.803633e+03,2.936826e+03,3.075954e+03,3.221074e+03,3.372272e+03,3.529685e+03,
3.693517e+03,3.864050e+03,4.041643e+03,4.226743e+03,4.419872e+03,4.621628e+03,4.832678e+03,
5.053744e+03,5.285606e+03,5.529085e+03,5.785043e+03,6.054376e+03,6.338009e+03,6.636897e+03,
6.952019e+03,7.284382e+03,7.635021e+03,8.005001e+03,8.395420e+03,8.807417e+03,9.242171e+03,
9.700911e+03,1.018492e+04,1.069554e+04,1.123417e+04,1.180231e+04,1.240150e+04,1.303339e+04,
1.369971e+04,1.440231e+04,1.514313e+04,1.592423e+04,1.674777e+04,1.761608e+04,1.853161e+04,
1.949694e+04,2.051483e+04,2.158819e+04,2.272013e+04,2.391392e+04,2.517305e+04,2.650119e+04,
2.790227e+04,2.938043e+04,3.094006e+04,3.258584e+04,3.432269e+04,3.615588e+04,3.809095e+04,
4.013379e+04,4.229066e+04,4.456817e+04,4.697334e+04,4.951363e+04,5.219692e+04,5.503158e+04,
5.802649e+04,6.119106e+04,6.453527e+04,6.806970e+04,7.180556e+04,7.575476e+04,7.992991e+04,
8.434440e+04,8.901242e+04,9.394902e+04,9.917018e+04,1.046928e+05,1.105349e+05,1.167156e+05,
1.232549e+05,1.301745e+05,1.374970e+05,1.452466e+05,1.534490e+05,1.621312e+05,1.713222e+05,
1.810526e+05,1.913548e+05,2.022635e+05,2.138150e+05,2.260484e+05,2.390048e+05,2.527280e+05,
2.672645e+05,2.826635e+05,2.989775e+05,3.162621e+05,3.345761e+05,3.539825e+05,3.745475e+05,
3.963420e+05,4.194409e+05,4.439240e+05,4.698758e+05,4.973862e+05,5.265506e+05,5.574705e+05,
5.902534e+05,6.250137e+05,6.618730e+05,7.009603e+05,7.424127e+05,7.863758e+05,8.330043e+05,
8.824628e+05,9.349258e+05,9.905789e+05,1.049619e+06,1.112257e+06,1.178714e+06,1.249227e+06,
1.324048e+06,1.403444e+06,1.487699e+06,1.577115e+06,1.672012e+06,1.772733e+06,1.879639e+06,
1.993117e+06,2.113574e+06,2.241448e+06,2.377201e+06,2.521325e+06,2.674344e+06,2.836814e+06,
3.009326e+06,3.192510e+06,3.387034e+06,3.593610e+06,3.812994e+06,4.045989e+06,4.293452e+06
  };
  static const G4double P09[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,7.515803e-06,1.731827e-05,
2.874685e-05,4.278170e-05,6.015953e-05,8.098262e-05,1.059240e-04,1.374744e-04,1.760954e-04,
2.225746e-04,2.816669e-04,3.547278e-04,4.438203e-04,5.576081e-04,6.979665e-04,8.731555e-04,
1.095439e-03,1.369207e-03,1.718647e-03,2.151439e-03,2.698367e-03,3.379582e-03,4.229614e-03,
5.283815e-03,6.581892e-03,8.166275e-03,1.007243e-02,1.232577e-02,1.488678e-02,1.770487e-02,
2.061095e-02,2.348657e-02,2.621326e-02,2.873192e-02,3.102728e-02,3.310043e-02,3.496870e-02,
3.665237e-02,3.817043e-02,3.954315e-02,4.078897e-02,4.192379e-02,4.296125e-02,4.391292e-02,
4.478859e-02,4.559645e-02,4.634342e-02,4.703529e-02,4.767699e-02,4.827277e-02,4.882633e-02,
4.934096e-02,4.981961e-02,5.026497e-02,5.067957e-02,5.106575e-02,5.142564e-02,5.176126e-02,
5.207454e-02,5.236721e-02,5.264095e-02,5.289729e-02,5.313769e-02,5.336347e-02,5.357588e-02,
5.377608e-02,5.396514e-02,5.414407e-02,5.431378e-02,5.447513e-02,5.462891e-02,5.477585e-02,
5.491664e-02,5.505189e-02,5.518220e-02,5.530808e-02,5.543004e-02,5.554853e-02,5.566397e-02,
5.577674e-02,5.588719e-02,5.599565e-02,5.610241e-02,5.620776e-02,5.631089e-02,5.641045e-02,
5.650990e-02,5.660948e-02,5.670951e-02,5.681040e-02,5.691268e-02,5.701709e-02,5.712461e-02,
5.723655e-02,5.735463e-02,5.748102e-02,5.761830e-02,5.776937e-02,5.793723e-02,5.812477e-02,
5.833443e-02,5.856813e-02,5.882715e-02,5.911232e-02,5.942417e-02,5.976317e-02,6.012990e-02,
6.052514e-02,6.094997e-02,6.140573e-02,6.189398e-02,6.241639e-02,6.297464e-02,6.357021e-02,
6.420419e-02,6.487694e-02,6.558787e-02,6.633509e-02,6.711523e-02,6.792326e-02,6.875260e-02,
6.959533e-02,7.044270e-02,7.128573e-02,7.211586e-02,7.292555e-02,7.370873e-02,7.446102e-02,
7.517973e-02,7.586367e-02,7.651297e-02,7.712872e-02,7.771273e-02,7.826729e-02,7.879494e-02,
7.929835e-02,7.978018e-02,8.024301e-02,8.068929e-02,8.112124e-02,8.154090e-02,8.195006e-02,
8.235029e-02,8.274289e-02,8.312891e-02,8.350916e-02,8.388422e-02,8.425443e-02,8.461996e-02,
8.498082e-02,8.533691e-02,8.568805e-02,8.603405e-02,8.637472e-02,8.670993e-02,8.703958e-02,
8.736369e-02,8.768234e-02,8.799567e-02,8.830390e-02,8.860731e-02,8.890618e-02,8.920084e-02,
8.949159e-02,8.977874e-02,9.006257e-02,9.034333e-02,9.062125e-02,9.089651e-02,9.116928e-02,
9.143968e-02,9.170781e-02,9.197375e-02,9.223754e-02,9.249923e-02,9.275884e-02,9.301639e-02,
9.327188e-02,9.352532e-02,9.377671e-02,9.402605e-02,9.427335e-02,9.451861e-02,9.476185e-02,
9.500306e-02,9.524227e-02,9.547949e-02,9.571474e-02,9.594804e-02,9.617943e-02,9.640892e-02,
9.663655e-02,9.686235e-02,9.708636e-02,9.730860e-02,9.752912e-02,9.774795e-02,9.796512e-02,
9.818069e-02,9.839467e-02,9.860712e-02,9.881806e-02,9.902755e-02,9.923560e-02,9.944226e-02,
9.964757e-02,9.985156e-02,1.000543e-01,1.002557e-01,1.004560e-01,1.006550e-01,1.008529e-01,
1.010497e-01,1.012454e-01,1.014400e-01,1.016336e-01,1.018262e-01,1.020178e-01,1.022085e-01,
1.023982e-01,1.025870e-01,1.027750e-01,1.029621e-01,1.031484e-01,1.033339e-01,1.035186e-01,
1.037025e-01,1.038858e-01,1.040682e-01,1.042500e-01,1.044311e-01,1.046116e-01,1.047914e-01,
1.049706e-01,1.051492e-01,1.053272e-01,1.055046e-01,1.056815e-01,1.058578e-01,1.060337e-01,
1.062090e-01,1.063838e-01,1.065581e-01,1.067320e-01,1.069054e-01,1.070784e-01,1.072510e-01,
1.074231e-01,1.075949e-01,1.077663e-01,1.079373e-01,1.081079e-01,1.082783e-01,1.084482e-01,
1.086179e-01,1.087872e-01,1.089563e-01,1.091250e-01,1.092935e-01,1.094617e-01,1.096296e-01,
1.097973e-01,1.099648e-01,1.101320e-01,1.102990e-01,1.104659e-01,1.106325e-01,1.107989e-01,
1.109651e-01,1.111312e-01,1.112971e-01,1.114629e-01,1.116285e-01,1.117940e-01,1.119593e-01,
1.121246e-01,1.122897e-01,1.124547e-01,1.126197e-01,1.127845e-01,1.129493e-01,1.131140e-01,
1.132786e-01,1.134432e-01,1.136078e-01,1.137723e-01,1.139367e-01,1.141012e-01,1.142656e-01
  };
  static const G4double P19[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,5.927935e-05,1.389914e-04,
2.349414e-04,3.566209e-04,5.121431e-04,7.045053e-04,9.423805e-04,1.252973e-03,1.645359e-03,
2.132778e-03,2.772367e-03,3.588379e-03,4.615362e-03,5.968874e-03,7.691507e-03,9.910494e-03,
1.281519e-02,1.650640e-02,2.136751e-02,2.757845e-02,3.567673e-02,4.608131e-02,5.947584e-02,
7.661025e-02,9.837506e-02,1.257751e-01,1.597774e-01,2.012338e-01,2.498273e-01,3.049689e-01,
3.636123e-01,4.234494e-01,4.819630e-01,5.377010e-01,5.900857e-01,6.388778e-01,6.842212e-01,
7.263609e-01,7.655421e-01,8.020772e-01,8.362684e-01,8.683839e-01,8.986585e-01,9.272944e-01,
9.544631e-01,9.803078e-01,1.004947e+00,1.028478e+00,1.050980e+00,1.072520e+00,1.093154e+00,
1.112933e+00,1.131898e+00,1.150092e+00,1.167553e+00,1.184321e+00,1.200431e+00,1.215920e+00,
1.230825e+00,1.245180e+00,1.259022e+00,1.272384e+00,1.285302e+00,1.297810e+00,1.309940e+00,
1.321726e+00,1.333200e+00,1.344393e+00,1.355338e+00,1.366064e+00,1.376603e+00,1.386983e+00,
1.397235e+00,1.407387e+00,1.417470e+00,1.427510e+00,1.437537e+00,1.447579e+00,1.457663e+00,
1.467817e+00,1.478068e+00,1.488444e+00,1.498973e+00,1.509682e+00,1.520485e+00,1.531238e+00,
1.542307e+00,1.553733e+00,1.565564e+00,1.577862e+00,1.590714e+00,1.604236e+00,1.618589e+00,
1.633991e+00,1.650739e+00,1.669215e+00,1.689901e+00,1.713363e+00,1.740235e+00,1.771177e+00,
1.806833e+00,1.847794e+00,1.894587e+00,1.947683e+00,2.007527e+00,2.074576e+00,2.149333e+00,
2.232374e+00,2.324368e+00,2.426086e+00,2.538395e+00,2.662248e+00,2.798655e+00,2.948644e+00,
3.113195e+00,3.293165e+00,3.489174e+00,3.701503e+00,3.929974e+00,4.173863e+00,4.431848e+00,
4.702026e+00,4.982009e+00,5.269084e+00,5.560423e+00,5.853292e+00,6.145244e+00,6.434268e+00,
6.718845e+00,6.997954e+00,7.271040e+00,7.537949e+00,7.798855e+00,8.054192e+00,8.304585e+00,
8.550792e+00,8.793666e+00,9.034113e+00,9.273058e+00,9.511420e+00,9.750093e+00,9.989927e+00,
1.023171e+01,1.047615e+01,1.072385e+01,1.097533e+01,1.123096e+01,1.149102e+01,1.175565e+01,
1.202490e+01,1.229872e+01,1.257701e+01,1.285961e+01,1.314639e+01,1.343720e+01,1.373195e+01,
1.403061e+01,1.433323e+01,1.463991e+01,1.495084e+01,1.526627e+01,1.558650e+01,1.591187e+01,
1.624276e+01,1.657956e+01,1.692265e+01,1.727242e+01,1.762926e+01,1.799350e+01,1.836549e+01,
1.874553e+01,1.913392e+01,1.953093e+01,1.993678e+01,2.035173e+01,2.077597e+01,2.120973e+01,
2.165319e+01,2.210656e+01,2.257002e+01,2.304378e+01,2.352804e+01,2.402301e+01,2.452891e+01,
2.504595e+01,2.557440e+01,2.611448e+01,2.666648e+01,2.723067e+01,2.780733e+01,2.839679e+01,
2.899935e+01,2.961537e+01,3.024518e+01,3.088917e+01,3.154771e+01,3.222121e+01,3.291007e+01,
3.361474e+01,3.433567e+01,3.507331e+01,3.582815e+01,3.660069e+01,3.739145e+01,3.820096e+01,
3.902978e+01,3.987847e+01,4.074762e+01,4.163784e+01,4.254976e+01,4.348402e+01,4.444130e+01,
4.542228e+01,4.642768e+01,4.745822e+01,4.851467e+01,4.959779e+01,5.070840e+01,5.184732e+01,
5.301540e+01,5.421353e+01,5.544261e+01,5.670357e+01,5.799738e+01,5.932503e+01,6.068754e+01,
6.208597e+01,6.352141e+01,6.499498e+01,6.650784e+01,6.806117e+01,6.965620e+01,7.129421e+01,
7.297649e+01,7.470440e+01,7.647932e+01,7.830269e+01,8.017598e+01,8.210072e+01,8.407848e+01,
8.611087e+01,8.819958e+01,9.034631e+01,9.255287e+01,9.482106e+01,9.715280e+01,9.955002e+01,
1.020148e+02,1.045491e+02,1.071551e+02,1.098351e+02,1.125913e+02,1.154261e+02,1.183418e+02,
1.213411e+02,1.244265e+02,1.276007e+02,1.308664e+02,1.342265e+02,1.376838e+02,1.412415e+02,
1.449027e+02,1.486706e+02,1.525485e+02,1.565399e+02,1.606483e+02,1.648774e+02,1.692309e+02,
1.737129e+02,1.783272e+02,1.830781e+02,1.879699e+02,1.930070e+02,1.981940e+02,2.035356e+02,
2.090368e+02,2.147025e+02,2.205380e+02,2.265486e+02,2.327400e+02,2.391178e+02,2.456880e+02,
2.524567e+02,2.594302e+02,2.666150e+02,2.740179e+02,2.816459e+02,2.895062e+02,2.976061e+02
  };
  static const G4double P29[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,4.675863e-04,1.115848e-03,
1.921474e-03,2.976502e-03,4.368459e-03,6.145635e-03,8.414529e-03,1.147236e-02,1.545930e-02,
2.057119e-02,2.749440e-02,3.660912e-02,4.844832e-02,6.454972e-02,8.569351e-02,1.138020e-01,
1.517623e-01,2.015348e-01,2.691639e-01,3.583034e-01,4.782232e-01,6.371509e-01,8.482346e-01,
1.126750e+00,1.491709e+00,1.965598e+00,2.572184e+00,3.334953e+00,4.257053e+00,5.336097e+00,
6.519588e+00,7.764801e+00,9.020575e+00,1.025416e+01,1.144977e+01,1.259820e+01,1.369879e+01,
1.475356e+01,1.576491e+01,1.673737e+01,1.767581e+01,1.858475e+01,1.946828e+01,2.033001e+01,
2.117302e+01,2.199989e+01,2.281269e+01,2.361303e+01,2.440215e+01,2.518099e+01,2.595023e+01,
2.671041e+01,2.746196e+01,2.820524e+01,2.894070e+01,2.966884e+01,3.039005e+01,3.110491e+01,
3.181409e+01,3.251822e+01,3.321818e+01,3.391476e+01,3.460900e+01,3.530194e+01,3.599472e+01,
3.668862e+01,3.738499e+01,3.808532e+01,3.879119e+01,3.950432e+01,4.022655e+01,4.095989e+01,
4.170647e+01,4.246862e+01,4.324879e+01,4.404966e+01,4.487409e+01,4.572515e+01,4.660610e+01,
4.752048e+01,4.847204e+01,4.946482e+01,5.050316e+01,5.159177e+01,5.272361e+01,5.388496e+01,
5.511729e+01,5.642837e+01,5.782761e+01,5.932695e+01,6.094192e+01,6.269331e+01,6.460937e+01,
6.672884e+01,6.910427e+01,7.180553e+01,7.492262e+01,7.856680e+01,8.286884e+01,8.797459e+01,
9.403866e+01,1.012187e+02,1.096725e+02,1.195593e+02,1.310443e+02,1.443066e+02,1.595468e+02,
1.769950e+02,1.969173e+02,2.196207e+02,2.454567e+02,2.748218e+02,3.081551e+02,3.459309e+02,
3.886448e+02,4.367924e+02,4.908381e+02,5.511773e+02,6.180927e+02,6.917113e+02,7.719694e+02,
8.585945e+02,9.511118e+02,1.048877e+03,1.151132e+03,1.257071e+03,1.365912e+03,1.476962e+03,
1.589651e+03,1.703560e+03,1.818425e+03,1.934131e+03,2.050699e+03,2.168274e+03,2.287105e+03,
2.407530e+03,2.529963e+03,2.654887e+03,2.782834e+03,2.914378e+03,3.050129e+03,3.190720e+03,
3.336794e+03,3.488995e+03,3.647955e+03,3.814276e+03,3.988525e+03,4.171219e+03,4.362818e+03,
4.563730e+03,4.774309e+03,4.994874e+03,5.225720e+03,5.467141e+03,5.719453e+03,5.983016e+03,
6.258253e+03,6.545669e+03,6.845863e+03,7.159534e+03,7.487485e+03,7.830622e+03,8.189944e+03,
8.566547e+03,8.961603e+03,9.376365e+03,9.812149e+03,1.027033e+04,1.075235e+04,1.125969e+04,
1.179388e+04,1.235652e+04,1.294923e+04,1.357370e+04,1.423170e+04,1.492505e+04,1.565562e+04,
1.642541e+04,1.723648e+04,1.809099e+04,1.899122e+04,1.993956e+04,2.093854e+04,2.199082e+04,
2.309921e+04,2.426670e+04,2.549643e+04,2.679175e+04,2.815618e+04,2.959348e+04,3.110761e+04,
3.270279e+04,3.438349e+04,3.615442e+04,3.802062e+04,3.998739e+04,4.206039e+04,4.424559e+04,
4.654932e+04,4.897832e+04,5.153970e+04,5.424101e+04,5.709028e+04,6.009598e+04,6.326712e+04,
6.661323e+04,7.014443e+04,7.387144e+04,7.780561e+04,8.195901e+04,8.634439e+04,9.097530e+04,
9.586609e+04,1.010320e+05,1.064891e+05,1.122546e+05,1.183466e+05,1.247843e+05,1.315882e+05,
1.387798e+05,1.463820e+05,1.544194e+05,1.629175e+05,1.719038e+05,1.814074e+05,1.914589e+05,
2.020912e+05,2.133387e+05,2.252383e+05,2.378290e+05,2.511522e+05,2.652516e+05,2.801740e+05,
2.959687e+05,3.126882e+05,3.303881e+05,3.491275e+05,3.689692e+05,3.899795e+05,4.122292e+05,
4.357933e+05,4.607512e+05,4.871874e+05,5.151917e+05,5.448592e+05,5.762909e+05,6.095943e+05,
6.448833e+05,6.822787e+05,7.219092e+05,7.639111e+05,8.084294e+05,8.556178e+05,9.056399e+05,
9.586691e+05,1.014890e+06,1.074498e+06,1.137701e+06,1.204721e+06,1.275792e+06,1.351163e+06,
1.431099e+06,1.515882e+06,1.605811e+06,1.701204e+06,1.802397e+06,1.909751e+06,2.023645e+06,
2.144486e+06,2.272704e+06,2.408756e+06,2.553128e+06,2.706338e+06,2.868935e+06,3.041503e+06,
3.224662e+06,3.419071e+06,3.625434e+06,3.844494e+06,4.077045e+06,4.323928e+06,4.586041e+06,
4.864335e+06,5.159824e+06,5.473583e+06,5.806757e+06,6.160565e+06,6.536300e+06,6.935340e+06
  };
  static const G4double P010[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
1.420000e-05,4.701887e-05,8.694089e-05,1.341836e-04,1.940077e-04,2.718261e-04,3.681895e-04,
4.837658e-04,6.323004e-04,8.240919e-04,1.060464e-03,1.350967e-03,1.731360e-03,2.208133e-03,
2.792245e-03,3.550245e-03,4.501045e-03,5.670796e-03,7.167402e-03,9.014729e-03,1.129037e-02,
1.410796e-02,1.749772e-02,2.157807e-02,2.639241e-02,3.199085e-02,3.841369e-02,4.559250e-02,
5.345748e-02,6.167108e-02,6.986856e-02,7.767193e-02,8.480019e-02,9.117305e-02,9.677277e-02,
1.016834e-01,1.059752e-01,1.097284e-01,1.130297e-01,1.159373e-01,1.185096e-01,1.207998e-01,
1.228498e-01,1.246924e-01,1.263587e-01,1.278743e-01,1.292600e-01,1.305329e-01,1.317074e-01,
1.327952e-01,1.338059e-01,1.347478e-01,1.356275e-01,1.364508e-01,1.372227e-01,1.379475e-01,
1.386291e-01,1.392709e-01,1.398763e-01,1.404481e-01,1.409894e-01,1.415026e-01,1.419903e-01,
1.424548e-01,1.428983e-01,1.433229e-01,1.437305e-01,1.441228e-01,1.445015e-01,1.448681e-01,
1.452240e-01,1.455704e-01,1.459086e-01,1.462395e-01,1.465641e-01,1.468832e-01,1.471976e-01,
1.475078e-01,1.478144e-01,1.481178e-01,1.484184e-01,1.487165e-01,1.490123e-01,1.493060e-01,
1.495976e-01,1.498872e-01,1.501749e-01,1.504606e-01,1.507445e-01,1.510251e-01,1.513010e-01,
1.515759e-01,1.518503e-01,1.521247e-01,1.524003e-01,1.526783e-01,1.529609e-01,1.532511e-01,
1.535529e-01,1.538711e-01,1.542117e-01,1.545816e-01,1.549877e-01,1.554367e-01,1.559341e-01,
1.564842e-01,1.570896e-01,1.577520e-01,1.584720e-01,1.592503e-01,1.600875e-01,1.609849e-01,
1.619445e-01,1.629686e-01,1.640604e-01,1.652234e-01,1.664613e-01,1.677779e-01,1.691764e-01,
1.706592e-01,1.722273e-01,1.738801e-01,1.756140e-01,1.774230e-01,1.792977e-01,1.812255e-01,
1.831911e-01,1.851767e-01,1.871639e-01,1.891341e-01,1.910700e-01,1.929567e-01,1.947824e-01,
1.965384e-01,1.982195e-01,1.998234e-01,2.013502e-01,2.028018e-01,2.041820e-01,2.054950e-01,
2.067461e-01,2.079405e-01,2.090837e-01,2.101810e-01,2.112372e-01,2.122572e-01,2.132451e-01,
2.142048e-01,2.151398e-01,2.160530e-01,2.169469e-01,2.178237e-01,2.186851e-01,2.195325e-01,
2.203669e-01,2.211890e-01,2.219995e-01,2.227987e-01,2.235869e-01,2.243642e-01,2.251309e-01,
2.258871e-01,2.266330e-01,2.273689e-01,2.280950e-01,2.288117e-01,2.295194e-01,2.302182e-01,
2.309088e-01,2.315914e-01,2.322663e-01,2.329339e-01,2.335944e-01,2.342481e-01,2.348952e-01,
2.355358e-01,2.361701e-01,2.367983e-01,2.374202e-01,2.380361e-01,2.386460e-01,2.392499e-01,
2.398478e-01,2.404398e-01,2.410260e-01,2.416063e-01,2.421808e-01,2.427496e-01,2.433128e-01,
2.438703e-01,2.444223e-01,2.449688e-01,2.455100e-01,2.460459e-01,2.465767e-01,2.471023e-01,
2.476231e-01,2.481389e-01,2.486500e-01,2.491565e-01,2.496584e-01,2.501560e-01,2.506492e-01,
2.511383e-01,2.516232e-01,2.521042e-01,2.525813e-01,2.530547e-01,2.535244e-01,2.539906e-01,
2.544533e-01,2.549127e-01,2.553688e-01,2.558217e-01,2.562715e-01,2.567184e-01,2.571623e-01,
2.576034e-01,2.580418e-01,2.584775e-01,2.589107e-01,2.593413e-01,2.597694e-01,2.601952e-01,
2.606187e-01,2.610399e-01,2.614590e-01,2.618760e-01,2.622909e-01,2.627038e-01,2.631147e-01,
2.635238e-01,2.639311e-01,2.643365e-01,2.647403e-01,2.651423e-01,2.655427e-01,2.659416e-01,
2.663389e-01,2.667347e-01,2.671291e-01,2.675220e-01,2.679136e-01,2.683038e-01,2.686928e-01,
2.690805e-01,2.694670e-01,2.698523e-01,2.702364e-01,2.706195e-01,2.710015e-01,2.713824e-01,
2.717623e-01,2.721413e-01,2.725193e-01,2.728963e-01,2.732725e-01,2.736479e-01,2.740223e-01,
2.743960e-01,2.747690e-01,2.751411e-01,2.755126e-01,2.758833e-01,2.762534e-01,2.766228e-01,
2.769916e-01,2.773598e-01,2.777275e-01,2.780946e-01,2.784611e-01,2.788272e-01,2.791927e-01,
2.795578e-01,2.799225e-01,2.802868e-01,2.806506e-01,2.810141e-01,2.813772e-01,2.817400e-01,
2.821025e-01,2.824646e-01,2.828265e-01,2.831881e-01,2.835495e-01,2.839107e-01,2.842716e-01,
2.846324e-01,2.849930e-01,2.853535e-01,2.857138e-01,2.860740e-01,2.864341e-01,2.867942e-01
  };
  static const G4double P110[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
9.591763e-05,3.231969e-04,6.087125e-04,9.576234e-04,1.414059e-03,2.027041e-03,2.810721e-03,
3.781121e-03,5.069066e-03,6.785649e-03,8.969373e-03,1.174027e-02,1.548527e-02,2.032949e-02,
2.645527e-02,3.465992e-02,4.527966e-02,5.876373e-02,7.656597e-02,9.923844e-02,1.280600e-01,
1.648770e-01,2.105763e-01,2.673302e-01,3.364120e-01,4.192864e-01,5.173684e-01,6.304481e-01,
7.582455e-01,8.958848e-01,1.037582e+00,1.176684e+00,1.307744e+00,1.428590e+00,1.538109e+00,
1.637162e+00,1.726448e+00,1.806975e+00,1.880027e+00,1.946380e+00,2.006917e+00,2.062500e+00,
2.113806e+00,2.161365e+00,2.205714e+00,2.247309e+00,2.286523e+00,2.323671e+00,2.359012e+00,
2.392761e+00,2.425097e+00,2.456165e+00,2.486085e+00,2.514957e+00,2.542866e+00,2.569884e+00,
2.596079e+00,2.621512e+00,2.646242e+00,2.670327e+00,2.693830e+00,2.716805e+00,2.739313e+00,
2.761415e+00,2.783170e+00,2.804641e+00,2.825886e+00,2.846969e+00,2.867949e+00,2.888885e+00,
2.909838e+00,2.930865e+00,2.952023e+00,2.973366e+00,2.994948e+00,3.016818e+00,3.039026e+00,
3.061616e+00,3.084631e+00,3.108110e+00,3.132088e+00,3.156597e+00,3.181666e+00,3.207319e+00,
3.233578e+00,3.260461e+00,3.287985e+00,3.316164e+00,3.345014e+00,3.374420e+00,3.404208e+00,
3.434812e+00,3.466295e+00,3.498755e+00,3.532340e+00,3.567273e+00,3.603879e+00,3.642620e+00,
3.684134e+00,3.729263e+00,3.779065e+00,3.834802e+00,3.897876e+00,3.969744e+00,4.051813e+00,
4.145355e+00,4.251473e+00,4.371124e+00,4.505184e+00,4.654526e+00,4.820112e+00,5.003050e+00,
5.204640e+00,5.426399e+00,5.670065e+00,5.937583e+00,6.231072e+00,6.552772e+00,6.904959e+00,
7.289825e+00,7.709327e+00,8.164993e+00,8.657708e+00,9.187498e+00,9.753341e+00,1.035304e+01,
1.098318e+01,1.163929e+01,1.231599e+01,1.300745e+01,1.370769e+01,1.441103e+01,1.511244e+01,
1.580777e+01,1.649383e+01,1.716840e+01,1.783020e+01,1.847874e+01,1.911420e+01,1.973730e+01,
2.034918e+01,2.095125e+01,2.154514e+01,2.213262e+01,2.271548e+01,2.329555e+01,2.387461e+01,
2.445438e+01,2.503650e+01,2.562246e+01,2.621364e+01,2.681125e+01,2.741634e+01,2.802980e+01,
2.865235e+01,2.928457e+01,2.992690e+01,3.057966e+01,3.124314e+01,3.191753e+01,3.260304e+01,
3.329988e+01,3.400829e+01,3.472857e+01,3.546106e+01,3.620616e+01,3.696434e+01,3.773610e+01,
3.852199e+01,3.932257e+01,4.013841e+01,4.097009e+01,4.181816e+01,4.268318e+01,4.356564e+01,
4.446606e+01,4.538488e+01,4.632254e+01,4.727946e+01,4.825604e+01,4.925267e+01,5.026972e+01,
5.130758e+01,5.236662e+01,5.344725e+01,5.454987e+01,5.567490e+01,5.682280e+01,5.799403e+01,
5.918909e+01,6.040851e+01,6.165285e+01,6.292269e+01,6.421865e+01,6.554139e+01,6.689158e+01,
6.826996e+01,6.967727e+01,7.111431e+01,7.258189e+01,7.408087e+01,7.561215e+01,7.717665e+01,
7.877533e+01,8.040918e+01,8.207925e+01,8.378660e+01,8.553234e+01,8.731762e+01,8.914361e+01,
9.101153e+01,9.292265e+01,9.487827e+01,9.687973e+01,9.892841e+01,1.010257e+02,1.031732e+02,
1.053723e+02,1.076245e+02,1.099316e+02,1.122952e+02,1.147170e+02,1.171987e+02,1.197421e+02,
1.223492e+02,1.250219e+02,1.277621e+02,1.305719e+02,1.334534e+02,1.364088e+02,1.394403e+02,
1.425502e+02,1.457410e+02,1.490151e+02,1.523749e+02,1.558232e+02,1.593625e+02,1.629957e+02,
1.667257e+02,1.705553e+02,1.744876e+02,1.785257e+02,1.826729e+02,1.869325e+02,1.913079e+02,
1.958027e+02,2.004204e+02,2.051650e+02,2.100402e+02,2.150500e+02,2.201987e+02,2.254904e+02,
2.309295e+02,2.365206e+02,2.422684e+02,2.481777e+02,2.542534e+02,2.605007e+02,2.669250e+02,
2.735316e+02,2.803262e+02,2.873146e+02,2.945029e+02,3.018973e+02,3.095041e+02,3.173299e+02,
3.253817e+02,3.336664e+02,3.421912e+02,3.509637e+02,3.599917e+02,3.692831e+02,3.788461e+02,
3.886893e+02,3.988214e+02,4.092516e+02,4.199893e+02,4.310440e+02,4.424257e+02,4.541449e+02,
4.662121e+02,4.786383e+02,4.914349e+02,5.046135e+02,5.181864e+02,5.321660e+02,5.465652e+02,
5.613974e+02,5.766763e+02,5.924161e+02,6.086315e+02,6.253377e+02,6.425504e+02,6.602857e+02
  };
  static const G4double P210[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
6.479160e-04,2.222020e-03,4.264157e-03,6.841262e-03,1.032399e-02,1.515291e-02,2.152677e-02,
2.967511e-02,4.084384e-02,5.620899e-02,7.638501e-02,1.028167e-01,1.396897e-01,1.889132e-01,
2.531616e-01,3.419767e-01,4.606012e-01,6.160489e-01,8.278253e-01,1.106110e+00,1.471173e+00,
1.952294e+00,2.568440e+00,3.357900e+00,4.349247e+00,5.576148e+00,7.074057e+00,8.855419e+00,
1.093215e+01,1.323883e+01,1.568832e+01,1.816813e+01,2.057800e+01,2.286974e+01,2.501187e+01,
2.701003e+01,2.886770e+01,3.059561e+01,3.221218e+01,3.372653e+01,3.515134e+01,3.650043e+01,
3.778462e+01,3.901221e+01,4.019267e+01,4.133432e+01,4.244420e+01,4.352833e+01,4.459184e+01,
4.563906e+01,4.667362e+01,4.769852e+01,4.871621e+01,4.972874e+01,5.073786e+01,5.174512e+01,
5.275195e+01,5.375980e+01,5.477015e+01,5.578467e+01,5.680527e+01,5.783387e+01,5.887274e+01,
5.992444e+01,6.099162e+01,6.207742e+01,6.318504e+01,6.431812e+01,6.548052e+01,6.667634e+01,
6.791004e+01,6.918632e+01,7.051016e+01,7.188680e+01,7.332174e+01,7.482073e+01,7.638975e+01,
7.803498e+01,7.976279e+01,8.157970e+01,8.349240e+01,8.550769e+01,8.763244e+01,8.987365e+01,
9.223838e+01,9.473382e+01,9.736730e+01,1.001464e+02,1.030792e+02,1.061603e+02,1.093776e+02,
1.127846e+02,1.163971e+02,1.202362e+02,1.243305e+02,1.287201e+02,1.334613e+02,1.386334e+02,
1.443459e+02,1.507468e+02,1.580279e+02,1.664268e+02,1.762236e+02,1.877291e+02,2.012708e+02,
2.171791e+02,2.357799e+02,2.573961e+02,2.823579e+02,3.110181e+02,3.437701e+02,3.810638e+02,
4.234203e+02,4.714435e+02,5.258289e+02,5.873688e+02,6.569534e+02,7.355652e+02,8.242650e+02,
9.241665e+02,1.036396e+03,1.162036e+03,1.302054e+03,1.457221e+03,1.628023e+03,1.814588e+03,
2.016629e+03,2.233434e+03,2.463894e+03,2.706587e+03,2.959889e+03,3.222102e+03,3.491605e+03,
3.766953e+03,4.046947e+03,4.330688e+03,4.617582e+03,4.907339e+03,5.199952e+03,5.495663e+03,
5.794940e+03,6.098442e+03,6.406997e+03,6.721566e+03,7.043226e+03,7.373149e+03,7.712587e+03,
8.062854e+03,8.425307e+03,8.801331e+03,9.192321e+03,9.599669e+03,1.002475e+04,1.046890e+04,
1.093345e+04,1.141964e+04,1.192874e+04,1.246195e+04,1.302049e+04,1.360562e+04,1.421860e+04,
1.486078e+04,1.553362e+04,1.623867e+04,1.697762e+04,1.775231e+04,1.856473e+04,1.941702e+04,
2.031147e+04,2.125054e+04,2.223681e+04,2.327300e+04,2.436195e+04,2.550666e+04,2.671021e+04,
2.797583e+04,2.930685e+04,3.070674e+04,3.217911e+04,3.372772e+04,3.535650e+04,3.706952e+04,
3.887109e+04,4.076570e+04,4.275810e+04,4.485325e+04,4.705643e+04,4.937318e+04,5.180937e+04,
5.437120e+04,5.706526e+04,5.989850e+04,6.287829e+04,6.601245e+04,6.930926e+04,7.277750e+04,
7.642649e+04,8.026610e+04,8.430679e+04,8.855965e+04,9.303645e+04,9.774966e+04,1.027125e+05,
1.079389e+05,1.134438e+05,1.192429e+05,1.253529e+05,1.317915e+05,1.385774e+05,1.457304e+05,
1.532716e+05,1.612234e+05,1.696092e+05,1.784543e+05,1.877851e+05,1.976298e+05,2.080182e+05,
2.189820e+05,2.305546e+05,2.427716e+05,2.556706e+05,2.692916e+05,2.836769e+05,2.988713e+05,
3.149225e+05,3.318808e+05,3.497998e+05,3.687362e+05,3.887501e+05,4.099053e+05,4.322695e+05,
4.559143e+05,4.809159e+05,5.073551e+05,5.353173e+05,5.648935e+05,5.961799e+05,6.292788e+05,
6.642985e+05,7.013543e+05,7.405681e+05,7.820695e+05,8.259959e+05,8.724933e+05,9.217165e+05,
9.738297e+05,1.029007e+06,1.087434e+06,1.149308e+06,1.214835e+06,1.284239e+06,1.357754e+06,
1.435629e+06,1.518129e+06,1.605537e+06,1.698150e+06,1.796285e+06,1.900280e+06,2.010491e+06,
2.127300e+06,2.251108e+06,2.382345e+06,2.521465e+06,2.668952e+06,2.825320e+06,2.991112e+06,
3.166910e+06,3.353328e+06,3.551019e+06,3.760679e+06,3.983046e+06,4.218903e+06,4.469084e+06,
4.734475e+06,5.016015e+06,5.314705e+06,5.631606e+06,5.967849e+06,6.324634e+06,6.703234e+06,
7.105007e+06,7.531392e+06,7.983922e+06,8.464224e+06,8.974029e+06,9.515176e+06,1.008962e+07,
1.069945e+07,1.134686e+07,1.203421e+07,1.276399e+07,1.353888e+07,1.436168e+07,1.523540e+07
  };
  static const G4double P011[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,8.561707e-05,1.906331e-04,3.154098e-04,
4.819877e-04,6.963055e-04,9.598242e-04,1.274050e-03,1.676401e-03,2.205375e-03,2.864847e-03,
3.659508e-03,4.684845e-03,6.012414e-03,7.651465e-03,9.662871e-03,1.227193e-02,1.552037e-02,
1.946512e-02,2.442244e-02,3.048669e-02,3.773801e-02,4.644389e-02,5.668052e-02,6.855143e-02,
8.219187e-02,9.763501e-02,1.147626e-01,1.335674e-01,1.528473e-01,1.718772e-01,1.898295e-01,
2.058575e-01,2.199478e-01,2.320907e-01,2.425378e-01,2.514765e-01,2.591877e-01,2.658072e-01,
2.715525e-01,2.765412e-01,2.808940e-01,2.847237e-01,2.881029e-01,2.911027e-01,2.937862e-01,
2.962019e-01,2.983884e-01,3.003811e-01,3.022094e-01,3.038971e-01,3.054638e-01,3.069258e-01,
3.082963e-01,3.095866e-01,3.108058e-01,3.119616e-01,3.130607e-01,3.141086e-01,3.151103e-01,
3.160703e-01,3.169924e-01,3.178803e-01,3.187374e-01,3.195668e-01,3.203714e-01,3.211538e-01,
3.219164e-01,3.226615e-01,3.233911e-01,3.241070e-01,3.248106e-01,3.255035e-01,3.261867e-01,
3.268613e-01,3.275281e-01,3.281876e-01,3.288405e-01,3.294869e-01,3.301272e-01,3.307613e-01,
3.313894e-01,3.320112e-01,3.326266e-01,3.332354e-01,3.338373e-01,3.344320e-01,3.350193e-01,
3.355989e-01,3.361705e-01,3.367340e-01,3.372893e-01,3.378364e-01,3.383744e-01,3.389023e-01,
3.394248e-01,3.399435e-01,3.404607e-01,3.409797e-01,3.415051e-01,3.420432e-01,3.426019e-01,
3.431912e-01,3.438227e-01,3.445089e-01,3.452623e-01,3.460940e-01,3.470130e-01,3.480253e-01,
3.491345e-01,3.503420e-01,3.516483e-01,3.530539e-01,3.545595e-01,3.561672e-01,3.578801e-01,
3.597024e-01,3.616397e-01,3.636979e-01,3.658841e-01,3.682050e-01,3.706672e-01,3.732763e-01,
3.760363e-01,3.789488e-01,3.820118e-01,3.852192e-01,3.885601e-01,3.920181e-01,3.955715e-01,
3.991939e-01,4.028554e-01,4.065239e-01,4.101673e-01,4.137552e-01,4.172611e-01,4.206628e-01,
4.239442e-01,4.270940e-01,4.301065e-01,4.329801e-01,4.357169e-01,4.383219e-01,4.408018e-01,
4.431646e-01,4.454194e-01,4.475751e-01,4.496409e-01,4.516255e-01,4.535373e-01,4.553840e-01,
4.571727e-01,4.589098e-01,4.606012e-01,4.622520e-01,4.638665e-01,4.654485e-01,4.670014e-01,
4.685277e-01,4.700296e-01,4.715090e-01,4.729673e-01,4.744057e-01,4.758250e-01,4.772260e-01,
4.786094e-01,4.799758e-01,4.813257e-01,4.826595e-01,4.839778e-01,4.852810e-01,4.865695e-01,
4.878438e-01,4.891042e-01,4.903511e-01,4.915848e-01,4.928055e-01,4.940135e-01,4.952088e-01,
4.963917e-01,4.975623e-01,4.987206e-01,4.998667e-01,5.010006e-01,5.021224e-01,5.032320e-01,
5.043297e-01,5.054154e-01,5.064892e-01,5.075511e-01,5.086014e-01,5.096401e-01,5.106674e-01,
5.116834e-01,5.126883e-01,5.136823e-01,5.146656e-01,5.156384e-01,5.166009e-01,5.175533e-01,
5.184958e-01,5.194287e-01,5.203523e-01,5.212667e-01,5.221722e-01,5.230690e-01,5.239574e-01,
5.248375e-01,5.257097e-01,5.265740e-01,5.274309e-01,5.282804e-01,5.291227e-01,5.299582e-01,
5.307869e-01,5.316092e-01,5.324250e-01,5.332348e-01,5.340385e-01,5.348365e-01,5.356289e-01,
5.364158e-01,5.371974e-01,5.379738e-01,5.387453e-01,5.395119e-01,5.402738e-01,5.410311e-01,
5.417840e-01,5.425326e-01,5.432770e-01,5.440173e-01,5.447537e-01,5.454863e-01,5.462151e-01,
5.469403e-01,5.476620e-01,5.483803e-01,5.490953e-01,5.498071e-01,5.505157e-01,5.512213e-01,
5.519240e-01,5.526238e-01,5.533208e-01,5.540151e-01,5.547068e-01,5.553960e-01,5.560827e-01,
5.567669e-01,5.574489e-01,5.581286e-01,5.588061e-01,5.594815e-01,5.601548e-01,5.608261e-01,
5.614955e-01,5.621630e-01,5.628287e-01,5.634926e-01,5.641548e-01,5.648154e-01,5.654744e-01,
5.661318e-01,5.667877e-01,5.674422e-01,5.680953e-01,5.687470e-01,5.693975e-01,5.700467e-01,
5.706946e-01,5.713415e-01,5.719872e-01,5.726318e-01,5.732754e-01,5.739180e-01,5.745597e-01,
5.752004e-01,5.758403e-01,5.764794e-01,5.771176e-01,5.777552e-01,5.783920e-01,5.790281e-01,
5.796636e-01,5.802985e-01,5.809328e-01,5.815666e-01,5.821998e-01,5.828326e-01,5.834650e-01,
5.840970e-01,5.847286e-01,5.853599e-01,5.859909e-01,5.866216e-01,5.872521e-01,5.878823e-01
  };
  static const G4double P111[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,5.209548e-04,1.180711e-03,1.990663e-03,
3.107743e-03,4.592002e-03,6.476688e-03,8.797445e-03,1.186734e-02,1.603420e-02,2.139747e-02,
2.806967e-02,3.696045e-02,4.884261e-02,6.398504e-02,8.316995e-02,1.088558e-01,1.418609e-01,
1.832285e-01,2.368826e-01,3.046122e-01,3.881885e-01,4.917300e-01,6.173520e-01,7.676664e-01,
9.458741e-01,1.154034e+00,1.392213e+00,1.661998e+00,1.947285e+00,2.237778e+00,2.520414e+00,
2.780710e+00,3.016745e+00,3.226555e+00,3.412755e+00,3.577076e+00,3.723291e+00,3.852749e+00,
3.968634e+00,4.072420e+00,4.165812e+00,4.250554e+00,4.327670e+00,4.398270e+00,4.463397e+00,
4.523860e+00,4.580294e+00,4.633332e+00,4.683510e+00,4.731274e+00,4.776995e+00,4.820987e+00,
4.863513e+00,4.904792e+00,4.945010e+00,4.984323e+00,5.022866e+00,5.060755e+00,5.098099e+00,
5.134994e+00,5.171534e+00,5.207809e+00,5.243910e+00,5.279928e+00,5.315948e+00,5.352059e+00,
5.388348e+00,5.424899e+00,5.461794e+00,5.499112e+00,5.536929e+00,5.575317e+00,5.614339e+00,
5.654057e+00,5.694526e+00,5.735793e+00,5.777899e+00,5.820879e+00,5.864759e+00,5.909560e+00,
5.955296e+00,6.001974e+00,6.049594e+00,6.098153e+00,6.147641e+00,6.198045e+00,6.249350e+00,
6.301538e+00,6.354593e+00,6.408503e+00,6.463261e+00,6.518875e+00,6.575240e+00,6.632248e+00,
6.690414e+00,6.749930e+00,6.811096e+00,6.874363e+00,6.940381e+00,7.010064e+00,7.084648e+00,
7.165738e+00,7.255305e+00,7.355625e+00,7.469147e+00,7.598323e+00,7.745430e+00,7.912453e+00,
8.101065e+00,8.312698e+00,8.548675e+00,8.810356e+00,9.099276e+00,9.417241e+00,9.766397e+00,
1.014926e+01,1.056874e+01,1.102811e+01,1.153097e+01,1.208119e+01,1.268281e+01,1.333988e+01,
1.405626e+01,1.483537e+01,1.567986e+01,1.659126e+01,1.756967e+01,1.861338e+01,1.971876e+01,
2.088013e+01,2.208995e+01,2.333920e+01,2.461789e+01,2.591570e+01,2.722262e+01,2.852960e+01,
2.982889e+01,3.111433e+01,3.238135e+01,3.362699e+01,3.484969e+01,3.604911e+01,3.722590e+01,
3.838153e+01,3.951805e+01,4.063795e+01,4.174401e+01,4.283916e+01,4.392643e+01,4.500884e+01,
4.608939e+01,4.717095e+01,4.825627e+01,4.934793e+01,5.044832e+01,5.155962e+01,5.268381e+01,
5.382263e+01,5.497760e+01,5.615006e+01,5.734118e+01,5.855195e+01,5.978329e+01,6.103600e+01,
6.231082e+01,6.360849e+01,6.492971e+01,6.627522e+01,6.764576e+01,6.904207e+01,7.046495e+01,
7.191516e+01,7.339351e+01,7.490076e+01,7.643768e+01,7.800500e+01,7.960342e+01,8.123360e+01,
8.289618e+01,8.459176e+01,8.632091e+01,8.808419e+01,8.988213e+01,9.171528e+01,9.358418e+01,
9.548939e+01,9.743150e+01,9.941112e+01,1.014289e+02,1.034855e+02,1.055817e+02,1.077184e+02,
1.098962e+02,1.121162e+02,1.143792e+02,1.166864e+02,1.190388e+02,1.214375e+02,1.238837e+02,
1.263787e+02,1.289239e+02,1.315205e+02,1.341702e+02,1.368742e+02,1.396344e+02,1.424522e+02,
1.453293e+02,1.482676e+02,1.512688e+02,1.543349e+02,1.574678e+02,1.606694e+02,1.639419e+02,
1.672875e+02,1.707082e+02,1.742066e+02,1.777848e+02,1.814453e+02,1.851906e+02,1.890234e+02,
1.929462e+02,1.969619e+02,2.010732e+02,2.052830e+02,2.095944e+02,2.140105e+02,2.185344e+02,
2.231694e+02,2.279189e+02,2.327864e+02,2.377754e+02,2.428897e+02,2.481330e+02,2.535094e+02,
2.590228e+02,2.646774e+02,2.704774e+02,2.764274e+02,2.825319e+02,2.887956e+02,2.952232e+02,
3.018199e+02,3.085907e+02,3.155410e+02,3.226763e+02,3.300021e+02,3.375242e+02,3.452488e+02,
3.531820e+02,3.613300e+02,3.696996e+02,3.782975e+02,3.871306e+02,3.962063e+02,4.055319e+02,
4.151151e+02,4.249638e+02,4.350862e+02,4.454907e+02,4.561860e+02,4.671811e+02,4.784852e+02,
4.901079e+02,5.020589e+02,5.143485e+02,5.269872e+02,5.399857e+02,5.533553e+02,5.671075e+02,
5.812541e+02,5.958075e+02,6.107802e+02,6.261855e+02,6.420367e+02,6.583478e+02,6.751333e+02,
6.924079e+02,7.101869e+02,7.284863e+02,7.473223e+02,7.667118e+02,7.866722e+02,8.072215e+02,
8.283782e+02,8.501616e+02,8.725913e+02,8.956879e+02,9.194725e+02,9.439667e+02,9.691932e+02,
9.951752e+02,1.021937e+03,1.049502e+03,1.077898e+03,1.107150e+03,1.137285e+03,1.168332e+03
  };
  static const G4double P211[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,3.170118e-03,7.315363e-03,1.257339e-02,
2.006524e-02,3.034536e-02,4.382582e-02,6.096755e-02,8.439254e-02,1.172188e-01,1.608402e-01,
2.168665e-01,2.939658e-01,4.003237e-01,5.402293e-01,7.232315e-01,9.761260e-01,1.311496e+00,
1.745343e+00,2.326101e+00,3.082613e+00,4.045963e+00,5.277513e+00,6.819248e+00,8.722748e+00,
1.105116e+01,1.385719e+01,1.716964e+01,2.104044e+01,2.526219e+01,2.969696e+01,3.414706e+01,
3.837462e+01,4.232892e+01,4.595438e+01,4.927332e+01,5.229426e+01,5.506693e+01,5.759896e+01,
5.993661e+01,6.209595e+01,6.409989e+01,6.597519e+01,6.773518e+01,6.939684e+01,7.097759e+01,
7.249103e+01,7.394773e+01,7.535948e+01,7.673677e+01,7.808864e+01,7.942302e+01,8.074691e+01,
8.206649e+01,8.338723e+01,8.471403e+01,8.605126e+01,8.740299e+01,8.877308e+01,9.016530e+01,
9.158348e+01,9.303155e+01,9.451368e+01,9.603439e+01,9.759857e+01,9.921130e+01,1.008781e+02,
1.026050e+02,1.043980e+02,1.062640e+02,1.082097e+02,1.102423e+02,1.123692e+02,1.145981e+02,
1.169368e+02,1.193933e+02,1.219754e+02,1.246913e+02,1.275491e+02,1.305566e+02,1.337220e+02,
1.370529e+02,1.405572e+02,1.442423e+02,1.481158e+02,1.521850e+02,1.564570e+02,1.609392e+02,
1.656389e+02,1.705636e+02,1.757217e+02,1.811220e+02,1.867754e+02,1.926811e+02,1.988382e+02,
2.053134e+02,2.121426e+02,2.193769e+02,2.270897e+02,2.353854e+02,2.444109e+02,2.543683e+02,
2.655272e+02,2.782318e+02,2.928987e+02,3.100059e+02,3.300699e+02,3.536201e+02,3.811791e+02,
4.132549e+02,4.503497e+02,4.929799e+02,5.417037e+02,5.971489e+02,6.600395e+02,7.312176e+02,
8.116615e+02,9.025013e+02,1.005029e+03,1.120706e+03,1.251160e+03,1.398173e+03,1.563658e+03,
1.749612e+03,1.958046e+03,2.190894e+03,2.449893e+03,2.736450e+03,3.051500e+03,3.395381e+03,
3.767744e+03,4.167523e+03,4.592970e+03,5.041778e+03,5.511247e+03,5.998488e+03,6.500670e+03,
7.015188e+03,7.539806e+03,8.072749e+03,8.612744e+03,9.159028e+03,9.711325e+03,1.026981e+04,
1.083504e+04,1.140796e+04,1.198979e+04,1.258204e+04,1.318641e+04,1.380480e+04,1.443930e+04,
1.509210e+04,1.576553e+04,1.646199e+04,1.718399e+04,1.793404e+04,1.871473e+04,1.952866e+04,
2.037842e+04,2.126663e+04,2.219590e+04,2.316886e+04,2.418816e+04,2.525650e+04,2.637667e+04,
2.755151e+04,2.878401e+04,3.007731e+04,3.143470e+04,3.285966e+04,3.435587e+04,3.592721e+04,
3.757777e+04,3.931185e+04,4.113396e+04,4.304881e+04,4.506131e+04,4.717656e+04,4.939988e+04,
5.173679e+04,5.419304e+04,5.677458e+04,5.948765e+04,6.233872e+04,6.533460e+04,6.848239e+04,
7.178955e+04,7.526394e+04,7.891383e+04,8.274793e+04,8.677546e+04,9.100617e+04,9.545036e+04,
1.001189e+05,1.050235e+05,1.101763e+05,1.155902e+05,1.212792e+05,1.272577e+05,1.335413e+05,
1.401464e+05,1.470904e+05,1.543917e+05,1.620700e+05,1.701459e+05,1.786414e+05,1.875799e+05,
1.969859e+05,2.068858e+05,2.173072e+05,2.282796e+05,2.398340e+05,2.520035e+05,2.648230e+05,
2.783297e+05,2.925628e+05,3.075639e+05,3.233770e+05,3.400491e+05,3.576294e+05,3.761706e+05,
3.957283e+05,4.163613e+05,4.381322e+05,4.611072e+05,4.853564e+05,5.109543e+05,5.379797e+05,
5.665162e+05,5.966524e+05,6.284822e+05,6.621053e+05,6.976273e+05,7.351601e+05,7.748225e+05,
8.167405e+05,8.610475e+05,9.078852e+05,9.574039e+05,1.009763e+06,1.065131e+06,1.123687e+06,
1.185622e+06,1.251138e+06,1.320447e+06,1.393779e+06,1.471372e+06,1.553484e+06,1.640385e+06,
1.732364e+06,1.829725e+06,1.932793e+06,2.041912e+06,2.157447e+06,2.279787e+06,2.409342e+06,
2.546550e+06,2.691874e+06,2.845807e+06,3.008872e+06,3.181623e+06,3.364649e+06,3.558578e+06,
3.764073e+06,3.981840e+06,4.212628e+06,4.457233e+06,4.716501e+06,4.991329e+06,5.282672e+06,
5.591541e+06,5.919014e+06,6.266233e+06,6.634412e+06,7.024842e+06,7.438893e+06,7.878021e+06,
8.343775e+06,8.837797e+06,9.361836e+06,9.917747e+06,1.050750e+07,1.113320e+07,1.179707e+07,
1.250148e+07,1.324894e+07,1.404213e+07,1.488389e+07,1.577725e+07,1.672542e+07,1.773181e+07,
1.880006e+07,1.993402e+07,2.113780e+07,2.241576e+07,2.377254e+07,2.521307e+07,2.674259e+07
  };
  static const G4double P012[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,4.229739e-04,1.122568e-03,
2.064930e-03,3.284750e-03,4.790517e-03,6.590986e-03,8.900132e-03,1.193812e-02,1.572724e-02,
2.029405e-02,2.612967e-02,3.360287e-02,4.276376e-02,5.386159e-02,6.777878e-02,8.469189e-02,
1.048143e-01,1.292053e-01,1.582020e-01,1.919795e-01,2.308545e-01,2.750062e-01,3.220332e-01,
3.681689e-01,4.129651e-01,4.533231e-01,4.884559e-01,5.187789e-01,5.444843e-01,5.662268e-01,
5.846749e-01,6.003010e-01,6.136180e-01,6.250143e-01,6.347777e-01,6.432341e-01,6.505388e-01,
6.569272e-01,6.625246e-01,6.674590e-01,6.718492e-01,6.757719e-01,6.793023e-01,6.825069e-01,
6.854375e-01,6.881350e-01,6.906373e-01,6.929758e-01,6.951759e-01,6.972587e-01,6.992414e-01,
7.011385e-01,7.029615e-01,7.047201e-01,7.064225e-01,7.080751e-01,7.096835e-01,7.112527e-01,
7.127865e-01,7.142885e-01,7.157619e-01,7.172092e-01,7.186329e-01,7.200349e-01,7.214168e-01,
7.227802e-01,7.241260e-01,7.254552e-01,7.267683e-01,7.280657e-01,7.293477e-01,7.306141e-01,
7.318649e-01,7.330998e-01,7.343184e-01,7.355204e-01,7.367051e-01,7.378722e-01,7.390210e-01,
7.401512e-01,7.412621e-01,7.423534e-01,7.434246e-01,7.444755e-01,7.455059e-01,7.465157e-01,
7.475050e-01,7.484740e-01,7.494234e-01,7.503538e-01,7.512667e-01,7.521635e-01,7.530473e-01,
7.539251e-01,7.548030e-01,7.556892e-01,7.565948e-01,7.575336e-01,7.585223e-01,7.595799e-01,
7.607266e-01,7.619819e-01,7.633625e-01,7.648809e-01,7.665452e-01,7.683588e-01,7.703228e-01,
7.724363e-01,7.746990e-01,7.771113e-01,7.796756e-01,7.823961e-01,7.852792e-01,7.883330e-01,
7.915675e-01,7.949937e-01,7.986237e-01,8.024699e-01,8.065444e-01,8.108585e-01,8.154213e-01,
8.202389e-01,8.253131e-01,8.306399e-01,8.362083e-01,8.419994e-01,8.479855e-01,8.541308e-01,
8.603917e-01,8.667190e-01,8.730602e-01,8.793625e-01,8.855761e-01,8.916562e-01,8.975657e-01,
9.032762e-01,9.087677e-01,9.140286e-01,9.190547e-01,9.238476e-01,9.284140e-01,9.327639e-01,
9.369096e-01,9.408651e-01,9.446450e-01,9.482641e-01,9.517370e-01,9.550773e-01,9.582984e-01,
9.614123e-01,9.644302e-01,9.673624e-01,9.702179e-01,9.730048e-01,9.757305e-01,9.784010e-01,
9.810219e-01,9.835976e-01,9.861323e-01,9.886290e-01,9.910907e-01,9.935195e-01,9.959175e-01,
9.982862e-01,1.000627e+00,1.002941e+00,1.005229e+00,1.007492e+00,1.009731e+00,1.011947e+00,
1.014139e+00,1.016308e+00,1.018455e+00,1.020580e+00,1.022683e+00,1.024763e+00,1.026822e+00,
1.028859e+00,1.030874e+00,1.032867e+00,1.034838e+00,1.036787e+00,1.038714e+00,1.040619e+00,
1.042502e+00,1.044363e+00,1.046202e+00,1.048019e+00,1.049815e+00,1.051590e+00,1.053344e+00,
1.055077e+00,1.056790e+00,1.058483e+00,1.060156e+00,1.061810e+00,1.063446e+00,1.065062e+00,
1.066661e+00,1.068243e+00,1.069807e+00,1.071355e+00,1.072886e+00,1.074402e+00,1.075902e+00,
1.077388e+00,1.078859e+00,1.080316e+00,1.081759e+00,1.083190e+00,1.084607e+00,1.086012e+00,
1.087404e+00,1.088785e+00,1.090155e+00,1.091513e+00,1.092861e+00,1.094198e+00,1.095525e+00,
1.096843e+00,1.098150e+00,1.099449e+00,1.100739e+00,1.102020e+00,1.103292e+00,1.104557e+00,
1.105813e+00,1.107062e+00,1.108303e+00,1.109537e+00,1.110764e+00,1.111984e+00,1.113197e+00,
1.114404e+00,1.115605e+00,1.116799e+00,1.117988e+00,1.119171e+00,1.120348e+00,1.121520e+00,
1.122687e+00,1.123848e+00,1.125005e+00,1.126156e+00,1.127304e+00,1.128446e+00,1.129584e+00,
1.130718e+00,1.131848e+00,1.132973e+00,1.134095e+00,1.135213e+00,1.136328e+00,1.137438e+00,
1.138545e+00,1.139649e+00,1.140750e+00,1.141847e+00,1.142942e+00,1.144033e+00,1.145122e+00,
1.146208e+00,1.147291e+00,1.148371e+00,1.149450e+00,1.150525e+00,1.151598e+00,1.152669e+00,
1.153738e+00,1.154805e+00,1.155870e+00,1.156933e+00,1.157993e+00,1.159053e+00,1.160110e+00,
1.161166e+00,1.162220e+00,1.163272e+00,1.164324e+00,1.165373e+00,1.166422e+00,1.167469e+00,
1.168515e+00,1.169560e+00,1.170604e+00,1.171647e+00,1.172688e+00,1.173729e+00,1.174769e+00,
1.175809e+00,1.176847e+00,1.177885e+00,1.178923e+00,1.179959e+00,1.180996e+00,1.182031e+00
  };
  static const G4double P112[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,2.669456e-03,7.210759e-03,
1.353042e-02,2.197839e-02,3.274774e-02,4.604543e-02,6.366403e-02,8.759503e-02,1.184107e-01,
1.567549e-01,2.073542e-01,2.742402e-01,3.588721e-01,4.647151e-01,6.017175e-01,7.735506e-01,
9.845531e-01,1.248518e+00,1.572353e+00,1.961623e+00,2.423926e+00,2.965705e+00,3.560928e+00,
4.163437e+00,4.766978e+00,5.327878e+00,5.831605e+00,6.280136e+00,6.672360e+00,7.014629e+00,
7.314181e+00,7.575938e+00,7.806026e+00,8.009145e+00,8.188626e+00,8.348974e+00,8.491835e+00,
8.620696e+00,8.737148e+00,8.843020e+00,8.940169e+00,9.029691e+00,9.112779e+00,9.190558e+00,
9.263911e+00,9.333538e+00,9.400141e+00,9.464325e+00,9.526593e+00,9.587378e+00,9.647045e+00,
9.705909e+00,9.764235e+00,9.822253e+00,9.880155e+00,9.938113e+00,9.996274e+00,1.005477e+01,
1.011373e+01,1.017325e+01,1.023345e+01,1.029441e+01,1.035624e+01,1.041901e+01,1.048279e+01,
1.054767e+01,1.061369e+01,1.068090e+01,1.074936e+01,1.081909e+01,1.089011e+01,1.096245e+01,
1.103609e+01,1.111104e+01,1.118729e+01,1.126481e+01,1.134358e+01,1.142356e+01,1.150472e+01,
1.158702e+01,1.167041e+01,1.175485e+01,1.184030e+01,1.192670e+01,1.201403e+01,1.210224e+01,
1.219132e+01,1.228126e+01,1.237208e+01,1.246383e+01,1.255663e+01,1.265058e+01,1.274603e+01,
1.284374e+01,1.294447e+01,1.304928e+01,1.315968e+01,1.327765e+01,1.340570e+01,1.354690e+01,
1.370470e+01,1.388274e+01,1.408458e+01,1.431338e+01,1.457185e+01,1.486217e+01,1.518618e+01,
1.554557e+01,1.594212e+01,1.637787e+01,1.685527e+01,1.737730e+01,1.794750e+01,1.857000e+01,
1.924953e+01,1.999143e+01,2.080156e+01,2.168625e+01,2.265222e+01,2.370633e+01,2.485539e+01,
2.610582e+01,2.746321e+01,2.893183e+01,3.051411e+01,3.221007e+01,3.401687e+01,3.592851e+01,
3.793576e+01,4.002641e+01,4.218581e+01,4.439773e+01,4.664525e+01,4.891184e+01,5.118231e+01,
5.344347e+01,5.568452e+01,5.789724e+01,6.007592e+01,6.221721e+01,6.431975e+01,6.638393e+01,
6.841150e+01,7.040530e+01,7.236897e+01,7.430671e+01,7.622307e+01,7.812281e+01,8.001079e+01,
8.189187e+01,8.377082e+01,8.565227e+01,8.754065e+01,8.944017e+01,9.135476e+01,9.328811e+01,
9.524359e+01,9.722429e+01,9.923302e+01,1.012723e+02,1.033445e+02,1.054517e+02,1.075958e+02,
1.097786e+02,1.120016e+02,1.142666e+02,1.165748e+02,1.189278e+02,1.213268e+02,1.237731e+02,
1.262679e+02,1.288124e+02,1.314077e+02,1.340547e+02,1.367544e+02,1.395077e+02,1.423154e+02,
1.451783e+02,1.480970e+02,1.510722e+02,1.541047e+02,1.571950e+02,1.603439e+02,1.635520e+02,
1.668202e+02,1.701491e+02,1.735398e+02,1.769930e+02,1.805099e+02,1.840917e+02,1.877394e+02,
1.914545e+02,1.952384e+02,1.990926e+02,2.030187e+02,2.070186e+02,2.110939e+02,2.152468e+02,
2.194793e+02,2.237936e+02,2.281918e+02,2.326765e+02,2.372501e+02,2.419151e+02,2.466743e+02,
2.515305e+02,2.564865e+02,2.615454e+02,2.667102e+02,2.719843e+02,2.773708e+02,2.828733e+02,
2.884953e+02,2.942405e+02,3.001125e+02,3.061153e+02,3.122530e+02,3.185296e+02,3.249493e+02,
3.315166e+02,3.382361e+02,3.451123e+02,3.521500e+02,3.593542e+02,3.667299e+02,3.742825e+02,
3.820173e+02,3.899398e+02,3.980558e+02,4.063711e+02,4.148920e+02,4.236245e+02,4.325751e+02,
4.417506e+02,4.511576e+02,4.608034e+02,4.706951e+02,4.808402e+02,4.912465e+02,5.019218e+02,
5.128744e+02,5.241128e+02,5.356456e+02,5.474817e+02,5.596305e+02,5.721016e+02,5.849046e+02,
5.980497e+02,6.115475e+02,6.254086e+02,6.396443e+02,6.542659e+02,6.692852e+02,6.847145e+02,
7.005664e+02,7.168537e+02,7.335900e+02,7.507889e+02,7.684647e+02,7.866322e+02,8.053064e+02,
8.245030e+02,8.442381e+02,8.645283e+02,8.853909e+02,9.068435e+02,9.289045e+02,9.515927e+02,
9.749276e+02,9.989293e+02,1.023618e+03,1.049017e+03,1.075146e+03,1.102029e+03,1.129689e+03,
1.158151e+03,1.187440e+03,1.217582e+03,1.248602e+03,1.280530e+03,1.313394e+03,1.347222e+03,
1.382046e+03,1.417896e+03,1.454806e+03,1.492808e+03,1.531936e+03,1.572228e+03,1.613719e+03,
1.656447e+03,1.700453e+03,1.745775e+03,1.792457e+03,1.840541e+03,1.890072e+03,1.941097e+03
  };
  static const G4double P212[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,1.684810e-02,4.632984e-02,
8.871441e-02,1.472267e-01,2.242564e-01,3.224773e-01,4.569173e-01,6.454442e-01,8.960796e-01,
1.218055e+00,1.656827e+00,2.255513e+00,3.037441e+00,4.046981e+00,5.395760e+00,7.141687e+00,
9.354437e+00,1.221137e+01,1.582824e+01,2.031476e+01,2.581295e+01,3.246157e+01,3.999597e+01,
4.786507e+01,5.599726e+01,6.379332e+01,7.101625e+01,7.765137e+01,8.363657e+01,8.902498e+01,
9.388937e+01,9.827447e+01,1.022502e+02,1.058707e+02,1.091704e+02,1.122111e+02,1.150053e+02,
1.176048e+02,1.200277e+02,1.222995e+02,1.244494e+02,1.264926e+02,1.284483e+02,1.303362e+02,
1.321723e+02,1.339697e+02,1.357426e+02,1.375044e+02,1.392669e+02,1.410410e+02,1.428367e+02,
1.446633e+02,1.465296e+02,1.484437e+02,1.504133e+02,1.524461e+02,1.545493e+02,1.567303e+02,
1.589966e+02,1.613556e+02,1.638152e+02,1.663834e+02,1.690685e+02,1.718788e+02,1.748232e+02,
1.779103e+02,1.811492e+02,1.845488e+02,1.881180e+02,1.918658e+02,1.958012e+02,1.999327e+02,
2.042690e+02,2.088184e+02,2.135892e+02,2.185894e+02,2.238267e+02,2.293087e+02,2.350430e+02,
2.410367e+02,2.472972e+02,2.538316e+02,2.606472e+02,2.677517e+02,2.751528e+02,2.828593e+02,
2.908809e+02,2.992293e+02,3.079187e+02,3.169676e+02,3.264006e+02,3.362449e+02,3.465537e+02,
3.574312e+02,3.689892e+02,3.813869e+02,3.948465e+02,4.096708e+02,4.262580e+02,4.451096e+02,
4.668260e+02,4.920818e+02,5.215910e+02,5.560706e+02,5.962149e+02,6.426899e+02,6.961495e+02,
7.572662e+02,8.267702e+02,9.054874e+02,9.943751e+02,1.094553e+03,1.207331e+03,1.334230e+03,
1.477006e+03,1.637665e+03,1.818480e+03,2.021994e+03,2.251016e+03,2.508599e+03,2.797992e+03,
3.122568e+03,3.485707e+03,3.890646e+03,4.340289e+03,4.837003e+03,5.382390e+03,5.977096e+03,
6.620672e+03,7.311514e+03,8.046928e+03,8.823291e+03,9.636310e+03,1.048133e+04,1.135372e+04,
1.224914e+04,1.316377e+04,1.409450e+04,1.503898e+04,1.599568e+04,1.696385e+04,1.794346e+04,
1.893518e+04,1.994025e+04,2.096045e+04,2.199802e+04,2.305557e+04,2.413607e+04,2.524277e+04,
2.637920e+04,2.754912e+04,2.875647e+04,3.000538e+04,3.130013e+04,3.264512e+04,3.404488e+04,
3.550403e+04,3.702724e+04,3.861931e+04,4.028511e+04,4.202960e+04,4.385786e+04,4.577509e+04,
4.778667e+04,4.989812e+04,5.211519e+04,5.444383e+04,5.689024e+04,5.946088e+04,6.216245e+04,
6.500194e+04,6.798661e+04,7.112399e+04,7.442190e+04,7.788844e+04,8.153200e+04,8.536127e+04,
8.938526e+04,9.361335e+04,9.805527e+04,1.027212e+05,1.076216e+05,1.127678e+05,1.181712e+05,
1.238442e+05,1.297996e+05,1.360510e+05,1.426127e+05,1.495000e+05,1.567288e+05,1.643162e+05,
1.722801e+05,1.806398e+05,1.894154e+05,1.986284e+05,2.083016e+05,2.184591e+05,2.291266e+05,
2.403312e+05,2.521018e+05,2.644689e+05,2.774648e+05,2.911240e+05,3.054827e+05,3.205795e+05,
3.364554e+05,3.531535e+05,3.707199e+05,3.892030e+05,4.086544e+05,4.291288e+05,4.506839e+05,
4.733811e+05,4.972852e+05,5.224649e+05,5.489933e+05,5.769474e+05,6.064091e+05,6.374651e+05,
6.702071e+05,7.047325e+05,7.411446e+05,7.795524e+05,8.200720e+05,8.628259e+05,9.079443e+05,
9.555652e+05,1.005835e+06,1.058907e+06,1.114948e+06,1.174130e+06,1.236639e+06,1.302669e+06,
1.372430e+06,1.446140e+06,1.524033e+06,1.606356e+06,1.693372e+06,1.785359e+06,1.882612e+06,
1.985444e+06,2.094187e+06,2.209195e+06,2.330839e+06,2.459517e+06,2.595650e+06,2.739683e+06,
2.892091e+06,3.053375e+06,3.224069e+06,3.404739e+06,3.595986e+06,3.798446e+06,4.012797e+06,
4.239756e+06,4.480087e+06,4.734597e+06,5.004147e+06,5.289647e+06,5.592067e+06,5.912434e+06,
6.251840e+06,6.611444e+06,6.992476e+06,7.396245e+06,7.824137e+06,8.277627e+06,8.758281e+06,
9.267762e+06,9.807836e+06,1.038038e+07,1.098738e+07,1.163097e+07,1.231338e+07,1.303701e+07,
1.380440e+07,1.461824e+07,1.548140e+07,1.639693e+07,1.736806e+07,1.839823e+07,1.949109e+07,
2.065053e+07,2.188068e+07,2.318592e+07,2.457091e+07,2.604061e+07,2.760029e+07,2.925554e+07,
3.101232e+07,3.287695e+07,3.485616e+07,3.695711e+07,3.918738e+07,4.155507e+07,4.406876e+07
  };
  static const G4double P013[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,3.486588e-04,9.620452e-04,1.718673e-03,2.622926e-03,3.703060e-03,
5.154692e-03,7.030543e-03,9.343593e-03,1.210723e-02,1.562440e-02,2.020681e-02,2.588705e-02,
3.270264e-02,4.120096e-02,5.179545e-02,6.455027e-02,7.958144e-02,9.716667e-02,1.174069e-01,
1.402537e-01,1.647614e-01,1.907608e-01,2.180777e-01,2.456543e-01,2.733481e-01,3.011684e-01,
3.291280e-01,3.571385e-01,3.844637e-01,4.109055e-01,4.353907e-01,4.572419e-01,4.764803e-01,
4.931270e-01,5.074779e-01,5.198558e-01,5.305684e-01,5.398436e-01,5.479555e-01,5.550351e-01,
5.612855e-01,5.668161e-01,5.717392e-01,5.761608e-01,5.801498e-01,5.837742e-01,5.870950e-01,
5.901599e-01,5.930068e-01,5.956710e-01,5.981820e-01,6.005639e-01,6.028363e-01,6.050157e-01,
6.071155e-01,6.091467e-01,6.111183e-01,6.130377e-01,6.149109e-01,6.167431e-01,6.185383e-01,
6.203001e-01,6.220315e-01,6.237350e-01,6.254128e-01,6.270666e-01,6.286979e-01,6.303078e-01,
6.318973e-01,6.334668e-01,6.350169e-01,6.365477e-01,6.380590e-01,6.395509e-01,6.410229e-01,
6.424746e-01,6.439056e-01,6.453153e-01,6.467032e-01,6.480686e-01,6.494110e-01,6.507298e-01,
6.520247e-01,6.532950e-01,6.545406e-01,6.557611e-01,6.569564e-01,6.581265e-01,6.592715e-01,
6.603918e-01,6.614881e-01,6.625614e-01,6.636131e-01,6.646453e-01,6.656609e-01,6.666658e-01,
6.676680e-01,6.686761e-01,6.697018e-01,6.707597e-01,6.718679e-01,6.730471e-01,6.743195e-01,
6.757071e-01,6.772293e-01,6.789009e-01,6.807315e-01,6.827259e-01,6.848850e-01,6.872080e-01,
6.896938e-01,6.923421e-01,6.951548e-01,6.981356e-01,7.012908e-01,7.046287e-01,7.081597e-01,
7.118958e-01,7.158503e-01,7.200374e-01,7.244715e-01,7.291667e-01,7.341357e-01,7.393889e-01,
7.449331e-01,7.507700e-01,7.568949e-01,7.632950e-01,7.699485e-01,7.768239e-01,7.838803e-01,
7.910682e-01,7.983318e-01,8.056115e-01,8.128477e-01,8.199835e-01,8.269680e-01,8.337591e-01,
8.403238e-01,8.466393e-01,8.526920e-01,8.584765e-01,8.639943e-01,8.692526e-01,8.742622e-01,
8.790371e-01,8.835929e-01,8.879462e-01,8.921136e-01,8.961116e-01,8.999559e-01,9.036615e-01,
9.072423e-01,9.107112e-01,9.140799e-01,9.173590e-01,9.205578e-01,9.236848e-01,9.267473e-01,
9.297517e-01,9.327034e-01,9.356072e-01,9.384670e-01,9.412861e-01,9.440675e-01,9.468134e-01,
9.495258e-01,9.522065e-01,9.548567e-01,9.574777e-01,9.600704e-01,9.626357e-01,9.651742e-01,
9.676864e-01,9.701728e-01,9.726336e-01,9.750691e-01,9.774794e-01,9.798646e-01,9.822246e-01,
9.845594e-01,9.868688e-01,9.891530e-01,9.914116e-01,9.936448e-01,9.958524e-01,9.980344e-01,
1.000191e+00,1.002322e+00,1.004428e+00,1.006508e+00,1.008564e+00,1.010594e+00,1.012601e+00,
1.014583e+00,1.016542e+00,1.018478e+00,1.020391e+00,1.022281e+00,1.024150e+00,1.025997e+00,
1.027824e+00,1.029630e+00,1.031416e+00,1.033183e+00,1.034931e+00,1.036661e+00,1.038373e+00,
1.040068e+00,1.041746e+00,1.043408e+00,1.045054e+00,1.046684e+00,1.048300e+00,1.049901e+00,
1.051488e+00,1.053062e+00,1.054622e+00,1.056169e+00,1.057704e+00,1.059227e+00,1.060738e+00,
1.062238e+00,1.063727e+00,1.065205e+00,1.066673e+00,1.068130e+00,1.069578e+00,1.071017e+00,
1.072446e+00,1.073866e+00,1.075278e+00,1.076681e+00,1.078076e+00,1.079463e+00,1.080843e+00,
1.082215e+00,1.083579e+00,1.084937e+00,1.086288e+00,1.087632e+00,1.088970e+00,1.090301e+00,
1.091626e+00,1.092946e+00,1.094259e+00,1.095567e+00,1.096870e+00,1.098168e+00,1.099460e+00,
1.100747e+00,1.102030e+00,1.103308e+00,1.104581e+00,1.105850e+00,1.107115e+00,1.108376e+00,
1.109632e+00,1.110885e+00,1.112134e+00,1.113379e+00,1.114621e+00,1.115859e+00,1.117094e+00,
1.118326e+00,1.119555e+00,1.120780e+00,1.122003e+00,1.123223e+00,1.124440e+00,1.125655e+00,
1.126867e+00,1.128077e+00,1.129284e+00,1.130489e+00,1.131692e+00,1.132893e+00,1.134092e+00,
1.135289e+00,1.136484e+00,1.137677e+00,1.138868e+00,1.140058e+00,1.141247e+00,1.142434e+00,
1.143619e+00,1.144803e+00,1.145986e+00,1.147168e+00,1.148349e+00,1.149528e+00,1.150707e+00,
1.151884e+00,1.153061e+00,1.154237e+00,1.155413e+00,1.156587e+00,1.157761e+00,1.158935e+00
  };
  static const G4double P113[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,1.995454e-03,5.606334e-03,1.020784e-02,1.588884e-02,2.290033e-02,
3.263520e-02,4.562653e-02,6.216957e-02,8.258086e-02,1.094157e-01,1.455118e-01,1.917068e-01,
2.489316e-01,3.226120e-01,4.174284e-01,5.352577e-01,6.785911e-01,8.516731e-01,1.057284e+00,
1.296801e+00,1.561942e+00,1.852224e+00,2.166939e+00,2.494770e+00,2.834492e+00,3.186635e+00,
3.551797e+00,3.929232e+00,4.309095e+00,4.688324e+00,5.050545e+00,5.384012e+00,5.686875e+00,
5.957193e+00,6.197595e+00,6.411465e+00,6.602403e+00,6.772914e+00,6.926734e+00,7.065197e+00,
7.191277e+00,7.306340e+00,7.411974e+00,7.509819e+00,7.600857e+00,7.686161e+00,7.766762e+00,
7.843477e+00,7.916960e+00,7.987876e+00,8.056797e+00,8.124211e+00,8.190532e+00,8.256119e+00,
8.321275e+00,8.386263e+00,8.451305e+00,8.516592e+00,8.582289e+00,8.648537e+00,8.715465e+00,
8.783185e+00,8.851798e+00,8.921398e+00,8.992070e+00,9.063890e+00,9.136925e+00,9.211233e+00,
9.286866e+00,9.363861e+00,9.442250e+00,9.522053e+00,9.603281e+00,9.685935e+00,9.770007e+00,
9.855482e+00,9.942335e+00,1.003054e+01,1.012005e+01,1.021082e+01,1.030282e+01,1.039600e+01,
1.049029e+01,1.058565e+01,1.068203e+01,1.077938e+01,1.087765e+01,1.097681e+01,1.107684e+01,
1.117772e+01,1.127947e+01,1.138215e+01,1.148585e+01,1.159077e+01,1.169718e+01,1.180571e+01,
1.191727e+01,1.203294e+01,1.215425e+01,1.228322e+01,1.242248e+01,1.257521e+01,1.274509e+01,
1.293604e+01,1.315194e+01,1.339632e+01,1.367216e+01,1.398189e+01,1.432750e+01,1.471074e+01,
1.513342e+01,1.559757e+01,1.610563e+01,1.666058e+01,1.726601e+01,1.792615e+01,1.864590e+01,
1.943082e+01,2.028712e+01,2.122158e+01,2.224152e+01,2.335462e+01,2.456876e+01,2.589168e+01,
2.733069e+01,2.889211e+01,3.058077e+01,3.239937e+01,3.434789e+01,3.642310e+01,3.861815e+01,
4.092260e+01,4.332263e+01,4.580166e+01,4.834130e+01,5.092240e+01,5.352617e+01,5.613531e+01,
5.873476e+01,6.131210e+01,6.385783e+01,6.636527e+01,6.883039e+01,7.125148e+01,7.362876e+01,
7.596408e+01,7.826048e+01,8.052200e+01,8.275327e+01,8.495940e+01,8.714572e+01,8.931771e+01,
9.148086e+01,9.364060e+01,9.580217e+01,9.797066e+01,1.001509e+02,1.023474e+02,1.045645e+02,
1.068062e+02,1.090760e+02,1.113772e+02,1.137131e+02,1.160862e+02,1.184992e+02,1.209544e+02,
1.234539e+02,1.259998e+02,1.285938e+02,1.312377e+02,1.339332e+02,1.366818e+02,1.394850e+02,
1.423440e+02,1.452603e+02,1.482350e+02,1.512691e+02,1.543638e+02,1.575200e+02,1.607384e+02,
1.640199e+02,1.673652e+02,1.707750e+02,1.742501e+02,1.777910e+02,1.813985e+02,1.850734e+02,
1.888164e+02,1.926284e+02,1.965104e+02,2.004634e+02,2.044885e+02,2.085870e+02,2.127603e+02,
2.170098e+02,2.213372e+02,2.257441e+02,2.302324e+02,2.348040e+02,2.394612e+02,2.442060e+02,
2.490408e+02,2.539681e+02,2.589904e+02,2.641105e+02,2.693311e+02,2.746552e+02,2.800858e+02,
2.856261e+02,2.912794e+02,2.970490e+02,3.029386e+02,3.089517e+02,3.150921e+02,3.213638e+02,
3.277707e+02,3.343169e+02,3.410068e+02,3.478447e+02,3.548352e+02,3.619830e+02,3.692928e+02,
3.767697e+02,3.844189e+02,3.922454e+02,4.002549e+02,4.084528e+02,4.168450e+02,4.254374e+02,
4.342361e+02,4.432473e+02,4.524776e+02,4.619337e+02,4.716224e+02,4.815508e+02,4.917263e+02,
5.021563e+02,5.128485e+02,5.238111e+02,5.350521e+02,5.465802e+02,5.584039e+02,5.705323e+02,
5.829747e+02,5.957407e+02,6.088401e+02,6.222830e+02,6.360800e+02,6.502418e+02,6.647796e+02,
6.797048e+02,6.950292e+02,7.107652e+02,7.269252e+02,7.435222e+02,7.605696e+02,7.780812e+02,
7.960713e+02,8.145545e+02,8.335460e+02,8.530613e+02,8.731166e+02,8.937286e+02,9.149143e+02,
9.366915e+02,9.590784e+02,9.820938e+02,1.005757e+03,1.030089e+03,1.055109e+03,1.080839e+03,
1.107302e+03,1.134519e+03,1.162515e+03,1.191313e+03,1.220939e+03,1.251419e+03,1.282778e+03,
1.315045e+03,1.348248e+03,1.382416e+03,1.417580e+03,1.453770e+03,1.491019e+03,1.529361e+03,
1.568829e+03,1.609460e+03,1.651289e+03,1.694355e+03,1.738697e+03,1.784354e+03,1.831369e+03,
1.879785e+03,1.929646e+03,1.980999e+03,2.033889e+03,2.088367e+03,2.144482e+03,2.202288e+03
  };
  static const G4double P213[nE]={
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,0.000000e+00,
0.000000e+00,0.000000e+00,1.142094e-02,3.267935e-02,6.066632e-02,9.636049e-02,1.418783e-01,
2.071675e-01,2.971476e-01,4.154746e-01,5.662385e-01,7.709965e-01,1.055354e+00,1.431069e+00,
1.911579e+00,2.550442e+00,3.399081e+00,4.487682e+00,5.854588e+00,7.558283e+00,9.647155e+00,
1.215838e+01,1.502709e+01,1.826833e+01,2.189443e+01,2.579202e+01,2.995976e+01,3.441746e+01,
3.918699e+01,4.427325e+01,4.955437e+01,5.499373e+01,6.035267e+01,6.544206e+01,7.021030e+01,
7.460021e+01,7.862768e+01,8.232331e+01,8.572679e+01,8.886164e+01,9.177863e+01,9.448690e+01,
9.703032e+01,9.942442e+01,1.016911e+02,1.038565e+02,1.059344e+02,1.079422e+02,1.098987e+02,
1.118190e+02,1.137160e+02,1.156037e+02,1.174956e+02,1.194037e+02,1.213395e+02,1.233134e+02,
1.253353e+02,1.274148e+02,1.295607e+02,1.317816e+02,1.340858e+02,1.364815e+02,1.389768e+02,
1.415800e+02,1.442993e+02,1.471432e+02,1.501202e+02,1.532394e+02,1.565095e+02,1.599395e+02,
1.635387e+02,1.673159e+02,1.712805e+02,1.754412e+02,1.798071e+02,1.843868e+02,1.891890e+02,
1.942219e+02,1.994937e+02,2.050125e+02,2.107860e+02,2.168218e+02,2.231274e+02,2.297101e+02,
2.365773e+02,2.437362e+02,2.511944e+02,2.589596e+02,2.670399e+02,2.754442e+02,2.841827e+02,
2.932671e+02,3.027119e+02,3.125358e+02,3.227634e+02,3.334288e+02,3.445781e+02,3.563001e+02,
3.687193e+02,3.819927e+02,3.963408e+02,4.120653e+02,4.295658e+02,4.493503e+02,4.720319e+02,
4.983109e+02,5.289362e+02,5.646649e+02,6.062317e+02,6.543377e+02,7.096627e+02,7.728953e+02,
8.447731e+02,9.261237e+02,1.017903e+03,1.121228e+03,1.237410e+03,1.367978e+03,1.514703e+03,
1.679620e+03,1.865051e+03,2.073617e+03,2.308241e+03,2.572149e+03,2.868835e+03,3.202016e+03,
3.575541e+03,3.993264e+03,4.458871e+03,4.975670e+03,5.546355e+03,6.172760e+03,6.855637e+03,
7.594502e+03,8.387576e+03,9.231842e+03,1.012323e+04,1.105692e+04,1.202765e+04,1.303017e+04,
1.405955e+04,1.511144e+04,1.618224e+04,1.726924e+04,1.837063e+04,1.948547e+04,2.061368e+04,
2.175592e+04,2.291353e+04,2.408848e+04,2.528322e+04,2.650068e+04,2.774417e+04,2.901736e+04,
3.032420e+04,3.166894e+04,3.305604e+04,3.449021e+04,3.597630e+04,3.751935e+04,3.912456e+04,
4.079722e+04,4.254275e+04,4.436670e+04,4.627470e+04,4.827254e+04,5.036614e+04,5.256157e+04,
5.486506e+04,5.728307e+04,5.982226e+04,6.248955e+04,6.529208e+04,6.823731e+04,7.133295e+04,
7.458700e+04,7.800776e+04,8.160383e+04,8.538411e+04,8.935781e+04,9.353444e+04,9.792387e+04,
1.025363e+05,1.073824e+05,1.124731e+05,1.178200e+05,1.234350e+05,1.293307e+05,1.355202e+05,
1.420175e+05,1.488372e+05,1.559945e+05,1.635058e+05,1.713882e+05,1.796600e+05,1.883405e+05,
1.974500e+05,2.070104e+05,2.170444e+05,2.275766e+05,2.386327e+05,2.502401e+05,2.624280e+05,
2.752272e+05,2.886704e+05,3.027922e+05,3.176294e+05,3.332210e+05,3.496083e+05,3.668349e+05,
3.849474e+05,4.039947e+05,4.240291e+05,4.451057e+05,4.672829e+05,4.906228e+05,5.151909e+05,
5.410568e+05,5.682940e+05,5.969807e+05,6.271995e+05,6.590381e+05,6.925891e+05,7.279509e+05,
7.652278e+05,8.045300e+05,8.459747e+05,8.896858e+05,9.357945e+05,9.844402e+05,1.035770e+06,
1.089941e+06,1.147119e+06,1.207478e+06,1.271207e+06,1.338501e+06,1.409570e+06,1.484636e+06,
1.563934e+06,1.647715e+06,1.736241e+06,1.829794e+06,1.928671e+06,2.033188e+06,2.143679e+06,
2.260499e+06,2.384024e+06,2.514653e+06,2.652811e+06,2.798946e+06,2.953535e+06,3.117084e+06,
3.290130e+06,3.473241e+06,3.667023e+06,3.872116e+06,4.089200e+06,4.318999e+06,4.562278e+06,
4.819852e+06,5.092583e+06,5.381389e+06,5.687242e+06,6.011176e+06,6.354288e+06,6.717741e+06,
7.102774e+06,7.510697e+06,7.942907e+06,8.400882e+06,8.886195e+06,9.400517e+06,9.945620e+06,
1.052339e+07,1.113582e+07,1.178504e+07,1.247331e+07,1.320302e+07,1.397672e+07,1.479713e+07,
1.566710e+07,1.658970e+07,1.756817e+07,1.860597e+07,1.970674e+07,2.087440e+07,2.211307e+07,
2.342715e+07,2.482132e+07,2.630054e+07,2.787009e+07,2.953558e+07,3.130298e+07,3.317862e+07,
3.516924e+07,3.728200e+07,3.952452e+07,4.190489e+07,4.443171e+07,4.711414e+07,4.996191e+07
};
  static const G4double* P0[nN]=
                           {P00,P01,P02,P03,P04,P05,P06,P07,P08,P09,P010,P011,P012,P013};
  static const G4double* P1[nN]=
                           {P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P110,P111,P112,P113};
  static const G4double* P2[nN]=
                           {P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P210,P211,P212,P213};
  // --------------------------------
  G4int r=-1;                          // Low channel for J-functions
  if(a<=.9999 || a>238.49)             // Plutonium 244 is forbidden
  {
    G4cout<<"***G4QTauNuclearCrossSection::GetFunctions:A="<<a<<".No CS returned"<<G4endl;
    return r;
  }
  G4int iA=static_cast<G4int>(a+.499); // Make the round integer of the atomic number
  G4double ai=iA;
  if(a!=ai) a=ai;
  for(G4int i=0; i<nN; i++)
  {
    if(std::fabs(a-A[i])<.0005)        // A coincides with one of basic A's -> get from Tab
    {
      for(G4int k=0; k<nE; k++)
      {
        x[k]=P0[i][k];                 // J0
        y[k]=P1[i][k];                 // J1
        z[k]=P2[i][k];                 // J2
	  }
      r=L[i];                          // Low channel for the J-functions
    }
    if(r<0)                            // Not the basic A-value -> must be calculated
    {
      G4int k=0;                       // !! To be good for different compilers !!
      for(k=1; k<nN; k++)if(a<A[k]) break;// Find the top basic A-value
      if(k<1) k=1;                     // Extrapolation from the first bin (D)
      if(k>=nN) k=nN-1;                // Extrapolation from the last bin (U)
      G4int     k1=k-1;
      G4double  xi=A[k1];
      G4double   b=(a-xi)/(A[k]-xi);
      for(G4int m=0; m<nE; m++)
      {
        G4double xi=P0[k1][m];
        x[m]=xi+(P0[k][m]-xi)*b;
        G4double yi=P1[k1][m];
        y[m]=yi+(P1[k][m]-yi)*b;
        G4double zi=P2[k1][m];
        z[m]=zi+(P2[k][m]-zi)*b;
      }
      r=L[k];
      if(L[k1]<r) r=L[k1];
	}
  }
  return r;
}

G4double G4QTauNuclearCrossSection::GetExchangeEnergy()
{
  // @@ All constants are copy of that from GetCrossSection funct. => Make them general.
  static const G4int nE=336; // !!  If change this, change it in GetFunctions() (*.hh) !!
  static const G4int mL=nE-1;
  static const G4double EMi=2.0612;          // Minimum Energy
  static const G4double EMa=50000.;          // Maximum Energy
  static const G4double lEMi=std::log(EMi);       // Minimum logarithmic Energy
  static const G4double lEMa=std::log(EMa);       // Maximum logarithmic Energy
  static const G4double dlnE=(lEMa-lEMi)/mL; // Logarithmic step in Energy
  static const G4double mtu=1777.;       // Mass of muon in MeV
  static const G4double lmtu=std::log(mtu);       // Log of muon mass
  G4double phLE=0.;                     // Prototype of the log(nu=E_gamma)
  G4double Y[nE];                       // Prepare the array for randomization
#ifdef debug
  G4cout<<"G4QTauNuclCrossSect::GetExchanEn: B="<<lastF<<",l="<<lastL<<",1="<<lastJ1[lastL]
        <<",2="<<lastJ2[lastL]<<",3="<<lastJ3[lastL]<<",S="<<lastSig<<",E="<<lastE<<G4endl;
#endif
  G4double lastLE=lastG+lmtu;           // recover log(eE) from the gamma (lastG)
  G4double dlg1=lastG+lastG-1.;
  G4double lgoe=lastG/lastE;
  for(G4int i=lastF; i<=lastL; i++)
                            Y[i]=dlg1*lastJ1[i]-lgoe*(lastJ2[i]+lastJ2[i]-lastJ3[i]/lastE);
  G4double ris=lastSig*G4UniformRand(); // Sig can be > Y[lastL=mL], then it is funct. reg.
#ifdef debug
  G4cout<<"G4QTauNuclearCrossSection::GetExchangeEnergy: "<<ris<<",Y="<<Y[lastL]<<G4endl;
#endif
  if(ris<Y[lastL])                      // Search in the table
  {
	   G4int j=lastF;
    G4double Yj=Y[j];                   // It mast be 0 (some times just very small)
    while (ris>Yj && j<lastL)           // Associative search
	   {
      j++;
      Yj=Y[j];                          // High value
	   }
    G4int j1=j-1;
    G4double Yi=Y[j1];                  // Low value
    phLE=lEMi+(j1+(ris-Yi)/(Yj-Yi))*dlnE;
#ifdef debug
	G4cout<<"G4QTauNuclearCS::E="<<phLE<<",l="<<lEMi<<",j="<<j<<",ris="<<ris<<",Yi="<<Yi
          <<",Y="<<Yj<<G4endl;
#endif
  }
  else                                  // Search with the function
  {
    if(lastL<mL)
      G4cerr<<"**G4QTauNucCS::GetExEn: L="<<lastL<<",S="<<lastSig<<",Y="<<Y[lastL]<<G4endl;
    G4double f=(ris-Y[lastL])/lastH;    // ScaledResidualValue of the cross-sec. integral
#ifdef pdebug
	   G4cout<<"G4QTauNucCS::GetExEn:HighEnergy f="<<f<<", ris="<<ris<<", lH="<<lastH<<G4endl;
#endif
    phLE=SolveTheEquation(f);           // Solve equation to find theLog(phE) (comp lastLE)
#ifdef pdebug
   	G4cout<<"G4QTauNuclearCrossSection::GetExchangeEnergy: HighEnergy lphE="<<phLE<<G4endl;
#endif
  }
  if(phLE>lastLE)
  {
    G4cerr<<"**G4QTauNuclearCS::GetExEnergy: N="<<lastN<<",Z="<<lastZ<<",lpE"<<phLE<<">leE"
          <<lastLE<<",Sig="<<lastSig<<",rndSig="<<ris<<",Beg="<<lastF<<",End="<<lastL
          <<",Y="<<Y[lastL]<<G4endl;
    if(lastLE<7.2) phLE=std::log(std::exp(lastLE)-mtu);
    else phLE=7.;
  }
  return std::exp(phLE);
}

G4double G4QTauNuclearCrossSection::SolveTheEquation(G4double f)
{
  // This parameters must correspond to the G4PhotonuclearCrossSec::GetCrossSec parameters
  static const G4double shd=1.0734;                    // HE PomShadowing(D)
  static const G4double poc=0.0375;                    // HE Pomeron coefficient
  static const G4double pos=16.5;                      // HE Pomeron shift
  static const G4double reg=.11;                       // HE Reggeon slope
  static const G4double EMa=50000.;                    // Maximum Energy
  static const G4double mtu=1777.;                     // Mass of muon in MeV
  static const G4double lmtu=std::log(mtu);                 // Log of muon mass
  static const G4double z=std::log(EMa);                    // Initial argument
  static const G4double p=poc*(z-pos)+shd*std::exp(-reg*z); // CrossX on theHighTabEdge
  static const G4int    imax=27;   // Not more than "imax" steps to find the solution
  static const G4double eps=0.001; // Accuracy which satisfies the search
  G4double lastLE=lastG+lmtu;                 // recover log(eE) from the gamma (lastG)
  G4double topLim=lastLE-.001;                // maximum log(phE) for equivalent photons
  G4double rE=EMa/std::exp(lastLE);                // r=EMa/Eel to make the firs guess
  G4double x=z+f/p/(lastG*(2.-rE*(2.-rE))-1.);// First guess (the first step from the edge)
#ifdef pdebug
  G4cout<<"G4QTauNucCS::SolveTheEq: e="<<eps<<",f="<<f<<",z="<<z<<",p="<<p<<",lastG="
        <<lastG<<",x="<<x<<G4endl;
#endif
  if(x>topLim) x=topLim;
  for(G4int i=0; i<imax; i++)
  {
    G4double fx=Fun(x);
    G4double df=DFun(x);
    G4double d=(f-fx)/df;
    x=x+d;
#ifdef pdebug
    G4cout<<"G4QTauNCS::SolveEq:i="<<i<<",d="<<d<<",x="<<x<<",fx="<<fx<<",df="<<df<<G4endl;
#endif
    if(x>=lastLE)
	   {
      G4cerr<<"*G4QTauNCS::SolveTheEq:*Correction*"<<i<<",d="<<d<<",x="<<x<<">lE="<<lastLE
            <<",f="<<f<<",fx="<<fx<<",df="<<df<<",A(Z="<<lastZ<<",N="<<lastN<<")"<<G4endl;
      x=topLim;
      if(i)G4Exception("G4QTauNuclearCrossSect::SolveTheEq()","009",FatalException,"E>eE");
    }
    if(std::fabs(d)<eps) break;
    if(i+1>=imax) G4cerr<<"G4QTauNucCS::SolveTheE:"<<i+2<<">"<<imax<<"->Use bigMax ln(eE)="
                        <<lastLE<<",Z="<<lastZ<<", N="<<lastN<<G4endl;
  }
  return x;
}

// Randomize Q2 for the scattered muon when nu is already known
G4double G4QTauNuclearCrossSection::GetExchangeQ2(G4double nu)
{
  static const G4double mtu=1777.;      // Mass of muon in MeV
  static const G4double mtu2=mtu*mtu;   // Squared Mass of muon in MeV
  G4double y=nu/lastE;                  // Part of energy carried by the equivalent pfoton
  if(y>=1.-1./(lastG+lastG)) return 0.; // The region where the method does not work
  G4double y2=y*y;                      // Squared photonic part of energy
  G4double ye=1.-y;                     // Part of energy carried by the secondary tau-lept
  G4double Qi2=mtu2*y2/ye;              // Minimum Q2
  G4double Qa2=4*lastE*lastE*ye;        // Maximum Q2
  G4double iar=Qi2/Qa2;                 // Q2min/Q2max ratio
  G4double Dy=ye+.5*y2;                 // D(y) function
  G4double Py=ye/Dy;                    // P(y) function
  G4double ePy=1.-std::exp(Py);              // 1-exp(P(y)) part
  G4double Uy=Py*(1.-iar);              // U(y) function
  G4double Fy=(ye+ye)*(1.+ye)*iar/y2;   // F(y) function
  G4double fr=iar/(1.-ePy*iar);         // Q-fraction
  if(Fy<=-fr)
  {
#ifdef edebug
    G4cerr<<"***G4QTauNuclCS::GetExchQ2: Fy="<<Fy<<"+fr="<<fr<<" <0"<<",iar="<<iar<<G4endl;
#endif
    return 0.;
  }    
  G4double LyQa2=std::log(Fy+fr);            // L(y,Q2max) function
  G4bool cond=true;
  G4int maxTry=3;
  G4int cntTry=0;
  G4double Q2=Qi2;
  while(cond&&cntTry<maxTry)            // The loop to avoid x>1.
  {
    G4double R=G4UniformRand();         // Random number (0,1)
    Q2=Qi2*(ePy+1./(std::exp(R*LyQa2-(1.-R)*Uy)-Fy));
    cntTry++;
    cond = Q2>1878.*nu;
  }
  if(Q2<Qi2)
  {
#ifdef edebug
    G4cerr<<"*G4QTauNuclearCrossSection::GetExchangeQ2: Q2="<<Q2<<" < Q2min="<<Qi2<<G4endl;
#endif
    return Qi2;
  }  
  if(Q2>Qa2)
  {
#ifdef edebug
    G4cerr<<"*G4QTauNuclearCrossSection::GetExchangeQ2: Q2="<<Q2<<" > Q2max="<<Qi2<<G4endl;
#endif
    return Qa2;
  }  
  return Q2;
}

// This class can provide only virtual exchange photons for all nu's and Q2's
G4int G4QTauNuclearCrossSection::GetExchangePDGCode() {return 22;}

G4double G4QTauNuclearCrossSection::GetVirtualFactor(G4double nu, G4double Q2)
{
  static const G4double dM=938.27+939.57;// Mean double nucleon mass = m_n+m_p (no binding)
  static const G4double Q0=843.;         // Coefficient of the dipole nucleonic form-factor
  static const G4double Q02=Q0*Q0;       // Squared coefficient of theDipoleNuclFormFactor
  static const G4double blK0=std::log(185.);  // Coefficient of the b-function
  static const G4double bp=0.85;         // Power of the b-function
  static const G4double clK0=std::log(1390.); // Coefficient of the c-function
  static const G4double cp=3.;           // Power of the c-function
  //G4double x=Q2/dM/nu;                 // Direct x definition
  G4double K=nu-Q2/dM;                   // K=nu*(1-x)
  if(K<0.)
  {
#ifdef edebug
    G4cerr<<"**G4QTauNuclCS::GetVirtFact:K="<<K<<",nu="<<nu<<",Q2="<<Q2<<",d="<<dM<<G4endl;
#endif
    return 0.;
  }
  G4double lK=std::log(K);               // ln(K)
  G4double x=1.-K/nu;                    // This definitin saves one div.
  G4double GD=1.+Q2/Q02;                 // Reversed nucleonic form-factor
  G4double b=std::exp(bp*(lK-blK0));     // b-factor
  G4double c=std::exp(cp*(lK-clK0));     // c-factor
  G4double r=.5*std::log(Q2+nu*nu)-lK;   // r=.5*log((Q^2+nu^2)/K^2)
  G4double ef=std::exp(r*(b-c*r*r));     // exponential factor
  return (1.-x)*ef/GD/GD;
}
