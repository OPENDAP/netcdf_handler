
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

static char rcsid[] not_used ={"$Id: NCUrl.cc,v 1.3 2003/09/25 23:09:36 jimg Exp $"};

#ifdef __GNUG__
#pragma implementation
#endif

#include "InternalErr.h"
#include "NCUrl.h"

Url *
NewUrl(const string &n)
{
    return new NCUrl(n);
}

NCUrl::NCUrl(const string &n) : Url(n)
{
}

BaseType *
NCUrl::ptr_duplicate()
{
    return new NCUrl(*this);
}

#if 0
bool
NCUrl::read(const string &)
{
  throw InternalErr(__FILE__, __LINE__, "Unimplemented read() method called!");
}
#endif

// $Log: NCUrl.cc,v $
// Revision 1.3  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.2.8.1  2003/09/06 23:33:14  jimg
// I modified the read() method implementations so that they test the new
// in_selection property. If it is true, the methods will read values
// even if the send_p property is not true. This is so that variables used
// in the selection part of the CE, or as function arguments, will be read.
// See bug 657.
//
// Revision 1.2  2000/10/06 01:22:03  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.1  1999/07/28 00:22:45  jimg
// Added
//
// Revision 1.7  1999/05/07 23:45:33  jimg
// String --> string fixes
//
// Revision 1.6  1998/08/06 16:33:26  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.5  1996/09/17 17:06:45  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.4.4.3  1996/09/17 00:26:33  jimg
// Merged changes from a side branch which contained various changes from
// Reza and Charles.
// Removed ncdump and netexec since ncdump is now in its own directory and
// netexec is no longer used.
//
// Revision 1.4.4.2  1996/07/10 21:44:28  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.4.4.1  1996/06/25 22:04:48  jimg
// Version 2.0 from Reza.
//
// Revision 1.4  1995/07/09  21:33:53  jimg
// Added copyright notice.
//
// Revision 1.3  1995/06/28  20:22:41  jimg
// Replaced malloc calls with calls to new (and calls to free with calls to
// delete).
//
// Revision 1.2  1995/03/16  16:56:48  reza
// Updated for the new DAP. All the read_val mfunc. and their memory management
// are now moved to their parent class.
// Data transfers are now in binary while DAS and DDS are still sent in ASCII.
//
// Revision 1.1  1995/02/10  04:57:58  reza
// Added read and read_val functions.
