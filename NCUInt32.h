
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1994-1996
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      reza            Reza Nekovei (reza@intcomm.net)

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples 
// Test<type>.c,h files.
//
// ReZa 1/12/95

/* 
 * $Log: NCUInt32.h,v $
 * Revision 1.1  1999/07/28 00:22:45  jimg
 * Added
 *
 * Revision 1.2  1999/05/07 23:45:32  jimg
 * String --> string fixes
 *
 * Revision 1.1  1997/01/24 20:10:36  jimg
 * Added
 *
 */

#ifndef _NCUInt32_h
#define _NCUInt32_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "UInt32.h"
extern UInt32 * NewUInt32(const string &n = "");

class NCUInt32: public UInt32 {
public:
    NCUInt32(const string &n = "");
    virtual ~NCUInt32() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset, int &error);
};

#endif

