
// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCUInt32.cc,v 1.7 2004/09/08 22:08:22 jimg Exp $"};

#ifdef __GNUG__
//#pragma implementation
#endif

#include "InternalErr.h"
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

nc_type
NCUInt32::get_nc_type() throw(InternalErr)
{
    return NC_LONG;
}

bool
NCUInt32::read(const string &dataset)
{
  int varid;                  /* variable Id */
  nc_type datatype;           /* variable data type */
  size_t cor[MAX_NC_DIMS];      /* corner coordinates */
  int num_dim;                /* number of dim. in variable */
  dods_uint32 uintg32;
  int id;

  if (read_p()) // nothing to do
    return false;

  int ncid, errstat;
 
  errstat = lnc_open(dataset.c_str(), NC_NOWRITE, &ncid); /* netCDF id */
  if (errstat != NC_NOERR)
    throw Error(errstat, "Could not open the dataset's file.");
 
  errstat = lnc_inq_varid(ncid, name().c_str(), &varid);
  if (errstat != NC_NOERR)
    throw Error(errstat, "Could not get variable ID during read.");

  errstat = lnc_inq_var(ncid, varid, (char *)0, &datatype, &num_dim, (int *)0, 
			(int *)0);
  if (errstat != NC_NOERR)
    throw Error(errstat, 
		string("Could not read information about the variable `") 
		+ name() + string("'."));

  for (id = 0; id <= num_dim; id++) 
    cor[id] = 0;

  if (datatype == NC_LONG)
    {
      nclong lng;
      //      long lng;
      errstat = lnc_get_var1(ncid, varid, cor, &lng);
	if (errstat != NC_NOERR)
	throw Error(errstat, 
		    string("Could not read the variable `") + name() 
		    + string("'."));

      set_read_p(true);

      uintg32 = (dods_uint32) lng;
      val2buf( &uintg32 );

      if (lnc_close(ncid) != NC_NOERR)
	throw InternalErr(__FILE__, __LINE__, 
			  "Could not close the dataset!");
    }
  else
    throw InternalErr(__FILE__, __LINE__,
		      "Entered NCUInt32::read() with non-long variable!");

  return false;
}

// $Log: NCUInt32.cc,v $
// Revision 1.7  2004/09/08 22:08:22  jimg
// More Massive changes: Code moved from the files that clone the netCDF
// function calls into NCConnect, NCAccess or nc_util.cc. Much of the
// translation functions are now methods. The netCDF type classes now
// inherit from NCAccess in addition to the DAP type classes.
//
// Revision 1.6  2003/12/08 18:06:37  edavis
// Merge release-3-4 into trunk
//
// Revision 1.5  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.4.8.2  2003/06/07 22:02:32  reza
// Fixed char vs. byte and long vs. nclong error checks.
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
