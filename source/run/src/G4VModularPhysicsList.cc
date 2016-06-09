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
// $Id: G4VModularPhysicsList.cc,v 1.4 2007/06/15 07:26:55 gcosmo Exp $
// GEANT4 tag $Name: geant4-09-00 $
//
// 
// ------------------------------------------------------------
//	GEANT 4 class implementation file 
// ------------------------------------------------------------

#include "G4VModularPhysicsList.hh"

G4VModularPhysicsList::G4VModularPhysicsList()
                  : G4VUserPhysicsList()
{
   physicsVector = new G4PhysConstVector();
}

G4VModularPhysicsList::~G4VModularPhysicsList()
{
  G4PhysConstVector::iterator itr;
  for (itr = physicsVector->begin(); itr!= physicsVector->end(); ++itr) {
    delete (*itr);
  }
  physicsVector->clear();
  delete physicsVector;
}

void G4VModularPhysicsList::ConstructParticle()
{
  // create particles
  G4PhysConstVector::iterator itr;
  for (itr = physicsVector->begin(); itr!= physicsVector->end(); ++itr) {
    (*itr)->ConstructParticle();;
  }
}


void G4VModularPhysicsList::ConstructProcess()
{
  AddTransportation();

  G4PhysConstVector::iterator itr;
  for (itr = physicsVector->begin(); itr!= physicsVector->end(); ++itr) {
    (*itr)->ConstructProcess();
  }
}

