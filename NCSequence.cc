
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

static char rcsid[] not_used ={"$Id: NCSequence.cc,v 1.5 2004/10/22 21:51:34 jimg Exp $"};

#ifdef __GNUG__
//#pragma implementation
#endif

#include <sstream>

#include "NCSequence.h"

#include "InternalErr.h"
#include "debug.h"

Sequence *
NewSequence(const string &n)
{
    return new NCSequence(n);
}

// protected

BaseType *
NCSequence::ptr_duplicate()
{
    return new NCSequence(*this);
}

// public

NCSequence::NCSequence(const string &n) : Sequence(n)
{
}

NCSequence::~NCSequence()
{
}

bool 
NCSequence::read(const string &)
{
    throw InternalErr(__FILE__, __LINE__, "Unimplemented read method called.");
}

/** Build a constraint for a Sequence. Build a CE for then entire Sequence 
    even when the client asks for just one field. A bug in the Sequence CE
    processing code. */
string
NCSequence::build_constraint(int outtype, const size_t *start,
    const size_t *edges, const ptrdiff_t *stride) throw(Error)
{
    string expr = name();      // CE always starts with the variable's name

    // Get dimension sizes and strings for constraint hyperslab
    ostringstream ce;       // Build CE here.
    // Note that a Sequence is always a one dimensional thing.
    int dm = 0;             // Index to start, edge, stride arrays 
    //int Zedge = 0;          // Search for zero size edges (no data)

    // Verify stride argument.
    if (stride != NULL && stride[dm] < 1)
        throw Error(NC_ESTRIDE, "Stride less than 1 (one).");

    // Set defaults:
    // *start         NULL => first corner 
    // *edges         NULL => everything following start[] 
    // *stride        NULL => unity strides 
    long instart = start != NULL ? start[dm] : 0;
    long inedges = edges != NULL ? edges[dm] : get_size() - instart;
    long instep = stride != NULL ? stride[dm] : 1;

    // external constraint (from ncopen)
    int ext_start = get_starting_row_number(); 
    int ext_step = get_row_stride();
    int ext_stop = get_ending_row_number();

    DBG(cerr << instart <<" "<< inedges <<" "<< dm << endl);
    DBG(cerr<< ext_start <<" "<< ext_step <<" " << ext_stop << endl);

    // create constraint expr. by combining the constraints
    int Tstart = ext_start + instart * ext_step;
    int Tstep = instep * ext_step;
    int Tstop = (ext_stop < ext_start+(instart+(inedges-1)*instep)*ext_step) 
                ? ext_stop : ext_start+(instart+(inedges-1)*instep)*ext_step;

    // Check the validity of the array constraints
    if ((instart >= get_size())
        || (instart < 0)||(inedges < 0))
        throw Error(NC_EINVALCOORDS, "Invalid coordinates.");      

    if (instart + inedges > get_size())
        throw Error(NC_EEDGE, "Hyperslab size exceeds dimension size.");

    // Zero edge found.
    if (inedges == 0) 
        throw Error(NC_NOERR, "The constraint would return no data.");

    ce << "[" << Tstart << ":" << Tstep << ":" << Tstop << "]";

    expr += ce.str();     // Return ce via value-result parameter.
    return expr;
}

void
NCSequence::extract_values(void *values, int outtype) throw(Error)
{
}

void
NCSequence::set_size(int size)
{
    d_size = size;
}

int
NCSequence::get_size()
{
    return d_size;
}

// $Log: NCSequence.cc,v $
// Revision 1.5  2004/10/22 21:51:34  jimg
// More massive changes: Translation of Sequences now works so long as the
// Sequence contains only atomic types.
//
// Revision 1.4  2004/09/08 22:08:22  jimg
// More Massive changes: Code moved from the files that clone the netCDF
// function calls into NCConnect, NCAccess or nc_util.cc. Much of the
// translation functions are now methods. The netCDF type classes now
// inherit from NCAccess in addition to the DAP type classes.
//
// Revision 1.3  2003/12/08 18:06:37  edavis
// Merge release-3-4 into trunk
//
// Revision 1.2  2000/10/06 01:22:02  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.1  1999/07/28 00:22:44  jimg
// Added
//
// Revision 1.6  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.5  1998/08/06 16:33:24  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.4  1996/09/17 17:06:37  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.3.4.4  1996/09/17 00:26:29  jimg
// Merged changes from a side branch which contained various changes from
// Reza and Charles.
// Removed ncdump and netexec since ncdump is now in its own directory and
// netexec is no longer used.
//
// Revision 1.3.4.3  1996/08/13 21:19:01  jimg
// *** empty log message ***
//
// Revision 1.3.4.2  1996/07/10 21:44:18  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.3.4.1  1996/06/25 22:04:40  jimg
// Version 2.0 from Reza.
//
// Revision 1.3  1995/07/09  21:33:49  jimg
// Added copyright notice.
//
// Revision 1.2  1995/03/16  16:56:41  reza
// Updated for the new DAP. All the read_val mfunc. and their memory management
// are now moved to their parent class.
// Data transfers are now in binary while DAS and DDS are still sent in ASCII.
//
// Revision 1.1  1995/02/10  04:57:43  reza
// Added read and read_val functions.
//
