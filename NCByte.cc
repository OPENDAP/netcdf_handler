
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

static char rcsid[] not_used ={"$Id: NCByte.cc,v 1.12 2005/03/31 00:04:51 jimg Exp $"};

#ifdef __GNUG__
//#pragma implementation
#endif

#include "InternalErr.h"

#include "NCByte.h"
#include "NCSequence.h"
#include "Dnetcdf.h"
#include "nc_util.h"

// This `helper function' creates a pointer to the a NCByte and returns
// that pointer. It takes the same arguments as the class's ctor. If any of
// the variable classes are subclassed (e.g., to make a new Byte like
// HDFByte) then the corresponding function here, and in the other class
// definition files, needs to be changed so that it creates an instnace of
// the new (sub)class. Continuing the earlier example, that would mean that
// NewByte() would return a HDFByte, not a Byte.
//
// It is important that these function's names and return types do not change
// - they are called by the parser code (for the dds, at least) so if their
// names changes, that will break.
//
// The declarations for these fuctions (in util.h) should *not* need
// changing. 

#if 0
Byte *
NewByte(const string &n)
{
    return new NCByte(n);
}
#endif

void 
NCByte::m_duplicate(const NCByte &bt)
{
    dynamic_cast<NCAccess&>(*this).clone(dynamic_cast<const NCAccess&>(bt));
}

NCByte::NCByte(const string &n) : Byte(n)
{
}

NCByte::NCByte(const NCByte &rhs) : Byte(rhs)
{
    m_duplicate(rhs);
}

NCByte::~NCByte()
{
}

NCByte &
NCByte::operator=(const NCByte &rhs)
{
    if (this == &rhs)
        return *this;

    dynamic_cast<NCByte&>(*this) = rhs;

    m_duplicate(rhs);

    return *this;
}

BaseType *
NCByte::ptr_duplicate()
{
    return new NCByte(*this);
}

nc_type
NCByte::get_nc_type() throw(InternalErr)
{
    return NC_BYTE;
}

bool
NCByte::read(const string &dataset)
{
    int varid;                  /* variable Id */
    nc_type datatype;           /* variable data type */
    size_t cor[MAX_NC_DIMS];      /* corner coordinates */
    int num_dim;                /* number of dim. in variable */
    int id;

    if (read_p()) // already done
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
      throw Error(errstat,string("Could not read information about the variable `") 
		  + name() + string("'."));

    for (id = 0; id <= num_dim; id++) 
      cor[id] = 0;

    if (datatype == NC_BYTE) {

      dods_byte Dbyte;
      errstat = lnc_get_var1(ncid, varid, cor, &Dbyte);
      if (errstat != NC_NOERR)
	throw Error(errstat, string("Could not read the variable `") + name() 
		    + string("'."));

	set_read_p(true);
          
	val2buf( &Dbyte );

	if (lnc_close(ncid) != NC_NOERR)
	  throw InternalErr(__FILE__, __LINE__, 
			    "Could not close the dataset!");
    }
    else
      throw InternalErr(__FILE__, __LINE__,
			"Entered NCByte::read() with non-byte variable!");

    return false;
}

// $Log: NCByte.cc,v $
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
// Revision 1.7  2004/09/08 22:08:21  jimg
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
// Revision 1.1.2.2  1999/10/29 05:05:21  jimg
// Reza's fixes plus the configure & Makefile update
//
// Revision 1.2  1999/10/21 13:19:06  reza
// IMAP and other bug fixed for version3.
//
// Revision 1.1.2.1  1999/10/15 19:50:56  jimg
// Changed return values and conditions for NC API entry points
//
// Revision 1.1  1999/07/28 00:22:42  jimg
// Added
//
// Revision 1.7.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.7  1999/05/07 23:45:31  jimg
// String --> string fixes
//
// Revision 1.6  1998/08/06 16:33:21  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.5  1996/09/17 17:06:19  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.4.4.3  1996/09/17 00:26:19  jimg
// Merged changes from a side branch which contained various changes from
// Reza and Charles.
// Removed ncdump and netexec since ncdump is now in its own directory and
// netexec is no longer used.
//
// Revision 1.4.4.2  1996/07/10 21:43:57  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.4.4.1  1996/06/25 22:04:20  jimg
// Version 2.0 from Reza.
//
// Revision 1.4  1995/07/09  21:33:40  jimg
// Added copyright notice.
//
// Revision 1.3  1995/06/23  13:47:41  reza
// Added scalar read and the constraint "point".
//
// Revision 1.2  1995/03/16  16:56:27  reza
// Updated for the new DAP. All the read_val mfunc. and their memory management
// are now moved to their parent class.
// Data transfers are now in binary while DAS and DDS are still sent in ASCII.
//
// Revision 1.1  1995/02/10  04:57:17  reza
// Added read and read_val functions.
