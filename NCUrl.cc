
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

// $Log: NCUrl.cc,v $
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
//
//

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCUrl.cc,v 1.1 1999/07/28 00:22:45 jimg Exp $"};

#ifdef __GNUG__
#pragma implementation
#endif

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

bool
NCUrl::read(const string &, int &error)
{
    error = 1;
    return false;
}


