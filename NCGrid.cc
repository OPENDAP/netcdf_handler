
/*
  Copyright 1995 The University of Rhode Island and The Massachusetts
  Institute of Technology

  Portions of this software were developed by the Graduate School of
  Oceanography (GSO) at the University of Rhode Island (URI) in collaboration
  with The Massachusetts Institute of Technology (MIT).

  Access and use of this software shall impose the following obligations and
  understandings on the user. The user is granted the right, without any fee
  or cost, to use, copy, modify, alter, enhance and distribute this software,
  and any derivative works thereof, and its supporting documentation for any
  purpose whatsoever, provided that this entire notice appears in all copies
  of the software, derivative works and supporting documentation.  Further,
  the user agrees to credit URI/MIT in any publications that result from the
  use of this software or in any product that includes this software. The
  names URI, MIT and/or GSO, however, may not be used in any advertising or
  publicity to endorse or promote any products or commercial entity unless
  specific written permission is obtained from URI/MIT. The user also
  understands that URI/MIT is not obligated to provide the user with any
  support, consulting, training or assistance of any kind with regard to the
  use, operation and performance of this software nor to provide the user
  with any updates, revisions, new versions or "bug fixes".

  THIS SOFTWARE IS PROVIDED BY URI/MIT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL URI/MIT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
  DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
  PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS
  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE ACCESS, USE OR PERFORMANCE
  OF THIS SOFTWARE.
*/

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples 
// Test<type>.c,h files.
//
// ReZa 1/12/95

// $Log: NCGrid.cc,v $
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
//
//

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCGrid.cc,v 1.1 1999/07/28 00:22:43 jimg Exp $"};

#include "NCGrid.h"

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
NCGrid::read(const string &dataset, int &error)
{
    bool status;

    if (read_p()) // nothing to do
        return false;

    // read array elements
    if (!(status = array_var()->read(dataset, error))) 
        return status;

    // read maps elements
    for (Pix p = first_map_var(); p; next_map_var(p))
	if(!(status = map_var(p)->read(dataset, error)))
            break;

    if( status ) 
	set_read_p(true);

    return status;
}

