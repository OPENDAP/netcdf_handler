
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

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCFloat64.cc,v 1.5 2003/09/25 23:09:36 jimg Exp $"};

#ifdef __GNUG__
#pragma implementation
#endif

#include "InternalErr.h"
#include "NCFloat64.h"
#include "Dnetcdf.h"

Float64 *
NewFloat64(const string &n)
{
    return new NCFloat64(n);
}

NCFloat64::NCFloat64(const string &n) : Float64(n)
{
}

BaseType *
NCFloat64::ptr_duplicate()
{
    return new NCFloat64(*this); // Copy ctor calls duplicate to do the work
}
 
bool
NCFloat64::read(const string &dataset)
{

    int varid;                  /* variable Id */
    nc_type datatype;           /* variable data type */
    size_t cor[MAX_NC_DIMS];      /* corner coordinates */
    int num_dim;                /* number of dim. in variable */
    dods_float64 flt64;
    int id;

    if (read_p()) // nothing to do here
        return false;

    int ncid, errstat;
    errstat = lnc_open(dataset.c_str(), NC_NOWRITE, &ncid); /* netCDF id */

    if (errstat != NC_NOERR)
      throw Error(errstat, "Could not open the dataset's file.");

    errstat = lnc_inq_varid(ncid, name().c_str(), &varid);
    if (errstat != NC_NOERR)
      throw Error(errstat, "Could not get variable ID.");

    errstat = lnc_inq_var(ncid, varid, (char *)0, &datatype, &num_dim, (int *)0,
			  (int *)0);
    if (errstat != NC_NOERR)
      throw Error(errstat, 
		  string("Could not read information about the variable `") 
		  + name() + string("'."));

    for (id = 0; id <= num_dim; id++) 
      cor[id] = 0;

    if (datatype == NC_DOUBLE){
	double dbl;

	errstat = lnc_get_var1_double(ncid, varid, cor, &dbl);
	if (errstat != NC_NOERR)
	  throw Error(errstat, 
		      string("Could not read the variable `") + name() 
		      + string("'."));

	set_read_p(true);

	flt64 = (dods_float64) dbl;
	val2buf((void *) &flt64 );

	if (lnc_close(ncid) != NC_NOERR)
	  throw InternalErr(__FILE__, __LINE__, 
			    "Could not close the dataset!");
    }
    else
      throw InternalErr(__FILE__, __LINE__,
			"Entered NCFloat64::read() with non-float64 variable!");

    return false;
}

// $Log: NCFloat64.cc,v $
// Revision 1.5  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.4.8.1  2003/06/06 08:23:41  reza
// Updated the servers to netCDF-3 and fixed error handling between client and server.
//
// Revision 1.4  2000/10/06 01:22:02  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.3  1999/11/05 05:15:05  jimg
// Result of merge woth 3-1-0
//
// Revision 1.1.2.1  1999/10/22 17:33:37  jimg
// Merged Reza's changes from the trunk
//
// Revision 1.2  1999/10/21 13:19:06  reza
// IMAP and other bug fixed for version3.
//
// Revision 1.1  1999/07/28 00:22:43  jimg
// Added
//
// Revision 1.8.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.8  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.7  1999/03/30 05:20:55  reza
// Added support for the new data types (Int16, UInt16, and Float32).
//
// Revision 1.6  1998/08/06 16:33:22  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.5  1996/09/17 17:06:24  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.4.4.3  1996/09/17 00:26:22  jimg
// Merged changes from a side branch which contained various changes from
// Reza and Charles.
// Removed ncdump and netexec since ncdump is now in its own directory and
// netexec is no longer used.
//
// Revision 1.4.4.2  1996/07/10 21:44:02  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.4.4.1  1996/06/25 22:04:25  jimg
// Version 2.0 from Reza.
//
// Revision 1.4  1995/07/09  21:33:43  jimg
// Added copyright notice.
//
// Revision 1.3  1995/06/23  13:47:44  reza
// Added scalar read and the constraint "point".
//
// Revision 1.2  1995/03/16  16:56:30  reza
// Updated for the new DAP. All the read_val mfunc. and their memory management
// are now moved to their parent class.
// Data transfers are now in binary while DAS and DDS are still sent in ASCII.
//
// Revision 1.1  1995/02/10  04:57:22  reza
// Added read and read_val functions.



