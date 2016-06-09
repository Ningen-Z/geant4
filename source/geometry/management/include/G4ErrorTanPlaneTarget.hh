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
// $Id: G4ErrorTanPlaneTarget.hh,v 1.1 2007/05/16 12:50:52 gcosmo Exp $
// GEANT4 tag $Name: geant4-09-00 $
//
//
// ------------------------------------------------------------
//      GEANT 4 class header file 
// ------------------------------------------------------------
//
// Class Description:
//
// Base class for G4ErrorTarget classes for which a tangent plane is defined.

// History:
// - Created. P. Arce, September 2004
// --------------------------------------------------------------------

#ifndef G4ErrorTanPlaneTarget_hh
#define G4ErrorTanPlaneTarget_hh

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4ErrorTarget.hh"
#include "G4Plane3D.hh"

class G4ErrorTanPlaneTarget : public G4ErrorTarget
{

  public:  // with description

    G4ErrorTanPlaneTarget();
    virtual ~G4ErrorTanPlaneTarget();

    virtual G4Plane3D GetTangentPlane( const G4ThreeVector& point ) const = 0;
      // Get tangent plane at point

    virtual void Dump( const G4String& msg ) const = 0;
      // Dump surface

};

#endif
