// BESCache.cc

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

#include "sys/types.h"
#include "sys/stat.h"
#include "dirent.h"
#include "stdio.h"
#include <errno.h>

#include <map>
#include <iostream>

using std::multimap ;
using std::pair ;
using std::greater ;
using std::endl ;

#include "BESCache.h"
#include "TheBESKeys.h"
#include "BESContainerStorageException.h"
#include "BESDebug.h"

#define BES_CACHE_CHAR '#'

typedef struct _cache_entry
{
    string name ;
    int size ;
} cache_entry ;

/** @brief Constructor that takes as arguments the values of the cache dir,
 * the file prefix, and the cache size
 *
 * @param cache_dir directory where the files are cached
 * @param prefix prefix used to prepend to the resulting cached file
 * @param size cache max size
 * throws BESContainerStorageException if cache_dir or prefix is empty, if size is
 * 0, or if the cache directory does not exist.
 */
BESCache::BESCache( const string &cache_dir,
		    const string &prefix,
		    int size )
    : _cache_dir( cache_dir ),
      _prefix( prefix ),
      _cache_size( size )
{
    if( _cache_dir.empty() )
    {
	string err = "The cache dir was not specified, must be non-empty" ;
	throw BESContainerStorageException( err, __FILE__, __LINE__ ) ;
    }

    DIR *dip = opendir( cache_dir.c_str() ) ;
    if( dip == NULL )
    {
	string err = "The cache dir " + _cache_dir + " does not exist" ;
	throw BESContainerStorageException( err, __FILE__, __LINE__ ) ;
    }
    closedir( dip ) ;

    if( prefix.empty() )
    {
	string err = "The prefix was not specified, must be non-empty" ;
	throw BESContainerStorageException( err, __FILE__, __LINE__ ) ;
    }

    if( !size )
    {
	string err = "The cache size was not specified, must be non-zero" ;
	throw BESContainerStorageException( err, __FILE__, __LINE__ ) ;
    }
}

/** @brief Constructor that takes as arguments keys to the cache directory,
 * file prefix, and size of the cache to be looked up a configuration file
 *
 * The keys specified are looked up in the specified keys object. If not
 * found or not set correctly then an exception is thrown. I.E., if the
 * cache directory is empty, the size is zero, or the prefix is empty.
 *
 * @param keys BESKeys object used to look up the keys
 * @param cache_dir_key key to look up in the keys file to find cache dir
 * @param prefix_key key to look up in the keys file to find the cache prefix
 * @param size_key key to look up in the keys file to find the cache size
 * @throws BESContainerStorageException if keys not set, cache dir or prefix empty,
 * size is 0, or if cache dir does not exist.
 */
BESCache::BESCache( BESKeys &keys,
		    const string &cache_dir_key,
		    const string &prefix_key,
		    const string &size_key )
    : _cache_size( 0 )
{
    bool found = false ;
    _cache_dir = keys.get_key( cache_dir_key, found ) ;
    if( !found || _cache_dir.empty() )
    {
	string err = "The cache dir key " + cache_dir_key
	             + " was not found or is not set" ;
	throw BESContainerStorageException( err, __FILE__, __LINE__ ) ;
    }

    found = false ;
    _prefix = keys.get_key( prefix_key, found ) ;
    if( !found || _prefix.empty() )
    {
	string err = "The prefix key " + prefix_key
	             + " was not found or is not set" ;
	throw BESContainerStorageException( err, __FILE__, __LINE__ ) ;
    }

    found = false ;
    string _cache_size_str = keys.get_key( size_key, found ) ;
    _cache_size = atoi( _cache_size_str.c_str() ) ;
    if( !found || !_cache_size )
    {
	string err = "The cache size could not be determine from "
	             + size_key + " = " + _cache_size_str ;
	throw BESContainerStorageException( err, __FILE__, __LINE__ ) ;
    }
}

/** @brief Determine if the file specified by src is cached
 *
 * The src is the full name of the file to be cached. We are assuming that
 * the file name passed has an extension on the end that will be stripped
 * once the file is cached. In other words, if the full path to the file
 * name is /usr/lib/data/fnoc1.nc.gz (a compressed netcdf file) then the
 * resulting file name set in target will be set to 
 * #&lt;prefix&gt;#usr#lib#data#fnoc1.nc.
 *
 * @param src src file that will be cached eventually
 * @param target set to the resulting cached file
 * @return true if the file is cached already
 */
bool
BESCache::is_cached( const string &src, string &target )
{
    bool is_it = true ;
    string tmp_target = src ;

    // Create the file that would be created in the cache directory
    //echo ${infile} | sed 's/^\///' | sed 's/\//#/g' | sed 's/\(.*\)\..*$/\1/g'
    if( tmp_target[0] == '/' )
    {
	tmp_target = src.substr( 1, tmp_target.length() - 1 ) ;
    }
    string::size_type slash = 0 ;
    while( ( slash = tmp_target.find( '/' ) ) != string::npos )
    {
	tmp_target.replace( slash, 1, 1, BES_CACHE_CHAR ) ;
    }
    string::size_type last_dot = tmp_target.rfind( '.' ) ;
    if( last_dot != string::npos )
    {
	tmp_target = tmp_target.substr( 0, last_dot ) ;
    }

    target = _cache_dir + "/" + _prefix + BES_CACHE_CHAR + tmp_target ;

    // Determine if the target file is already in the cache or not
    struct stat buf;
    int statret = stat( target.c_str(), &buf ) ;
    if( statret != 0 )
    {
	is_it = false ;
    }

    return is_it ;
}

/** @brief Check to see if the cache size exceeds the size specified in the
 * constructor and purge older files until size is less
 *
 * Usually called prior to caching a new file, this method will purge any
 * files, oldest to newest, if the current size of the cache exceeds the
 * size of the cache specified in the constructor.
 *
 */
void
BESCache::purge( )
{
    int max_size = _cache_size * 1048576 ; // Bytes/Meg
    struct stat buf;
    int size = 0 ; // total size of all cached files
    time_t curr_time = time( NULL ) ; // grab the current time so we can
    				      // determine the oldest file
    // map of time,entry values
    multimap<double,cache_entry,greater<double> > contents ;

    // the prefix is actually the specified prefix plus the cache char '#'
    string match_prefix = _prefix + BES_CACHE_CHAR ;

    // go through the cache directory and collect all of the files that
    // start with the matching prefix
    DIR *dip = opendir( _cache_dir.c_str() ) ;
    if( dip != NULL )
    {
	struct dirent *dit;
	while( ( dit = readdir( dip ) ) != NULL )
	{
	    string dirEntry = dit->d_name ;
	    if( dirEntry.compare( 0, match_prefix.length(), match_prefix ) == 0)
	    {
		// Now that we have found a match we want to get the size of
		// the file and the last access time from the file.
		string fullPath = _cache_dir + "/" + dirEntry ;
		int statret = stat( fullPath.c_str(), &buf ) ;
		if( statret == 0 )
		{
		    size += buf.st_size ;

		    // Find out how old the file is
		    time_t file_time = buf.st_atime ;
		    double time_diff = difftime( curr_time, file_time ) ;
		    cache_entry entry ;
		    entry.name = fullPath ;
		    entry.size = buf.st_size ;
		    contents.insert( pair<double,cache_entry>( time_diff, entry ) );
		}
	    }
	}

	// We're done looking in the directory, close it
	closedir( dip ) ;

	/*
	cout << endl << "BEFORE" << endl ;
	multimap<double,cache_entry,greater<double> >::iterator ti = contents.begin() ;
	multimap<double,cache_entry,greater<double> >::iterator te = contents.end() ;
	for( ; ti != te; ti++ )
	{
	    cout << (*ti).first << ": " << (*ti).second.name << ": size " << (*ti).second.size << endl ;
	}
	*/

	// if the size of files is greater than max allowed then we need to
	// purge the cache directory. Keep going until the size is less than
	// the max.
	cout << endl ;
	multimap<double,cache_entry,greater<double> >::iterator i ;
	if( size > max_size )
	{
	    while( size > max_size )
	    {
		i = contents.begin() ;
		BESDEBUG( "BESCache::purge - removing " << (*i).second.name << endl )
		if( remove( (*i).second.name.c_str() ) != 0 )
		{
		    char *s_err = strerror( errno ) ;
		    string err = "Unable to remove the file "
		                 + (*i).second.name + " from the cache: " ;
		    if( s_err )
		    {
			err.append( s_err ) ;
		    }
		    else
		    {
			err.append( "Unknown error" ) ;
		    }
		    throw BESContainerStorageException( err, __FILE__, __LINE__ ) ;
		}
		size -= (*i).second.size ;
		contents.erase( i ) ;
	    }
	}

	/*
	cout << endl << "AFTER" << endl ;
	ti = contents.begin() ;
	te = contents.end() ;
	for( ; ti != te; ti++ )
	{
	    cout << (*ti).first << ": " << (*ti).second.name << ": size " << (*ti).second.size << endl ;
	}
	*/
    }
    else
    {
	string err = "Unable to open cache directory " + _cache_dir ;
	throw BESContainerStorageException( err, __FILE__, __LINE__ ) ;
    }
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance along with information about
 * this cache.
 *
 * @param strm C++ i/o stream to dump the information to
 */
void
BESCache::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "BESCache::dump - ("
			     << (void *)this << ")" << endl ;
    BESIndent::Indent() ;
    strm << BESIndent::LMarg << "cache dir: " << _cache_dir << endl ;
    strm << BESIndent::LMarg << "prefix: " << _prefix << endl ;
    strm << BESIndent::LMarg << "size: " << _cache_size << endl ;
    BESIndent::UnIndent() ;
}

