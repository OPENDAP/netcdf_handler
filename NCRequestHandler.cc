// NCRequestHandler.cc

#include <iostream>
#include <fstream>

using std::ifstream ;
using std::cerr ;
using std::endl ;

#include "NCRequestHandler.h"
#include "DODSResponseHandler.h"
#include "DODSResponseNames.h"
#if 0
#include "NCreadAttributes.h"
#include "NCreadDescriptors.h"
#endif
#include "DAS.h"
#include "DDS.h"
#include "DODSConstraintFuncs.h"
#include "DODSInfo.h"
#include "TheDODSKeys.h"
#include "DODSResponseException.h"

#if 0
#include "version.h"
#endif

extern void read_variables(DAS &das, const string &filename) throw (Error);
extern void read_descriptors(DDS &dds, const string &filename)  throw (Error);

NCRequestHandler::NCRequestHandler( string name )
    : DODSRequestHandler( name )
{
    add_handler( DAS_RESPONSE, NCRequestHandler::nc_build_das ) ;
    add_handler( DDS_RESPONSE, NCRequestHandler::nc_build_dds ) ;
    add_handler( DATA_RESPONSE, NCRequestHandler::nc_build_data ) ;
    add_handler( HELP_RESPONSE, NCRequestHandler::nc_build_help ) ;
    add_handler( VERS_RESPONSE, NCRequestHandler::nc_build_version ) ;
}

NCRequestHandler::~NCRequestHandler()
{
}

bool
NCRequestHandler::nc_build_das( DODSDataHandlerInterface &dhi )
{
    DAS *das = (DAS *)dhi.response_handler->get_response_object() ;

    read_variables( *das, dhi.container->get_real_name() );

#if 0
    read_variables(das, df.get_dataset_name());
    df.read_ancillary_das(das);
#endif
    
#if 0
    if( !readAttributes( *das, dhi.container->get_real_name() ) )
    {
	throw DODSResponseException( "NC could not build the DAS response" ) ;
    }
#endif

    return true ;
}

bool
NCRequestHandler::nc_build_dds( DODSDataHandlerInterface &dhi )
{
    DDS *dds = (DDS *)dhi.response_handler->get_response_object() ;

    read_descriptors( *dds, dhi.container->get_real_name() );
    DODSConstraintFuncs::post_append( dhi ) ;

#if 0
    read_descriptors(dds, df.get_dataset_name());
    df.read_ancillary_dds(dds);
#endif

#if 0
    if( !readDescriptors( *dds, dhi.container->get_real_name(),
			  dhi.container->get_symbolic_name() ) )
    {
	throw DODSResponseException( "NC could not build the DDS response" ) ;
    }
    DODSConstraintFuncs::post_append( dhi ) ;
#endif

    return true ;
}

bool
NCRequestHandler::nc_build_data( DODSDataHandlerInterface &dhi )
{
#if 0
    DDS *dds = (DDS *)dhi.response_handler->get_response_object() ;
#endif
#if 0
    if( !readDescriptors( *dds, dhi.container->get_real_name(),
			  dhi.container->get_symbolic_name() ) )
    {
	throw DODSResponseException( "NC could not build the DDS response" ) ;
    }
    DODSConstraintFuncs::post_append( dhi ) ;
#endif

    return true ;
}

bool
NCRequestHandler::nc_build_help( DODSDataHandlerInterface &dhi )
{
    DODSInfo *info = (DODSInfo *)dhi.response_handler->get_response_object() ;
    info->add_data( (string)"No help for netCDF handler.\n" ) ;

#if 0
    info->add_data( (string)"cdf-dods help: " + nc_version() + "\n" ) ;
    bool found = false ;
    string key = (string)"NC.Help." + dhi.transmit_protocol ;
    string file = TheDODSKeys->get_key( key, found ) ;
    if( found == false )
    {
	info->add_data( "no help information available for cdf-dods\n" ) ;
    }
    else
    {
	ifstream ifs( file.c_str() ) ;
	if( !ifs )
	{
	    info->add_data( "cdf-dods help file not found, help information not available\n" ) ;
	}
	else
	{
	    char line[4096] ;
	    while( !ifs.eof() )
	    {
		ifs.getline( line, 4096 ) ;
		if( !ifs.eof() )
		{
		    info->add_data( line ) ;
		    info->add_data( "\n" ) ;
		}
	    }
	    ifs.close() ;
	}
    }
#endif

    return true ;
}

bool
NCRequestHandler::nc_build_version( DODSDataHandlerInterface &dhi )
{
    DODSInfo *info = (DODSInfo *)dhi.response_handler->get_response_object() ;
    info->add_data( (string)"    0.9\n" ) ;
    info->add_data( (string)"    libnc-dods 0.9\n" ) ;
#if 0
    info->add_data( (string)"    " + nc_version() + "\n" ) ;
    info->add_data( (string)"        libcdf2.7\n" ) ;
#endif
    return true ;
}

