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

static const basic_string <char>::size_type npos = -1;

Settings::Settings()
{

    m_bLoaded = false;
    m_pkFile  = NULL;

}

Settings::~Settings()
{
}

void Settings::Shutdown()
{

    // release all setting elements
    for( size_t uiCnt = 0; uiCnt < m_vpkSettings.size(); uiCnt++ ) {

        delete m_vpkSettings[ uiCnt ];

    }
    m_bLoaded       = false;
    m_strFileBuffer = "";
    delete m_pkFile;

}

bool Settings::Load( const std::string &strSettingsFile )
{

    m_pkFile = NeoEngine::Core::Get()->GetFileManager()->GetByName( strSettingsFile );
    if ( !m_pkFile ) {
        
        neolog << LogLevel( INFO ) << "*** cannot find settings file '" << strSettingsFile << "'" << endl;
        return false;

    }
    return Load( m_pkFile );

}

bool Settings::Load( File *pkFile )
{

    if ( !m_pkFile->Open( "", m_pkFile->GetName(), ios_base::in | ios_base::binary ) ) {

        neolog << LogLevel( INFO ) << "*** cannot open settings file '" << m_pkFile->GetName() << "'" << endl;
        return false;

    }

    // load the settings into the file buffer
    m_strFileBuffer = "";
    char*    pcBuffer = new char[ m_pkFile->GetSize() * sizeof( char ) + 1 ];
    m_pkFile->Read( pcBuffer, m_pkFile->GetSize() );
    m_strFileBuffer   = pcBuffer;
    m_strFileBuffer[ m_pkFile->GetSize() ] = 0; // terminate the buffer string
    delete[] pcBuffer;

    m_pkFile->Close();

    // format the intput
    size_t uiTokens = m_vpkSettings.size();
    for( size_t uiCnt = 0; uiCnt < uiTokens; uiCnt++ ) {

        SettingBase *pkSetting = m_vpkSettings[ uiCnt ];
        
        if ( pkSetting->GetTypeInfo() == typeid( bool ) ) {
            Read( pkSetting->GetTokenName(), static_cast< Setting< bool >* >( pkSetting )->m_Value );
        } else 
        if ( pkSetting->GetTypeInfo() == typeid( int ) ) {
            Read( pkSetting->GetTokenName(), static_cast< Setting< int >* >( pkSetting )->m_Value );
        } else 
        if ( pkSetting->GetTypeInfo() == typeid( string ) ) {
            Read( pkSetting->GetTokenName(), static_cast< Setting< string >* >( pkSetting )->m_Value );
        } else 
        if ( pkSetting->GetTypeInfo() == typeid( float ) ) {
            Read( pkSetting->GetTokenName(), static_cast< Setting< float >* >( pkSetting )->m_Value );
        } else 
         if ( pkSetting->GetTypeInfo() == typeid( Vector3d ) ) {
            Read( pkSetting->GetTokenName(), static_cast< Setting< Vector3d >* >( pkSetting )->m_Value );
        } else 
        assert( NULL && "unsupported setting type" );

    }

    m_strSettingsFile = m_pkFile->GetName();
    m_bLoaded         = true;

    return true;

}


bool Settings::Store( File *pkFile )
{

    if ( !m_bLoaded && !pkFile ) {

        neolog << LogLevel( WARNING ) << "*** no settings file was previously loaded" << endl;
        return false;
    }

    if ( pkFile ) {

        m_pkFile = pkFile;

    }

    if ( !m_pkFile->Open( "", m_pkFile->GetName(), ios_base::out | ios_base::binary ) ) {

        neolog << LogLevel( INFO ) << "*** cannot write to settings file '" << m_pkFile->GetName() << "'" << endl;
        return false;

    }

    // clean the content of settings file buffer
    m_strFileBuffer = "";
    
    // format the output
    size_t uiTokens = m_vpkSettings.size();
    for( size_t uiCnt = 0; uiCnt < uiTokens; uiCnt++ ) {

        SettingBase *pkSetting = m_vpkSettings[ uiCnt ];
        
        if ( pkSetting->GetTypeInfo() == typeid( bool ) ) {
            Write( pkSetting->GetTokenName(), static_cast< Setting< bool >* >( pkSetting )->m_Value );
        } else 
        if ( pkSetting->GetTypeInfo() == typeid( int ) ) {
            Write( pkSetting->GetTokenName(), static_cast< Setting< int >* >( pkSetting )->m_Value );
        } else 
        if ( pkSetting->GetTypeInfo() == typeid( string ) ) {
            Write( pkSetting->GetTokenName(), static_cast< Setting< string >* >( pkSetting )->m_Value );
        } else 
        if ( pkSetting->GetTypeInfo() == typeid( float ) ) {
            Write( pkSetting->GetTokenName(), static_cast< Setting< float >* >( pkSetting )->m_Value );
        } else
        if ( pkSetting->GetTypeInfo() == typeid( Vector3d ) ) {
            Write( pkSetting->GetTokenName(), static_cast< Setting< Vector3d >* >( pkSetting )->m_Value );
        } else
            assert( NULL && "unsupported setting type" );

    }

    // write the buffer into file
    m_pkFile->Write( m_strFileBuffer.c_str(), ( int )m_strFileBuffer.length() );

    m_pkFile->Close();

    return true;

}

bool Settings::Read( const string &strToken, string &strValue )
{

    size_t  iPos = -1;

    // search the exact match of the token string
    do {

        iPos++;
        iPos = m_strFileBuffer.find( strToken.c_str(), iPos, strToken.length() );
        if ( iPos == npos ) {

            neolog << LogLevel( DEBUG ) << "*** settings manager could not read requested token '" << strToken << "', skipping..." << endl;
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


bool Settings::Read( const string &strToken, bool &bValue )
{

    string  strValue;

    if ( Read( strToken, strValue ) == false ) {

        return false;

    }

    if ( strValue == "true" ) {

        bValue = true;

    } else {

        bValue = false;

    }

    return true;

}

bool Settings::Read( const string &strToken, int &iValue )
{

    string  strValue;

    if ( Read( strToken, strValue ) == false ) {

        return false;

    }

    iValue = atoi( strValue.c_str() );

    return true;

}

bool Settings::Read( const string &strToken, float &fValue )
{

    string  strValue;

    if ( Read( strToken, strValue ) == false ) {

        return false;

    }

    fValue = ( float )atof( strValue.c_str() );

    return true;

}

bool Settings::Read( const string &strToken, Vector3d &kValue )
{

    string  strValue;

    if ( Read( strToken, strValue ) == false ) {

        return false;

    }

    sscanf( strValue.c_str(), "%f %f %f", &kValue.x, &kValue.y, &kValue.z );

    return true;

}

bool Settings::Write( const string &strToken, const string &strValue )
{

    m_strFileBuffer += string ( strToken + "=" + strValue + "\r" + "\n" );

    return true;

}

bool Settings::Write( const string &strToken, const bool &bValue )
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

bool Settings::Write( const string &strToken, const int &iValue )
{

    char pcBuff[ 64 ];
    itoa( iValue, pcBuff, 10 );
    m_strFileBuffer += string ( strToken + "=" + string ( pcBuff ) + "\r" + "\n" );

    return true;

}

bool Settings::Write( const string &strToken, const float &fValue )
{

    char pcBuff[ 64 ];
    sprintf( pcBuff, "%f", fValue );
    m_strFileBuffer += string ( strToken + "=" + string ( pcBuff ) + "\r" + "\n" );

    return true;

}

bool Settings::Write( const string &strToken, const Vector3d &kValue )
{

    char pcBuff[ 64 ];
    sprintf( pcBuff, "%f %f %f", kValue.x, kValue.y, kValue.z );
    m_strFileBuffer += string ( strToken + "=" + string ( pcBuff ) + "\r" + "\n" );

    return true;

}

} // namespace CTD
