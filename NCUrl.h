
// -*- C++ -*-

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

#ifndef _ncurl_h
#define _ncurl_h 1

#ifndef __POWERPC__
#ifdef __GNUG__
//#pragma interface
#endif
#endif

#include "Url.h"
#include "NCAccess.h"

extern Url * NewUrl(const string &n = "");

class NCUrl: public Url, public NCAccess {
public:
    NCUrl(const string &n = "");
    virtual ~NCUrl() {}

    virtual BaseType *ptr_duplicate();
    virtual nc_type get_nc_type() throw(InternalErr);
    virtual void extract_values(void *values, int outtype) throw(Error);
};

/* 
 * $Log: NCUrl.h,v $
 * Revision 1.6  2004/11/30 22:11:35  jimg
 * I replaced the flatten_*() functions with a flatten() method in
 * NCAccess. The default version of this method is in NCAccess and works
 * for the atomic types; constructors must provide a specialization.
 * Then I removed the code that copied the variables from vectors to
 * lists. The translation code in NCConnect was modified to use the
 * new method.
 *
 * Revision 1.5  2004/09/08 22:08:22  jimg
 * More Massive changes: Code moved from the files that clone the netCDF
 * function calls into NCConnect, NCAccess or nc_util.cc. Much of the
 * translation functions are now methods. The netCDF type classes now
 * inherit from NCAccess in addition to the DAP type classes.
 *
 * Revision 1.4  2003/12/08 18:06:37  edavis
 * Merge release-3-4 into trunk
 *
 * Revision 1.3  2003/09/25 23:09:36  jimg
 * Meerged from 3.4.1.
 *
 * Revision 1.2.8.2  2003/09/06 23:33:14  jimg
 * I modified the read() method implementations so that they test the new
 * in_selection property. If it is true, the methods will read values
 * even if the send_p property is not true. This is so that variables used
 * in the selection part of the CE, or as function arguments, will be read.
 * See bug 657.
 *
 * Revision 1.2.8.1  2003/06/24 11:36:32  rmorris
 * Removed #pragma interface directives for the OS X.
 *
 * Revision 1.2  2000/10/06 01:22:03  jimg
 * Moved the CVS Log entries to the ends of files.
 * Modified the read() methods to match the new definition in the dap library.
 * Added exception handlers in various places to catch exceptions thrown
 * by the dap library.
 *
 * Revision 1.1  1999/07/28 00:22:45  jimg
 * Added
 *
 * Revision 1.4  1999/05/07 23:45:33  jimg
 * String --> string fixes
 *
 * Revision 1.3  1996/09/17 17:06:47  jimg
 * Merge the release-2-0 tagged files (which were off on a branch) back into
 * the trunk revision.
 *
 * Revision 1.2.4.2  1996/07/10 21:44:30  jimg
 * Changes for version 2.06. These fixed lingering problems from the migration
 * from version 1.x to version 2.x.
 * Removed some (but not all) warning generated with gcc's -Wall option.
 *
 * Revision 1.2.4.1  1996/06/25 22:04:50  jimg
 * Version 2.0 from Reza.
 *
 * Revision 1.2  1995/03/16  16:56:49  reza
 * Updated for the new DAP. All the read_val mfunc. and their memory management
 * are now moved to their parent class.
 * Data transfers are now in binary while DAS and DDS are still sent in ASCII.
 *
 * Revision 1.1  1995/02/10  04:58:01  reza
 * Added read and read_val functions.
 *
*/

#endif

