
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

/* $Log: NCStr.h,v $
/* Revision 1.1  1999/07/28 00:22:44  jimg
/* Added
/*
/* Revision 1.4  1999/05/07 23:45:32  jimg
/* String --> string fixes
/*
/* Revision 1.3  1996/09/17 17:06:41  jimg
/* Merge the release-2-0 tagged files (which were off on a branch) back into
/* the trunk revision.
/*
 * Revision 1.2.4.2  1996/07/10 21:44:24  jimg
 * Changes for version 2.06. These fixed lingering problems from the migration
 * from version 1.x to version 2.x.
 * Removed some (but not all) warning generated with gcc's -Wall option.
 *
 * Revision 1.2.4.1  1996/06/25 22:04:44  jimg
 * Version 2.0 from Reza.
 *
 * Revision 1.2  1995/03/16  16:56:44  reza
 * Updated for the new DAP. All the read_val mfunc. and their memory management
 * are now moved to their parent class.
 * Data transfers are now in binary while DAS and DDS are still sent in ASCII.
 *
 * Revision 1.1  1995/02/10  04:57:50  reza
 * Added read and read_val functions.
 *
*/

#ifndef _NCStr_h
#define _NCStr_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include <limits.h>

#include "Str.h"
extern Str * NewStr(const string &n = "");

class NCStr: public Str {
public:
    NCStr(const string &n = "");
    virtual ~NCStr() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset, int &error);
};



#endif

