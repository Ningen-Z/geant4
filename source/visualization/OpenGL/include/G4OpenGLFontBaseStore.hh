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
// $Id: G4OpenGLFontBaseStore.hh,v 1.3 2006/06/29 21:17:38 gunter Exp $
// GEANT4 tag $Name: geant4-09-00 $
//
// J.Allison  Apr 2005.

// Class Description:
// Facilitates sharing of font bases between scene handler and viewers.

#ifndef G4OPENGLFONTBASESTORE_HH
#define G4OPENGLFONTBASESTORE_HH

#include "globals.hh"
#include <map>
#include <vector>

class G4VViewer;

class G4OpenGLFontBaseStore {
public:
  static void AddFontBase(G4VViewer*, G4int fontBase,
			  G4double size, const G4String& fontName);
  static G4int GetFontBase(G4VViewer*, G4double size);
private:
  struct FontInfo {
    FontInfo():
      fFontName(""), fSize(0), fFontBase(-1) {}
    FontInfo(const G4String& fontName, G4double size, G4int fontBase):
      fFontName(fontName), fSize(size), fFontBase(fontBase) {}
    G4String fFontName;
    G4double fSize;  // In terms of G4VMarker Screen Size.
    G4int fFontBase;
  };
  static std::map<G4VViewer*,std::vector<FontInfo> > fFontBaseMap;
};

#endif
