
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1994-1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      reza            Reza Nekovei (rnekovei@ieee.org)

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples 
// Test<type>.c,h files.
//
// ReZa 3/25/99

#ifndef _ncfloat32_h
#define _ncfloat32_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "Float32.h"
extern Float32 * NewFloat32(const string &n);

class NCFloat32: public Float32 {
public:
    NCFloat32(const string &n = "");
    virtual ~NCFloat32() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset);
};

// $Log: NCFloat32.h,v $
// Revision 1.3  2002/05/03 00:01:52  jimg
// Merged with release-3-2-7.
//
// Revision 1.2.4.1  2001/12/26 03:32:24  rmorris
// Removed redundant default args.  VC++ only allows them to be specified
// a single time.
//
// Revision 1.2  2000/10/06 01:22:02  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.1  1999/07/28 00:22:43  jimg
// Added
//
// Revision 1.3  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.2  1999/04/09 17:11:56  jimg
// Fixed comments and copyright statement
//
// Revision 1.1  1999/03/30 21:12:07  reza
// Added the new types

#endif

