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
// $Id: G4StatMFMacroTriNucleon.cc,v 1.5 2006/06/29 20:25:25 gunter Exp $
// GEANT4 tag $Name: geant4-09-00 $
//
// Hadronic Process: Nuclear De-excitations
// by V. Lara

#include "G4StatMFMacroTriNucleon.hh"

// Operators

G4StatMFMacroTriNucleon & G4StatMFMacroTriNucleon::
operator=(const G4StatMFMacroTriNucleon & )
{
    throw G4HadronicException(__FILE__, __LINE__, "G4StatMFMacroTriNucleon::operator= meant to not be accessable");
    return *this;
}


G4bool G4StatMFMacroTriNucleon::operator==(const G4StatMFMacroTriNucleon & ) const
{
    throw G4HadronicException(__FILE__, __LINE__, "G4StatMFMacroTriNucleon::operator== meant to not be accessable");
    return false;
}
 

G4bool G4StatMFMacroTriNucleon::operator!=(const G4StatMFMacroTriNucleon & ) const
{
    throw G4HadronicException(__FILE__, __LINE__, "G4StatMFMacroTriNucleon::operator!= meant to not be accessable");
    return true;
}



G4double G4StatMFMacroTriNucleon::CalcMeanMultiplicity(const G4double FreeVol, const G4double mu, 
						       const G4double nu, const G4double T)
{
    const G4double ThermalWaveLenght = 16.15*fermi/std::sqrt(T);
	
    const G4double lambda3 = ThermalWaveLenght*ThermalWaveLenght*ThermalWaveLenght;
	
    const G4double degeneracy = 2.0+2.0;  // H3 + He3
	
    const G4double Coulomb = (3./5.)*(elm_coupling/G4StatMFParameters::Getr0())*
	(1.0 - 1.0/std::pow(1.0+G4StatMFParameters::GetKappaCoulomb(),1./3.));

    const G4double BindingE = G4NucleiPropertiesTable::GetBindingEnergy(1,theA); // old value was 9.224*MeV
//							+ G4NucleiProperties::GetBindingEnergy(2,theA);

    G4double exponent = (BindingE+ theA*(mu+nu*theZARatio) - 
			 Coulomb*theZARatio*theZARatio*std::pow(static_cast<G4double>(theA),5./3.))/T;
    if (exponent > 700.0) exponent = 700.0;

    _MeanMultiplicity = (degeneracy*FreeVol*static_cast<G4double>(theA)*
			 std::sqrt(static_cast<G4double>(theA))/lambda3)*
	std::exp(exponent);
			 
    return _MeanMultiplicity;
}


G4double G4StatMFMacroTriNucleon::CalcEnergy(const G4double T)
{
    const G4double Coulomb = (3./5.)*(elm_coupling/G4StatMFParameters::Getr0())*
	(1.0 - 1.0/std::pow(1.0+G4StatMFParameters::GetKappaCoulomb(),1./3.));
									
    return _Energy  = -G4NucleiPropertiesTable::GetBindingEnergy(1,theA) + 
	Coulomb * theZARatio * theZARatio * std::pow(static_cast<G4double>(theA),5./3.) +
	(3./2.) * T;
							
}


G4double G4StatMFMacroTriNucleon::CalcEntropy(const G4double T, const G4double FreeVol)
{
    const G4double ThermalWaveLenght = 16.15*fermi/std::sqrt(T);
    const G4double lambda3 = ThermalWaveLenght*ThermalWaveLenght*ThermalWaveLenght;

    G4double Entropy = 0.0;
    if (_MeanMultiplicity > 0.0)
	Entropy = _MeanMultiplicity*(5./2.+
				     std::log(4.0*static_cast<G4double>(theA)*
					 std::sqrt(static_cast<G4double>(theA))*FreeVol/(lambda3*_MeanMultiplicity)));
								
								
    return Entropy;
}
