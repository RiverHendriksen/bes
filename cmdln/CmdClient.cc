// CmdClient.cc

#include <iostream>

using std::cout ;
using std::endl ;
using std::cerr ;

#include "readline.h"
#include "history.h"

#define SIZE_COMMUNICATION_BUFFER 4096*4096

#include "CmdClient.h"
#include "PPTClient.h"

/**
* Connect the OpenDAP client to the OpenDAP server.
* <p>
* Connects to the OpenDAP server on the specified machine listening on
* the specified port.
*
* @param  hostStr  The name of the host machine where the server is
*                  running.
* @param  portVal  The port on which the server on the host hostStr is
*                  listening for requests.
* @throws PPTException Thrown if unable to connect to the specified host
*                      machine given the specified port.
* @see    PPTException
*/
void
CmdClient::startClient( const string &host, int portVal )
{
    _client = new PPTClient( host, portVal ) ;
    _client->initConnection() ;
}

/**
* Connect the OpenDAP client to the OpenDAP server using the unix socket
* <p>
* Connects to the OpenDAP server using the specified unix socket
*
* @param  unixStr  Full path to the unix socket
* @throws PPTException Thrown if unable to connect to the OPeNDAP server
* @see    PPTException
*/
void
CmdClient::startClient( const string &unixStr )
{
    _client = new PPTClient( unixStr ) ;
    _client->initConnection() ;
}

/**
* Closes the connection to the OpeNDAP server and closes the output stream.
*
* @throws PPTException Thrown if unable to close the connection or close
*                      the output stream.
*                      machine given the specified port.
* @see    OutputStream
* @see    PPTException
*/
void
CmdClient::shutdownClient()
{
    _client->closeConnection() ;
}

/**
* Set the output stream for responses from the OpenDAP server.
* <p>
* Specify where the response output from your OpenDAP request will be
* sent. Set to null if you wish to ignore the response from the OpenDAP
* server.
* 
* @param  strm  an OutputStream specifying where to send responses from
*               the OpenDAP server. If null then the output will not be
*               output but will be thrown away.
* @throws PPTException catches any problems with opening or writing to
*                      the output stream and creates a PPTException
* @see    OutputStream
* @see    PPTException
*/
void
CmdClient::setOutput( ostream *strm )
{
    _strm = strm ;
}

/**
* Sends a single OpeNDAP request ending in a semicolon (;) to the
* OpeNDAP server.
* <p>
* The response is written to the output stream if one is specified,
* otherwise the output is ignored.
*
* @param  cmd  The OpenDAP request, ending in a semicolon, that is sent to
*              the OpenDAP server to handle.
* @throws PPTException Thrown if there is a problem sending the request
*                      to the server or a problem receiving the response
*                      from the server.
* @see    PPTException
*/
void
CmdClient::executeCommand( const string &cmd )
{
    _client->send( cmd ) ;
    _client->receive( _strm ) ;
}

/**
* Execute each of the commands in the cmd_list, separated by a * semicolon.
* <p>
* The response is written to the output stream if one is specified,
* otherwise the output is ignored.
*
* @param  cmd_list  The list of OpenDAP requests, separated by semicolons
*                   and ending in a semicolon, that will be sent to the
*                   OpenDAP server to handle, one at a time.
* @throws PPTException Thrown if there is a problem sending any of the
*                      request to the server or a problem receiving any
*                      of the responses from the server.
* @see    PPTException
*/
void
CmdClient::executeCommands( const string &cmd_list )
{
    unsigned int start = 0 ;
    unsigned int end = 0 ;
    while( ( end = cmd_list.find( ';', start ) ) != string::npos )
    {
	string cmd = cmd_list.substr( start, end - start + 1 ) ;
	executeCommand( cmd ) ;
	start = end + 1 ;
    }
}

/**
* Sends the requests listed in the specified file to the OpenDAP server,
* each command ending with a semicolon.
* <p>
* The requests do not have to be one per line but can span multiple
* lines and there can be more than one command per line.
* <p>
* The response is written to the output stream if one is specified,
* otherwise the output is ignored.
*
* @param  inputFile  The file holding the list of OpenDAP requests, each
*                    ending with a semicolon, that will be sent to the
*                    OpenDAP server to handle.
* @throws PPTException Thrown if there is a problem opening the file to
*                      read, reading the requests from the file, sending
*                      any of the requests to the server or a problem
*                      receiving any of the responses from the server.
* @see    File
* @see    PPTException
*/
void
CmdClient::executeCommands( ifstream &istrm )
{
    string cmd ;
    bool done = false ;
    while( !done )
    {
	char line[4096] ;
	line[0] = '\0' ;
	istrm.getline( line, 4096, '\n' ) ;
	string nextLine = line ;
	if( nextLine == "" )
	{
	    if( cmd != "" )
	    {
		this->executeCommands( cmd ) ;
	    }
	    done = true ;
	}
	else
	{
	    unsigned int i = nextLine.find_last_of( ';' ) ;
	    if( i == string::npos )
	    {
		if( cmd == "" )
		{
		    cmd = nextLine ;
		}
		else
		{
		    cmd += " " + nextLine ;
		}
	    }
	    else
	    {
		string sub = nextLine.substr( 0, i ) ;
		if( cmd == "" )
		{
		    cmd = sub ;
		}
		else
		{
		    cmd += " " + sub ;
		}
		this->executeCommands( cmd ) ;
		if( i == nextLine.length() || i == nextLine.length() - 1 )
		{
		    cmd = "" ;
		}
		else
		{
		    cmd = nextLine.substr( i+1, nextLine.length() ) ;
		}
	    }
	}
    }
}

/**
* An interactive OpenDAP client that takes OpenDAP requests on the command
* line.
* <p>
* There can be more than one command per line, but commands can NOT span
* multiple lines. The user will be prompted to enter a new OpenDAP request.
* <p>
* OpenDAPClient:
* <p>
* The response is written to the output stream if one is specified,
* otherwise the output is ignored.
*
* @throws PPTException Thrown if there is a problem sending any of the
*                      requests to the server or a problem receiving any
*                      of the responses from the server.
* @see    PPTException
*/
void
CmdClient::interact()
{
    bool done = false ;
    while( !done )
    {
	string message = "" ;
	int len = this->readLine( message ) ;
	if( message == "exit" )
	{
	    done = true ;
	}
	else if( len != 0 && message != "" )
	{
	    this->executeCommands( message ) ;
	}
    }
}

int
CmdClient::readLine( string &msg )
{
    int len = 0 ;
    char *buf = (char*)NULL ;
    buf = ::readline( "OPeNDAPClient> " ) ;
    if( buf && *buf )
    {
	len = strlen( buf ) ;
	add_history( buf ) ;
	if( len > SIZE_COMMUNICATION_BUFFER )
	{
	    cerr << __FILE__ << __LINE__
		 << ": incoming data buffer exceeds maximum capacity with lenght "
		 << len << endl ;
	    exit( 1 ) ;
	}
	else
	{
	    msg = buf ;
	}
    }
    if( buf )
    {
	free( buf ) ;
	buf = (char*)NULL ;
    }
    return len ;
}

bool
CmdClient::isConnected()
{
    if( _client )
	return _client->isConnected() ;
    return false ;
}

void
CmdClient::brokenPipe()
{
    if( _client )
	_client->brokenPipe() ;
}

