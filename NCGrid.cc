
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

#ifdef __GNUG__
#pragma implementation
#endif

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCGrid.cc,v 1.4 2002/05/03 00:01:52 jimg Exp $"};

#include "NCGrid.h"
#include "debug.h"

Grid *
NewGrid(const string &n)
{
    return new NCGrid(n);
}

// protected

BaseType *
NCGrid::ptr_duplicate()
{
    return new NCGrid(*this);
}

// public

NCGrid::NCGrid(const string &n) : Grid(n)
{
}

NCGrid::~NCGrid()
{
}

bool
NCGrid::read(const string &dataset)
{
    DBG(cerr << "In NCGrid::read" << endl);

    if (read_p()) // nothing to do
        return false;

    DBG(cerr << "In NCGrid, reading components for " << name() << endl);

    // read array elements
    if (array_var()->send_p())
	array_var()->read(dataset);

    // read maps elements
    for (Pix p = first_map_var(); p; next_map_var(p))
	if (map_var(p)->send_p())
	    map_var(p)->read(dataset);

    set_read_p(true);

    return false;
}

// $Log: NCGrid.cc,v $
// Revision 1.4  2002/05/03 00:01:52  jimg
// Merged with release-3-2-7.
//
// Revision 1.2.4.2  2002/01/29 18:55:40  jimg
// I modified NCGrid::read so that before reading the array and each map, it
// first checks to make sure that field has been marked as one that's to be
// sent. In the DAP if any field of an aggregate type is to be sent, then both
// that field *and* the container that holds it (i.e., the Grid) will be marked
// as things to send. The fix makes sure that only the parts the clients wants
// are actually read. This saves reading data that will never be sent.
//
// Revision 1.3  2001/09/28 17:18:41  jimg
// Merged with 3.2.5.
// CVS  Committing in .
//
// Revision 1.2.4.1  2001/09/27 05:59:00  jimg
// Added debug.h and some instrumentation.
//
// Revision 1.2  2000/10/06 01:22:02  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.1  1999/07/28 00:22:43  jimg
// Added
//
// Revision 1.7  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.6  1998/08/06 16:33:23  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.5  1996/09/17 17:06:30  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.4.4.3  1996/09/17 00:26:25  jimg
// Merged changes from a side branch which contained various changes from
// Reza and Charles.
// Removed ncdump and netexec since ncdump is now in its own directory and
// netexec is no longer used.
//
// Revision 1.4.4.2  1996/07/10 21:44:08  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.4.4.1  1996/06/25 22:04:31  jimg
// Version 2.0 from Reza.
//
// Revision 1.5  1995/11/18  11:33:22  reza
// Updated member function names for DAP-1.1.1.
//
// Revision 1.4  1995/07/09  21:33:45  jimg
// Added copyright notice.
//
// Revision 1.3  1995/04/28  20:40:00  reza
// Moved hyperslab access to the server side (passed as a constraint argument).
//
// Revision 1.2  1995/03/16  16:56:34  reza
// Updated for the new DAP. All the read_val mfunc. and their memory management
// are now moved to their parent class.
// Data transfers are now in binary while DAS and DDS are still sent in ASCII.
//
// Revision 1.1  1995/02/10  04:57:30  reza
// Added read and read_val functions.
