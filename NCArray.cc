
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

static char rcsid[] not_used ={"$Id: NCArray.cc,v 1.17 2005/02/17 23:44:13 jimg Exp $"};

#ifdef __GNUG__
//#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <iostream>
#include <sstream>

#define DODS_DEBUG 1

#include "Error.h"
#include "InternalErr.h"

#include "Dnetcdf.h"
#include "Dncx.h"
#include "NCArray.h"
#include "NCSequence.h"
#include "nc_util.h"
#include "debug.h"

const string spr = "."; // structure rename

Array *
NewArray(const string &n, BaseType *v)
{
    DBG2(cerr << "Entering NC NewArray" << endl);
    return new NCArray(n, v);
    DBG2(cerr << "Exiting NC NewArray" << endl);
}

BaseType *
NCArray::ptr_duplicate()
{
    DBG2(cerr << "Entering NCArray::ptr_duplicate()" << endl);
    return new NCArray(*this);
    DBG2(cerr << "Exiting NCArray::ptr_duplicate()" << endl);
}

NCArray::NCArray(const string &n, BaseType *v) : Array(n, v)
{
    DBG2(cerr << "Entering NCArray::NCArray(const string &n, BaseType *v)" << endl);
    // d_source = 0;         // A value of zero indicates no translation.
    DBG2(cerr << "Exiting NCArray::NCArray(const string &n, BaseType *v)" << endl);
}

NCArray::NCArray(const NCArray &rhs) : Array(rhs)
{
    DBG2(cerr << "Entering NCArray::NCArray(const NCArray &rhs)" << endl);
    m_duplicate(rhs);
    DBG2(cerr << "Exiting NCArray::NCArray(const NCArray &rhs)" << endl);
}

NCArray::~NCArray()
{
    DBG2(cerr << "Entering ~NCArray()" << endl);
    // delete d_source;
    DBG2(cerr << "Exiting ~NCArray()" << endl);
}

void
NCArray::m_duplicate(const NCArray &nca)
{
    DBG2(cerr << "Entering NCArray::_duplicate()" << endl);
    dynamic_cast<NCAccess&>(*this).clone(dynamic_cast<const NCAccess&>(nca));

    DBG2(cerr << "Exiting NCArray::_duplicate()" << endl);
}

NCArray &
NCArray::operator=(const NCArray &rhs)
{
    DBG2(cerr << "Entering NCArray::operator=(const NCArray &rhs)" << endl);
    if (this == &rhs)
        return *this;

    dynamic_cast<Array &>(*this) = rhs;

    m_duplicate(rhs);

    return *this;
    DBG2(cerr << "Exiting NCArray::operator=(const NCArray &rhs)" << endl);
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


void
NCArray::store_projection(const string &proj)
{
    // Scan the projections looking for one about this variable
    string::size_type name_pos = proj.find(name());
    if (name_pos == string::npos)
        return;
    
    // OK, this variable is in there. At name_pos
    string clause;
    string::size_type end_clause_pos = proj.find(',', name_pos);
    if (end_clause_pos == string::npos)
        clause = proj.substr(name_pos);
    else 
        clause = proj.substr(name_pos, end_clause_pos - name_pos);

    // For each bracketed group of numbers, get information about a
    // dimension.
    string::size_type idx = 0;
    string::size_type clause_start = clause.find("[", idx);
    string::size_type clause_end = clause.find("]", idx);
    while (clause_start != string::npos || clause_end != string::npos) {
        string dim_proj = clause.substr(clause_start, clause_end - clause_start + 1);

        struct dim_proj_info dpi(dim_proj);
        cl_proj.push_back(dpi);
        
        idx = clause_end + 1;
        clause_start = clause.find("[", idx);
        clause_end = clause.find("]", idx);
    }
        
}

string
NCArray::build_constraint(int outtype, const size_t *start,
            const size_t *edges, const ptrdiff_t *stride) throw(Error)
{
    // Test for bad conversion here, before we may call NCSequence (which
    // cannot run the test).
    if (!is_convertable(outtype))
        throw Error(NC_ECHAR, "Character conversion not supported.");

    // CE always starts with the variable's name

    string expr = name();

    // Get dimension sizes and strings for constraint hyperslab
    ostringstream ce;       // Build CE here.
    int dm = 0;             // Index to start, edge, stride arrays 
    int Zedge = 0;          // Search for zero size edges (no data)

    // If we found an array...
    Array::Dim_iter d;
    vector<dim_proj_info>::iterator cl_iter = cl_proj.begin();
    for (d = dim_begin(); d != dim_end(); ++d, ++dm) {
        // Verify stride argument.
        if (stride != NULL && stride[dm] < 1)
            throw Error(NC_ESTRIDE, "Stride less than 1 (one).");
    
        // Set defaults:
        // *start         NULL => first corner 
        // *edges         NULL => everything following start[] 
        // *stride        NULL => unity strides 
        long instart = start != NULL ? start[dm] : 0;
        long inedges = edges != NULL ? edges[dm] : dimension_size(d) - instart;
        long instep = stride != NULL ? stride[dm] : 1;
 
        int ext_start;
        int ext_step;
        int ext_stop;
        if (cl_iter != cl_proj.end()) {
            ext_start = (*cl_iter).start;
            ext_step = (*cl_iter).stride;
            ext_stop = (*cl_iter).stop;
            ++cl_iter;
        }
        else {
            ext_start = 0;
            ext_step = 1;
            ext_stop = dimension_size(d) - 1;
        }

        DBG(cerr << instart <<" "<< inedges <<" "<< dm << endl);
        DBG(cerr<< ext_start <<" "<< ext_step <<" " << ext_stop << endl);
    
        // create constraint expr. by combining the constraint from the
        // command line with the constraint gleaned from the API call.
        int Tstart = ext_start + instart * ext_step;
        int Tstep = instep * ext_step;
        int Tstop = (ext_stop < ext_start+(instart+(inedges-1)*instep)*ext_step) 
                    ? ext_stop : ext_start+(instart+(inedges-1)*instep)*ext_step;
    
        // Check the validity of the array constraints
        if ((instart >= dimension_size(d))
            || (instart < 0)||(inedges < 0))
            throw Error(NC_EINVALCOORDS, "Invalid coordinates.");      
    
        if (instart + inedges > dimension_size(d))
            throw Error(NC_EEDGE, "Hyperslab size exceeds dimension size.");
    
        // Zero edge found, but first check remaining dimensions for error
        if (inedges == 0) 
            Zedge = 1; 
    
        ce << "[" << Tstart << ":" << Tstep << ":" << Tstop << "]";
    }

    if (Zedge == 1)
        throw Error(NC_NOERR, "The constraint would return no data.");

    expr += ce.str();     // Return ce via value-result parameter.
    return expr;
}

bool
NCArray::is_convertable(int outtype)
{
    Type intype = var()->type();
    
    if (((outtype == Ttext) && (intype != dods_str_c) &&(intype != dods_url_c))
        || ((outtype != Ttext) && (intype == dods_str_c) &&(outtype != Tvoid))
        || ((outtype != Ttext)&&(intype == dods_url_c)&&(outtype != Tvoid)))
        return false;
    else
        return true;
}

void
NCArray::extract_values(void *values, int outtype) throw(Error)
{
    int nels  = length();       // number of elements
    
    switch (var()->type()) {
      case dods_byte_c:
      case dods_int16_c:
      case dods_uint16_c:
      case dods_int32_c:
      case dods_uint32_c: 
      case dods_float32_c:
      case dods_float64_c: {
        void *tmpbufin = 0;
        int bytes = buf2val((void **)&tmpbufin); 
        if (bytes == 0)
            throw Error(-1, "Could not read any data from remote server.");
    
        // Get the netCDF type code for this variable.
        nc_type typep = dynamic_cast<NCAccess*>(var())->get_nc_type();
        int rcode = convert_nc_type(typep, outtype, nels, tmpbufin, values);
        if (rcode != NC_NOERR)
            throw Error(rcode, "Error copying values between internal buffers [NCArray::extract_values()]");
            
        break;
      } // End of the case where the array template type is a byte, ..., float64

      case dods_str_c:
      case dods_url_c: {
        // rcode = NC_NOERR;
        char * tbfr = (char *)values;
        for (int i = 0; i < nels; i++) {
            Str *s = dynamic_cast<Str*>(var(i));
            if (!s)
                throw InternalErr(__FILE__, __LINE__, "Bad csat to Str.");
            string *cp = 0;
            string **cpp = &cp;
            s->buf2val((void **)cpp);
     
            for (unsigned int cntr=0; cntr<cp->length() || 
                 (cp->length()==0 && cntr==0); cntr++) {
                *tbfr = *(cp->c_str()+cntr);
                tbfr++;
            }
            
            // Now get rid of the C++ string object.
            delete cp;
         }
         break;
      } // End of the case where the array template type is a string or url
   
      default:
        throw Error(NC_EBADTYPE,
            string("The netCDF Client Library cannot access variables of type: ")
            + dynamic_cast<BaseType*>(this)->type_name()
            + " [NCArray::extract_values()]");
    } // End of the switch
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
	&& (nctypelen(datatype) != sizeof(dods_float32))) {

        fltbuf = (float *) new char [(nels*nctypelen(datatype))];

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
	&& (nctypelen(datatype) != sizeof(dods_float64))) {

        dblbuf = (double *) new char [(nels*nctypelen(datatype))];

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
	&& (nctypelen(datatype) != sizeof(dods_int16))) {

        shtbuf = (short *)new char [(nels*nctypelen(datatype))];

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
	&& (nctypelen(datatype) != sizeof(dods_int32))) {

      lngbuf = (nclong *)new char [(nels*nctypelen(datatype))];

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

        chrbuf = (char *)new char [(nels*nctypelen(datatype))];

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
      char *convbuf = new char [(nels*nctypelen(datatype))];

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

nc_type
NCArray::get_nc_type() throw(InternalErr)
{
    return dynamic_cast<NCAccess*>(var())->get_nc_type();
}

void
NCArray::set_source(BaseType *s) throw(InternalErr)
{
    if (s->type() == dods_array_c)
        throw InternalErr(__FILE__, __LINE__, "Array's source is an Array!");
        
    d_source = s->ptr_duplicate();
}

/** Arrays are really constructor types. To flateen one, flatten the 
    template variable and then append the array's dimensions to each of
    the variable returned as a result.
    
    @note Four cases: 1. top level array, 2. top level array of structs,
    3. array inside a struct, 4. struct array inside a struct.
    
    @todo Optimize by not doing all that if the contained variable is
    an atomic type. */
VarList
NCArray::flatten(const ClientParams &cp, const string &parent_name)
{
    VarList template_vars = dynamic_cast<NCAccess*>(var())->flatten(cp, parent_name);

    // At this point we can delete the original template variable (which 
    // might be a Structure or Sequence, so there may be several variables
    // in place of the single template). Deleting the template keeps from
    // copying it repeatedly in the while-loop.
    add_var(0);
    
    VarListIter tv = template_vars.begin();
    VarListIter tv_end = template_vars.end();
    VarList new_vars;
    while (tv != tv_end) {
        // Copy this, cast to Array, load the template, push to list
        Array *ap = dynamic_cast<Array*>(ptr_duplicate());
        ap->add_var(*tv);
        ap->set_name((*tv)->name());
        new_vars.push_back(ap);
        ++tv;
    }
    
    return new_vars;
}

// $Log: NCArray.cc,v $
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
