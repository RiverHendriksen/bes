// PPTConnection.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.edu> and Jose Garcia <jgarcia@ucar.edu>
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
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301
 
// (c) COPYRIGHT University Corporation for Atmostpheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include <poll.h>
#include <errno.h>
#include <iostream>

using std::cout ;
using std::flush ;

#include "PPTConnection.h"
#include "PPTProtocol.h"
#include "Socket.h"
#include "PPTException.h"

void
PPTConnection::send( const string &buffer )
{
    if( buffer != "" )
    {
	writeBuffer( buffer ) ;
    }
    writeBuffer( PPTProtocol::PPT_COMPLETE_DATA_TRANSMITION ) ;
}

void
PPTConnection::sendExit()
{
    writeBuffer( PPTProtocol::PPT_EXIT_NOW ) ;
}

void
PPTConnection::writeBuffer( const string &buffer )
{
    _mySock->send( buffer, 0, buffer.length() ) ;
}
    
bool
PPTConnection::receive( ostream *strm )
{
    bool isDone = false ;
    ostream *use_strm = _out ;
    if( strm )
	use_strm = strm ;

    int termlen = PPTProtocol::PPT_COMPLETE_DATA_TRANSMITION.length() ;
    int exitlen = PPTProtocol::PPT_EXIT_NOW.length() ;
    int start = termlen ;
    bool done = false ;
    char *inBuff = new char[4096+termlen+1] ;
    while( done == false )
    {
	int bytesRead = readBuffer( inBuff+termlen ) ;
	if( bytesRead != 0 )
	{
	    if( !strncmp( inBuff+start, PPTProtocol::PPT_EXIT_NOW.c_str(), exitlen ) )
	    {
		done = true ;
		isDone = true ;
	    }
	    else
	    {
		int charsInBuff = bytesRead + ( termlen - start ) ;
		int writeBytes = charsInBuff - termlen ;
		if( charsInBuff >= termlen )
		{
		    if( !strncmp( inBuff+bytesRead, PPTProtocol::PPT_COMPLETE_DATA_TRANSMITION.c_str(), termlen ) )
		    {
			done = true ;
		    }

		    for( int j = 0; j < writeBytes; j++ )
		    {
			if( use_strm )
			{
			    (*use_strm) << inBuff[start+j] ;
			}
		    }

		    if( !done )
		    {
			strncpy( inBuff, inBuff + bytesRead, termlen ) ;
			start = 0 ;
		    }
		}
		else
		{
		    int newstart = termlen - charsInBuff ;
		    strncpy( inBuff + newstart, inBuff + start, charsInBuff ) ;
		    start = newstart ;
		}
	    }
	}
	else
	{
	    done = true ;
	}
    }
    delete [] inBuff ;
    return isDone ;
}

int
PPTConnection::readBuffer( char *inBuff )
{
    return _mySock->receive( inBuff, 4096 ) ;
}

int
PPTConnection::readBufferNonBlocking( char *inBuff )
{
    struct pollfd p ;
    p.fd = getSocket()->getSocketDescriptor();
    p.events = POLLIN ;
    struct pollfd arr[1] ;
    arr[0] = p ;

    // Lets loop 6 times with a delay block on poll of 1000 milliseconds
    // (duh! 6 seconds) and see if there is any data.
    for( int j = 0; j < _timeout; j++ )
    {
	if( poll( arr, 1, 1000 ) < 0 )
	{
	    string error( "poll error" ) ;
	    const char* error_info = strerror( errno ) ;
	    if( error_info )
		error += " " + (string)error_info ;
	    throw PPTException( error ) ;
	}
	else
	{
	    if (arr[0].revents==POLLIN)
	    {
		return readBuffer( inBuff ) ;
	    }
	    else
	    {
		cout << " " << j << flush ;
	    }
	}
    }
    cout << endl ;
    return -1 ;
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance
 *
 * @param strm C++ i/o stream to dump the information to
 */
void
PPTConnection::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "PPTConnection::dump - ("
			     << (void *)this << ")" << endl ;
    BESIndent::Indent() ;
    Connection::dump( strm ) ;
    BESIndent::UnIndent() ;
}

