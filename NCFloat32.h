
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

#ifndef __POWERPC__
#ifdef __GNUG__
//#pragma interface
#endif
#endif

#include "Float32.h"
#include "NCAccess.h"

extern Float32 * NewFloat32(const string &n);

class NCFloat32: public Float32, public NCAccess {
public:
    NCFloat32(const string &n = "");
    virtual ~NCFloat32() {}

    virtual BaseType *ptr_duplicate();

    virtual nc_type get_nc_type() throw(InternalErr);

    virtual bool read(const string &dataset);
};

// $Log: NCFloat32.h,v $
// Revision 1.8  2004/11/30 22:11:35  jimg
// I replaced the flatten_*() functions with a flatten() method in
// NCAccess. The default version of this method is in NCAccess and works
// for the atomic types; constructors must provide a specialization.
// Then I removed the code that copied the variables from vectors to
// lists. The translation code in NCConnect was modified to use the
// new method.
//
// Revision 1.7  2004/10/22 21:51:34  jimg
// More massive changes: Translation of Sequences now works so long as the
// Sequence contains only atomic types.
//
// Revision 1.6  2004/09/08 22:08:22  jimg
// More Massive changes: Code moved from the files that clone the netCDF
// function calls into NCConnect, NCAccess or nc_util.cc. Much of the
// translation functions are now methods. The netCDF type classes now
// inherit from NCAccess in addition to the DAP type classes.
//
// Revision 1.5  2003/12/08 18:06:37  edavis
// Merge release-3-4 into trunk
//
// Revision 1.4  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.3.4.1  2003/06/24 11:36:32  rmorris
// Removed #pragma interface directives for the OS X.
//
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

