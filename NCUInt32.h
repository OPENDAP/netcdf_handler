
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1994-1996
// Please read the full copyright statement in the file COPYRIGHT.  
//
// Authors:
//      reza            Reza Nekovei (reza@intcomm.net)

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples 
// Test<type>.c,h files.
//
// ReZa 1/12/95

#ifndef _ncuint32_h
#define _ncuint32_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "UInt32.h"
extern UInt32 * NewUInt32(const string &n);

class NCUInt32: public UInt32 {
public:
    NCUInt32(const string &n = "");
    virtual ~NCUInt32() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset);
};

/* 
 * $Log: NCUInt32.h,v $
 * Revision 1.3  2002/05/03 00:01:52  jimg
 * Merged with release-3-2-7.
 *
 * Revision 1.2.4.1  2001/12/26 01:56:22  rmorris
 * Reduncant default argument removed.  VC++ doesn't tolerate a default
 * arg value being specified twice (in the .c* and the .h).
 *
 * Revision 1.2  2000/10/06 01:22:03  jimg
 * Moved the CVS Log entries to the ends of files.
 * Modified the read() methods to match the new definition in the dap library.
 * Added exception handlers in various places to catch exceptions thrown
 * by the dap library.
 *
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

#endif

