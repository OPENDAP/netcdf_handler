
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

/* $Log: NCUrl.h,v $
/* Revision 1.1  1999/07/28 00:22:45  jimg
/* Added
/*
/* Revision 1.4  1999/05/07 23:45:33  jimg
/* String --> string fixes
/*
/* Revision 1.3  1996/09/17 17:06:47  jimg
/* Merge the release-2-0 tagged files (which were off on a branch) back into
/* the trunk revision.
/*
 * Revision 1.2.4.2  1996/07/10 21:44:30  jimg
 * Changes for version 2.06. These fixed lingering problems from the migration
 * from version 1.x to version 2.x.
 * Removed some (but not all) warning generated with gcc's -Wall option.
 *
 * Revision 1.2.4.1  1996/06/25 22:04:50  jimg
 * Version 2.0 from Reza.
 *
 * Revision 1.2  1995/03/16  16:56:49  reza
 * Updated for the new DAP. All the read_val mfunc. and their memory management
 * are now moved to their parent class.
 * Data transfers are now in binary while DAS and DDS are still sent in ASCII.
 *
 * Revision 1.1  1995/02/10  04:58:01  reza
 * Added read and read_val functions.
 *
*/

#ifndef _NCUrl_h
#define _NCUrl_h 1

#ifdef __GNUG__
#pragma interface
#endif

#include "Url.h"
extern Url * NewUrl(const string &n = "");

class NCUrl: public Url {
public:
    NCUrl(const string &n = "");
    virtual ~NCUrl() {}

    virtual BaseType *ptr_duplicate();
    
    virtual bool read(const string &dataset, int &error);
};



#endif

