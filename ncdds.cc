
// (c) COPYRIGHT URI/MIT 1994-1996
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      reza            Reza Nekovei (reza@intcomm.net)

// This file contains functions which read the variables and their description
// from a netcdf file and build the in-memeory DDS. These functions form the
// core of the server-side software necessary to extract the DDS from a
// netcdf data file.
//
// It also contains test code which will print the in-memeory DDS to
// stdout. 
//
// ReZa 10/20/94

#include "config_nc.h"

static char not_used rcsid[]={"$Id: ncdds.cc,v 1.7 2005/03/31 00:04:51 jimg Exp $"};

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <string>

#include "Dnetcdf.h"
#include "DDS.h"
#include "NCInt32.h"
#include "NCUInt32.h"
#include "NCUInt16.h"
#include "NCInt16.h"
#include "NCFloat64.h"
#include "NCFloat32.h"
#include "NCByte.h"
#include "NCArray.h"
#include "NCGrid.h"
#include "NCStr.h"
#include "cgi_util.h"

// Used by ErrMsgT

static char Msgt[255];

// This function returns the appropriate DODS BaseType for the given 
// netCDF data type.
//

static BaseType *
Get_bt(BaseTypeFactory *factory, string varname, nc_type datatype) 
{
    switch (datatype) {
      case NC_CHAR:
	return (factory->NewStr(varname));

      case NC_BYTE:
	return (factory->NewByte(varname));
	
      case NC_SHORT:
	return (factory->NewInt16(varname));

      case NC_LONG:
	return (factory->NewInt32(varname));

      case NC_FLOAT:
	return (factory->NewFloat32(varname));

      case NC_DOUBLE:
	return (factory->NewFloat64(varname));
	
      default:
	return (factory->NewStr(varname));
    }
}

// Read given number of variables (nvars) from the opened netCDF file 
// (ncid) and add them with their appropriate type and dimensions to 
// the given instance of the DDS class.
//
// Returns: false if an error accessing the netcdf file was detected, true
// otherwise.

int
read_class(DDS &dds_table, int ncid, int nvars, string *error)
{
    char varname1[MAX_NC_NAME];
    char varname2[MAX_NC_VARS][MAX_NC_NAME];
    char dimname[MAX_NC_NAME];
    char dimname2[MAX_NC_NAME];
    char *var_match[MAX_NC_VARS];
    int dim_ids[MAX_VAR_DIMS];
    int dim_szs[MAX_VAR_DIMS];
    char dim_nms[MAX_VAR_DIMS][MAX_NC_NAME];
    int tmp_dim_ids[MAX_VAR_DIMS];
    nc_type typ_match[MAX_NC_VARS];
    int ndims, ndims2, dim_match;
    size_t dim_sz, tmp_sz;  
    nc_type nctype;
    Array *ar;
    Grid *gr;
    Part pr;
    int errstat;

    //add all the variables in this file to DDS table 

    for (int v1 = 0; v1 < nvars; ++v1) {
      errstat = lnc_inq_var(ncid,v1,varname1,&nctype,&ndims,dim_ids,(int *)0);
      if (errstat != NC_NOERR){
	sprintf (Msgt,"ncdds server: could not get variable name or dimension number for variable %d ",v1);
	    ErrMsgT(Msgt); //local error messag
	    *error = (string)"\"" + (string)Msgt + (string)"\"";
	    return errstat;
	}

	BaseType *bt = Get_bt(dds_table.get_factory(), varname1, nctype);
    
	// is an Atomic-class ?

	if (ndims == 0){
	    dds_table.add_var(bt);
	}
	
	// Grid vs. Array type matching
    
	else {
      
	    dim_match = 0;
	  
	    // match all the dimensions of this variable to other variables
	    int d;
	    for (d = 0; d < ndims; ++d){
	      errstat = lnc_inq_dim(ncid, dim_ids[d], dimname, &dim_sz);
	      if (errstat != NC_NOERR){
		sprintf (Msgt,"ncdds server: could not get dimension size for dimension %d in variable %d ",d,v1);
		ErrMsgT(Msgt); //server error messag
		*error = (string)"\"" + (string)Msgt + (string)"\"";
		return errstat;
	      }
	      dim_szs[d] = (int) dim_sz;
	      (void) strcpy(dim_nms[d],dimname);
	
	      for (int v2 = 0; v2 < nvars; ++v2) { 
		errstat = lnc_inq_var(ncid,v2,varname2[v2],&nctype,&ndims2,tmp_dim_ids,(int *)0);
		    if (errstat != NC_NOERR) {
			sprintf (Msgt,"ncdds server: could not get variable name or dimension number for variable %d ",v2);
			ErrMsgT(Msgt); 
			*error = (string)"\"" + (string)Msgt + (string)"\"";
			return errstat;
		    }
	  
		    // Is it a Grid ?     1) variable name = the dimension name
		    //                    2) The variable has only one dimension
		    //                    3) It is not itself
		    //                    4) They are the same size
		    if ((v1 != v2) && (strcmp(dimname,varname2[v2]) == 0) && 
			(ndims2 == 1)){

		      errstat = lnc_inq_dim(ncid,tmp_dim_ids[0],(char *)0, &tmp_sz);
		      if (errstat != NC_NOERR){
			sprintf (Msgt,"ncdds server: could not get dimension size for dimension %d in variable %d ",d,v2);
			ErrMsgT(Msgt);
			*error = (string)"\"" + (string)Msgt + (string)"\"";
			return errstat;
		      }
			if (tmp_sz == dim_sz){    
			    typ_match[dim_match] = nctype; 
			    var_match[dim_match] = varname2[v2]; 
			    dim_match++;
			    break; // Stop var search, matching variable 
			    // for the given dimension was found
			}
		    }
		}		
	
		// Stop dimensions search, the variable does not 
		// fit into a grid, due to a dimension mis-match.
		// Also, get the size for the remainder of the   
		// dimensions in the variable.
		if (dim_match != d+1) {
		    for (int d2 = d+1; d2 < ndims; ++d2){
		      errstat = lnc_inq_dim(ncid, dim_ids[d2], dimname2, &dim_sz);
		      if (errstat != NC_NOERR){
			sprintf (Msgt,"ncdds server: could not get dimension size for dimension %d in variable %d (ncdds)",d2,v1);
			    ErrMsgT(Msgt);
			    *error = (string)"\"" + (string)Msgt + (string)"\"";
			    return errstat;
			}
			dim_szs[d2] = (int) dim_sz;
			(void) strcpy(dim_nms[d2],dimname2);
		    }
		    break;
		}
	    }
      
	    // Create common array for both Array and Grid types
      
	    ar = dds_table.get_factory()->NewArray(varname1);
	    ar->add_var(bt);
	    for (d = 0; d < ndims; ++d) 
		ar->append_dim(dim_szs[d],dim_nms[d]);
      
#ifndef NOGRID
      
	    if (ndims == dim_match){   // Found Grid type, add it
		gr = dds_table.get_factory()->NewGrid(varname1);
		pr = array;
		gr->add_var(ar,pr);
		pr = maps;
		for ( d = 0; d < ndims; ++d){
		    ar = new NCArray; 
		    bt = Get_bt(dds_table.get_factory(), var_match[d],
				typ_match[d]);
		    ar->add_var(bt);     
		    ar->append_dim(dim_szs[d],dim_nms[d]);
		    gr->add_var(ar,pr);
		}
		dds_table.add_var(gr);
	    }
	    else {                    // must be an Array, add it
		dds_table.add_var(ar);
	    }
#else
      
	    dds_table.add_var(ar);
      
#endif
	}
    }
  
    return NC_NOERR;
}

// Given a reference to an instance of class DDS and a filename that refers
// to a netcdf file, read the netcdf file and extract all the dimensions of
// each of its variables. Add the variables and their dimensions to the
// instance of DDS.
//
// Returns: false if an error accessing the netcdf file was detected, true
// otherwise. 

void
read_descriptors(DDS &dds_table, const string &filename) throw (Error) 

{
  ncopts = 0;
  int ncid, errstat;
  int nvars; 
  
    errstat = lnc_open(filename.c_str(), NC_NOWRITE, &ncid);
    if (errstat != NC_NOERR) {
     //local error
      sprintf (Msgt,"netCDF server: Could not open file %s ", filename.c_str());
      ErrMsgT(Msgt); //local error messag
      string msg = (string)"Could not open " + path_to_filename(filename) + "."; 
      throw Error(errstat, msg);
     
     }
  
    // how many variables? 

    errstat = lnc_inq_nvars(ncid, &nvars);
    if (errstat != NC_NOERR) {
      ErrMsgT("Could not inquire about netcdf file (ncdds)");
      string msg = (string)"Could not inquire about netcdf file: " 
	+ path_to_filename(filename) + "."; 
      throw Error(errstat, msg);
    }
    // dataset name
    dds_table.set_dataset_name(name_path(filename));
  
    // read variables class
    string *error;

    errstat = read_class(dds_table,ncid,nvars,error);
    if (errstat != NC_NOERR) {
	string msg = (string) *error;
	throw Error(errstat, msg);
    }
}

#ifdef TEST

int
main(int argc, char *argv[])
{
  DDS dds;
  /*  
    if(!read_descriptors(dds, argv[1]))
    abort();
  */
  try {
      read_descriptors(dds, (string)argv[1]);
      dds.print();
    }
    catch (Error &e) {
      e.display_message();
      return 1;
    }

    //  dds.print();
}

#endif

// $Log: ncdds.cc,v $
// Revision 1.7  2005/03/31 00:04:51  jimg
// Modified to use the factory class in libdap++ 3.5.
//
// Revision 1.6  2003/12/08 18:06:37  edavis
// Merge release-3-4 into trunk
//
// Revision 1.5  2003/09/25 23:09:36  jimg
// Meerged from 3.4.1.
//
// Revision 1.4.4.1  2003/06/06 08:23:41  reza
// Updated the servers to netCDF-3 and fixed error handling between client and server.
//
// Revision 1.4  2003/01/28 07:08:24  jimg
// Merged with release-3-2-8.
//
// Revision 1.2.4.2  2002/12/18 23:40:33  pwest
// gcc3.2 compile corrections, mainly regarding using statements. Also,
// problems with multi line string literatls.
//
// Revision 1.3  2001/08/30 23:08:24  jimg
// Merged with 3.2.4
//
// Revision 1.2.4.1  2001/06/22 04:45:28  reza
// Added/Fixed exception handling.
//
// Revision 1.2  2000/10/06 01:22:03  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.1  1999/07/28 00:22:47  jimg
// Added
//
// Revision 1.15.2.1  1999/05/27 17:43:23  reza
// Fixed bugs in string changes
//
// Revision 1.15  1999/05/08 00:38:21  jimg
// Fixes for the String --> string changes
//
// Revision 1.14  1999/05/07 23:45:33  jimg
// String --> string fixes
//
// Revision 1.13  1999/04/08 13:45:52  reza
// Added support for more data types.
//
// Revision 1.12  1999/03/30 05:20:56  reza
// Added support for the new data types (Int16, UInt16, and Float32).
//
// Revision 1.11  1998/03/20 00:31:07  jimg
// Changed calls to the set_mime_*() to match the new parameters
//
// Revision 1.10  1997/03/10 16:24:23  reza
// Added error object and upgraded to DODS core release 2.12.
//
// Revision 1.9  1996/09/17 17:07:07  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.8.2.2  1996/07/10 21:44:45  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.8.2.1  1996/06/25 22:05:08  jimg
// Version 2.0 from Reza.
//
// Revision 1.7  1995/07/09  21:34:02  jimg
// Added copyright notice.
//
// Revision 1.6  1995/06/23  15:35:24  jimg
// Added netio.h.
// Fixed some misc problems which may have come about do to the modifications in
// nc_das.cc and nc_dds.cc (they went from CGIs to simple UNIX filters).
//
// Revision 1.5  1995/02/10  04:47:35  reza
// Updated to use type subclasses, NCArray, NCByte, NCInt32, NCGrid....
//
// Revision 1.4  1995/01/31  20:21:34  reza
//  Modified for new type subclassed DAP implementation.
//
// Revision 1.3  1994/12/22  04:51:44  reza
// Updated to use DODS new named dimension capability.
//
// Revision 1.2  1994/11/03  05:22:02  reza
// Added variable names to variable types Grid and Array.
//
// Revision 1.1  1994/10/28  14:34:03  reza
// First version
//
