//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// $Id: Em8EventAction.cc,v 1.3.4.1 2001/06/28 19:07:12 gunter Exp $
// GEANT4 tag $Name:  $
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#include "Em8EventAction.hh"

#include "Em8RunAction.hh"

#include "Em8CalorHit.hh"
#include "Em8EventActionMessenger.hh"

#include "g4rw/tvordvec.h"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"
#include "G4UImanager.hh"
#include "G4TrajectoryContainer.hh"
#include "G4Trajectory.hh"
#include "G4VVisManager.hh"
#include "G4ios.hh"
#include "G4UnitsTable.hh"
#include "Randomize.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

Em8EventAction::Em8EventAction(Em8RunAction* Em8RA)
:calorimeterCollID(-1),eventMessenger(NULL),
 verboselevel(0),runaction(Em8RA),drawFlag("all"),printModulo(10000)
{
  eventMessenger = new Em8EventActionMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

Em8EventAction::~Em8EventAction()
{
  delete eventMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::BeginOfEventAction(const G4Event* evt)
{
 G4int evtNb = evt->GetEventID();
 if (evtNb%printModulo == 0) 
    G4cout << "\n---> Begin of Event: " << evtNb << G4endl;
     
  if(verboselevel>1)
    G4cout << "<<< Event  " << evtNb << " started." << G4endl;
    
  if (calorimeterCollID==-1)
    {
     G4SDManager * SDman = G4SDManager::GetSDMpointer();
     calorimeterCollID = SDman->GetCollectionID("CalCollection");
    } 

  nstep = 0. ;
  nstepCharged = 0. ;
  nstepNeutral = 0. ;
  Nch = 0. ;
  Nne = 0. ;
  NE=0.;
  NP=0.;
  Transmitted=0.;
  Reflected  =0.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::EndOfEventAction(const G4Event* evt)
{
  G4HCofThisEvent* HCE = evt->GetHCofThisEvent();
  Em8CalorHitsCollection* CHC = NULL;
  if (HCE)
      CHC = (Em8CalorHitsCollection*)(HCE->GetHC(calorimeterCollID));

  if (CHC)
   {
    int n_hit = CHC->entries();
   // if(verboselevel==2)
   // G4cout << "     " << n_hit
   //      << " hits are stored in Em8CalorHitsCollection." << G4endl;

    G4double totEAbs=0, totLAbs=0;
    for (int i=0;i<n_hit;i++)
      { totEAbs += (*CHC)[i]->GetEdepAbs(); 
        totLAbs += (*CHC)[i]->GetTrakAbs();
      }
  if(verboselevel==2)
    G4cout
       << "   Absorber: total energy: " << G4std::setw(7) << 
                             G4BestUnit(totEAbs,"Energy")
       << "       total track length: " << G4std::setw(7) <<
                             G4BestUnit(totLAbs,"Length")
       << G4endl;           

   // count event, add deposits to the sum ...
    runaction->CountEvent() ;
    runaction->AddTrackLength(totLAbs) ;
    runaction->AddnStepsCharged(nstepCharged) ;
    runaction->AddnStepsNeutral(nstepNeutral) ;
    if(verboselevel==2)
      G4cout << " Ncharged=" << Nch << "  ,   Nneutral=" << Nne << G4endl;
    runaction->CountParticles(Nch,Nne);
    runaction->AddEP(NE,NP);
    runaction->AddTrRef(Transmitted,Reflected) ;
    runaction->AddEdeps(totEAbs) ;
    runaction->FillEn(totEAbs) ;

    nstep=nstepCharged+nstepNeutral ;
    runaction->FillNbOfSteps(nstep);
  }
  
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();

  if(pVVisManager)
  {
   G4TrajectoryContainer* trajectoryContainer = evt->GetTrajectoryContainer();
   G4int n_trajectories = 0;
   if (trajectoryContainer) n_trajectories = trajectoryContainer->entries();  
   for(G4int i=0; i<n_trajectories; i++) 
      { G4Trajectory* trj = (G4Trajectory *)((*(evt->GetTrajectoryContainer()))[i]);
        if (drawFlag == "all") trj->DrawTrajectory(50);
        else if ((drawFlag == "charged")&&(trj->GetCharge() != 0.))
                               trj->DrawTrajectory(50); 
      }
  }  

  if(verboselevel>0)
    G4cout << "<<< Event  " << evt->GetEventID() << " ended." << G4endl;
  
  
  //save rndm status
  if (runaction->GetRndmFreq() == 2)
    { 
     HepRandom::saveEngineStatus("endOfEvent.rndm");   
     G4int evtNb = evt->GetEventID();
     if (evtNb%printModulo == 0)
       { 
        G4cout << "\n---> End of Event: " << evtNb << G4endl;
        HepRandom::showEngineStatus();
       }
    }     
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4int Em8EventAction::GetEventno()
{
  G4int evno = fpEventManager->GetConstCurrentEvent()->GetEventID() ;
  return evno ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::setEventVerbose(G4int level)
{
  verboselevel = level ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::CountStepsCharged()
{
  nstepCharged += 1. ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::CountStepsNeutral()
{
  nstepNeutral += 1. ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::AddCharged() 
{
  Nch += 1.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::AddNeutral() 
{
  Nne += 1.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::AddE() 
{
  NE += 1.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::AddP() 
{
  NP += 1.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::SetTr()
{
  Transmitted = 1.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Em8EventAction::SetRef()
{
  Reflected   = 1.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
  
