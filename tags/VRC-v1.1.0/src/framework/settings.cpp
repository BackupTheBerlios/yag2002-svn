/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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

#include <base.h>
#include "log.h"
#include "settings.h"


namespace yaf3d
{

YAF3D_SINGLETON_IMPL( SettingsManager );

// implementation of SettingsManager
SettingsManager::SettingsManager()
{
}

SettingsManager::~SettingsManager()
{
    // delete all profiles
    std::map< std::string, Settings* >::iterator pp_profile = _profiles.begin(), pp_profileEnd = _profiles.end();
    while ( pp_profile != pp_profileEnd )
        delete pp_profile->second;
}

void SettingsManager::shutdown()
{
    // destroy singleton
    destroy();
}

Settings* SettingsManager::createProfile( const std::string& profilename, const std::string& filename )
{
    std::map< std::string, Settings* >::iterator pp_profile = _profiles.find( profilename ), pp_profileEnd = _profiles.end();
    if ( pp_profile != pp_profileEnd )
    {
        log_error << "*** requested profile '" << profilename << "' already exists!" << std::endl;
        return NULL;
    }

    Settings* p_settings = new Settings;
    p_settings->setFileName( filename );
    _profiles.insert( make_pair( profilename, p_settings ) );
    return p_settings;
}

void SettingsManager::destroyProfile( const std::string& profilename )
{
    std::map< std::string, Settings* >::iterator pp_profile = _profiles.find( profilename ), pp_profileEnd = _profiles.end();
    if ( pp_profile == pp_profileEnd )
    {
        log_error << "*** profile '" << profilename << "' does not exist!" << std::endl;
        return;
    }

    delete pp_profile->second;
    _profiles.erase( pp_profile );
}

bool SettingsManager::loadProfile( const std::string& profilename )
{
    std::map< std::string, Settings* >::iterator pp_profile = _profiles.find( profilename ), pp_profileEnd = _profiles.end();
    if ( pp_profile == pp_profileEnd )
    {
        log_error << "*** profile '" << profilename << "' does not exist!" << std::endl;
        return false;
    }

    return pp_profile->second->load();
}

bool SettingsManager::storeProfile( const std::string& profilename )
{
    std::map< std::string, Settings* >::iterator pp_profile = _profiles.find( profilename ), pp_profileEnd = _profiles.end();
    if ( pp_profile == pp_profileEnd )
    {
        log_error << "*** profile '" << profilename << "' does not exist!" << std::endl;
        return false;
    }

    return pp_profile->second->store();
}

Settings* SettingsManager::getProfile( const std::string& profilename )
{
    std::map< std::string, Settings* >::iterator pp_profile = _profiles.find( profilename ), pp_profileEnd = _profiles.end();
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
    std::string openfile = filename;
    if ( !filename.length() )
        openfile = _settingsFile;

    std::auto_ptr< std::fstream > p_stream( new std::fstream );

    p_stream->open( openfile.c_str(), std::ios_base::binary | std::ios_base::in );
    // if the file does not exist then create one
    if ( !*p_stream )
    {   
        p_stream->open( openfile.c_str(), std::ios_base::binary | std::ios_base::out );
        log_warning << "*** settings file '" << openfile << "' does not exist. one has been created." << std::endl;
        _loaded   = true;
        return false;
    }

    // get file size
    p_stream->seekg( 0, std::ios_base::end );
    int filesize = ( int )p_stream->tellg();
    // check for empty file
    if ( filesize == 0 )
    {
        p_stream->close();
        return false;
    }

    p_stream->seekg( 0, std::ios_base::beg );

    // load the settings into the file buffer
    _fileBuffer = "";
    char* p_buf = new char[ filesize ];
    p_stream->read( p_buf, filesize );
    p_buf[ filesize - 1 ] = 0;
    _fileBuffer = p_buf;
    _fileBuffer += "\n"; // terminate the buffer std::string by a CR
    delete[] p_buf;

    p_stream->close();

    // format the intput
    size_t tokens = _settings.size();
    for( size_t cnt = 0; cnt < tokens; ++cnt ) 
    {
        SettingBase* p_setting = _settings[ cnt ];
        const std::type_info& settings_typeinfo = p_setting->getTypeInfo();
        if ( settings_typeinfo == typeid( bool ) ) {
            read( p_setting->getTokenName(), static_cast< Setting< bool >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( int ) ) {
            read( p_setting->getTokenName(), static_cast< Setting< int >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( unsigned int ) ) {
            read( p_setting->getTokenName(), static_cast< Setting< unsigned int >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( std::string ) ) {
            read( p_setting->getTokenName(), static_cast< Setting< std::string >* >( p_setting )->_value );
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

bool Settings::store( const std::string& filename )
{
    std::auto_ptr< std::fstream > p_stream( new std::fstream );

    if ( !filename.length() )
    {
        if ( !_loaded ) 
        {
            log_warning << "*** no settings file was previously loaded" << std::endl;
            return false;
        }
        p_stream->open( _settingsFile.c_str(), std::ios_base::binary | std::ios_base::out );
        if ( !*p_stream )
        {
            log_error << "*** cannot open settings file '" << filename << "'" << std::endl;
            assert( NULL && " internal error, cannot open settings file for writing" );
        }
    } 
    else
    {
        p_stream->open( filename.c_str(), std::ios_base::binary | std::ios_base::out );
        if ( !*p_stream )
        {   
            log_error << "*** cannot open settings file '" << filename << "'" << std::endl;
            return false;
        }
    }

    // clean the content of settings file buffer
    _fileBuffer = "";
    
    // format the output
    size_t tokens = _settings.size();
    for( size_t cnt = 0; cnt < tokens; ++cnt ) {

        SettingBase* p_setting = _settings[ cnt ];
        const std::type_info& settings_typeinfo = p_setting->getTypeInfo();
        if ( settings_typeinfo == typeid( bool ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< bool >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( int ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< int >* >( p_setting )->_value );
        } else
        if ( settings_typeinfo == typeid( unsigned int ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< unsigned int >* >( p_setting )->_value );
        } else 
        if ( settings_typeinfo == typeid( std::string ) ) {
            write( p_setting->getTokenName(), static_cast< Setting< std::string >* >( p_setting )->_value );
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

bool Settings::read( const std::string& token, std::string& value )
{
    size_t  curpos = ( size_t )-1;
    // search the exact match of the token std::string
    do {

        ++curpos;
        curpos = _fileBuffer.find( token.c_str(), curpos, token.length() );
        if ( curpos == std::basic_string< char >::npos ) 
        {
            log_debug << "*** settings manager could not read requested token '" << token << "', skipping..." << std::endl;
            return false;

        }

    } while ( _fileBuffer[ curpos + token.length() ] != '=' );

    value = "";
    curpos += token.length() + 1;
    while ( _fileBuffer[ curpos ] != 13 ) 
    {
        value += _fileBuffer[ curpos ];
        ++curpos;
    }

    return true;
}

bool Settings::read( const std::string& token, bool& value )
{
    std::string  strvalue;
    if ( read( token, strvalue ) == false )
        return false;

    value = ( strvalue == "true" ) ? true : false;
    return true;
}

bool Settings::read( const std::string& token, int& value )
{
    std::string  strvalue;
    if ( read( token, strvalue ) == false )
        return false;

    std::stringstream strstream( strvalue );
    strstream >> value;
    return true;
}

bool Settings::read( const std::string& token, unsigned int& value )
{
    std::string  strvalue;
    if ( read( token, strvalue ) == false )
        return false;

    std::stringstream strstream( strvalue );
    strstream >> value;
    return true;
}

bool Settings::read( const std::string& token, float& value )
{
    std::string  strvalue;
    if ( read( token, strvalue ) == false ) 
        return false;

    std::stringstream strstream( strvalue );
    strstream >> value;
    return true;
}

bool Settings::read( const std::string& token, osg::Vec3f& value )
{
    std::string  strvalue;
    if ( read( token, strvalue ) == false ) 
        return false;

    std::stringstream strstream( strvalue );
    strstream >> value._v [ 0 ] >> value._v [ 1 ] >> value._v [ 2 ];
    return true;
}

bool Settings::write( const std::string& token, const std::string& value )
{
    _fileBuffer += std::string( token + "=" + value + "\r\n" );
    return true;
}

bool Settings::write( const std::string& token, const bool& value )
{
    std::string  strvalue = value ? "true" : "false";
    _fileBuffer += std::string( token + "=" + strvalue + "\r\n" );
    return true;
}

bool Settings::write( const std::string& token, const int& value )
{
    std::stringstream str;
    str << token << "=" << value << "\r\n";
    _fileBuffer += str.str();
    return true;
}

bool Settings::write( const std::string& token, const unsigned int& value )
{
    std::stringstream str;
    str << token << "=" << value << "\r\n";
    _fileBuffer += str.str();
    return true;
}

bool Settings::write( const std::string& token, const float& value )
{
    std::stringstream str;
    str << token << "=" << value << "\r\n";
    _fileBuffer += str.str();
    return true;
}

bool Settings::write( const std::string& token, const osg::Vec3f& value )
{
    std::stringstream str;
    str << token << "=" << value._v[ 0 ] << " " << value._v[ 1 ] << " " << value._v[ 2 ] << "\r\n";
    _fileBuffer += str.str();
    return true;
}

} // namespace yaf3d
