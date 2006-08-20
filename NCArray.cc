
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
 
// (c) COPYRIGHT URI/MIT 1994-1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      reza            Reza Nekovei (rnekovei@ieee.org)

// netCDF sub-class implementation for NCByte,...NCGrid.
// The files are patterned after the subcalssing examples 
// Test<type>.c,h files.
//
// ReZa 1/12/95

#include "config_nc.h"

static char rcsid[] not_used ={"$Id$"};

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <algorithm>

#include <netcdf.h>

// #define DODS_DEBUG 1

#include "Error.h"
#include "InternalErr.h"

#include "NCArray.h"

#include "debug.h"

BaseType *
NCArray::ptr_duplicate()
{
    return new NCArray(*this);
}

/** Build an NCArray instance.
    @param n The name of the array.
    @param v Use this variable as a template for type of array elements. 
    The variable will be copied, so the caller is responsible for freeing
    storage used by the actual parameter. Also, if the actual parameter
    is an Array, libdap++ code will use the template of that Array as the
    template for this NCArray. */
NCArray::NCArray(const string &n, BaseType *v) : Array(n, v)
{
}

NCArray::NCArray(const NCArray &rhs) : Array(rhs)
{
}

NCArray::~NCArray()
{
    DBG(cerr << "delete d_source " << d_source << endl);
}

NCArray &
NCArray::operator=(const NCArray &rhs)
{
    if (this == &rhs)
        return *this;

    dynamic_cast<Array &>(*this) = rhs;

    return *this;
}


// Should this be a private method? jhrg 11/3/04
long
NCArray::format_constraint(size_t *cor, ptrdiff_t *step, size_t *edg,
                           bool *has_stride)
{
    int start, stride, stop;
    int id = 0;
    long nels = 1;

    *has_stride = false;

    for (Dim_iter p = dim_begin(); p != dim_end(); ++p, id++) {
    	start = dimension_start(p, true); 
    	stride = dimension_stride(p, true);
    	stop = dimension_stop(p, true);
    	// Check for empty constraint
    	if (start + stop + stride == 0)
    	    return -1;
    
    	cor[id] = start;
    	step[id] = stride;
    	edg[id] = ((stop - start)/stride) + 1; // count of elements
    	nels *= edg[id];      // total number of values for variable
    
    	if (stride != 1)
    	    *has_stride = true;
    }
    
    return nels;
}


bool
NCArray::read(const string &dataset)
{
    size_t cor[MAX_NC_DIMS];      /* corner coordinates */
    size_t edg[MAX_NC_DIMS];      /* edges of hypercube */
    ptrdiff_t step[MAX_NC_DIMS];  /* stride of hypercube */

    DBG(cerr << "In NCArray::read" << endl);

    if (read_p())  // Nothing to do
        return false;

    DBG(cerr << "In NCArray, opening the dataset, reading " << name() << endl);

    int ncid;
    int errstat = nc_open(dataset.c_str(), NC_NOWRITE, &ncid); /* netCDF id */

    if (errstat != NC_NOERR)
    {
	string err = (string)"Could not open the dataset's file ("
	             + dataset.c_str() + ")" ;
	throw Error(errstat, err);
    }
 
    int varid;                  /* variable Id */
    errstat = nc_inq_varid(ncid, name().c_str(), &varid);
    if (errstat != NC_NOERR)
	throw Error(errstat, "Could not get variable ID.");

    nc_type datatype;           /* variable data type */
    int vdims[MAX_VAR_DIMS];    /* variable dimension sizes */
    int num_dim;                /* number of dim. in variable */
    errstat = nc_inq_var(ncid, varid, (char *)0, &datatype, &num_dim, vdims,
			 (int *)0);
    if (errstat != NC_NOERR)
	throw Error(errstat, 
		    string("Could not read information about the variable `") 
		    + name() + string("'."));

    bool has_stride;
    long nels = format_constraint(cor, step, edg, &has_stride);

    // without constraint (read everything)
    if ( nels == -1 ){
        nels = 1;
	has_stride = false;
        for (int id = 0; id < num_dim; id++) {
            cor[id] = 0;

            size_t vdim_siz;
            errstat = nc_inq_dim(ncid, vdims[id], (char *)0, &vdim_siz);
	    if (errstat != NC_NOERR)
		throw Error(errstat, 
			    string("Could not inquire dimension information for variable `") 
			    + name() + string("'."));

            edg[id] = vdim_siz;
            nels *= vdim_siz;      /* total number of values for variable */
        }
    }

    // Correct data types to match with the local machine data types
    switch (datatype) {
      case NC_FLOAT:{
        float *fltbuf = (float *) new char[(nels * nctypelen(datatype))];
        
        if (has_stride)
            errstat = nc_get_vars_float(ncid, varid, cor, edg, step, fltbuf);
        else
            errstat = nc_get_vara_float(ncid, varid, cor, edg, fltbuf);
        
        if (errstat != NC_NOERR)
            throw Error(errstat, string("Could not read the variable `") + name()
                        + string("'."));
        
        if (nctypelen(datatype) != sizeof(dods_float32)) {
            dods_float32 *flt32 = new dods_float32[nels];
        
            for (int id = 0; id < nels; id++)
                *(flt32 + id) = (dods_float32) * (fltbuf + id);
        
            val2buf((void *) flt32);
            delete[]flt32;
        } else {
            val2buf((void *) fltbuf);
        }

        set_read_p(true);
        delete[]fltbuf;
        
        break;
      }
    
      case NC_DOUBLE: {

        double *dblbuf = (double *) new char [(nels*nctypelen(datatype))];

	if( has_stride)
	    errstat = nc_get_vars_double(ncid, varid, cor, edg, step, dblbuf);
	else
	    errstat = nc_get_vara_double(ncid, varid, cor, edg, dblbuf);

	if (errstat != NC_NOERR)
	    throw Error(errstat,string("Could not read the variable `") + name() 
			+ string("'."));

	if (nctypelen(datatype) != sizeof(dods_float64)) {
	dods_float64 *flt64 = new dods_float64 [nels]; 

        for (int id = 0; id < nels; id++) 
            *(flt64+id) = (dods_float64) *(dblbuf+id);

        val2buf((void *)flt64);
        delete [] flt64;
	}
	else {
        val2buf((void *)dblbuf);
	}

	set_read_p(true);  
        delete [] dblbuf;
        
        break;
    }
    
    case NC_SHORT: {

        short *shtbuf = (short *)new char [(nels*nctypelen(datatype))];

	if( has_stride)
	    errstat = nc_get_vars_short(ncid, varid, cor, edg, step, shtbuf);
	else 
	    errstat = nc_get_vara_short(ncid, varid, cor, edg, shtbuf);
	
	if (errstat != NC_NOERR)
	    throw Error(errstat, 
			string("Could not read the variable `") + name() 
			+ string("'."));

	if (nctypelen(datatype) != sizeof(dods_int16)) {
	dods_int16 *intg16 = new dods_int16 [nels];

        for (int id = 0; id < nels; id++) 
            *(intg16+id) = (dods_int16) *(shtbuf+id);

        val2buf((void *)intg16);
        delete [] intg16;
	}
	else {
        val2buf((void *)shtbuf);
	}
	set_read_p(true);  
        delete [] shtbuf;
        
        break;
    }
    
    case NC_INT: {
	int *lngbuf = (int *)new char [(nels*nctypelen(datatype))];

	if( has_stride)
	    errstat = nc_get_vars_int(ncid, varid, cor, edg, step, lngbuf);
	else
	    errstat = nc_get_vara_int(ncid, varid, cor, edg, lngbuf);

	if (errstat != NC_NOERR)
	    throw Error(errstat, string("Could not read the variable `")
			+ name() + string("'."));

	if (nctypelen(datatype) != sizeof(dods_int32)) {
	    dods_int32 *intg32 = new dods_int32 [nels];

	    for (int id = 0; id < nels; id++) 
		*(intg32+id) = (dods_int32) *(lngbuf+id);

	    val2buf((void *) intg32);
	    delete [] intg32;
	}
	else {
	    val2buf((void*)lngbuf);
	}

	set_read_p(true);  
        delete [] lngbuf;
        
        break;
    }
    
    case NC_CHAR: {

        char *chrbuf = (char *)new char [(nels*nctypelen(datatype))];

	// read the vlaues in from the local netCDF file
	if( has_stride)
	    errstat = nc_get_vars_text(ncid, varid, cor, edg, step, chrbuf);
	else
	    errstat = nc_get_vara_text(ncid, varid, cor, edg, chrbuf);

	if (errstat != NC_NOERR)
	    throw Error(errstat,string("Could not read the variable `") + name() 
			+ string("'."));

	string *strg = new string [nels]; // array of strings
	char buf[2] = " "; // one char and EOS

	// put the char values in the string array
	for (int id = 0; id < nels; id++){	  
	    strncpy(buf, (chrbuf+id), 1);
	    strg[id] = (string) buf;
	}

	// reading is done (dont need to read each individual array value)
	set_read_p(true);  
	// put values in the buffers
	val2buf(strg);

	// clean up
	delete [] strg;
	delete [] chrbuf;
        
        break;
    }
    
    case NC_BYTE: {
        //default (no type conversion needed and the type Byte)
        char *convbuf = new char [(nels*nctypelen(datatype))];

        if( has_stride)
            errstat = nc_get_vars_uchar(ncid, varid, cor, edg, step, (unsigned char*)convbuf);
        else
            errstat = nc_get_vara_uchar(ncid, varid, cor, edg, (unsigned char*)convbuf);
 
        if (errstat != NC_NOERR)
            throw Error(errstat,string("Could not read the variable `") + name() 
                        + string("'."));

        set_read_p(true);  
        val2buf((void *)convbuf);

        delete [] convbuf;
        
        break;
    }
    
    default:
        throw InternalErr(__FILE__, __LINE__, 
                          string("Unknow data type for the variable '")
                          + name() + string("'."));
    }
    
    if (nc_close(ncid) != NC_NOERR)
	throw InternalErr(__FILE__, __LINE__, "Could not close the dataset!");

    return false;
}

// $Log: NCArray.cc,v $
// Revision 1.25  2005/04/19 23:16:18  jimg
// Removed client side parts; the client library is now in libnc-dap.
//
// Revision 1.24  2005/04/11 18:38:20  jimg
// Fixed a problem with NCSequence where nested sequences were not flagged
// but instead were translated. The extract_values software cannot process a
// nested sequence yet. Now the code inserts an attribute that notes that a
// nested sequence has been elided.
//
// Revision 1.23  2005/04/08 17:08:47  jimg
// Removed old 'virtual ctor' functions which have now been replaced by the
// factory class code in libdap++.
//
// Revision 1.22  2005/03/31 00:04:51  jimg
// Modified to use the factory class in libdap++ 3.5.
//
// Revision 1.21  2005/03/05 00:16:58  jimg
// checkpoint: working on memory leaks found using unit tests
//
// Revision 1.20  2005/03/04 18:10:49  jimg
// At this point valgrind runs the Unidata tests for both local and remote access
// and shows no errors or leaks. There are 8 bytes still reachable from an
// exception, but that's it.
//
// Revision 1.19  2005/03/02 17:51:50  jimg
// Considerable reduction in memory leaks and fixed all errant memory
// accesses found with nc_test. OPeNDAP error codes and Error object
// message strings are now reported using the nc_strerrror() function!
//
// Revision 1.18  2005/02/26 00:43:20  jimg
// Check point: This version of the CL can now translate strings from the
// server into char arrays. This is controlled by two things: First a
// compile-time directive STRING_AS_ARRAY can be used to remove/include
// this feature. When included in the code, only Strings associated with
// variables created by the translation process will be turned into char
// arrays. Other String variables are assumed to be single character strings
// (although there may be a bug with the way these are handled, see
// NCAccess::extract_values()).
//
// Revision 1.17  2005/02/17 23:44:13  jimg
// Modifications for processing of command line projections combined
// with the limit stuff and projection info passed in from the API. I also
// consolodated some of the code by moving d_source from various
// classes to NCAccess. This may it so that DODvario() could be simplified
// as could build_constraint() and store_projection() in NCArray.
//
// Revision 1.16  2005/02/02 03:35:35  jimg
// Removed DODS_DEBUG
//
// Revision 1.15  2005/01/29 00:20:29  jimg
// Checkpoint: CEs ont he command line/ncopen() almost work.
//
// Revision 1.14  2005/01/26 23:25:51  jimg
// Implemented a fix for Sequence access by row number when talking to a
// 3.4 or earlier server (which contains a bug in is_end_of_rows()).
//
// Revision 1.13  2004/11/30 22:11:35  jimg
// I replaced the flatten_*() functions with a flatten() method in
// NCAccess. The default version of this method is in NCAccess and works
// for the atomic types; constructors must provide a specialization.
// Then I removed the code that copied the variables from vectors to
// lists. The translation code in NCConnect was modified to use the
// new method.
//
// Revision 1.12  2004/11/05 17:10:14  jimg
// Fiddled with some comments and added DBG() macros to some instrumentation.
//
// Revision 1.11  2004/10/28 16:38:19  jimg
// Added support for error handling to ClientParams. Added use of
// ClientParams to NCConnect, although that's not complete yet. NCConnect
// now has an instance of ClientParams. The instance is first built and
// then passed into NCConnect's ctor which stores a const reference to the CP
// object.
//
// Revision 1.10  2004/10/22 21:51:34  jimg
// More massive changes: Translation of Sequences now works so long as the
// Sequence contains only atomic types.
//
// Revision 1.9  2004/09/08 22:08:21  jimg
// More Massive changes: Code moved from the files that clone the netCDF
// function calls into NCConnect, NCAccess or nc_util.cc. Much of the
// translation functions are now methods. The netCDF type classes now
// inherit from NCAccess in addition to the DAP type classes.
//
// Revision 1.8  2004/03/08 19:08:33  jimg
// This version of the code uses the Unidata netCDF 3.5.1 version of the
// netCDF 2 API emulation. This functions call our netCDF 3 API functions
// which may either interact with a DAP server r call the local netCDF 3
// functions.
//
// Revision 1.7  2003/12/08 18:06:37  edavis
// Merge release-3-4 into trunk
//
// Revision 1.6  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.5.4.1  2003/06/07 22:02:32  reza
// Fixed char vs. byte and long vs. nclong error checks.
//
// Revision 1.5  2002/05/03 00:01:52  jimg
// Merged with release-3-2-7.
//
// Revision 1.3.4.2  2001/12/26 03:41:25  rmorris
// Force in <iostream> instead of <iostream.h>.  Under unix <iostream> is
// typically a wrapper around <iostream.h>, but under win32 it is something
// completely different from iostream.h
//
// Revision 1.4  2001/09/28 17:18:41  jimg
// Merged with 3.2.5.
// CVS  Committing in .
//
// Revision 1.3.4.1  2001/09/27 06:00:01  jimg
// Added debug.h and instrumentation.
//
// Revision 1.3  2000/10/06 01:22:02  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.2  1999/11/05 05:15:05  jimg
// Result of merge woth 3-1-0
//
// Revision 1.1.2.1  1999/10/29 05:05:20  jimg
// Reza's fixes plus the configure & Makefile update
//
// Revision 1.1  1999/07/28 00:22:42  jimg
// Added
//
// Revision 1.15.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.15  1999/05/07 23:45:31  jimg
// String --> string fixes
//
// Revision 1.14  1999/04/09 17:11:55  jimg
// Fixed comments and copyright statement
//
// Revision 1.13  1999/03/30 05:20:55  reza
// Added support for the new data types (Int16, UInt16, and Float32).
//
// Revision 1.12  1998/08/06 16:33:20  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.11  1997/03/10 16:24:17  reza
// Added error object and upgraded to DODS core release 2.12.
//
// Revision 1.10  1996/10/12 04:36:28  jimg
// Added status checks to all of the netcdf calls in the read member function.
//
// Revision 1.9  1996/09/17 17:06:15  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.8.2.3  1996/09/17 00:26:18  jimg
// Merged changes from a side branch which contained various changes from
// Reza and Charles.
// Removed ncdump and netexec since ncdump is now in its own directory and
// netexec is no longer used.
//
// Revision 1.8.2.2  1996/07/10 21:43:54  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.8.2.1  1996/06/25 22:04:17  jimg
// Version 2.0 from Reza.
//
// Revision 1.7  1995/07/09  21:33:38  jimg
// Added copyright notice.
//
// Revision 1.6  1995/06/29  15:42:06  jimg
// Fixed nc_dods.cc
//
// Revision 1.5  1995/06/28  20:22:40  jimg
// Replaced malloc calls with calls to new (and calls to free with calls to
// delete).
//
// Revision 1.4  1995/06/23  13:51:35  reza
// Added scalar read and the constraint "point".
//
// Revision 1.3  1995/04/28  20:39:59  reza
// Moved hyperslab access to the server side (passed as a constraint argument).
//
// Revision 1.2  1995/03/16  16:56:25  reza
// Updated for the new DAP. All the read_val mfunc. and their memory management
// are now moved to their parent class.
// Data transfers are now in binary while DAS and DDS are still sent in ASCII.
//
// Revision 1.1  1995/02/10  04:57:11  reza
// Added read and read_val functions.
