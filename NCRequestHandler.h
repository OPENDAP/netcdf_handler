// CDFRequestHandler.h

#ifndef I_NCRequestHandler_H
#define I_NCRequestHandler_H 1

#include "DODSRequestHandler.h"

class NCRequestHandler : public DODSRequestHandler {
public:
			NCRequestHandler( string name ) ;
    virtual		~NCRequestHandler( void ) ;

    static bool		nc_build_das( DODSDataHandlerInterface &dhi ) ;
    static bool		nc_build_dds( DODSDataHandlerInterface &dhi ) ;
    static bool		nc_build_data( DODSDataHandlerInterface &dhi ) ;
    static bool		nc_build_help( DODSDataHandlerInterface &dhi ) ;
    static bool		nc_build_version( DODSDataHandlerInterface &dhi ) ;
};

#endif

