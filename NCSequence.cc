
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

static char rcsid[] not_used ={"$Id: NCSequence.cc,v 1.8 2005/01/26 23:25:51 jimg Exp $"};

#include <sstream>
#include <algorithm>

// #define DODS_DEBUG 1

#include "InternalErr.h"
#include "debug.h"

#include "NCSequence.h"
#include "NCArray.h"
#include "NCAccess.h"
#include "ClientParams.h"
#include "nc_util.h"

const string spr = ".";

Sequence *
NewSequence(const string &n)
{
    return new NCSequence(n);
}

// protected

void
NCSequence::m_duplicate(const NCSequence &rhs)
{
    d_size = rhs.d_size;

    dynamic_cast<NCAccess&>(*this).clone(dynamic_cast<const NCAccess&>(rhs));
    
}

// public

BaseType *
NCSequence::ptr_duplicate()
{
    return new NCSequence(*this);
}

NCSequence::NCSequence(const string &n) : Sequence(n)
{
}

NCSequence::NCSequence(const NCSequence &rhs) : Sequence(rhs)
{
    m_duplicate(rhs);
}

NCSequence::~NCSequence()
{
}

NCSequence &
NCSequence::operator=(const NCSequence &rhs)
{
    if (this == &rhs)
        return *this;

    dynamic_cast<Sequence &>(*this) = rhs; // run Sequence assignment
        
    m_duplicate(rhs);
    
    return *this;
}

/** Build a constraint for a Sequence. Build a CE for then entire Sequence 
    even when the client asks for just one field. A bug in the Sequence CE
    processing code. */
string
NCSequence::build_constraint(int outtype, const size_t *start,
    const size_t *edges, const ptrdiff_t *stride) throw(Error)
{
    string expr = name();      // CE always starts with the variable's name

    // Get dimension sizes and strings for constraint hyperslab
    ostringstream ce;       // Build CE here.
    // Note that a Sequence is always a one dimensional thing.
    int dm = 0;             // Index to start, edge, stride arrays 
    //int Zedge = 0;          // Search for zero size edges (no data)

    // Verify stride argument.
    if (stride != NULL && stride[dm] < 1)
        throw Error(NC_ESTRIDE, "Stride less than 1 (one).");

    // Set defaults:
    // *start         NULL => first corner 
    // *edges         NULL => everything following start[] 
    // *stride        NULL => unity strides 
    long instart = start != NULL ? start[dm] : 0;
    long inedges = edges != NULL ? edges[dm] : get_size() - instart;
    long instep = stride != NULL ? stride[dm] : 1;

    // external constraint (from ncopen)
    int ext_start = get_starting_row_number(); 
    int ext_step = get_row_stride();
    int ext_stop = get_ending_row_number();

    string version_info = get_implementation_version();

    version_info.replace(version_info.find("/"), 1, " ");

    istringstream iss(version_info);
    string implementation;
    float version;
    iss >> implementation;
    iss >> version;
        
    DBG(cerr << instart <<" "<< inedges <<" "<< dm << endl);
    DBG(cerr<< ext_start <<" "<< ext_step <<" " << ext_stop << endl);

    // create constraint expr. by combining the constraints
    int Tstart = ext_start + instart * ext_step;
    int Tstep = instep * ext_step;
    int Tstop = (ext_stop < ext_start+(instart+(inedges-1)*instep)*ext_step) 
                ? ext_stop : ext_start+(instart+(inedges-1)*instep)*ext_step;

    // Fix bug in the 3.4 and prior Sequence CE code.
    if (implementation == "dap" && version < 3.5) {
        // ext_start--;
        Tstop++;
    }
    
    // Check the validity of the array constraints
    if ((instart >= get_size())
        || (instart < 0)||(inedges < 0))
        throw Error(NC_EINVALCOORDS, "Invalid coordinates.");      

    if (instart + inedges > get_size())
        throw Error(NC_EEDGE, "Hyperslab size exceeds dimension size.");

    // Zero edge found.
    if (inedges == 0) 
        throw Error(NC_NOERR, "The constraint would return no data.");

    ce << "[" << Tstart << ":" << Tstep << ":" << Tstop << "]";
    DBG(cerr << "ce: " << ce.str() << endl);
    
    expr += ce.str();     // Return ce via value-result parameter.
    return expr;
}

/** When flattening a Sequence, turn all non-array variables into a single
    dimension array. Add a dimension to all array variables. */
class AddDimension : public unary_function<BaseType *, void> {
    Sequence *d_seq;        /// The Seq that is the real parent of this var.
    const ClientParams &d_cp;
    VarList d_new_vars;     /// New vars with add dimension. Accumulate.
    
public:
    AddDimension(Sequence *s, const ClientParams &cp) : d_seq(s), d_cp(cp) {}

    operator VarList() { return d_new_vars; }   // Implicit type conversion.

    void add_translation_attribute(BaseType *a) {
        AttrTable *at;
        AttrTable::Attr_iter aiter;
        a->get_attr_table().find("translation", &at, &aiter);
        if (a->get_attr_table().attr_end() == aiter)
            a->get_attr_table().append_attr("translation", "String",
                                            "\"translated\"");
    }
    
    void size_new_dimension(NCArray *a, BaseType *e) {
        // Find any limits set for either the Sequence and/or this field.
        int field_limit = d_cp.get_limit(e->name());
        int seq_limit = d_cp.get_limit(d_seq->name());
        // Rule: If there's a limit on the field, it supersedes a limit
        // on the containing sequence. Create a new named dimension in
        // that case. Note that if only a default limit is set, then
        // both field_limit and seq_limit will be the same.
        if (field_limit != 0 && field_limit != seq_limit)
            a->append_dim(field_limit, e->name());
        else if (seq_limit != 0)
            a->append_dim(seq_limit, d_seq->name());
        else
            a->append_dim(1, d_seq->name());
    }
    
    void operator()(BaseType *e) {
        if (e->type() == dods_array_c) {
            // Since the design calls for the first dimension to be the 
            // sequence, we must creae a new Array, add the new dim and then 
            // copy the existing dimensions. Shoulda used lists...
            NCArray *src_array = dynamic_cast<NCArray*>(e);
            BaseType *btp = src_array->var()->ptr_duplicate();
            NCArray *a = new NCArray("", btp);
            a->set_source(d_seq);

            size_new_dimension(a, e);
            
            Array::Dim_iter i = src_array->dim_begin();
            while (i != src_array->dim_end()) {
                a->append_dim(src_array->dimension_size(i), 
                              src_array->dimension_name(i));
                ++i;
            }
            
            add_translation_attribute(a);
            
            d_new_vars.push_back(a);
        }
        else {
            BaseType *btp = e->ptr_duplicate();
            NCArray *a = new NCArray("", btp);
            a->set_source(d_seq);

            size_new_dimension(a, e);
#if 0
            // See comment above about limit values
            int field_limit = d_cp.get_limit(e->name());
            int seq_limit = d_cp.get_limit(d_seq->name());
            if (field_limit != 0 && field_limit != seq_limit)
                a->append_dim(field_limit, e->name());
            else if (seq_limit != 0)
                a->append_dim(seq_limit, d_seq->name());
            else
                a->append_dim(1, d_seq->name());
#endif
            add_translation_attribute(a);
#if 0
            a->get_attr_table().append_attr("translation", "String",
                                            "\"translated\"");
#endif
            // Insert the new variable right after the 
            d_new_vars.push_back(a);
        }
    }
};

VarList
NCSequence::flatten(const ClientParams &cp, const string &parent_name)
{
    Constructor::Vars_iter field = var_begin();
    Constructor::Vars_iter field_end = var_end();
    VarList new_vars;       // Store new vars here
    string local_name = (!parent_name.empty()) 
                        ? parent_name + spr + name()
                        : name();

    while (field != field_end) {
        VarList embedded_vars = dynamic_cast<NCAccess*>(*field)->flatten(cp, local_name);
        // AddDimension ad(this, cp);
        VarList ev_added_dim = for_each(embedded_vars.begin(), 
                                        embedded_vars.end(), 
                                        AddDimension(this, cp));
        new_vars.splice(new_vars.end(), ev_added_dim);
        ++field;
    }

    return new_vars;
}

/** @brief Get the BaseType pointer to the named variable of a given row.
    
    This version specializes Sequence::var_value() so that if the Sequence 
    holds Constructor types, it looks inside those ctors for \e name. This 
    works for the netCDF CL since we assume that the DDS used by the CL
    was translated. The CL will think that the atomic type is an Array and
    won't know anything about the nesting imposed by Structures (e.g., the HDF4
    server uses Structures to represent the hierarchy of the HDF4 file).
    
    @param row Read from <i>row</i> in the sequence.
    @param name Return <i>name</i> from <i>row</i>.
    @return A BaseType which holds the variable and its value. 
    @see number_of_rows */
BaseType *
NCSequence::var_value(size_t row, const string &name)
{
    BaseTypeRow *bt_row_ptr = row_value(row);
    if (!bt_row_ptr)
        return 0;

    BaseTypeRow::iterator bt_row_iter = bt_row_ptr->begin();
    BaseTypeRow::iterator bt_row_end = bt_row_ptr->end();    
    while (bt_row_iter != bt_row_end) {
        // This part is specialized for NCSequence. If we've gotten a Sequence
        // that holds a Structure, for example, the nerCDF CL will not know
        // about that because the translation process will have flattened the
        // Structure. So, this code looks inside Constructors that held
        // by Sequences for the atomic variable it contains. Note that this
        // is presumes that the netCDF CL only asks for one variable at a 
        // time.
        if ((*bt_row_iter)->is_constructor_type()) {
            BaseType *field = (*bt_row_iter)->var(name);
            if (field)
                return field;
        }
        else if ((*bt_row_iter)->name() == name)
            return *bt_row_iter;
        ++bt_row_iter;
    }
    
    return 0;
}

// $Log: NCSequence.cc,v $
// Revision 1.8  2005/01/26 23:25:51  jimg
// Implemented a fix for Sequence access by row number when talking to a
// 3.4 or earlier server (which contains a bug in is_end_of_rows()).
//
// Revision 1.7  2004/11/30 22:11:35  jimg
// I replaced the flatten_*() functions with a flatten() method in
// NCAccess. The default version of this method is in NCAccess and works
// for the atomic types; constructors must provide a specialization.
// Then I removed the code that copied the variables from vectors to
// lists. The translation code in NCConnect was modified to use the
// new method.
//
// Revision 1.6  2004/11/05 17:13:57  jimg
// Added code to copy the BaseType pointers from the vector container into
// a list. This will enable more efficient translation software to be
// written.
//
// Revision 1.5  2004/10/22 21:51:34  jimg
// More massive changes: Translation of Sequences now works so long as the
// Sequence contains only atomic types.
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
// Revision 1.5  1998/08/06 16:33:24  jimg
// Fixed misuse of the read(...) member function. Return true if more data
// is to be read, false is if not and error if an error is detected
//
// Revision 1.4  1996/09/17 17:06:37  jimg
// Merge the release-2-0 tagged files (which were off on a branch) back into
// the trunk revision.
//
// Revision 1.3.4.4  1996/09/17 00:26:29  jimg
// Merged changes from a side branch which contained various changes from
// Reza and Charles.
// Removed ncdump and netexec since ncdump is now in its own directory and
// netexec is no longer used.
//
// Revision 1.3.4.3  1996/08/13 21:19:01  jimg
// *** empty log message ***
//
// Revision 1.3.4.2  1996/07/10 21:44:18  jimg
// Changes for version 2.06. These fixed lingering problems from the migration
// from version 1.x to version 2.x.
// Removed some (but not all) warning generated with gcc's -Wall option.
//
// Revision 1.3.4.1  1996/06/25 22:04:40  jimg
// Version 2.0 from Reza.
//
// Revision 1.3  1995/07/09  21:33:49  jimg
// Added copyright notice.
//
// Revision 1.2  1995/03/16  16:56:41  reza
// Updated for the new DAP. All the read_val mfunc. and their memory management
// are now moved to their parent class.
// Data transfers are now in binary while DAS and DDS are still sent in ASCII.
//
// Revision 1.1  1995/02/10  04:57:43  reza
// Added read and read_val functions.
//
