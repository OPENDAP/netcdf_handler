
// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCUInt32.cc,v 1.4 2000/10/06 01:22:02 jimg Exp $"};

#ifdef __GNUG__
#pragma implementation
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

bool
NCUInt32::read(const string &dataset)
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

  if (ncid == -1)
    throw Error(no_such_file, "Could not open the dataset's file.");
 
  varid = lncvarid(ncid, name().c_str());

  if (lncvarinq(ncid, varid, (char *)0, &datatype, &num_dim, (int *)0, 
		(int *)0) == -1)
    throw Error(unknown_error, 
		string("Could not read information about the variable `") 
		+ name() + string("'."));

  for (id = 0; id <= num_dim; id++) 
    cor[id] = 0;

  if (datatype == NC_LONG)
    {
      nclong lng;

      if (lncvarget1 (ncid, varid, cor, &lng) == -1)
	throw Error(no_such_variable, 
		    string("Could not read the variable `") + name() 
		    + string("'."));

      set_read_p(true);

      uintg32 = (dods_uint32) lng;
      val2buf( &uintg32 );

      if (lncclose(ncid) == -1)
	throw InternalErr(__FILE__, __LINE__, 
			  "Could not close the dataset!");
    }
  else
    throw InternalErr(__FILE__, __LINE__,
		      "Entered NCByte::read() with non-byte variable!");

  return false;
}

// $Log: NCUInt32.cc,v $
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
