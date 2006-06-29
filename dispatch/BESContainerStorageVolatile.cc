// BESContainerStorageVolatile.cc

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

#include "BESContainerStorageVolatile.h"
#include "BESContainer.h"
#include "BESContainerStorageException.h"
#include "BESInfo.h"

/** @brief create an instance of this persistent store with the given name.
 *
 * Creates an instances of BESContainerStorageVolatile with the given name.
 *
 * @param n name of this persistent store
 * @see BESContainer
 */
BESContainerStorageVolatile::BESContainerStorageVolatile( const string &n )
    : BESContainerStorage( n )
{
}

BESContainerStorageVolatile::~BESContainerStorageVolatile()
{ 
}

/** @brief looks for the specified container using the given name
 *
 * If a match is made with the symbolic name found in the container then the
 * information is stored in the passed container object and the is_valid flag
 * is set to true. If not found, then is_valid is set to false.
 *
 * @param d container to look for and, if found, store the information in.
 */
void
BESContainerStorageVolatile::look_for( BESContainer &d )
{
    d.set_valid_flag( false ) ;

    string sym_name = d.get_symbolic_name() ;
    
    BESContainerStorageVolatile::Container_citer i =
	    _container_list.begin() ;

    i = _container_list.find( sym_name ) ;
    if( i != _container_list.end() )
    {
	BESContainer *c = (*i).second ;
	d.set_real_name( c->get_real_name() ) ;
	d.set_container_type( c->get_container_type() ) ;
	d.set_valid_flag( true ) ;
    }
}

void
BESContainerStorageVolatile::add_container( const string &s_name,
					    const string &r_name,
					    const string &type )
{
    if( type == "" )
    {
	string s = "Unable to add container, type of data must be specified"  ;
	throw BESContainerStorageException( s, __FILE__, __LINE__ ) ;
    }

    BESContainerStorageVolatile::Container_citer i =
	    _container_list.begin() ;

    i = _container_list.find( s_name ) ;
    if( i != _container_list.end() )
    {
	string s = (string)"A container with the name " + s_name
	           + " already exists" ;
	throw BESContainerStorageException( s, __FILE__, __LINE__ ) ;
    }
    BESContainer *c = new BESContainer( s_name ) ;
    c->set_real_name( r_name ) ;
    c->set_container_type( type ) ;
    _container_list[s_name] = c ;
}

/** @brief removes a container with the given symbolic name
 *
 * This method removes a container to the persistence store with the
 * given symbolic name. It deletes the container.
 *
 * @param s_name symbolic name for the container
 * @return true if successfully removed and false otherwise
 */
bool
BESContainerStorageVolatile::del_container( const string &s_name )
{
    bool ret = false ;
    BESContainerStorageVolatile::Container_iter i ;
    i = _container_list.find( s_name ) ;
    if( i != _container_list.end() )
    {
	BESContainer *c = (*i).second;
	_container_list.erase( i ) ;
	delete c ;
	ret = true ;
    }
    return ret ;
}

/** @brief removes all container
 *
 * This method removes all containers from the persistent store. It does
 * not delete the real data behind the container.
 *
 * @return true if successfully removed and false otherwise
 */
bool
BESContainerStorageVolatile::del_containers( )
{
    while( _container_list.size() != 0 )
    {
	Container_iter ci = _container_list.begin() ;
	BESContainer *c = (*ci).second ;
	_container_list.erase( ci ) ;
	if( c )
	{
	    delete c ;
	}
    }
    return true ;
}

/** @brief show information for each container in this persistent store
 *
 * For each container in this persistent store, add infomation about each of
 * those containers. The information added to the information object
 * includes a line for each container within this persistent store which 
 * includes the symbolic name, the real name, and the data type, 
 * separated by commas.
 *
 * In the case of this persistent store information from each container
 * added to the volatile list is added to the information object.
 *
 * @param info object to store the container and persistent store information
 * @see BESInfo
 */
void
BESContainerStorageVolatile::show_containers( BESInfo &info )
{
    info.add_tag( "name", get_name() ) ;
    BESContainerStorageVolatile::Container_iter i = _container_list.begin() ;
    for( ; i != _container_list.end(); i++ )
    {
	info.begin_tag( "container" ) ;
	BESContainer *c = (*i).second;
	string sym = c->get_symbolic_name() ;
	info.add_tag( "symbolicName", sym ) ;
	string real = c->get_real_name() ;
	info.add_tag( "realName", real ) ;
	string type = c->get_container_type() ;
	info.add_tag( "dataType", type ) ;
	info.end_tag( "container" ) ;
    }
}
