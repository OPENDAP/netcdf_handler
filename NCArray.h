
// -*- C++ -*-

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

#ifndef _ncarray_h
#define _ncarray_h 1

#ifndef __POWERPC__
#ifdef __GNUG__
//#pragma interface
#endif
#endif

#include "Array.h"
#include "NCAccess.h"

extern Array * NewArray(const string &n, BaseType *v);

class NCArray: public Array, public NCAccess {
    BaseType *d_source;       /// Reference to source var if translated

    void _duplicate(const NCArray &nca);
    
public:
    NCArray(const string &n = "", BaseType *v = 0);
    NCArray(const NCArray &nc_array);
    NCArray &operator=(const NCArray &rhs);
    virtual ~NCArray();

    virtual BaseType *ptr_duplicate();

    virtual bool read(const string &dataset);
    
    virtual long format_constraint(size_t *cor, ptrdiff_t *step, size_t *edg, 
			bool *has_stride);

    virtual string build_constraint(int outtype, const size_t *start,
            const size_t *edges, const ptrdiff_t *stride) throw(Error);
            
    virtual bool is_convertable(int outtype);
    virtual nc_type get_nc_type() throw(InternalErr);
    
    virtual void extract_values(void *values, int outtype) throw(Error);
    virtual BaseType *get_source();
    virtual void set_source(BaseType *s) throw(InternalErr);
};

/* 
 * $Log: NCArray.h,v $
 * Revision 1.8  2004/10/28 16:38:19  jimg
 * Added support for error handling to ClientParams. Added use of
 * ClientParams to NCConnect, although that's not complete yet. NCConnect
 * now has an instance of ClientParams. The instance is first built and
 * then passed into NCConnect's ctor which stores a const reference to the CP
 * object.
 *
 * Revision 1.7  2004/10/22 21:51:34  jimg
 * More massive changes: Translation of Sequences now works so long as the
 * Sequence contains only atomic types.
 *
 * Revision 1.6  2004/09/08 22:08:21  jimg
 * More Massive changes: Code moved from the files that clone the netCDF
 * function calls into NCConnect, NCAccess or nc_util.cc. Much of the
 * translation functions are now methods. The netCDF type classes now
 * inherit from NCAccess in addition to the DAP type classes.
 *
 * Revision 1.5  2003/12/08 18:06:37  edavis
 * Merge release-3-4 into trunk
 *
 * Revision 1.4  2003/09/25 23:09:36  jimg
 * Meerged from 3.4.1.
 *
 * Revision 1.3.4.2  2003/06/24 11:36:32  rmorris
 * Removed #pragma interface directives for the OS X.
 *
 * Revision 1.3.4.1  2003/06/07 22:02:32  reza
 * Fixed char vs. byte and long vs. nclong error checks.
 *
 * Revision 1.3  2002/05/03 00:01:52  jimg
 * Merged with release-3-2-7.
 *
 * Revision 1.2.4.1  2001/12/26 03:32:24  rmorris
 * Removed redundant default args.  VC++ only allows them to be specified
 * a single time.
 *
 * Revision 1.2  2000/10/06 01:22:02  jimg
 * Moved the CVS Log entries to the ends of files.
 * Modified the read() methods to match the new definition in the dap library.
 * Added exception handlers in various places to catch exceptions thrown
 * by the dap library.
 *
 * Revision 1.1  1999/07/28 00:22:42  jimg
 * Added
 *
 * Revision 1.5  1999/05/07 23:45:31  jimg
 * String --> string fixes
 *
 * Revision 1.4  1996/09/17 17:06:17  jimg
 * Merge the release-2-0 tagged files (which were off on a branch) back into
 * the trunk revision.
 *
 * Revision 1.3.4.2  1996/07/10 21:43:55  jimg
 * Changes for version 2.06. These fixed lingering problems from the migration
 * from version 1.x to version 2.x.
 * Removed some (but not all) warning generated with gcc's -Wall option.
 *
 * Revision 1.3.4.1  1996/06/25 22:04:19  jimg
 * Version 2.0 from Reza.
 *
 * Revision 1.3  1995/03/21  20:58:16  jimg
 * Resolved conflicts between my (jhrg) files and those checked in by Reza.
 *
 * Revision 1.2  1995/03/16  16:56:26  reza
 * Updated for the new DAP. All the read_val mfunc. and their memory management
 * are now moved to their parent class.
 * Data transfers are now in binary while DAS and DDS are still sent in ASCII.
 *
 * Revision 1.1  1995/02/10  04:57:15  reza
 * Added read and read_val functions.
 */

#endif


