
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

static char rcsid[] not_used ={"$Id: NCStructure.cc,v 1.13 2005/04/19 23:16:18 jimg Exp $"};

#include <algorithm>

#include <netcdf.h>
#include "InternalErr.h"

#include "NCStructure.h"
#if 0
#include "NCArray.h"
#include "nc_util.h"
#endif

#if 0
const string spr = "."; // structure rename

// protected

void
NCStructure::m_duplicate(const NCStructure &rhs)
{
    dynamic_cast<NCAccess&>(*this).clone(dynamic_cast<const NCAccess&>(rhs));      
}
#endif

BaseType *
NCStructure::ptr_duplicate()
{
    return new NCStructure(*this);
}

NCStructure::NCStructure(const string &n) : Structure(n)
{
}

NCStructure::NCStructure(const NCStructure &rhs) : Structure(rhs)
{
#if 0
    m_duplicate(rhs);
#endif
}

NCStructure::~NCStructure()
{
}

NCStructure &
NCStructure::operator=(const NCStructure &rhs)
{
    if (this == &rhs)
        return *this;

    dynamic_cast<Structure&>(*this) = rhs; // run Structure assignment
        
#if 0
    m_duplicate(rhs);
#endif
    
    return *this;
}

/** When flattening a Structure, make sure to add an attribute to the 
    new variables that indicate they are the product of translation. */

class AddAttribute: public unary_function<BaseType *, void> {
    
public:
    AddAttribute() {}

    void operator()(BaseType *a) {
        AttrTable *at;
        AttrTable::Attr_iter aiter;
        a->get_attr_table().find("translation", &at, &aiter);
        if (a->get_attr_table().attr_end() == aiter) {
            a->get_attr_table().append_attr("translation", "String",
                                            "\"flatten\"");
        }
    }
};

#if 0
VarList
NCStructure::flatten(const ClientParams &cp, const string &parent_name)
{
    Constructor::Vars_iter field = var_begin();
    Constructor::Vars_iter field_end = var_end();
    VarList new_vars;       // Store new vars here
    string local_name = (!parent_name.empty()) 
                        ? parent_name + spr + name()
                        : name();

    while (field != field_end) {
        VarList embedded_vars = dynamic_cast<NCAccess*>(*field)->flatten(cp, local_name);
        for_each(embedded_vars.begin(), embedded_vars.end(), AddAttribute());
        new_vars.splice(new_vars.end(), embedded_vars);
        
        // This mirrors code in NCConnect::translate_dds() _except_ that
        // here the code moves the attribute up one level while in NCConnect
        // the code moves the attribute into the global attribute table.
        string trans = (*field)->get_attr_table().get_attr("translation");
        if (!trans.empty())
            get_attr_table().append_attr("translation", "String", trans);
        
        ++field;
    }

    return new_vars;
}

BaseType *
NCStructure::find_child_sequence()
{
    Constructor::Vars_iter field = var_begin();
    Constructor::Vars_iter field_end = var_end();

    while (field != field_end) {
        if ((*field)->type() == dods_sequence_c)
            return (*field);
        
        // Depth-first search; see find_child_sequence(BaseType *parent)
        NCAccess *fld = dynamic_cast<NCAccess*>(*field);
        if (!fld)
            throw InternalErr(__FILE__, __LINE__, "Not an NCAccess!");
            
        BaseType *btp = fld->find_child_sequence();
        if (btp)
            return btp;
            
        ++field;
    }
    
    return 0;
}
#endif


// $Log: NCStructure.cc,v $
// Revision 1.13  2005/04/19 23:16:18  jimg
// Removed client side parts; the client library is now in libnc-dap.
//
// Revision 1.12  2005/04/11 18:38:20  jimg
// Fixed a problem with NCSequence where nested sequences were not flagged
// but instead were translated. The extract_values software cannot process a
// nested sequence yet. Now the code inserts an attribute that notes that a
// nested sequence has been elided.
//
// Revision 1.11  2005/04/08 17:08:47  jimg
// Removed old 'virtual ctor' functions which have now been replaced by the
// factory class code in libdap++.
//
// Revision 1.10  2005/04/07 23:35:36  jimg
// Changed the value of the translation attribute from "translated" to "flatten".
//
// Revision 1.9  2005/03/31 00:04:51  jimg
// Modified to use the factory class in libdap++ 3.5.
//
// Revision 1.8  2005/02/26 00:43:20  jimg
// Check point: This version of the CL can now translate strings from the
// server into char arrays. This is controlled by two things: First a
// compile-time directive STRING_AS_ARRAY can be used to remove/include
// this feature. When included in the code, only Strings associated with
// variables created by the translation process will be turned into char
// arrays. Other String variables are assumed to be single character strings
// (although there may be a bug with the way these are handled, see
// NCAccess::extract_values()).
//
// Revision 1.7  2005/01/26 23:25:51  jimg
// Implemented a fix for Sequence access by row number when talking to a
// 3.4 or earlier server (which contains a bug in is_end_of_rows()).
//
// Revision 1.6  2004/11/30 22:11:35  jimg
// I replaced the flatten_*() functions with a flatten() method in
// NCAccess. The default version of this method is in NCAccess and works
// for the atomic types; constructors must provide a specialization.
// Then I removed the code that copied the variables from vectors to
// lists. The translation code in NCConnect was modified to use the
// new method.
//
// Revision 1.5  2004/11/05 17:13:57  jimg
// Added code to copy the BaseType pointers from the vector container into
// a list. This will enable more efficient translation software to be
// written.
//
// Revision 1.4  2004/09/08 22:08:22  jimg
// More Massive changes: Code moved from the files that clone the netCDF
// function calls into NCConnect, NCAccess or nc_util.cc. Much of the
// translation functions are now methods. The netCDF type classes now
// inherit from NCAccess in addition to the DAP type classes.
//
// Revision 1.3  2003/12/08 18:06:37  edavis
// Merge release-3-4 into trunk
//
// Revision 1.2  2000/10/06 01:22:02  jimg
// Moved the CVS Log entries to the ends of files.
// Modified the read() methods to match the new definition in the dap library.
// Added exception handlers in various places to catch exceptions thrown
// by the dap library.
//
// Revision 1.1  1999/07/28 00:22:44  jimg
// Added
//
// Revision 1.6  1999/05/07 23:45:32  jimg
// String --> string fixes
//
// Revision 1.5  1998/08/06 16:33:25  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.4  1996/09/17 17:06:43  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.3.4.3  1996/09/17 00:26:32  jimg
// Merged changes from a side branch which contained various changes from
// Reza and Charles.
// Removed ncdump and netexec since ncdump is now in its own directory and
// netexec is no longer used.
//
// Revision 1.3.4.2  1996/07/10 21:44:25  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.3.4.1  1996/06/25 22:04:46  jimg
// Version 2.0 from Reza.
//
// Revision 1.3  1995/07/09  21:33:51  jimg
// Added copyright notice.
//
// Revision 1.2  1995/03/16  16:56:45  reza
// Updated for the new DAP. All the read_val mfunc. and their memory management
// are now moved to their parent class.
// Data transfers are now in binary while DAS and DDS are still sent in ASCII.
//
// Revision 1.1  1995/02/10  04:57:53  reza
// Added read and read_val functions.
