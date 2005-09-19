// file_persistence.cc

// 2004 Copyright University Corporation for Atmospheric Research

#include <iostream>

using std::endl ;

#include "DODSInitList.h"
#include "DODSInitOrder.h"
#include "DODSContainerPersistenceFile.h"
#include "DODSContainerPersistenceList.h"
#include "DODSLog.h"

static bool
FilePersistenceInit(int, char**) {
    if( DODSLog::TheLog()->is_verbose() )
	(*DODSLog::TheLog()) << "Adding File Persistence" << endl;
    DODSContainerPersistenceFile *cpf =
	    new DODSContainerPersistenceFile( "DODSFile" ) ;
    DODSContainerPersistenceList::TheList()->add_persistence( cpf ) ;
    return true;
}

static bool
FilePersistenceTerm(void) {
    if( DODSLog::TheLog()->is_verbose() )
	(*DODSLog::TheLog()) << "Removing File Persistence" << endl;
    DODSContainerPersistenceList::TheList()->rem_persistence( "DODSFile" ) ;
    return true ;
}

FUNINITQUIT( FilePersistenceInit, FilePersistenceTerm, PERSISTENCE_INIT ) ;

// $Log: file_persistence.cc,v $
// Revision 1.2  2004/09/09 17:17:12  pwest
// Added copywrite information
//
// Revision 1.1  2004/06/30 20:16:24  pwest
// dods dispatch code, can be used for apache modules or simple cgi script
// invocation or opendap daemon. Built during cedar server development.
//
