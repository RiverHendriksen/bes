// BESCatalogList.cc

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
 
// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include "BESCatalogList.h"
#include "BESCatalog.h"
#include "BESResponseException.h"
#include "BESInfo.h"
#include "BESHandlerException.h"

BESCatalogList *BESCatalogList::_instance = 0 ;

BESCatalogList::~BESCatalogList()
{
    catalog_iter i = _catalogs.begin() ;
    catalog_iter e = _catalogs.end() ;
    for( ; i != e; i++ )
    {
	BESCatalog *catalog = (*i).second ;
	if( catalog ) delete catalog ;
    }
}

bool BESCatalogList::add_catalog(BESCatalog * catalog)
{
    bool stat = false;
    if (find_catalog(catalog->get_catalog_name()) == 0) {
#if 0
        _catalogs[catalog->get_catalog_name()] = catalog;
#endif
        std::pair<const std::string, BESCatalog*> p = std::make_pair("test", catalog);
        stat = _catalogs.insert(p).second;
#if 0
        stat = true;
#endif
    }
    return stat;
}

bool
BESCatalogList::del_catalog( const string &catalog_name )
{
    bool ret = false ;
    BESCatalog *cat = 0 ;
    BESCatalogList::catalog_iter i ;
    i = _catalogs.find( catalog_name ) ;
    if( i != _catalogs.end() )
    {
	cat = (*i).second;
	_catalogs.erase( i ) ;
	delete cat ;
    }
    return ret ;
}

BESCatalog *
BESCatalogList::find_catalog( const string &catalog_name )
{
    BESCatalog *ret = 0 ;
    BESCatalogList::catalog_citer i ;
    i = _catalogs.find( catalog_name ) ;
    if( i != _catalogs.end() )
    {
	ret = (*i).second;
    }
    return ret ;
}

void
BESCatalogList::show_catalog( const string &container,
			   const string &coi,
			   BESInfo *info )
{
    catalog_citer i = _catalogs.begin() ;
    catalog_citer e = _catalogs.end() ;
    bool done = false ;
    for( ; i != e && done == false; i++ )
    {
	BESCatalog *catalog = (*i).second ;
	done = catalog->show_catalog( container, coi, info ) ;
    }
    if( done == false )
    {
	string serr ;
	if( container != "" )
	{
	    serr = (string)"Unable to find catalog information for container "
		   + container ;
	}
	else
	{
	    serr = "Unable to find catalog information for root" ;
	}
	throw BESHandlerException( serr, __FILE__, __LINE__ ) ;
	//info->add_exception( "Error", serr, __FILE__, __LINE__ ) ;
    }
}

BESCatalogList *
BESCatalogList::TheCatalogList()
{
    if( _instance == 0 )
    {
	_instance = new BESCatalogList ;
    }
    return _instance ;
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance along with the catalog entries
 * in this list.
 *
 * @param strm C++ i/o stream to dump the information to
 */
void
BESCatalogList::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "BESCatalogList::dump - ("
			     << (void *)this << ")" << endl ;
    BESIndent::Indent() ;
    if( _catalogs.size() )
    {
	strm << BESIndent::LMarg << "catalog list:" << endl ;
	BESIndent::Indent() ;
	catalog_citer i = _catalogs.begin() ;
	catalog_citer e = _catalogs.end() ;
	for( ; i != e; i++ )
	{
	    BESCatalog *catalog = (*i).second ;
	    strm << BESIndent::LMarg << (*i).first << catalog << endl ;
	}
	BESIndent::UnIndent() ;
    }
    else
    {
	strm << BESIndent::LMarg << "catalog list: empty" << endl ;
    }
    BESIndent::UnIndent() ;
}

