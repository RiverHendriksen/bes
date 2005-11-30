// CatalogList.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.org>
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

#include "CatalogList.h"
#include "OPeNDAPCatalog.h"
#include "DODSResponseException.h"

CatalogList *CatalogList::_instance = 0 ;

CatalogList::~CatalogList()
{
    catalog_iterator i = _catalogs.begin() ;
    catalog_iterator e = _catalogs.end() ;
    for( ; i != e; i++ )
    {
	OPeNDAPCatalog *catalog = (*i) ;
	if( catalog ) delete catalog ;
    }
}

void
CatalogList::add_catalog( OPeNDAPCatalog *catalog )
{
    _catalogs.push_back( catalog ) ;
}

void
CatalogList::show_nodes( const string &node, DODSTextInfo *info )
{
    catalog_iterator i = _catalogs.begin() ;
    catalog_iterator e = _catalogs.end() ;
    bool done = false ;
    for( ; i != e && done == false; i++ )
    {
	OPeNDAPCatalog *catalog = (*i) ;
	done = catalog->show_nodes( node, info ) ;
    }
    if( done == false )
    {
	string serr ;
	if( node != "" )
	{
	    serr = (string)"Unable to find catalog information for the node "
		   + node ;
	}
	else
	{
	    serr = "Unable to find catalog information for the root node" ;
	}
	DODSResponseException e( serr ) ;
	throw e ;
    }
}

void
CatalogList::show_leaves( const string &node, DODSTextInfo *info )
{
    catalog_iterator i = _catalogs.begin() ;
    catalog_iterator e = _catalogs.end() ;
    bool done = false ;
    for( ; i != e && done == false; i++ )
    {
	OPeNDAPCatalog *catalog = (*i) ;
	done = catalog->show_leaves( node, info ) ;
    }
    if( done == false )
    {
	string serr ;
	if( node != "" )
	{
	    serr = (string)"Unable to find catalog information for the node "
		   + node ;
	}
	else
	{
	    serr = "Unable to find catalog information for the root node" ;
	}
	DODSResponseException e( serr ) ;
	throw e ;
    }
}

CatalogList *
CatalogList::TheCatalogList()
{
    if( _instance == 0 )
    {
	_instance = new CatalogList ;
    }
    return _instance ;
}

// $Log: CatalogList.cc,v $
