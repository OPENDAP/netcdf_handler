
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1994-1996
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      reza            Reza Nekovei (rnekovei@ieee.org)

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples 
// Test<type>.c,h files.
//
// ReZa 3/26/99

/* 
 * $Log: NCUInt16.h,v $
 * Revision 1.1  1999/07/28 00:22:44  jimg
 * Added
 *
 * Revision 1.2  1999/05/07 23:45:32  jimg
 * String --> string fixes
 *
 * Revision 1.1  1999/03/30 21:12:07  reza
 * Added the new types
 *
 *
 */

#ifndef _NCUInt16_h
#define _NCUInt16_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "UInt16.h"
extern UInt16 * NewUInt16(const string &n = "");

class NCUInt16: public UInt16 {
public:
    NCUInt16(const string &n = "");
    virtual ~NCUInt16() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset, int &error);
};

#endif

