/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
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
 # neoengine, settings
 #
 # this class implements the management of game settings
 #
 #
 #   date of creation:  06/15/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_frbase.h>
#include "ctd_settings.h"

using namespace NeoEngine;
using namespace std;

namespace CTD
{

Settings    *Settings::s_SingletonSettings = NULL;

static const basic_string <char>::size_type npos = -1;

// default settings
#define CTD_SVALUE_PLAYERNAME       "noname"
#define CTD_SVALUE_LEVEL            "noname"
#define CTD_SVALUE_SERVER           "localhost"
#define CTD_BVALUE_NETWORKING       true
#define CTD_BVALUE_SERVER           false
#define CTD_BVALUE_CLIENT           true
#define CTD_SVALUE_SERVERNAME       "ctd-server"
#define CTD_SVALUE_SERVERIP         "localhost"
#define CTD_SVALUE_CLIENTNAME       "ctd-client"
#define CTD_IVALUE_SERVERPORT       32000
#define CTD_IVALUE_CLIENTPORT       32001

Settings::Settings()
{

    // register settings
    Setting *pkSetting;

    pkSetting   = new Setting( string( CTD_STOKEN_PLAYERNAME ), string( CTD_SVALUE_PLAYERNAME ) );
    m_vpkSettings.push_back( pkSetting );

    pkSetting   = new Setting( string( CTD_STOKEN_LEVEL ), string( CTD_SVALUE_LEVEL ) );
    m_vpkSettings.push_back( pkSetting );

    pkSetting   = new Setting( string( CTD_BTOKEN_NETWORKING ), ( bool )CTD_BVALUE_NETWORKING );
    m_vpkSettings.push_back( pkSetting );

    pkSetting   = new Setting( string( CTD_BTOKEN_SERVER ), ( bool )CTD_BVALUE_SERVER );
    m_vpkSettings.push_back( pkSetting );

    pkSetting   = new Setting( string( CTD_BTOKEN_CLIENT ), ( bool )CTD_BVALUE_CLIENT );
    m_vpkSettings.push_back( pkSetting );

    pkSetting   = new Setting( string( CTD_STOKEN_SERVERNAME ), string( CTD_SVALUE_SERVERNAME ) );
    m_vpkSettings.push_back( pkSetting );

    pkSetting   = new Setting( string( CTD_STOKEN_SERVERIP ), string( CTD_SVALUE_SERVERIP ) );
    m_vpkSettings.push_back( pkSetting );

    pkSetting   = new Setting( string( CTD_ITOKEN_CLIENTPORT ), ( int )CTD_IVALUE_CLIENTPORT );
    m_vpkSettings.push_back( pkSetting );

    pkSetting   = new Setting( string( CTD_ITOKEN_SERVERPORT ), ( int )CTD_IVALUE_SERVERPORT );
    m_vpkSettings.push_back( pkSetting );

    pkSetting   = new Setting( string( CTD_STOKEN_CLIENTNAME ), string( CTD_SVALUE_CLIENTNAME ) );
    m_vpkSettings.push_back( pkSetting );

}

Settings::~Settings()
{

    // release all setting elements
    for( size_t uiCnt = 0; uiCnt < m_vpkSettings.size(); uiCnt++ ) {

        delete m_vpkSettings[ uiCnt ];

    }

}

void Settings::Shutdown()
{

    delete s_SingletonSettings;
    s_SingletonSettings = NULL;

}


bool Settings::Load( const std::string &strCfgFileName )
{

    FILE    *pkFile = NULL;
    pkFile = fopen( strCfgFileName.c_str(), "rb" );

    // if settings file does not exist then create one
    if ( pkFile == NULL ) {

        neolog << LogLevel( INFO ) << " creating game settings file '" << strCfgFileName << "'" << endl;
        
        // store default settings
        Store( strCfgFileName );
        
        pkFile = fopen( strCfgFileName.c_str(), "rb" );
        if ( pkFile == NULL ) {

            neolog << LogLevel( ERROR ) << "*** cannot load game settings file '" << strCfgFileName << "'" << endl;
            return false;

        }

    }

    // reset the file buffer
    m_strFileBuffer = "";

    char    cBuffer;
    while( fread( &cBuffer, sizeof( char ), 1, pkFile ) != 0 ) {

        m_strFileBuffer += cBuffer;

    }
    fclose( pkFile );


    Setting *pkSetting;

    // read registered settings
    pkSetting = FindSetting( CTD_STOKEN_PLAYERNAME );           
    assert ( pkSetting );
    ReadString( string( CTD_STOKEN_PLAYERNAME ), pkSetting->m_strValue );

    pkSetting = FindSetting( CTD_STOKEN_LEVEL );            
    assert ( pkSetting );
    ReadString( string( CTD_STOKEN_LEVEL ), pkSetting->m_strValue );

    pkSetting = FindSetting( CTD_BTOKEN_NETWORKING );           
    assert ( pkSetting );
    ReadBool( string( CTD_BTOKEN_NETWORKING ), pkSetting->m_bValue );

    pkSetting = FindSetting( CTD_BTOKEN_SERVER );           
    assert ( pkSetting );
    ReadBool( string( CTD_BTOKEN_SERVER ), pkSetting->m_bValue );

    pkSetting = FindSetting( CTD_BTOKEN_CLIENT );           
    assert ( pkSetting );
    ReadBool( string( CTD_BTOKEN_CLIENT ), pkSetting->m_bValue );

    pkSetting = FindSetting( CTD_STOKEN_SERVERNAME );           
    assert ( pkSetting );
    ReadString( string( CTD_STOKEN_SERVERNAME ), pkSetting->m_strValue );

    pkSetting = FindSetting( CTD_STOKEN_SERVERIP );         
    assert ( pkSetting );
    ReadString( string( CTD_STOKEN_SERVERIP ), pkSetting->m_strValue );

    pkSetting = FindSetting( CTD_STOKEN_CLIENTNAME );           
    assert ( pkSetting );
    ReadString( string( CTD_STOKEN_CLIENTNAME ), pkSetting->m_strValue );
    
    pkSetting = FindSetting( CTD_ITOKEN_CLIENTPORT );           
    assert ( pkSetting );
    ReadInt( string( CTD_ITOKEN_CLIENTPORT ), pkSetting->m_iValue );

    pkSetting = FindSetting( CTD_ITOKEN_SERVERPORT );           
    assert ( pkSetting );
    ReadInt( string( CTD_ITOKEN_SERVERPORT ), pkSetting->m_iValue );

    return true;

}


bool Settings::Store( const string &strCfgFileName )
{

    FILE    *pkFile = NULL;
    pkFile = fopen( strCfgFileName.c_str(), "wb+" );
    if ( pkFile == NULL ) {

        neolog << LogLevel( WARNING ) << " cannot store game settings file '" << strCfgFileName << "'" << endl;
        return false;

    }

    // clean the content of settings file buffer
    m_strFileBuffer = "";
    
    Setting *pkSetting;

    // fill the file buffer
    pkSetting = FindSetting( CTD_STOKEN_PLAYERNAME );           
    assert ( pkSetting );
    WriteString( string( CTD_STOKEN_PLAYERNAME ), pkSetting->m_strValue );

    pkSetting = FindSetting( CTD_STOKEN_LEVEL );            
    assert ( pkSetting );
    WriteString( string( CTD_STOKEN_LEVEL ), pkSetting->m_strValue );

    pkSetting = FindSetting( CTD_BTOKEN_NETWORKING );           
    assert ( pkSetting );
    WriteBool( string( CTD_BTOKEN_NETWORKING ), pkSetting->m_bValue );

    pkSetting = FindSetting( CTD_BTOKEN_SERVER );           
    assert ( pkSetting );
    WriteBool( string( CTD_BTOKEN_SERVER ), pkSetting->m_bValue );

    pkSetting = FindSetting( CTD_BTOKEN_CLIENT );           
    assert ( pkSetting );
    WriteBool( string( CTD_BTOKEN_CLIENT ), pkSetting->m_bValue );

    pkSetting = FindSetting( CTD_STOKEN_SERVERNAME );           
    assert ( pkSetting );
    WriteString( string( CTD_STOKEN_SERVERNAME ), pkSetting->m_strValue );

    pkSetting = FindSetting( CTD_STOKEN_SERVERIP );         
    assert ( pkSetting );
    WriteString( string( CTD_STOKEN_SERVERIP ), pkSetting->m_strValue );

    pkSetting = FindSetting( CTD_STOKEN_CLIENTNAME );           
    assert ( pkSetting );
    WriteString( string( CTD_STOKEN_CLIENTNAME ), pkSetting->m_strValue );
    
    pkSetting = FindSetting( CTD_ITOKEN_CLIENTPORT );           
    assert ( pkSetting );
    WriteInt( string( CTD_ITOKEN_CLIENTPORT ), pkSetting->m_iValue );

    pkSetting = FindSetting( CTD_ITOKEN_SERVERPORT );           
    assert ( pkSetting );
    WriteInt( string( CTD_ITOKEN_SERVERPORT ), pkSetting->m_iValue );
                                                                    
    // write the buffer into file
    size_t  uiBufferLength = m_strFileBuffer.length();
    char    cBuffer;
    for ( size_t uiCnt = 0; uiCnt < uiBufferLength; uiCnt++ ) {

        cBuffer =  m_strFileBuffer[ uiCnt ];
        fwrite( &cBuffer, sizeof( char ), 1, pkFile );

    }
    fclose( pkFile );

    return true;

}

bool Settings::ReadString( const string &strToken, string &strValue )
{

    size_t  iPos = -1;

    // search the exact match of the token string
    do {

        iPos++;
        iPos = m_strFileBuffer.find( strToken.c_str(), iPos, strToken.length() );
        if ( iPos == npos ) {

            return false;
        }

    } while ( m_strFileBuffer[ iPos + strToken.length() ] != '=' );

    strValue = "";
    iPos += strToken.length() + 1;
    while ( m_strFileBuffer[ iPos ] != 13 ) {

        strValue += m_strFileBuffer[ iPos ];
        iPos++;

    }

    return true;

}


bool Settings::ReadBool( const string &strToken, bool &bValue )
{

    string  strValue;

    if ( ReadString( strToken, strValue ) == false ) {

        return false;

    }

    if ( strValue == "true" ) {

        bValue = true;

    } else {

        bValue = false;

    }

    return true;

}

bool Settings::ReadInt( const string &strToken, int &iValue )
{

    string  strValue;

    if ( ReadString( strToken, strValue ) == false ) {

        return false;

    }

    iValue = atoi( strValue.c_str() );

    return true;

}

bool Settings::WriteString( const string &strToken, const string &strValue )
{

    m_strFileBuffer += string ( strToken + "=" + strValue + "\r" + "\n" );

    return true;

}

bool Settings::WriteBool( const string &strToken, const bool &bValue )
{

    string  strStringValue;
    if ( bValue == true ) {

        strStringValue = "true";

    } else {

        strStringValue = "false";

    }

    m_strFileBuffer += string ( strToken + "=" + strStringValue + "\r" + "\n" );

    return true;

}

bool Settings::WriteInt( const string &strToken, const int &iValue )
{

    char pcBuff[ 64 ];
    itoa( iValue, pcBuff, 10 );
    m_strFileBuffer += string ( strToken + "=" + string ( pcBuff ) + "\r" + "\n" );

    return true;

}

} // namespace CTD
