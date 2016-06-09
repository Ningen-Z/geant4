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
// $Id: G4StatMFMicroManager.hh,v 1.3 2006/06/29 20:24:29 gunter Exp $
// GEANT4 tag $Name: geant4-09-00 $
//
// Hadronic Process: Nuclear De-excitations
// by V. Lara


#ifndef G4StatMFMicroManager_h
#define G4StatMFMicroManager_h 1

#include "G4StatMFMicroPartition.hh"
#include "G4StatMFParameters.hh"
#include "G4StatMFChannel.hh"

#include "G4Fragment.hh"
#include "Randomize.hh"


class G4StatMFMicroManager {

public:

    // G4StatMFMicroManager class must be initialized with a G4Fragment, multiplicity,
    // free internal energy and the entropy of the compund nucleus.
    G4StatMFMicroManager(const G4Fragment & theFragment, const G4int multiplicity,
			 const G4double FreeIntE, const G4double SCompNuc);

    // destructor
    ~G4StatMFMicroManager();

private:
    // default constructor
    G4StatMFMicroManager() {};


    // copy constructor
    G4StatMFMicroManager(const G4StatMFMicroManager &right);


    // operators
    G4StatMFMicroManager & operator=(const G4StatMFMicroManager & right);

public:
    G4bool operator==(const G4StatMFMicroManager & right) const;
    G4bool operator!=(const G4StatMFMicroManager & right) const;


public:

    // Choice of fragment atomic numbers and charges.
    G4StatMFChannel * ChooseChannel(const G4double A0, const G4double Z0, const G4double MeanT);
	
    G4double GetProbability(void) const {return _WW;}

    void Normalize(const G4double Norm);
	
    G4double GetMeanMultiplicity(void) const {return _MeanMultiplicity; }

    G4double GetMeanTemperature(void) const {return _MeanTemperature; }

    G4double GetMeanEntropy(void) const {return _MeanEntropy; }

private:

    // Initailization method
    void Initialize(const G4Fragment & theFragment, const G4int m,
		    const G4double FreeIntE, const G4double SCompNuc);

    G4bool MakePartition(const G4int k, G4int * ANumbers); 
								


	
// Data members
private:


    // Partitions vector
    std::vector<G4StatMFMicroPartition*> _Partition;
	

    // Statistical weight
    G4double _WW;

    G4double _Normalization;
	
    G4double _MeanMultiplicity;

    G4double _MeanTemperature;
	
    G4double _MeanEntropy;

  struct DeleteFragment 
  {
    template<typename T>
    void operator()(const T* ptr) const
    {
      delete ptr;
    }
  };
  
};

#endif






