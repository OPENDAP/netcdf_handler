
// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      Reza       Reza Nekovei (rnekovei@ieee.org)

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCUInt16.cc,v 1.5 2003/09/25 23:09:36 jimg Exp $"};

#ifdef __GNUG__
#pragma implementation
#endif

#include "InternalErr.h"
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
NCUInt16::read(const string &dataset)
{
  int varid;                  /* variable Id */
  nc_type datatype;           /* variable data type */
  size_t cor[MAX_NC_DIMS];      /* corner coordinates */
  int num_dim;                /* number of dim. in variable */
  dods_uint16 uintg16;
  int id;

  if (read_p()) // nothing to do
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

  if (datatype == NC_SHORT)
    {
      short sht;

      errstat = lnc_get_var1_short(ncid, varid, cor, &sht);
      if(errstat != NC_NOERR)
	throw Error(errstat, string("Could not read the variable `") + name() 
		    + string("'."));

      set_read_p(true);

      uintg16 = (dods_uint16) sht;
      val2buf( &uintg16 );

      if (lnc_close(ncid) != NC_NOERR)
	throw InternalErr(__FILE__, __LINE__, 
			  "Could not close the dataset!");
    }
  else 
    throw InternalErr(__FILE__, __LINE__,
		      "Entered NCUInt16::read() with non-short variable!");

  return false;
}

// $Log: NCUInt16.cc,v $
// Revision 1.5  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.4.8.2  2003/06/07 22:15:43  reza
// Fixed a typo in checking the netcdf return code.
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
