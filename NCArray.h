
// -*- C++ -*-

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples 
// Test<type>.c,h files.
//
// ReZa 1/12/95

/* $Log: NCArray.h,v $
/* Revision 1.1  1999/07/28 00:22:42  jimg
/* Added
/*
/* Revision 1.5  1999/05/07 23:45:31  jimg
/* String --> string fixes
/*
/* Revision 1.4  1996/09/17 17:06:17  jimg
/* Merge the release-2-0 tagged files (which were off on a branch) back into
/* the trunk revision.
/*
 * Revision 1.3.4.2  1996/07/10 21:43:55  jimg
 * Changes for version 2.06. These fixed lingering problems from the migration
 * from version 1.x to version 2.x.
 * Removed some (but not all) warning generated with gcc's -Wall option.
 *
 * Revision 1.3.4.1  1996/06/25 22:04:19  jimg
 * Version 2.0 from Reza.
 *
 * Revision 1.3  1995/03/21  20:58:16  jimg
 * Resolved conflicts between my (jhrg) files and those checked in by Reza.
 *
 * Revision 1.2  1995/03/16  16:56:26  reza
 * Updated for the new DAP. All the read_val mfunc. and their memory management
 * are now moved to their parent class.
 * Data transfers are now in binary while DAS and DDS are still sent in ASCII.
 *
 * Revision 1.1  1995/02/10  04:57:15  reza
 * Added read and read_val functions.
 */

#ifndef _NCArray_h
#define _NCArray_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "Array.h"
extern Array * NewArray(const string &n = "", BaseType *v = 0);

class NCArray: public Array {
public:
    NCArray(const string &n = "", BaseType *v = 0);
    virtual ~NCArray();

    virtual BaseType *ptr_duplicate();

    virtual bool read(const string &dataset, int &error);
    long format_constraint(long *cor, long *step, long *edg, bool *has_stride);

//    virtual bool read(String dataset, String var_name, String constraint);
//    virtual bool read_val(void *stuff);
};

#endif


