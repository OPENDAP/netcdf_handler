
// -*- C++ -*-

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

static char rcsid[] not_used ={"$Id: NCArray.cc,v 1.6 2003/09/25 23:09:36 jimg Exp $"};

#ifdef __GNUG__
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <iostream>

#include "Error.h"
#include "InternalErr.h"

#include "Dnetcdf.h"
#include "NCArray.h"
#include "debug.h"

Array *
NewArray(const string &n, BaseType *v)
{
    return new NCArray(n, v);
}

BaseType *
NCArray::ptr_duplicate()
{
    return new NCArray(*this);
}

NCArray::NCArray(const string &n, BaseType *v) : Array(n, v)
{
}

NCArray::~NCArray()
{
}

// parse constraint expr. and make netcdf coordinate point location.
// return number of elements to read. 
long
NCArray::format_constraint(size_t *cor, ptrdiff_t *step, size_t *edg, bool *has_stride)
{
    int start, stride, stop;
    int id = 0;
    long nels = 1;

    *has_stride = false;

    for (Pix p = first_dim(); p ; next_dim(p), id++) {
      start = dimension_start(p, true); 
      stride = dimension_stride(p, true);
      stop = dimension_stop(p, true);
      // Check for empty constraint
      if(start+stop+stride == 0)
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
    int varid;                  /* variable Id */
    nc_type datatype;           /* variable data type */
    size_t cor[MAX_NC_DIMS];      /* corner coordinates */
    size_t edg[MAX_NC_DIMS];      /* edges of hypercube */
    ptrdiff_t step[MAX_NC_DIMS];     /* stride of hypercube */
    int vdims[MAX_VAR_DIMS];    /* variable dimension sizes */
    int num_dim;                /* number of dim. in variable */
    long nels;                  /* number of elements in buffer */
    size_t vdim_siz;
    // pointers to buffers for incoming data
    double *dblbuf;
    float *fltbuf;
    short *shtbuf;
    nclong *lngbuf;
    char *chrbuf;
    // type conversion pointers
    dods_int16 *intg16;
    dods_int32 *intg32;
    dods_float32 *flt32;
    dods_float64 *flt64;
    // misc.
    int id;
    bool has_stride;

    DBG(cerr << "In NCArray::read" << endl);

    if (read_p())  // Nothing to do
        return false;

    DBG(cerr << "In NCArray, opening the dataset, reading " << name() << endl);

    int ncid, errstat;

    errstat = lnc_open(dataset.c_str(), NC_NOWRITE, &ncid); /* netCDF id */

    if (errstat != NC_NOERR)
      throw Error(errstat, "Could not open the dataset's file.");
 
    errstat = lnc_inq_varid(ncid, name().c_str(), &varid);
    if (errstat != NC_NOERR)
      throw Error(errstat, "Could not get variable ID.");

    errstat = lnc_inq_var(ncid, varid, (char *)0, &datatype, &num_dim, vdims,
			(int *)0);
    if (errstat != NC_NOERR)
    throw Error(errstat, 
		string("Could not read information about the variable `") 
		+ name() + string("'."));

    nels = format_constraint(cor, step, edg, &has_stride);

    // without constraint (read everything)
    if ( nels == -1 ){
        nels = 1;
	has_stride = false;
        for (id = 0; id < num_dim; id++) {
            cor[id] = 0;

            errstat = lnc_inq_dim(ncid, vdims[id], (char *)0, &vdim_siz);
	    if (errstat != NC_NOERR)
	      throw Error(errstat, 
			  string("Could not inquire dimension information for variable `") 
			  + name() + string("'."));

            edg[id] = vdim_siz;
            nels *= vdim_siz;      /* total number of values for variable */
        }
    }

    // Correct data types to match with the local machine data types

    if ((datatype == NC_FLOAT) 
	&& (lnctypelen(datatype) != sizeof(dods_float32))) {

        fltbuf = (float *) new char [(nels*lnctypelen(datatype))];

	if( has_stride)
	  errstat = lnc_get_vars_float(ncid, varid, cor, edg, step, fltbuf);
	else
	  errstat = lnc_get_vara_float(ncid, varid, cor, edg, fltbuf);

	if (errstat != NC_NOERR)
	  throw Error(errstat, 
		      string("Could not read the variable `") + name() 
		      + string("'."));

	flt32 = new dods_float32 [nels]; 

        for (id = 0; id < nels; id++) 
            *(flt32+id) = (dods_float32) *(fltbuf+id);

	set_read_p(true);  
        val2buf((void *)flt32);

	// clean up
        delete [] flt32;
        delete [] fltbuf;
    }
    else if ((datatype == NC_DOUBLE) 
	&& (lnctypelen(datatype) != sizeof(dods_float64))) {

        dblbuf = (double *) new char [(nels*lnctypelen(datatype))];

	if( has_stride)
	    errstat = lnc_get_vars_double(ncid, varid, cor, edg, step, dblbuf);
	else
	    errstat = lnc_get_vara_double(ncid, varid, cor, edg, dblbuf);

	if (errstat != NC_NOERR)
	  throw Error(errstat,string("Could not read the variable `") + name() 
		      + string("'."));

	flt64 = new dods_float64 [nels]; 

        for (id = 0; id < nels; id++) 
            *(flt64+id) = (dods_float64) *(dblbuf+id);

	set_read_p(true);  
        val2buf((void *)flt64);

	// clean up
        delete [] flt64;
        delete [] dblbuf;
    }
    else if ((datatype == NC_SHORT) 
	&& (lnctypelen(datatype) != sizeof(dods_int16))) {

        shtbuf = (short *)new char [(nels*lnctypelen(datatype))];

	if( has_stride)
	    errstat = lnc_get_vars_short(ncid, varid, cor, edg, step, shtbuf);
	else 
	    errstat = lnc_get_vara_short(ncid, varid, cor, edg, shtbuf);
	
	if (errstat != NC_NOERR)
	  throw Error(errstat, 
		      string("Could not read the variable `") + name() 
		      + string("'."));

	intg16 = new dods_int16 [nels];

        for (id = 0; id < nels; id++) 
            *(intg16+id) = (dods_int16) *(shtbuf+id);

	set_read_p(true);  
        val2buf((void *)intg16);

	// clean up
        delete [] intg16;
        delete [] shtbuf;
    }
    else if ((datatype == NC_LONG) 
	&& (lnctypelen(datatype) != sizeof(dods_int32))) {

      lngbuf = (nclong *)new char [(nels*lnctypelen(datatype))];

	if( has_stride)
	    errstat = lnc_get_vars(ncid, varid, cor, edg, step, lngbuf);
	else
	    errstat = lnc_get_vara(ncid, varid, cor, edg, lngbuf);

	if (errstat != NC_NOERR)
	  throw Error(errstat,string("Could not read the variable `") + name() 
		      + string("'."));

	intg32 = new dods_int32 [nels];

        for (id = 0; id < nels; id++) 
            *(intg32+id) = (dods_int32) *(lngbuf+id);

	set_read_p(true);  
        val2buf((void *) intg32);

	// clean up
        delete [] intg32;
        delete [] lngbuf;
    }
    else if (datatype == NC_CHAR) {

        chrbuf = (char *)new char [(nels*lnctypelen(datatype))];

	// read the vlaues in from the local netCDF file
	if( has_stride)
	    errstat = lnc_get_vars_text(ncid, varid, cor, edg, step,chrbuf);
	else
	    errstat = lnc_get_vara_text(ncid, varid, cor, edg, chrbuf);

	if (errstat != NC_NOERR)
	  throw Error(errstat,string("Could not read the variable `") + name() 
		      + string("'."));

	string *strg = new string [nels]; // array of strings
	char buf[2] = " "; // one char and EOS

	// put the char values in the string array
	for (id = 0; id < nels; id++){	  
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
    }
    else {
      //default (no type conversion needed and the type Byte)
      char *convbuf = new char [(nels*lnctypelen(datatype))];

      if( has_stride)
	errstat = lnc_get_vars(ncid, varid, cor, edg, step, (void *)convbuf);
      else
	errstat = lnc_get_vara(ncid, varid, cor, edg, (void *)convbuf);
 
      if (errstat != NC_NOERR)
	  throw Error(errstat,string("Could not read the variable `") + name() 
		      + string("'."));

      set_read_p(true);  
      val2buf((void *)convbuf);

      delete [] convbuf;
    }

    if (lnc_close(ncid) != NC_NOERR)
      throw InternalErr(__FILE__, __LINE__, "Could not close the dataset!");

    return false;
}

// $Log: NCArray.cc,v $
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
