
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

static char rcsid[] not_used ={"$Id: NCStructure.cc,v 1.6 2004/11/30 22:11:35 jimg Exp $"};

#include "InternalErr.h"

#include "NCStructure.h"
#include "NCArray.h"
#include "nc_util.h"

const string spr = "."; // structure rename

Structure *
NewStructure(const string &n)
{
    return new NCStructure(n);
}

// protected

void
NCStructure::_duplicate(const NCStructure &rhs)
{
}

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
    _duplicate(rhs);
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
        
    _duplicate(rhs);
    
    return *this;
}

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
        new_vars.splice(new_vars.end(), embedded_vars);
        ++field;
    }

    return new_vars;
}

// $Log: NCStructure.cc,v $
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
