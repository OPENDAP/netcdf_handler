// cdf_module.cc

#include <iostream>

using std::endl ;

#include "DODSInitList.h"
#include "TheRequestHandlerList.h"
#include "NCRequestHandler.h"
#include "TheDODSLog.h"

#define NC_NAME "nc"

static bool
NCInit(int, char**)
{
    if( TheDODSLog->is_verbose() )
	(*TheDODSLog) << "Initializing NC:" << endl ;

    if( TheDODSLog->is_verbose() )
	(*TheDODSLog) << "    adding " << NC_NAME << " request handler" 
		      << endl ;
    TheRequestHandlerList->add_handler( NC_NAME,
					new NCRequestHandler( NC_NAME ) ) ;

    return true ;
}

static bool
NCTerm(void)
{
    if( TheDODSLog->is_verbose() )
	(*TheDODSLog) << "Removing NC Handlers" << endl;
    DODSRequestHandler *rh = TheRequestHandlerList->remove_handler( NC_NAME ) ;
    if( rh ) delete rh ;
    return true ;
}

FUNINITQUIT( NCInit, NCTerm, 3 ) ;

