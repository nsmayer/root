/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooRefCountList.cc,v 1.2 2002/09/17 06:39:34 verkerke Exp $
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2004, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

// -- CLASS DESCRIPTION [AUX] --
// A RooRefCountList is a RooLinkedList that keeps a reference counter
// with each added node. Multiple Add()s of the same object will increase
// the counter instead of adding multiple copies. Remove() decrements the 
// reference count until zero, when the object is actually removed.

#include "RooFitCore/RooRefCountList.hh"

#include <iostream.h>
#include <stdlib.h>

ClassImp(RooRefCountList)
  ;


void RooRefCountList::Add(TObject* obj, Int_t count) 
{
  // Check if we already have it
  RooLinkedListElem* link = findLink(obj) ;
  if (!link) {
    // Add to list with reference count 
    RooLinkedList::Add(obj, count) ;
    //cout << "RooRefCountList::AddLast(" << obj << ") adding object" << endl ;
  } else {
    while(count--) link->incRefCount() ;    
    //cout << "RooRefCountList::AddLast(" << obj << ") incremented reference count to " << link->refCount() << endl ;
  }

}


Bool_t RooRefCountList::Remove(TObject* obj) 
{
  RooLinkedListElem* link = findLink(obj) ;
  if (!link) {
    return 0 ;
  } else {
    if (link->decRefCount()==0) {
      //cout << "RooRefCountList::AddLast(" << obj << ") removed object" << endl ;
      return RooLinkedList::Remove(obj) ;
    }
    //cout << "RooRefCountList::AddLast(" << obj << ") decremented reference count to " << link->refCount() << endl ;
  }
  return 0 ;
}


Bool_t RooRefCountList::RemoveAll(TObject* obj)
{
  return RooLinkedList::Remove(obj) ;
}


Int_t RooRefCountList::refCount(TObject* obj) 
{
  RooLinkedListElem* link = findLink(obj) ;
  if (!link) {
    return 0 ;
  } else {
    return link->refCount() ;
  }
}
