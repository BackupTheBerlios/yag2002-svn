/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # common utilities
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_utils.h"

using namespace std;

namespace CTD
{

string::size_type explode( const string& str, const string& separators, vector< string >* p_result )
{
	string::size_type len = str.length();
	if( !len )
		return 0;

	if( !separators.length() )
	{
		p_result->push_back( str );
		return 1;
	}

	string::size_type token = 0;
	string::size_type end   = 0;
	unsigned int      org   = p_result->size();

	while( end < len )
	{
		token = str.find_first_not_of( separators, end );
		end   = str.find_first_of( separators, token );

		if( token != string::npos )
			p_result->push_back( str.substr( token, ( end != string::npos ) ? ( end - token ) : string::npos ) );
	}

	return( p_result->size() - org );
}

std::string extractPath( const std::string& fullpath )
{
    string res = fullpath;
    // first clean the path
    for ( string::iterator i = res.begin(), e = res.end(); i != e; i++ ) if ( *i == '\\') *i = '/';
    res = res.substr( 0, res.rfind( "/" ) );
    if ( !res.empty() )
        return res;

    return "";
}

std::string extractFileName( const std::string& fullpath )
{
    string res = fullpath;
    // first clean the path
    for ( string::iterator i = res.begin(), e = res.end(); i != e; i++ ) if ( *i == '\\') *i = '/';
    res = res.substr( res.rfind( "/" ) );
    if ( res[ 0 ] == '/' ) res.erase( 0, 1 ); // cut leading slash
    if ( !res.empty() )
        return res;

    return "";
}

void getDirectoryListing( std::vector< std::string >& listing, const std::string& dir, const std::string& extension )
{
	WIN32_FIND_DATA findData;
	HANDLE          fileHandle;
	int             flag = 1;
	std::string     search ( "*." );
	std::string     directory( dir );
	
    if ( directory == "" ) 
        directory = ".";

	directory += "/";
	search = directory + search + extension;
	// SetCurrentDirectory(dir.c_str());
	fileHandle = FindFirstFile( search.c_str(), &findData );
	if ( fileHandle == INVALID_HANDLE_VALUE ) 
        return;

	while ( flag )
	{
		listing.push_back( findData.cFileName );
		flag = FindNextFile( fileHandle, &findData );
	}
	FindClose( fileHandle );
}

} // namespace CTD
