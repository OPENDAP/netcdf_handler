// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// (c) COPYRIGHT URI/MIT 1999
// Please read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//      jhrg,jimg       James Gallagher <jgallagher@gso.uri.edu>


// These functions are used by the CE evaluator
//
// 1/15/99 jhrg

#include "config.h"

#include <limits.h>

#include <cstdlib>      // used by strtod()
#include <cerrno>
#include <cmath>
#include <iostream>
#include <sstream>

//#define DODS_DEBUG

#include "BaseType.h"
#include "Byte.h"
#include "Int16.h"
#include "UInt16.h"
#include "Int32.h"
#include "UInt32.h"
#include "Float32.h"
#include "Float64.h"
#include "Str.h"
#include "Url.h"
#include "Array.h"
#include "Structure.h"
#include "Sequence.h"
#include "Grid.h"
#include "Error.h"

#include "debug.h"
#include "util.h"

#include <gridfields/restrict.h>
#include <gridfields/gridfield.h>
#include <gridfields/grid.h>
#include <gridfields/cell.h>
#include <gridfields/cellarray.h>
#include <gridfields/array.h>
#include <gridfields/implicit0cells.h>
#include <gridfields/gridfieldoperator.h>

//  We wrapped VC++ 6.x strtod() to account for a short coming
//  in that function in regards to "NaN".  I don't know if this
//  still applies in more recent versions of that product.
//  ROM - 12/2007
#ifdef WIN32
#include <limits>
double w32strtod(const char *, char **);
#endif

using namespace std;

namespace netcdf_handler {

/** Is \e lhs equal to \e rhs? Use epsilon to determine equality. */
inline bool double_eq(double lhs, double rhs, double epsilon = 1.0e-5)
{
    return fabs(lhs - rhs) < epsilon;
}

/** Given a BaseType pointer, extract the string value it contains and return
 it.

 @param arg The BaseType pointer
 @return A C++ string
 @exception Error thrown if the referenced BaseType object does not contain
 a DAP String. */
string extract_string_argument(BaseType * arg)
{
    if (arg->type() != dods_str_c)
        throw Error(malformed_expr,
                "The function requires a DAP string argument.");

    if (!arg->read_p())
        throw InternalErr(__FILE__, __LINE__,
                "The CE Evaluator built an argument list where some constants held no values.");

    string s = dynamic_cast<Str&>(*arg).value();

    DBG(cerr << "s: " << s << endl);

    return s;
}

template<class T> static void set_array_using_double_helper(Array * a,
        double *src, int src_len)
{
    T *values = new T[src_len];
    for (int i = 0; i < src_len; ++i)
        values[i] = (T) src[i];

    a->set_value(values, src_len);

    delete[]values;
}

/** Given an array that holds some sort of numeric data, load it with values
 using an array of doubles. This function makes several assumptions. First,
 it assumes the caller really wants to put the doubles into whatever types
 the array holds! Caveat emptor. Second, it assumes that if the size of
 source (\e src) array is different than the destination (\e dest) the
 caller has made a mistake. In that case it will throw an Error object.

 After setting the values, this method sets the \c read_p property for
 \e dest. Setting \e read_p tells the serialization methods in libdap
 that this variable already holds data values and, given that, the
 serialization code will not try to read the values.

 @param dest An Array. The values are written to this array, reusing
 its storage. Existing values are lost.
 @param src The source data.
 @param src_len The number of elements in the \e src array.
 @exception Error Thrown if \e dest is not a numeric-type array (Byte, ...,
 Float64) or if the number of elements in \e src does not match the number
 is \e dest. */
void set_array_using_double(Array * dest, double *src, int src_len)
{
    // Simple types are Byte, ..., Float64, String and Url.
    if ((dest->type() == dods_array_c && !dest->var()->is_simple_type()) 
	|| dest->var()->type() == dods_str_c 
	|| dest->var()->type() == dods_url_c)
        throw InternalErr(__FILE__, __LINE__,
                "The function requires a DAP numeric-type array argument.");

    // Test sizes. Note that Array::length() takes any constraint into account
    // when it returns the length. Even if this was removed, the 'helper'
    // function this uses calls Vector::val2buf() which uses Vector::width()
    // which in turn uses length().
    if (dest->length() != src_len)
        throw InternalErr(__FILE__, __LINE__,
                "The source and destination array sizes don't match ("
                + long_to_string(src_len) + " versus "
                + long_to_string(dest->length()) + ").");

    // The types of arguments that the CE Parser will build for numeric
    // constants are limited to Uint32, Int32 and Float64. See ce_expr.y.
    // Expanded to work for any numeric type so it can be used for more than
    // just arguments.
    switch (dest->var()->type()) {
    case dods_byte_c:
        set_array_using_double_helper<dods_byte>(dest, src, src_len);
        break;
    case dods_uint16_c:
        set_array_using_double_helper<dods_uint16>(dest, src, src_len);
        break;
    case dods_int16_c:
        set_array_using_double_helper<dods_int16>(dest, src, src_len);
        break;
    case dods_uint32_c:
        set_array_using_double_helper<dods_uint32>(dest, src, src_len);
        break;
    case dods_int32_c:
        set_array_using_double_helper<dods_int32>(dest, src, src_len);
        break;
    case dods_float32_c:
        set_array_using_double_helper<dods_float32>(dest, src, src_len);
        break;
    case dods_float64_c:
        set_array_using_double_helper<dods_float64>(dest, src, src_len);
        break;
    default:
        throw InternalErr(__FILE__, __LINE__,
                "The argument list built by the CE parser contained an unsupported numeric type.");
    }

    // Set the read_p property.
    dest->set_read_p(true);
}

template<typename DODS, typename T> T *extract_array_helper(Array *a) 
{
  DBG(cerr << "Extracting array values..." << endl);
  int length = a->length();

  DBG(cerr << "Allocating..." << length << endl);
  DODS *b = new DODS[length];
  DBG(cerr << "Assigning value..." << endl);
  a->value(b);
  DBG(cerr << "array values extracted.  Casting..." << endl);
  T *dest = new T[length];
  for (int i = 0; i < length; ++i)
    dest[i] = (T) b[i];
  delete[]b;
  DBG(cerr << "Returning extracted values." << endl);

  return dest;
}

GF::Array *extract_gridfield_array(Array *a) {
    if ((a->type() == dods_array_c && !a->var()->is_simple_type())
  || a->var()->type() == dods_str_c || a->var()->type() == dods_url_c)
        throw Error(malformed_expr,
                "The function requires a DAP numeric-type array argument.");

    a->set_send_p(true);
    a->read();

    // Construct a GridField array from a DODS array
    GF::Array *gfa;

    switch (a->var()->type()) {
    case dods_byte_c:
       gfa = new GF::Array(a->var()->name(), GF::INT);
       gfa->shareIntData(extract_array_helper<dods_byte, int>(a), a->length());
       break;
    case dods_uint16_c:
       gfa = new GF::Array(a->var()->name(), GF::INT);
       gfa->shareIntData(extract_array_helper<dods_uint16, int>(a), a->length());
       break;
    case dods_int16_c:
       gfa = new GF::Array(a->var()->name(), GF::INT);
       gfa->shareIntData(extract_array_helper<dods_int16, int>(a), a->length());
       break;
    case dods_uint32_c:
       gfa = new GF::Array(a->var()->name(), GF::INT);
       gfa->shareIntData(extract_array_helper<dods_uint32, int>(a), a->length());
       break;
    case dods_int32_c:
       gfa = new GF::Array(a->var()->name(), GF::INT);
       gfa->shareIntData(extract_array_helper<dods_int32, int>(a), a->length());
       break;
    case dods_float32_c:
       gfa = new GF::Array(a->var()->name(), GF::FLOAT);
       gfa->shareFloatData(extract_array_helper<dods_float32, float>(a), a->length());
       break;
    case dods_float64_c:
       gfa = new GF::Array(a->var()->name(), GF::FLOAT);
       gfa->shareFloatData(extract_array_helper<dods_float64, float>(a), a->length());
       break;
    default:
        throw InternalErr(__FILE__, __LINE__,
                "Unknown DDS type encountered when converting to gridfields array");
    }
    return gfa;
};

/*
If the array has the exact dimensions in the vector dims, in the same order, 
return true.  Otherwise return false. 

*/
bool same_dimensions(Array *arr, vector<Array::dimension> &dims) {
  vector<Array::dimension>::iterator dit;
  Array::Dim_iter ait;
  DBG(cerr << "same_dimensions test for array " << arr->name() << endl);
  DBG(cerr << << "  array dims: ");
  for (ait = arr->dim_begin(); ait!=arr->dim_end(); ++ait) {
    DBG(cerr << (*ait).name << ", ");
  }
  DBG(cerr << endl);
  DBG(cerr << "  rank dims: ");
  for (dit = dims.begin(); dit!=dims.end(); ++dit) {
    DBG(cerr << (*dit).name << ", " << endl);
    for (ait = arr->dim_begin(); ait!=arr->dim_end(); ++ait) {
      Array::dimension dd = *dit;
      Array::dimension ad = *ait;
      if (dd.name != ad.name 
       or dd.size != ad.size 
       or dd.stride != ad.stride
       or dd.stop != ad.stop) 
       return false;
    }
    DBG(cerr << endl);
  }
  return true;
}

/** Given a pointer to an Array which holds a numeric type, extract the
 values and return in an array of T. This function allocates the
 array using 'new T[n]' so delete[] can be used when you are done
 the data. */
template<typename T>
T *extract_array(Array * a)
{
    // Simple types are Byte, ..., Float64, String and Url.
    if ((a->type() == dods_array_c && !a->var()->is_simple_type())
  || a->var()->type() == dods_str_c || a->var()->type() == dods_url_c)
        throw Error(malformed_expr,
                "The function requires a DAP numeric-type array argument.");

    a->set_send_p(true);
    a->read();
    // This test should never pass due to the previous two lines; 
    // reading here seems to make 
    // sense rather than letting the caller forget to do so.
    // is read() idemopotent?
    if (!a->read_p())
        throw InternalErr(__FILE__, __LINE__,
                string("The Array '") + a->name() +
                "'does not contain values. send_read_p() not called?");

    // The types of arguments that the CE Parser will build for numeric
    // constants are limited to Uint32, Int32 and Float64. See ce_expr.y.
    // Expanded to work for any numeric type so it can be used for more than
    // just arguments.
    switch (a->var()->type()) {
    case dods_byte_c:
        return extract_array_helper<dods_byte, T>(a);
    case dods_uint16_c:
        DBG(cerr << "dods_uint32_c" << endl);
        return extract_array_helper<dods_uint16, T>(a);
    case dods_int16_c:
        DBG(cerr << "dods_int16_c" << endl);
        return extract_array_helper<dods_int16, T>(a);
    case dods_uint32_c:
        DBG(cerr << "dods_uint32_c" << endl);
        return extract_array_helper<dods_uint32, T>(a);
    case dods_int32_c:
        DBG(cerr << "dods_int32_c" << endl);
        return extract_array_helper<dods_int32, T>(a);
    case dods_float32_c:
        DBG(cerr << "dods_float32_c" << endl);
        return extract_array_helper<dods_float32, T>(a);
    case dods_float64_c:
        DBG(cerr << "dods_float64_c" << endl);
        return extract_array_helper<dods_float64, T>(a);
    default:
        throw InternalErr(__FILE__, __LINE__,
                "The argument list built by the CE parser contained an unsupported numeric type.");
    }
}

template<class T> static double *extract_double_array_helper(Array * a)
{
    int length = a->length();

    T *b = new T[length];
    a->value(b);

    double *dest = new double[length];
    for (int i = 0; i < length; ++i)
        dest[i] = (double) b[i];
    delete[]b;

    return dest;
}

/** Given a pointer to an Array which holds a numeric type, extract the
 values and return in an array of doubles. This function allocates the
 array using 'new double[n]' so delete[] can be used when you are done
 the data. */
double *extract_double_array(Array * a)
{
    // Simple types are Byte, ..., Float64, String and Url.
    if ((a->type() == dods_array_c && !a->var()->is_simple_type())
	|| a->var()->type() == dods_str_c || a->var()->type() == dods_url_c)
        throw Error(malformed_expr,
                "The function requires a DAP numeric-type array argument.");

    if (!a->read_p())
        throw InternalErr(__FILE__, __LINE__,
                string("The Array '") + a->name() +
                "'does not contain values.");

    // The types of arguments that the CE Parser will build for numeric
    // constants are limited to Uint32, Int32 and Float64. See ce_expr.y.
    // Expanded to work for any numeric type so it can be used for more than
    // just arguments.
    switch (a->var()->type()) {
    case dods_byte_c:
        return extract_double_array_helper<dods_byte>(a);
    case dods_uint16_c:
        return extract_double_array_helper<dods_uint16>(a);
    case dods_int16_c:
        return extract_double_array_helper<dods_int16>(a);
    case dods_uint32_c:
        return extract_double_array_helper<dods_uint32>(a);
    case dods_int32_c:
        return extract_double_array_helper<dods_int32>(a);
    case dods_float32_c:
        return extract_double_array_helper<dods_float32>(a);
    case dods_float64_c:
        return extract_double_array_helper<dods_float64>(a);
    default:
        throw InternalErr(__FILE__, __LINE__,
                "The argument list built by the CE parser contained an unsupported numeric type.");
    }
}

/** Given a BaseType pointer, extract the numeric value it contains and return
 it in a C++ double.

 @param arg The BaseType pointer
 @return A C++ double
 @exception Error thrown if the referenced BaseType object does not contain
 a DAP numeric value. */
double extract_double_value(BaseType * arg)
{
    // Simple types are Byte, ..., Float64, String and Url.
    if (!arg->is_simple_type() || arg->type() == dods_str_c || arg->type()
            == dods_url_c)
        throw Error(malformed_expr,
                "The function requires a DAP numeric-type argument.");

    if (!arg->read_p())
        throw InternalErr(__FILE__, __LINE__,
                "The CE Evaluator built an argument list where some constants held no values.");

    // The types of arguments that the CE Parser will build for numeric
    // constants are limited to Uint32, Int32 and Float64. See ce_expr.y.
    // Expanded to work for any numeric type so it can be used for more than
    // just arguments.
    switch (arg->type()) {
    case dods_byte_c:
        return (double)(dynamic_cast<Byte&>(*arg).value());
    case dods_uint16_c:
        return (double)(dynamic_cast<UInt16&>(*arg).value());
    case dods_int16_c:
        return (double)(dynamic_cast<Int16&>(*arg).value());
    case dods_uint32_c:
        return (double)(dynamic_cast<UInt32&>(*arg).value());
    case dods_int32_c:
        return (double)(dynamic_cast<Int32&>(*arg).value());
    case dods_float32_c:
        return (double)(dynamic_cast<Float32&>(*arg).value());
    case dods_float64_c:
        return dynamic_cast<Float64&>(*arg).value();
    default:
        throw InternalErr(__FILE__, __LINE__,
                "The argument list built by the CE parser contained an unsupported numeric type.");
    }
}
// These static functions could be moved to a class that provides a more
// general interface for COARDS/CF someday. Assume each BaseType comes bundled
// with an attribute table.

// This was ripped from parser-util.cc
static double string_to_double(const char *val)
{
    char *ptr;
    errno = 0;
    // Clear previous value. 5/21/2001 jhrg

#ifdef WIN32
    double v = w32strtod(val, &ptr);
#else
    double v = strtod(val, &ptr);
#endif

    if ((v == 0.0 && (val == ptr || errno == HUGE_VAL || errno == ERANGE))
            || *ptr != '\0') {
        throw Error(malformed_expr,string("Could not convert the string '") + val + "' to a double.");
    }

    double abs_val = fabs(v);
    if (abs_val > DODS_DBL_MAX || (abs_val != 0.0 && abs_val < DODS_DBL_MIN))
        throw Error(malformed_expr,string("Could not convert the string '") + val + "' to a double.");

    return v;
}

/** Look for any one of a series of attribute values in the attribute table
 for \e var. This function treats the list of attributes as if they are ordered
 from most to least likely/important. It stops when the first of the vector of
 values is found. If the variable (var) is a Grid, this function also looks
 at the Grid's Array for the named attributes. In all cases it returns the
 first value found.
 @param var Look for attributes in this BaseType variable.
 @param attributes A vector of attributes; the first one found will be returned.
 @return The attribute value in a double. */
static double get_attribute_double_value(BaseType *var,
        vector<string> &attributes)
{
    // This code also builds a list of the attribute values that have been
    // passed in but not found so that an informative message can be returned.
    AttrTable &attr = var->get_attr_table();
    string attribute_value = "";
    string values = "";
    vector<string>::iterator i = attributes.begin();
    while (attribute_value == "" && i != attributes.end()) {
        values += *i;
        if (!values.empty())
            values += ", ";
        attribute_value = attr.get_attr(*i++);
    }

    // If the value string is empty, then look at the grid's array (if it's a
    // grid) or throw an Error.
    if (attribute_value.empty()) {
        if (var->type() == dods_grid_c)
            return get_attribute_double_value(dynamic_cast<Grid&>(*var).get_array(), attributes);
        else
            throw Error(malformed_expr,string("No COARDS/CF '") + values.substr(0, values.length() - 2)
                    + "' attribute was found for the variable '"
                    + var->name() + "'.");
    }

    return string_to_double(remove_quotes(attribute_value).c_str());
}

static double get_attribute_double_value(BaseType *var, const string &attribute)
{
    AttrTable &attr = var->get_attr_table();
    string attribute_value = attr.get_attr(attribute);

    // If the value string is empty, then look at the grid's array (if it's a
    // grid or throw an Error.
    if (attribute_value.empty()) {
        if (var->type() == dods_grid_c)
            return get_attribute_double_value(dynamic_cast<Grid&>(*var).get_array(), attribute);
        else
            throw Error(malformed_expr,string("No COARDS '") + attribute
                    + "' attribute was found for the variable '"
                    + var->name() + "'.");
    }

    return string_to_double(remove_quotes(attribute_value).c_str());
}

/** This is a stub Constraint Expression (i.e., server-side) function
    that will evolve into an interface for Unstructured Grid
    operations. 

    The function takes a single variable that should be a one
    dimensional array variable and scales it by a factor of ten. The 
    result is returned as the value of the CE expression.

    @param argc Count of the function's arguments
    @param argv Array of pointers to the functions arguments
    @param dds Reference to the DDS object for the complete dataset.
    This holds pointers to all of the variables and attributes in the
    dataset. 
    @param btpp Return the function result in an instance of BaseType
    referenced by this pointer to a pointer. We could have used a
    BaseType reference, instead of pointer to a pointer, but we didn't.
    This is a value-result parameter.

    @return void

    @exception Error Thrown If the Array is not a one dimensional
    array. */
void
function_ugrid_demo(int argc, BaseType * argv[], DDS &dds, BaseType **btpp)
{

    // This is the nascent on-line help for these functions. 
    static string info =
	string("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n") +
	"<function name=\"ugrid_demo\" version=\"0.1\">\n" +
	"Fledgling code for Unstructured grid operations.\n" +
	"</function>";
    
    if (argc == 0) {
        Str *response = new Str("info");
        response->set_value(info);
        *btpp = response;
        return;
    }

    // Check number of arguments; DBG is a macro. Use #define
    // DODS_DEBUG to activate the debugging stuff.
    if (argc != 2)
        throw Error(malformed_expr,"Wrong number of arguments to ugrid_demo. ugrid_demo(dim:int32, condition:string); was passed " + long_to_string(argc) + " argument(s)");

    //FIXME Neither of the input parameters are used.
    if (argv[0]->type() != dods_int32_c) {
        throw Error(malformed_expr,"Wrong type for first argument. ugrid_demo(dim:int32, condition:string); was passed a/an " + argv[1]->type_name());
    }
    if (argv[1]->type() != dods_str_c) {
        throw Error(malformed_expr,"Wrong type for second argument. ugrid_demo(dim:int32, condition:string); was passed a/an " + argv[1]->type_name());
    }

    BaseType *result = 0;		// This will hold the result
    
    // keep track of which DDS dimensions correspond to GF dimensions
    map<GF::Dim_t, vector<Array::dimension> > rank_dimensions; 

    GF::Grid *G = new GF::Grid("result");
  
    // 1) Find the nodes
    DBG(cerr << "Reading 0-cells" << endl);
    GF::AbstractCellArray *nodes = NULL;
    for (DDS::Vars_iter vi = dds.var_begin(); vi != dds.var_end(); vi++) {
      BaseType *bt = *vi;
      Array &arr = dynamic_cast<Array&>(*bt);
      AttrTable &at = arr.get_attr_table();
      DBG(cerr << "Array: " << arr.name() << endl);

      int node_count = -1;  //error condition
      AttrTable::Attr_iter loc = at.simple_find("grid_location");
      if (loc != at.attr_end()) {
        if (at.get_attr(loc, 0) == "node") {
          node_count = 1; 
          Array::Dim_iter di = arr.dim_begin();
          DBG(cerr << "Interpreting 0-cells from dimensions: ");
          rank_dimensions[0] = vector<Array::dimension>();
          for (Array::Dim_iter di = arr.dim_begin(); di!= arr.dim_end(); di++) {
            // These dimensions define the nodes.
            DBG(cerr << di->name << ", ");
            rank_dimensions[0].push_back(*di);
            node_count *= di->c_size;
          }
          DBG(cerr << endl);
        } // Bound to nodes?  
      } // Has a "grid_location" attribute?
      /* 
We should use implicit0cells here, but that's limited to 
cell ids that are numbered from zero.  The class can 
and should be generalized, but I don't want to get 
distracted fixing it.  
Instead, we materialize the nodes as explciit cells.  Wastes memory.

Update: We do use implicit0cells, noting that the index_origin attribute
is currently only defined for k-cells, where k>0.
For now, we do NOT preserve index_origin the result.
We can therefore guarantee that nodes are numbered from 0.
       */  
      nodes = new GF::Implicit0Cells(node_count);
/*

      nodes = new GF::CellArray();
      for (int i=index_origin; i<node_count; i++) {
        // insert a cell of one node with id i
        nodes->addCellNodes(&i, 1);
      }
*/    

      // We've figured out which dimensions are associated with
      // the nodes, so stop 
      // TODO: Assumes only one grid per file!
      break; 
    }

    // Attach the nodes to the grid
    G->setKCells(nodes, 0);

    // 2) For each k, find the k-cells
    // k = 2, for now
    DBG(cerr << "Reading 2-cells" << endl);
    GF::CellArray *twocells = NULL;
    for (DDS::Vars_iter vi = dds.var_begin(); vi != dds.var_end(); vi++) {
      BaseType *bt = *vi;
      Array &arr = dynamic_cast<Array&>(*bt);
      DBG(cerr << "Array: " << arr.name() << endl);
      AttrTable &at = arr.get_attr_table();

      AttrTable::Attr_iter iter_cell_type = at.simple_find("cell_type");

      if (iter_cell_type != at.attr_end()) {        
        string cell_type = at.get_attr(iter_cell_type, 0);
        DBG(cerr << cell_type << endl);
        if (cell_type == "tri_ccw") {
          // Ok, we expect triangles
          // which means a shape of 3xN
          int twocell_count = -1, i=0;
          int total_size = 1;
          rank_dimensions[2] = vector<Array::dimension>();
          for (Array::Dim_iter di = arr.dim_begin(); di!= arr.dim_end(); di++) {
            total_size *= di->c_size;
            rank_dimensions[2].push_back(*di);
            if (i == 0) {
              if (di->c_size != 3) {
                DBG(cerr << "Cell array of type 'tri_ccw' must have a shape of 3xN, since triangles have three nodes." << endl);
                throw Error(malformed_expr,"Cell array of type 'tri_ccw' must have a shape of 3xN, since triangles have three nodes.");
              }
            }
            if (i == 1) {
              twocell_count = di->c_size;
            }
            if (i>1) {
              DBG(cerr << "Too many dimensions for a cell array of type 'tri_ccw'.  Expected shape of 3XN" << endl);
              throw Error(malformed_expr,"Too many dimensions for a cell array of type 'tri_ccw'.  Expected shape of 3XN");
            }
            i++;
          }

          // interpret the array data as triangles
          GF::Node *cellids = extract_array<GF::Node>(&arr);
         
          // adjust for index origin
          AttrTable::Attr_iter iter_index_origin = at.simple_find("index_origin");
          if (iter_index_origin != at.attr_end()) {
            DBG(cerr << "Found an index origin attribute." << endl);
            AttrTable::entry *index_origin_entry = *iter_index_origin;
            int index_origin;
            if (index_origin_entry->attr->size() == 1) {
              AttrTable::entry *index_origin_entry = *iter_index_origin;
              string val = (*index_origin_entry->attr)[0];
              DBG(cerr << "Value: " << val << endl);
              stringstream buffer(val);
              // what happens if string cannot be converted to an integer?
              buffer >> index_origin;
              DBG(cerr << "converted: " << index_origin << endl);
              if (index_origin != 0) {
                for (int j=0; j<total_size; j++) {
                  cellids[j] -= index_origin;
                }
              }
            } else {
              throw Error(malformed_expr,"Index origin attribute exists, but either no value supplied, or more than one value supplied.");
            }
          }

          // Create the cell array
          twocells = new GF::CellArray(cellids, twocell_count, 3);
          // Attach it to the grid
          G->setKCells(twocells, 2);
        }
      }

    }
 
    // 3) For each var, bind it to the appropriate dimension

    // For each variable in the data source:
    GF::GridField *input = new GF::GridField(G);
    for (DDS::Vars_iter vi = dds.var_begin(); vi != dds.var_end(); vi++) {
      BaseType *bt = *vi;
      if (bt->type() == dods_array_c) {
        Array *arr = (Array *)bt;
        DBG(cerr << "Data Array: " << arr->name() << endl);
        // Each rank is associated with a sequence of dimensions
        // Vars that have the same dimensions should be bound to the grid at that rank
        // (Note that in gridfields, Dimension and rank are synonyms.  We
        // use the latter here to avoid confusion).
        map<GF::Dim_t, vector<Array::dimension> >::iterator iter;
        for( iter = rank_dimensions.begin(); iter != rank_dimensions.end(); ++iter ) {
          bool same = same_dimensions(arr, iter->second);
          if (same) {
            // This var should be bound to rank k
            GF::Array *gfa = extract_gridfield_array(arr);
            DBG(cerr << "Adding Attribute: " << gfa->sname() << endl);
            input->AddAttribute(iter->first, gfa);
          } else {
            //This array does not appear to be associated with any rank of the unstructured grid.  Ignore for now.  Anything else we should do?
          }
        }
      } // Ignore if not an array type.  Anything else we should do?
    }
    input->print();
/*
    input->print();
    GF::GridFieldOperator *op = GF::RestrictOp(expr, dim, input);
    GF::GridField *R = op->getResult(); 
        
    R->print();
*/

    // 4) Convert back to a DDS BaseType

    /*
    // Create variables for each cell dimension
    // Create variables for each attribute at each rank
    */

#if 1
    //FIXME HACKHACKHACK
    result = new Str("result");
    string result_string = "The gridfields function is not complete yet.";
    dynamic_cast<Str&>(*result).set_value(result_string);
#endif

    *btpp = result;
    return;
}

} // namespace libdap
