/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU Lesser General Public 
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # Class for managing game settings
 #
 #   date of creation:  03/08/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_log.h"
#include "ctd_settings.h"

using namespace std;

namespace CTD
{

CTD_SINGLETON_IMPL( SettingsManager );

// implementation of SettingsManager
SettingsManager::SettingsManager()
{
}

SettingsManager::~SettingsManager()
{
    // delete all profiles
    map< string, Settings* >::iterator pp_profile = NULL, pp_profileEnd = _profiles.end();
    while ( pp_profile != pp_profileEnd )
        delete pp_profile->second;
}

Settings* SettingsManager::createProfile( const string& profilename, const string& filename )
{
    map< string, Settings* >::iterator pp_profile = NULL, pp_profileEnd = _profiles.end();
    pp_profile = _profiles.find( profilename );
    if ( pp_profile != pp_profileEnd )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** requested profile '" << profilename << "' already exists!" << endl;
        return NULL;
    }

    Settings* p_settings = new Settings;
    p_settings->setFileName( filename );
    _profiles.insert( make_pair( profilename, p_settings ) );
    return p_settings;
}

void SettingsManager::destroyProfile( const std::string& profilename )
{
    map< string, Settings* >::iterator pp_profile = NULL, pp_profileEnd = _profiles.end();
    pp_profile = _profiles.find( profilename );
    if ( pp_profile == pp_profileEnd )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** profile '" << profilename << "' does not exist!" << endl;
        return;
    }

    delete pp_profile->second;
    _profiles.erase( pp_profile );
}

bool SettingsManager::loadProfile( const string& profilename )
{
    map< string, Settings* >::iterator pp_profile = NULL, pp_profileEnd = _profiles.end();
    pp_profile = _profiles.find( profilename );
    if ( pp_profile == pp_profileEnd )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** profile '" << profilename << "' does not exist!" << endl;
        return false;
    }

    return pp_profile->second->load();
}

bool SettingsManager::storeProfile( const string& profilename )
{
    map< string, Settings* >::iterator pp_profile = NULL, pp_profileEnd = _profiles.end();
    pp_profile = _profiles.find( profilename );
    if ( pp_profile == pp_profileEnd )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** profile '" << profilename << "' does not exist!" << endl;
        return false;
    }

    return pp_profile->second->store();
}

Settings* SettingsManager::getProfile( const std::string& profilename )
{
    map< string, Settings* >::iterator pp_profile = NULL, pp_profileEnd = _profiles.end();
    pp_profile = _profiles.find( profilename );
    if ( pp_profile != pp_profileEnd )
        return pp_profile->second;
    
    return NULL;
}

// implementation of Settings
Settings::Settings() :
_loaded( false )
{
}

Settings::~Settings()
{
    // release all setting elements
    std::vector< SettingBase* >::iterator pp_setting = _settings.begin(), pp_settingEnd = _settings.end();
    for ( ; pp_setting != pp_settingEnd; pp_setting++ )
        delete ( *pp_setting );
}

bool Settings::load( const std::string& filename )
{
    if ( _loaded )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** there is already an open settings file '" << filename << "'." << endl;
        return false;
    }

    string openfile = filename;
    if ( !filename.length() )
        openfile = _settingsFile;

    auto_ptr< fstream > p_stream( new fstream );

    p_stream->open( openfile.c_str(), ios_base::binary | ios_base::in );
    // if the file does not exist then create one
    if ( !*p_stream )
    {   
        p_stream->open( openfile.c_str(), ios_base::binary | ios_base::out );
        log << Log::LogLevel( Log::L_WARNING ) << "*** settings file '" << openfile << "' does not exist. one has been created." << endl;
        _loaded   = true;
        return false;
    }

    // get file size
    p_stream->seekg( 0, ios_base::end );
    int filesize = ( int )p_stream->tellg();
    p_stream->seekg( 0, ios_base::beg );

    // load the settings into the file buffer
    _fileBuffer = "";
    char* p_buf = new char[ filesize * sizeof( char ) + 2 ];
    p_stream->read( p_buf, filesize );
    _fileBuffer = p_buf;
    _fileBuffer[ filesize ]     = 13; // terminate the buffer string
    _fileBuffer[ filesize + 1 ] = 0;
    delete[] p_buf;

    p_stream->close();

    // format the intput
    size_t tokens = _settings.size();
    for( size_t cnt = 0; cnt < tokens; cnt++ ) 
    {
        SettingBase* p_setting = _settings[ cnt ];
        const type_info& settings_typeinfo = p_setting->getTypeInfo();
        if ( settings_typeinfo == typeid( bool ) ) {
            read( p_setting->getTokenName(), static_cast< Setting< bool >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( int ) ) {
            read( p_setting->getTokenName(), static_cast< Setting< int >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( unsigned int ) ) {
            read( p_setting->getTokenName(), static_cast< Setting< unsigned int >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( string ) ) {
            read( p_setting->getTokenName(), static_cast< Setting< string >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( float ) ) {
            read( p_setting->getTokenName(), static_cast< Setting< float >* >( p_setting )->_value );
        } else 
         if ( settings_typeinfo == typeid( osg::Vec3f ) ) {
             read( p_setting->getTokenName(), static_cast< Setting< osg::Vec3f >* >( p_setting )->_value );
        } else 
            assert( NULL && "unsupported setting type" );
    }

    _settingsFile = openfile;
    _loaded       = true;

    return true;
}

bool Settings::store( const string& filename )
{
    auto_ptr< fstream > p_stream( new fstream );

    if ( !filename.length() )
    {
        if ( !_loaded ) 
        {
            log << Log::LogLevel( Log::L_WARNING ) << "*** no settings file was previously loaded" << endl;
            return false;
        }
        p_stream->open( _settingsFile.c_str(), ios_base::binary | ios_base::out );
        if ( !*p_stream )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "*** cannot open settings file '" << filename << "'" << endl;
            assert( NULL && " internal error, cannot open settings file for writing" );
        }
    } 
    else
    {
        p_stream->open( filename.c_str(), ios_base::binary | ios_base::out );
        if ( !*p_stream )
        {   
            log << Log::LogLevel( Log::L_ERROR ) << "*** cannot open settings file '" << filename << "'" << endl;
            return false;
        }
    }

    // clean the content of settings file buffer
    _fileBuffer = "";
    
    // format the output
    size_t tokens = _settings.size();
    for( size_t cnt = 0; cnt < tokens; cnt++ ) {

        SettingBase* p_setting = _settings[ cnt ];
        const type_info& settings_typeinfo = p_setting->getTypeInfo();
        if ( settings_typeinfo == typeid( bool ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< bool >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( int ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< int >* >( p_setting )->_value );
        } else
        if ( settings_typeinfo == typeid( unsigned int ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< unsigned int >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( string ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< string >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( float ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< float >* >( p_setting )->_value );
        } else
        if ( settings_typeinfo == typeid( osg::Vec3f ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< osg::Vec3f >* >( p_setting )->_value );
        } else
            assert( NULL && "unsupported setting type" );

    }

    // write the buffer into file
    p_stream->write( _fileBuffer.c_str(), ( int )_fileBuffer.length() );
    p_stream->close();

    return true;
}

bool Settings::read( const string& token, string& value )
{
    size_t  curpos = -1;
    // search the exact match of the token string
    do {

        curpos++;
        curpos = _fileBuffer.find( token.c_str(), curpos, token.length() );
        if ( curpos == basic_string< char >::npos ) 
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "*** settings manager could not read requested token '" << token << "', skipping..." << endl;
            return false;

        }

    } while ( _fileBuffer[ curpos + token.length() ] != '=' );

    value = "";
    curpos += token.length() + 1;
    while ( _fileBuffer[ curpos ] != 13 ) 
    {
        value += _fileBuffer[ curpos ];
        curpos++;
    }

    return true;
}

bool Settings::read( const string& token, bool& value )
{
    string  strvalue;
    if ( read( token, strvalue ) == false )
        return false;

    value = ( strvalue == "true" ) ? true : false;
    return true;
}

bool Settings::read( const string& token, int& value )
{
    string  strvalue;
    if ( read( token, strvalue ) == false )
        return false;

    stringstream strstream( strvalue );
    strstream >> value;
    return true;
}

bool Settings::read( const string& token, unsigned int& value )
{
    string  strvalue;
    if ( read( token, strvalue ) == false )
        return false;

    stringstream strstream( strvalue );
    strstream >> value;
    return true;
}

bool Settings::read( const string& token, float& value )
{
    string  strvalue;
    if ( read( token, strvalue ) == false ) 
        return false;

    stringstream strstream( strvalue );
    strstream >> value;
    return true;
}

bool Settings::read( const string& token, osg::Vec3f& value )
{
    string  strvalue;
    if ( read( token, strvalue ) == false ) 
        return false;

    stringstream strstream( strvalue );
    strstream >> value._v [ 0 ] >> value._v [ 1 ] >> value._v [ 2 ];
    return true;
}

bool Settings::write( const string& token, const string& value )
{
    _fileBuffer += string( token + "=" + value + "\r\n" );
    return true;
}

bool Settings::write( const string& token, const bool& value )
{
    string  strvalue = value ? "true" : "false";
    _fileBuffer += string( token + "=" + strvalue + "\r\n" );
    return true;
}

bool Settings::write( const string& token, const int& value )
{
    stringstream str;
    str << token << "=" << value << "\r\n";
    _fileBuffer += str.str();
    return true;
}

bool Settings::write( const string& token, const unsigned int& value )
{
    stringstream str;
    str << token << "=" << value << "\r\n";
    _fileBuffer += str.str();
    return true;
}

bool Settings::write( const string& token, const float& value )
{
    stringstream str;
    str << token << "=" << value << "\r\n";
    _fileBuffer += str.str();
    return true;
}

bool Settings::write( const string& token, const osg::Vec3f& value )
{
    stringstream str;
    str << token << "=" << value._v[ 0 ] << " " << value._v[ 1 ] << " " << value._v[ 2 ] << "\r\n";
    _fileBuffer += str.str();
    return true;
}

} // namespace CTD