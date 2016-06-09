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
// File name:     RadmonVDetectorEntitiesConstructorsFactory.hh
// Creation date: Sep 2005
// Main author:   Riccardo Capra <capra@ge.infn.it>
//
// Id:            $Id: RadmonVDetectorEntitiesConstructorsFactory.hh,v 1.4 2006/06/29 16:12:52 gunter Exp $
// Tag:           $Name: geant4-09-00 $
//
// Description:   Abstract class of a factory of detector-entity constructor
//

#ifndef   RADMONVDETECTORENTITIESCONSTRUCTORSFACTORY_HH
 #define  RADMONVDETECTORENTITIESCONSTRUCTORSFACTORY_HH

 // Forward declaration
 class RadmonVDetectorEntityConstructor;
 class G4String;

 class RadmonVDetectorEntitiesConstructorsFactory
 {
  public:
   inline virtual                              ~RadmonVDetectorEntitiesConstructorsFactory();

   virtual RadmonVDetectorEntityConstructor *   CreateEntityConstructor(const G4String & entityName) = 0;

  protected:
   inline                                       RadmonVDetectorEntitiesConstructorsFactory();

  private:
  // Hidden constructors and operators
                                                RadmonVDetectorEntitiesConstructorsFactory(const RadmonVDetectorEntitiesConstructorsFactory & copy);
   RadmonVDetectorEntitiesConstructorsFactory & operator=(const RadmonVDetectorEntitiesConstructorsFactory & copy);
 };
 
 // Inline implementations
 #include "RadmonVDetectorEntitiesConstructorsFactory.icc"
#endif /* RADMONVDETECTORENTITIESCONSTRUCTORSFACTORY_HH */
