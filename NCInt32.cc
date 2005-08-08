
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

static char rcsid[] not_used ={"$Id$"};

#include <netcdf.h>
#include "InternalErr.h"

#if 0
#include "Dnetcdf.h"
#include "nc_util.h"
#include "NCSequence.h"
#endif
#include "NCInt32.h"

#if 0
void 
NCInt32::m_duplicate(const NCInt32 &bt)
{
    dynamic_cast<NCAccess&>(*this).clone(dynamic_cast<const NCAccess&>(bt));
}
#endif

NCInt32::NCInt32(const string &n) : Int32(n)
{
}

NCInt32::NCInt32(const NCInt32 &rhs) : Int32(rhs)
{
#if 0
    m_duplicate(rhs);
#endif
}

NCInt32::~NCInt32()
{
}

NCInt32 &
NCInt32::operator=(const NCInt32 &rhs)
{
    if (this == &rhs)
        return *this;

    dynamic_cast<NCInt32&>(*this) = rhs;

#if 0
    m_duplicate(rhs);
#endif

    return *this;
}


BaseType *
NCInt32::ptr_duplicate(){

    return new NCInt32(*this);
}

#if 0
nc_type
NCInt32::get_nc_type() throw(InternalErr)
{
    return NC_LONG;
}
#endif

bool
NCInt32::read(const string &dataset)
{
  int varid;                  /* variable Id */
  nc_type datatype;           /* variable data type */
  size_t cor[MAX_NC_DIMS];      /* corner coordinates */
  int num_dim;                /* number of dim. in variable */
  dods_int32 intg32;
  int id;

  if (read_p()) // nothing to do
    return false;

  int ncid, errstat;

  errstat = nc_open(dataset.c_str(), NC_NOWRITE, &ncid); /* netCDF id */

  if (errstat != NC_NOERR)
    throw Error(errstat, "Could not open the dataset's file.");
 
  errstat = nc_inq_varid(ncid, name().c_str(), &varid);
  if (errstat != NC_NOERR)
    throw Error(errstat, "Could not get variable ID.");

  errstat = nc_inq_var(ncid, varid, (char *)0, &datatype, &num_dim, (int *)0,
			(int *)0);
  if (errstat != NC_NOERR)
    throw Error(errstat, 
		string("Could not read information about the variable `") 
		+ name() + string("'."));

  for (id = 0; id <= num_dim; id++) 
    cor[id] = 0;

  if (datatype == NC_LONG)
    {
      long int lng;
      errstat = nc_get_var1_long(ncid, varid, cor, &lng);

      if (errstat != NC_NOERR)
	throw Error(errstat, string("Could not read the variable `") + name() 
		    + string("'."));

      set_read_p(true);

      intg32 = (dods_int32) lng;
      val2buf( &intg32 );

      if (nc_close(ncid) != NC_NOERR)
	throw InternalErr(__FILE__, __LINE__, 
			  "Could not close the dataset!");
    }
  else 
    throw InternalErr(__FILE__, __LINE__,
		      "Entered NCInt32::read() with non-long variable!");

  return false;
}

// $Log: NCInt32.cc,v $
// Revision 1.14  2005/04/19 23:16:18  jimg
// Removed client side parts; the client library is now in libnc-dap.
//
// Revision 1.13  2005/04/08 17:08:47  jimg
// Removed old 'virtual ctor' functions which have now been replaced by the
// factory class code in libdap++.
//
// Revision 1.12  2005/03/31 00:04:51  jimg
// Modified to use the factory class in libdap++ 3.5.
//
// Revision 1.11  2005/02/17 23:44:13  jimg
// Modifications for processing of command line projections combined
// with the limit stuff and projection info passed in from the API. I also
// consolodated some of the code by moving d_source from various
// classes to NCAccess. This may it so that DODvario() could be simplified
// as could build_constraint() and store_projection() in NCArray.
//
// Revision 1.10  2005/01/26 23:25:51  jimg
// Implemented a fix for Sequence access by row number when talking to a
// 3.4 or earlier server (which contains a bug in is_end_of_rows()).
//
// Revision 1.9  2004/11/30 22:11:35  jimg
// I replaced the flatten_*() functions with a flatten() method in
// NCAccess. The default version of this method is in NCAccess and works
// for the atomic types; constructors must provide a specialization.
// Then I removed the code that copied the variables from vectors to
// lists. The translation code in NCConnect was modified to use the
// new method.
//
// Revision 1.8  2004/10/22 21:51:34  jimg
// More massive changes: Translation of Sequences now works so long as the
// Sequence contains only atomic types.
//
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
// Revision 1.1  1999/07/28 00:22:43  jimg
// Added
//
// Revision 1.8.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.8  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.7  1999/03/30 05:20:56  reza
// Added support for the new data types (Int16, UInt16, and Float32).
//
// Revision 1.6  1998/08/06 16:33:23  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.5  1996/09/17 17:06:32  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.4.4.3  1996/09/17 00:26:26  jimg
// Merged changes from a side branch which contained various changes from
// Reza and Charles.
// Removed ncdump and netexec since ncdump is now in its own directory and
// netexec is no longer used.
//
// Revision 1.4.4.2  1996/07/10 21:44:11  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.4.4.1  1996/06/25 22:04:33  jimg
// Version 2.0 from Reza.
//
// Revision 1.4  1995/07/09  21:33:47  jimg
// Added copyright notice.
//
// Revision 1.3  1995/06/23  13:47:43  reza
// Added scalar read and the constraint "point".
//
// Revision 1.2  1995/03/16  16:56:36  reza
// Updated for the new DAP. All the read_val mfunc. and their memory management
// are now moved to their parent class.
// Data transfers are now in binary while DAS and DDS are still sent in ASCII.
//
// Revision 1.1  1995/02/10  04:57:34  reza
// Added read and read_val functions.
