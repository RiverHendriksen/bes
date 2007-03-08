// SayRequestHandler.cc

#include "config.h"

#include "SayRequestHandler.h"
#include "BESResponseHandler.h"
#include "BESResponseException.h"
#include "BESResponseNames.h"
#include "SayResponseNames.h"
#include "BESVersionInfo.h"
#include "BESTextInfo.h"
#include "BESConstraintFuncs.h"

SayRequestHandler::SayRequestHandler( string name )
    : BESRequestHandler( name )
{
    add_handler( VERS_RESPONSE, SayRequestHandler::say_build_vers ) ;
    add_handler( HELP_RESPONSE, SayRequestHandler::say_build_help ) ;
}

SayRequestHandler::~SayRequestHandler()
{
}

bool
SayRequestHandler::say_build_vers( BESDataHandlerInterface &dhi )
{
    bool ret = true ;
    BESVersionInfo *info = dynamic_cast<BESVersionInfo *>(dhi.response_handler->get_response_object() ) ;
    info->addHandlerVersion( PACKAGE_NAME, PACKAGE_VERSION ) ;
    return ret ;
}

bool
SayRequestHandler::say_build_help( BESDataHandlerInterface &dhi )
{
    bool ret = true ;
    BESInfo *info = dynamic_cast<BESInfo *>(dhi.response_handler->get_response_object());

    info->begin_tag("Handler");
    info->add_tag("name", PACKAGE_NAME);
    info->add_tag("version", PACKAGE_STRING);
    info->add_data_from_file( "Say.Help", "Say Help" ) ;
    info->end_tag("Handler");

    return ret ;
}

void
SayRequestHandler::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "SayRequestHandler::dump - ("
			     << (void *)this << ")" << endl ;
    BESIndent::Indent() ;
    BESRequestHandler::dump( strm ) ;
    BESIndent::UnIndent() ;
}
