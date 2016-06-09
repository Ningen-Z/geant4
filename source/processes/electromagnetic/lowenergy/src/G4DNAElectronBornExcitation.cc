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
// $Id: G4DNAElectronBornExcitation.cc,v 1.3 2006/06/29 19:39:04 gunter Exp $
// GEANT4 tag $Name: geant4-09-00 $

#include "G4DNAElectronBornExcitation.hh"
#include "G4Electron.hh"

                                        G4DNAElectronBornExcitationEnergyLimitsPolicy::G4DNAElectronBornExcitationEnergyLimitsPolicy()
:
 lowEnergyLimit(7*eV),
 zeroBelowLowEnergyLimit(true),
 highEnergyLimit(10*keV),
 zeroAboveHighEnergyLimit(false)
{
}

                                        G4DNAElectronBornExcitationIncomingParticlePolicy :: G4DNAElectronBornExcitationIncomingParticlePolicy()

{
}

                                        G4DNAElectronBornExcitationDataFilePolicy :: G4DNAElectronBornExcitationDataFilePolicy()
:
 lowEnergyLimit(7*eV),
 zeroBelowLowEnergyLimit(true),
 highEnergyLimit(10*keV),
 zeroAboveHighEnergyLimit(false),
 dataFileEnergyUnit(eV),
 dataFileCrossSectionUnit(m*m),
 dataFileName("ElectronBornExcitationCrossSection")
{
}

const G4ParticleDefinition *            G4DNAElectronBornExcitationIncomingParticlePolicy :: IncomingParticleDefinition(void) const
{
 return G4Electron::Electron();
}

