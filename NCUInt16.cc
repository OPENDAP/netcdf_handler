
// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      Reza       Reza Nekovei (rnekovei@ieee.org)

// $Log: NCUInt16.cc,v $
// Revision 1.2  1999/10/21 13:19:06  reza
// IMAP and other bug fixed for version3.
//
// Revision 1.1  1999/07/28 00:22:44  jimg
// Added
//
// Revision 1.2.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.2  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.1  1999/03/30 21:12:07  reza
// Added the new types
//
//

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCUInt16.cc,v 1.2 1999/10/21 13:19:06 reza Exp $"};

#ifdef __GNUG__
#pragma implementation
#endif

#include <assert.h>

#include "Dnetcdf.h"
#include "NCUInt16.h"

UInt16 *
NewUInt16(const string &n)
{
    return new NCUInt16(n);
}

NCUInt16::NCUInt16(const string &n) : UInt16(n)
{
}

BaseType *
NCUInt16::ptr_duplicate(){

    return new NCUInt16(*this);
}

bool
NCUInt16::read(const string &dataset, int &error)
{

    int varid;                  /* variable Id */
    nc_type datatype;           /* variable data type */
    long cor[MAX_NC_DIMS];      /* corner coordinates */
    int num_dim;                /* number of dim. in variable */
    dods_uint16 uintg16;
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


    if (datatype == NC_SHORT){
	short sht;

	(void) lncvarget1 (ncid, varid, cor, &sht);
	set_read_p(true);

	uintg16 = (dods_uint16) sht;
	val2buf( &uintg16 );

	(void) lncclose(ncid);  
	return true;
    }

    error = 1;
    return false;
}


