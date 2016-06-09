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
// $Id: G4VFigureFileMaker.hh,v 1.5 2006/06/29 21:23:51 gunter Exp $
// GEANT4 tag $Name: geant4-09-00 $
//
//

// class description:
//
//  This is an abstract class represents a class which generates a figure file
// converting from RGB 8 bits unsigned integers to a kind of picture format.
//

#ifndef G4VFigureFileMaker_H
#define G4VFigureFileMaker_H 1

#include "globals.hh"

class G4VFigureFileMaker
{
  public:
    G4VFigureFileMaker() {;}
    virtual ~G4VFigureFileMaker() {;}

  public:
    virtual void CreateFigureFile(G4String fileName,
              int nColumn,int nRow,
              unsigned char* colorR, unsigned char* colorG, 
              unsigned char* colorB) = 0;
};

#endif

