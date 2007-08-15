
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of nc_handler, a data handler for the OPeNDAP data
// server. 

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
// 
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.
 

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

#include <sstream>

#include <netcdf.h>

#include <Error.h>
#include <InternalErr.h>
#include <util.h>

#include "NCGrid.h"
#include <debug.h>

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

NCGrid::NCGrid(const NCGrid &rhs) : Grid(rhs)
{
}


NCGrid::~NCGrid()
{
}

NCGrid &
NCGrid::operator=(const NCGrid &rhs)
{
    if (this == &rhs)
        return *this;

    dynamic_cast<NCGrid&>(*this) = rhs;


    return *this;
}


bool
NCGrid::read(const string &dataset)
{
    DBG(cerr << "In NCGrid::read" << endl);

    if (read_p()) // nothing to do
        return false;

    DBG(cerr << "In NCGrid, reading components for " << name() << endl);

    // read array elements
    if (array_var()->send_p() || array_var()->is_in_selection())
	   array_var()->read(dataset);

    // read maps elements
    for (Map_iter p = map_begin(); p != map_end(); ++p)
    	if ((*p)->send_p() || (*p)->is_in_selection())
    	    (*p)->read(dataset);

    set_read_p(true);

    return false;
}


// $Log: NCGrid.cc,v $
// Revision 1.16  2005/04/19 23:16:18  jimg
// Removed client side parts; the client library is now in libnc-dap.
//
// Revision 1.15  2005/04/08 17:08:47  jimg
// Removed old 'virtual ctor' functions which have now been replaced by the
// factory class code in libdap++.
//
// Revision 1.14  2005/03/31 00:04:51  jimg
// Modified to use the factory class in libdap++ 3.5.
//
// Revision 1.13  2005/02/26 00:43:20  jimg
// Check point: This version of the CL can now translate strings from the
// server into char arrays. This is controlled by two things: First a
// compile-time directive STRING_AS_ARRAY can be used to remove/include
// this feature. When included in the code, only Strings associated with
// variables created by the translation process will be turned into char
// arrays. Other String variables are assumed to be single character strings
// (although there may be a bug with the way these are handled, see
// NCAccess::extract_values()).
//
// Revision 1.12  2005/02/17 23:44:13  jimg
// Modifications for processing of command line projections combined
// with the limit stuff and projection info passed in from the API. I also
// consolodated some of the code by moving d_source from various
// classes to NCAccess. This may it so that DODvario() could be simplified
// as could build_constraint() and store_projection() in NCArray.
//
// Revision 1.11  2005/01/26 23:25:51  jimg
// Implemented a fix for Sequence access by row number when talking to a
// 3.4 or earlier server (which contains a bug in is_end_of_rows()).
//
// Revision 1.10  2004/09/08 22:08:22  jimg
// More Massive changes: Code moved from the files that clone the netCDF
// function calls into NCConnect, NCAccess or nc_util.cc. Much of the
// translation functions are now methods. The netCDF type classes now
// inherit from NCAccess in addition to the DAP type classes.
//
// Revision 1.9  2004/03/08 19:08:33  jimg
// This version of the code uses the Unidata netCDF 3.5.1 version of the
// netCDF 2 API emulation. This functions call our netCDF 3 API functions
// which may either interact with a DAP server r call the local netCDF 3
// functions.
//
// Revision 1.8  2003/12/08 18:06:37  edavis
// Merge release-3-4 into trunk
//
// Revision 1.7  2003/09/30 22:33:52  jimg
// I've removed calls to the new BaseType::is_in_selection() method. This method
// is only present on the 3.4 branch at this time. Once we're ready to merge
// that code, switch back to the code I removed.
//
// Revision 1.6  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.5.4.1  2003/09/06 23:33:14  jimg
// I modified the read() method implementations so that they test the new
// in_selection property. If it is true, the methods will read values
// even if the send_p property is not true. This is so that variables used
// in the selection part of the CE, or as function arguments, will be read.
// See bug 657.
//
// Revision 1.5  2003/01/28 07:08:24  jimg
// Merged with release-3-2-8.
//
// Revision 1.2.4.3  2002/12/18 23:40:33  pwest
// gcc3.2 compile corrections, mainly regarding using statements. Also,
// problems with multi line string literatls.
//
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
