
// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// $Log: NCUInt32.cc,v $
// Revision 1.3  1999/11/05 05:15:05  jimg
// Result of merge woth 3-1-0
//
// Revision 1.1.2.1  1999/10/29 05:05:21  jimg
// Reza's fixes plus the configure & Makefile update
//
// Revision 1.2  1999/10/21 13:19:06  reza
// IMAP and other bug fixed for version3.
//
// Revision 1.1  1999/07/28 00:22:45  jimg
// Added
//
// Revision 1.3.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.3  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.2  1998/08/06 16:33:26  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.1  1997/01/24 20:10:35  jimg
// Added
//

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCUInt32.cc,v 1.3 1999/11/05 05:15:05 jimg Exp $"};

#ifdef __GNUG__
#pragma implementation
#endif

#include <assert.h>

#include "Dnetcdf.h"
#include "NCUInt32.h"

UInt32 *
NewUInt32(const string &n)
{
    return new NCUInt32(n);
}

NCUInt32::NCUInt32(const string &n) : UInt32(n)
{
}

BaseType *
NCUInt32::ptr_duplicate(){

    return new NCUInt32(*this);
}

bool
NCUInt32::read(const string &dataset, int &error)
{

    int varid;                  /* variable Id */
    nc_type datatype;           /* variable data type */
    long cor[MAX_NC_DIMS];      /* corner coordinates */
    int num_dim;                /* number of dim. in variable */
    dods_uint32 uintg32;
    int id;

    if (read_p()) // nothing to do
        return false;

    int ncid = lncopen(dataset.c_str(), NC_NOWRITE); /* netCDF id */

    if (ncid == -1) { 
        cerr << "ncopen failed on " << dataset<< endl;
	error = 1;
        return false;
    }
 
    varid = lncvarid(ncid, name().c_str());

    (void)lncvarinq(ncid,varid,(char *)0,&datatype,&num_dim,(int *)0,(int *)0);

    for (id = 0; id <= num_dim; id++) 
      cor[id] = 0;

    if (datatype == NC_LONG){
	nclong lng;

	(void) lncvarget1 (ncid, varid, cor, &lng);
	set_read_p(true);

	uintg32 = (dods_uint32) lng;
	val2buf( &uintg32 );

	(void) lncclose(ncid);
	return true;
    }

    error = 1;
    return false;
}


