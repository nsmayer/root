/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooArgSet.rdl,v 1.36 2002/09/05 04:33:14 verkerke Exp $
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
#ifndef ROO_ARG_SET
#define ROO_ARG_SET

#include "THashList.h"
#include "TString.h"
#include "TClass.h"
#include "RooFitCore/RooAbsCollection.hh"
#include "RooFitCore/RooErrorHandler.hh"

class RooArgList ;

class RooArgSet : public RooAbsCollection {
public:

  // Constructors, assignment etc.
  RooArgSet();
  RooArgSet(const RooArgList& list) ;
  explicit RooArgSet(const TCollection& tcoll, const char* name="") ;
  explicit RooArgSet(const char *name);
  RooArgSet(const RooAbsArg& var1, 
	    const char *name="");
  RooArgSet(const RooAbsArg& var1, const RooAbsArg& var2, 
	    const char *name="");
  RooArgSet(const RooAbsArg& var1, const RooAbsArg& var2,
	    const RooAbsArg& var3, 
	    const char *name="");
  RooArgSet(const RooAbsArg& var1, const RooAbsArg& var2,
	    const RooAbsArg& var3, const RooAbsArg& var4, 
	    const char *name="");
  RooArgSet(const RooAbsArg& var1, const RooAbsArg& var2,
	    const RooAbsArg& var3, const RooAbsArg& var4, 
	    const RooAbsArg& var5, 
	    const char *name="");
  RooArgSet(const RooAbsArg& var1, const RooAbsArg& var2,
	    const RooAbsArg& var3, const RooAbsArg& var4, 
	    const RooAbsArg& var5, const RooAbsArg& var6, 
	    const char *name="");
  RooArgSet(const RooAbsArg& var1, const RooAbsArg& var2,
            const RooAbsArg& var3, const RooAbsArg& var4, 
	    const RooAbsArg& var5, const RooAbsArg& var6, 
	    const RooAbsArg& var7, 
	    const char *name="");
  RooArgSet(const RooAbsArg& var1, const RooAbsArg& var2,
            const RooAbsArg& var3, const RooAbsArg& var4, 
	    const RooAbsArg& var5, const RooAbsArg& var6, 
	    const RooAbsArg& var7, const RooAbsArg& var8, 
	    const char *name="");
  RooArgSet(const RooAbsArg& var1, const RooAbsArg& var2,
            const RooAbsArg& var3, const RooAbsArg& var4, 
	    const RooAbsArg& var5, const RooAbsArg& var6, 
	    const RooAbsArg& var7, const RooAbsArg& var8, 
	    const RooAbsArg& var9, const char *name="");

  virtual ~RooArgSet();
  // Create a copy of an existing list. New variables cannot be added
  // to a copied list. The variables in the copied list are independent
  // of the original variables.
  RooArgSet(const RooArgSet& other, const char *name="");
  virtual TObject* clone(const char* newname) const { return new RooArgSet(*this,newname); }
  virtual TObject* create(const char* newname) const { return new RooArgSet(newname); }
  RooArgSet& operator=(const RooArgSet& other) { RooAbsCollection::operator=(other) ; return *this ;}

  virtual Bool_t add(const RooAbsArg& var, Bool_t silent=kFALSE) ;
  virtual Bool_t add(const RooAbsCollection& list, Bool_t silent=kFALSE) { return RooAbsCollection::add(list,silent) ; }
  virtual Bool_t addOwned(RooAbsArg& var, Bool_t silent=kFALSE);
  virtual Bool_t addOwned(const RooAbsCollection& list, Bool_t silent=kFALSE) { return RooAbsCollection::addOwned(list,silent) ; }
  virtual RooAbsArg *addClone(const RooAbsArg& var, Bool_t silent=kFALSE) ;
  virtual void addClone(const RooAbsCollection& list, Bool_t silent=kFALSE) { RooAbsCollection::addClone(list,silent) ; }

  RooAbsArg& operator[](const char* name) const ;   

  // I/O streaming interface (machine readable)
  virtual Bool_t readFromStream(istream& is, Bool_t compact, Bool_t verbose=kFALSE) {
    return readFromStream(is, compact, 0, 0, verbose) ;
  }
  Bool_t readFromStream(istream& is, Bool_t compact, const char* flagReadAtt, const char* section, Bool_t verbose=kFALSE) ;
  virtual void writeToStream(ostream& os, Bool_t compact, const char* section=0) ;  
  void writeToFile(const char* fileName) ;
  Bool_t readFromFile(const char* fileName, const char* flagReadAtt=0, const char* section=0) ;

protected:

  Bool_t checkForDup(const RooAbsArg& arg, Bool_t silent) const ;

  ClassDef(RooArgSet,1) // Set of RooAbsArg objects
};

#endif
