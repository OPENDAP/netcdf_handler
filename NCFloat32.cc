
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
// ReZa 3/26/99

// $Log: NCFloat32.cc,v $
// Revision 1.1  1999/07/28 00:22:43  jimg
// Added
//
// Revision 1.3.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.3  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.2  1999/04/09 17:11:56  jimg
// Fixed comments and copyright statement
//
// Revision 1.1  1999/03/30 21:12:06  reza
// Added the new types
//
//
//

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCFloat32.cc,v 1.1 1999/07/28 00:22:43 jimg Exp $"};

#ifdef __GNUG__
#pragma implementation
#endif

#include <assert.h>

#include "NCFloat32.h"
#include "Dnetcdf.h"

Float32 *
NewFloat32(const string &n)
{
    return new NCFloat32(n);
}

NCFloat32::NCFloat32(const string &n) : Float32(n)
{
}

BaseType *
NCFloat32::ptr_duplicate()
{
    return new NCFloat32(*this); // Copy ctor calls duplicate to do the work
}
 
bool
NCFloat32::read(const string &dataset, int &error)
{

    int varid;                  /* variable Id */
    nc_type datatype;           /* variable data type */
    long cor[MAX_NC_DIMS];      /* corner coordinates */
    int num_dim;                /* number of dim. in variable */
    long nels = -1;              /* number of elements in buffer */
    dods_float32 flt32;
    int id;

    if (read_p()) // nothing to do here
        return false;

    int ncid = lncopen(dataset.c_str(), NC_NOWRITE); /* netCDF id */

    if (ncid == -1) { 
        cerr << "ncopen failed on " << dataset<< endl;
	error = 1;
        return false;
    }
 
    varid = lncvarid(ncid, name().c_str());

    (void)lncvarinq(ncid,varid,(char *)0,&datatype,&num_dim,(int *)0,(int *)0);

    if(nels == -1){  // No point coordinate, get the first element 
	for (id = 0; id < num_dim; id++) 
	    cor[id] = 0;
    }

    if (datatype == NC_FLOAT) {
	float flt;

	(void) lncvarget1 (ncid, varid, cor, &flt);
	set_read_p(true);

	flt32 = (dods_float32) flt;
	val2buf( &flt32 );

	(void) lncclose(ncid);  
	return false;
    }


    error = 1;
    return false;
}




