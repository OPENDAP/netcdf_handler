
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

static char rcsid[] not_used ={"$Id: NCInt32.cc,v 1.8 2004/10/22 21:51:34 jimg Exp $"};

#ifdef __GNUG__
//#pragma implementation
#endif

#include "InternalErr.h"

#include "Dnetcdf.h"
#include "nc_util.h"
#include "NCSequence.h"
#include "NCInt32.h"

Int32 *
NewInt32(const string &n)
{
    return new NCInt32(n);
}

NCInt32::NCInt32(const string &n) : Int32(n)
{
}

BaseType *
NCInt32::ptr_duplicate(){

    return new NCInt32(*this);
}

nc_type
NCInt32::get_nc_type() throw(InternalErr)
{
    return NC_LONG;
}

void
NCInt32::extract_values(void *values, int outtype) throw(Error)
{    
    int nels = 1;               // default value (for scalars)
    NCSequence *ncq = 0;
    
    // If this (apparently) scalar variable is part of a Sequence, then
    // The netCDF library must think it's an Array. Get the number of 
    // elements (which is the number of rows in the returned Sequence).
    if (get_parent()->type() == dods_sequence_c) {
        ncq = dynamic_cast<NCSequence*>(get_parent());
        nels = ncq->number_of_rows();
    }
    
    // Allocate storage for the values
    dods_int32 *tmpbufin = new dods_int32[nels];
    int bytes = 0;

    if (ncq) {
        dods_int32 *tptr = tmpbufin;
        for (int i = 0; i < nels; ++i) {
            bytes += ncq->var_value(i, name())->buf2val((void **)&tptr);
            ++tptr;
        }
    }
    else {
        bytes = buf2val((void **)&tmpbufin);
    }
    
    if (bytes == 0)
        throw Error(-1, "Could not read any data from remote server.");

    // Get the netCDF type code for this variable.
    nc_type typep = dynamic_cast<NCAccess*>(this)->get_nc_type();

    int rcode = convert_nc_type(typep, outtype, nels, (void*)tmpbufin, values);
    if (rcode != NC_NOERR)
        throw Error(rcode,
            "Error copying values between internal buffers [NCAccess::extract_values()]");
}

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

  if (datatype == NC_LONG)
    {
      nclong lng;
      //      long lng;
      errstat = lnc_get_var1(ncid, varid, cor, &lng);
      //      errstat = lnc_get_var1_long (ncid, varid, cor, &lng);

      if (errstat != NC_NOERR)
	throw Error(errstat, string("Could not read the variable `") + name() 
		    + string("'."));

      set_read_p(true);

      intg32 = (dods_int32) lng;
      val2buf( &intg32 );

      if (lnc_close(ncid) != NC_NOERR)
	throw InternalErr(__FILE__, __LINE__, 
			  "Could not close the dataset!");
    }
  else 
    throw InternalErr(__FILE__, __LINE__,
		      "Entered NCInt32::read() with non-long variable!");

  return false;
}

// $Log: NCInt32.cc,v $
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
