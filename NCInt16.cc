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
// ReZa 3/27/99

// $Log: NCInt16.cc,v $
// Revision 1.3  1999/11/05 05:15:05  jimg
// Result of merge woth 3-1-0
//
// Revision 1.1.2.1  1999/10/29 05:05:21  jimg
// Reza's fixes plus the configure & Makefile update
//
// Revision 1.2  1999/10/21 13:19:06  reza
// IMAP and other bug fixed for version3.
//
// Revision 1.1  1999/07/28 00:22:43  jimg
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
//
//
//

#include "config_nc.h"

static char rcsid[] not_used ={"$Id: NCInt16.cc,v 1.3 1999/11/05 05:15:05 jimg Exp $"};

#ifdef __GNUG__
#pragma implementation
#endif

#include <assert.h>

#include "Dnetcdf.h"
#include "NCInt16.h"

Int16 *
NewInt16(const string &n)
{
    return new NCInt16(n);
}

NCInt16::NCInt16(const string &n) : Int16(n)
{
}

BaseType *
NCInt16::ptr_duplicate(){

    return new NCInt16(*this);
}

bool
NCInt16::read(const string &dataset, int &error)
{

    int varid;                  /* variable Id */
    nc_type datatype;           /* variable data type */
    long cor[MAX_NC_DIMS];      /* corner coordinates */
    int num_dim;                /* number of dim. in variable */
    dods_int16 intg16;
    int id;

    if (read_p()) // nothing to do
        return false;

    int ncid = lncopen(dataset.c_str(), NC_NOWRITE); /* netCDF id */

    if (ncid == -1) { 
        cerr << "ncopen failed on " << dataset<< endl;
	error = 1;
        return false;
    }
 
    varid = lncvarid(ncid, name().c_str());

    (void)lncvarinq(ncid,varid,(char *)0,&datatype,&num_dim,(int *)0,(int *)0);

    for (id = 0; id <= num_dim; id++) 
      cor[id] = 0;

    if (datatype == NC_SHORT){

	short sht;

	(void) lncvarget1 (ncid, varid, cor, &sht);
	set_read_p(true);

	intg16 = (dods_int16) sht;
	val2buf( &intg16 );

	(void) lncclose(ncid);  
	return true;
    }

    error = 1;
    return false;
}
