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
// GEANT4 tag $Name: geant4-09-00 $
//
//---------------------------------------------------------------------------
//
// ClassName:   G4LEPProtonBuilder
//
// Author: 2002 J.P. Wellisch
//
// Modified:
// 16.11.2005 G.Folger: don't  keep processes as data members, but new these
// 13.06.2006 G.Folger: (re)move elastic scatterring 
//
//----------------------------------------------------------------------------
#ifndef G4LEPProtonBuilder_h
#define G4LEPProtonBuilder_h 1

#include "globals.hh"

#include "G4HadronElasticProcess.hh"
#include "G4ProtonInelasticProcess.hh"
#include "G4VProtonBuilder.hh"

#include "G4LEProtonInelastic.hh"

class G4LEPProtonBuilder : public G4VProtonBuilder
{
  public: 
    G4LEPProtonBuilder();
    virtual ~G4LEPProtonBuilder();

  public: 
    virtual void Build(G4ProtonInelasticProcess * aP);
    virtual void Build(G4HadronElasticProcess * aP);
    
    void SetMinEnergy(G4double aM) 
    {
      theMin=aM;
    }
    void SetMaxEnergy(G4double aM) 
    {
      theMax=aM;
    }
    
  private:
    G4double theMin;
    G4double theMax;
    G4LEProtonInelastic * theLEProtonModel;

};

// 2002 by J.P. Wellisch

#endif

