
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
 

// (c) COPYRIGHT URI/MIT 1996
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      Reza       Reza Nekovei (rnekovei@ieee.org)

#include "config_nc.h"

static char rcsid[] not_used ={"$Id$"};

#include <netcdf.h>
#include "InternalErr.h"

#include "NCUInt16.h"


NCUInt16::NCUInt16(const string &n, const string &ds) : UInt16(n, ds)
{
}

NCUInt16::NCUInt16(const NCUInt16 &rhs) : UInt16(rhs)
{
}

NCUInt16::~NCUInt16()
{
}

NCUInt16 &
NCUInt16::operator=(const NCUInt16 &rhs)
{
    if (this == &rhs)
        return *this;

    dynamic_cast<NCUInt16&>(*this) = rhs;


    return *this;
}


BaseType *
NCUInt16::ptr_duplicate(){

    return new NCUInt16(*this);
}


bool
NCUInt16::read(const string &ds)
{
  int varid;                  /* variable Id */
  nc_type datatype;           /* variable data type */
  size_t cor[MAX_NC_DIMS];      /* corner coordinates */
  int num_dim;                /* number of dim. in variable */
  dods_uint16 uintg16;
  int id;

    string use_dataset = dataset() ;
    if( use_dataset.empty() )
    {
	use_dataset = ds ;
    }

  if (read_p()) // nothing to do
    return false;

  int ncid, errstat;

  errstat = nc_open(use_dataset.c_str(), NC_NOWRITE, &ncid); /* netCDF id */
  if (errstat != NC_NOERR)
    {
	string err = (string)"Could not open the dataset's file ("
	             + use_dataset.c_str() + ")" ;
	throw Error(errstat, err);
    }
 
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

  if (datatype == NC_SHORT)
    {
      short sht;

      errstat = nc_get_var1_short(ncid, varid, cor, &sht);
      if(errstat != NC_NOERR)
	throw Error(errstat, string("Could not read the variable `") + name() 
		    + string("'."));

      set_read_p(true);

      uintg16 = (dods_uint16) sht;
      val2buf( &uintg16 );

      if (nc_close(ncid) != NC_NOERR)
	throw InternalErr(__FILE__, __LINE__, 
			  "Could not close the dataset!");
    }
  else 
    throw InternalErr(__FILE__, __LINE__,
		      "Entered NCUInt16::read() with non-short variable!");

  return false;
}

// $Log: NCUInt16.cc,v $
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
// Revision 1.4.8.2  2003/06/07 22:15:43  reza
// Fixed a typo in checking the netcdf return code.
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
// Revision 1.1  1999/07/28 00:22:44  jimg
// Added
//
// Revision 1.2.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.2  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.1  1999/03/30 21:12:07  reza
// Added the new types
