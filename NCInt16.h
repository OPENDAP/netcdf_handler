
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1994-1996
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      reza            Reza Nekovei (rnekovei@ieee.org)

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples 
// Test<type>.c,h files.
//
// ReZa 3/26/99

#ifndef _ncint16_h
#define _ncint16_h 1

#ifndef __POWERPC__
#ifdef __GNUG__
#pragma interface
#endif
#endif

#include "Int16.h"
extern Int16 * NewInt16(const string &n);

class NCInt16: public Int16 {
public:
    NCInt16(const string &n = "");
    virtual ~NCInt16() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset);
};

/* 
 * $Log: NCInt16.h,v $
 * Revision 1.4  2003/09/25 23:09:36  jimg
 * Meerged from 3.4.1.
 *
 * Revision 1.3.4.1  2003/06/24 11:36:32  rmorris
 * Removed #pragma interface directives for the OS X.
 *
 * Revision 1.3  2002/05/03 00:01:52  jimg
 * Merged with release-3-2-7.
 *
 * Revision 1.2.4.1  2001/12/26 03:32:24  rmorris
 * Removed redundant default args.  VC++ only allows them to be specified
 * a single time.
 *
 * Revision 1.2  2000/10/06 01:22:02  jimg
 * Moved the CVS Log entries to the ends of files.
 * Modified the read() methods to match the new definition in the dap library.
 * Added exception handlers in various places to catch exceptions thrown
 * by the dap library.
 *
 * Revision 1.1  1999/07/28 00:22:43  jimg
 * Added
 *
 * Revision 1.2  1999/05/07 23:45:32  jimg
 * String --> string fixes
 *
 * Revision 1.1  1999/03/30 21:12:07  reza
 * Added the new types
 */

#endif

