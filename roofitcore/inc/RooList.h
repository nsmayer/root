/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooList.rdl,v 1.6 2002/09/05 04:33:39 verkerke Exp $
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
#ifndef ROO_LIST
#define ROO_LIST

#include "TList.h"

class RooList : public TList {
public:
  inline RooList() : TList() { }
  virtual ~RooList() {} ;
  TObjOptLink *findLink(const char *name, const char *caller= 0) const;
  Bool_t moveBefore(const char *before, const char *target, const char *caller= 0);
  Bool_t moveAfter(const char *after, const char *target, const char *caller= 0);
protected:  
  ClassDef(RooList,1) // TList with extra support for Option_t associations
};

#endif
