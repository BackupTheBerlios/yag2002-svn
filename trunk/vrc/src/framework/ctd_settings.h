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
 # Class for managing game settings
 #
 #   date of creation:  03/08/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/08/2005 boto       creation of Settings
 #
 ################################################################*/

#ifndef _CTD_SETTINGS_H_
#define _CTD_SETTINGS_H_

#include <ctd_base.h>
#include <ctd_singleton.h>

namespace CTD
{

class Settings;

//! Settings manager ( look at the end of file for an usage example )
class SettingsManager : public Singleton< SettingsManager >
{
    public:

        //! Create a new named profile and its associated file.
        Settings*                               createProfile( const std::string& profilename, const std::string& filename );

        //! Destroy given profile if it exists.
        void                                    destroyProfile( const std::string& profilename );

        //! Load an already created profile.
        bool                                    loadProfile( const std::string& profilename );

        //! Store an already created and loaded profile.
        bool                                    storeProfile( const std::string& profilename );

        //! Given a profile name return its instance, returns NULL if the profile does not exist.
        Settings*                               getProfile( const std::string& profilename );

    protected:

                                                SettingsManager();

                                                ~SettingsManager();

        SettingsManager&                        operator = ( const SettingsManager& );

        std::map< std::string, Settings* >      _profiles;

    friend class Singleton< SettingsManager >;
};

//! Class for managing game settings
class Settings
{

    public:

        //! Register one single setting
        template< typename TypeT >
        bool                                    registerSetting( const std::string& token, const TypeT& value );

        //! Get token value
        template< class TypeT >
        bool                                    getValue( const std::string& token, TypeT& value );

        //! Set token value
        template< class TypeT >
        bool                                    setValue( const std::string& token, const TypeT& value );

    protected:

                                                Settings();

                                                ~Settings();

        Settings&                               operator = ( const Settings& );


        //! Load settings from given file. If filename is empty the preset file name is used.
        bool                                    load( const std::string& filename = "" );

        //! Store settings. If filename is empty the already loaded file is used for storing.
        bool                                    store( const std::string& filename = "" );

        //! Set file name associated with this settings instance
        void                                    setFileName( const std::string& filename );

        //! Base class for all types of settings
        class SettingBase
        {

            public:
                
                                                SettingBase() {};

                //! Get setting's name
                const std::string&              getTokenName()
                                                {
                                                    return _token;
                                                }

                //! Retrieve the type info of value
                virtual const std::type_info&   getTypeInfo() = 0;

            protected:

                //! Token name
                std::string                     _token;

        };

        //! This class contains one setting with associated value
        template< class TypeT >
        class Setting : public SettingBase
        {

            public:

                typedef typename TypeT          TokenType;

                                                Setting( const std::string& name, TokenType value )
                                                {
                                                    _token = name;
                                                    _value = value;
                                                }

                                                ~Setting() {}

                const std::type_info&           getTypeInfo()
                                                {
                                                    return typeid( TokenType );
                                                }

            protected:


                //! Setting's value
                TokenType                       _value;

            friend  class Settings;

        };

        //! Find the setting with given name in settings list 
        SettingBase*                            findSetting( const std::string& token );

        //! Helper method for writing one setting of type string
        bool                                    write( const std::string& token, const std::string& value );

        //! Helper method for writing one setting of type bool
        bool                                    write( const std::string& token, const bool& value );

        //! Helper method for writing one setting of type integer
        bool                                    write( const std::string& token, const int& value );

        //! Helper method for writing one setting of type float
        bool                                    write( const std::string& token, const float& value );

        //! Helper method for writing one setting of type Vec3f
        bool                                    write( const std::string& token, const osg::Vec3f& value );

        //! Helper method for reading given setting and its value of type string
        bool                                    read( const std::string& token, std::string& value );

        //! Helper method for reading given setting and its value of type bool
        bool                                    read( const std::string& token, bool& value );

        //! Helper method for reading given setting and its value of type integer
        bool                                    read( const std::string& token, int& value );

        //! Helper method for reading given setting and its value of type float
        bool                                    read( const std::string& token, float& value );

        //! Helper method for reading given setting and its value of type Vector3d
        bool                                    read( const std::string& token, osg::Vec3f& value );

        //! List of game settings
        std::vector< SettingBase* >             _settings;

        //! File buffer
        std::string                             _fileBuffer;

        //! file name where the settings are loaded and stored
        std::string                             _settingsFile;

        //! Shows that a setting file has been successfully loaded
        bool                                    _loaded;

        std::fstream*                           _p_stream;

    friend class SettingsManager;
};


#include "ctd_settings_inl.h"

} // namespace CTD

#endif // _CTD_SETTINGS_H_


/**
Usage example:

    Settings* p_settings = SettingsManager::get()->createProfile( "gamesettings", rootpath + "settings.cfg" );
    bool   value_bool    = true;
    int    value_int     = 12345;
    float  value_float   = 0.98765f;
    string value_string  = string( "hello world" );
    Vec3f  value_vec     = Vec3f( 1.1f, 2.2f, 3.3f );

    // register token/default value settings
    p_settings->registerSetting( "token_bool",   value_bool   );
    p_settings->registerSetting( "token_int",    value_int    );
    p_settings->registerSetting( "token_float",  value_float  );
    p_settings->registerSetting( "token_string", value_string );
    p_settings->registerSetting( "token_vec",    value_vec    );
    
    // load profile
    SettingsManager::get()->loadProfile( "gamesettings" );
    
    // get loaded values
    p_settings->getValue( "token_bool",   value_bool );
    p_settings->getValue( "token_int",    value_int );
    p_settings->getValue( "token_float",  value_float );
    p_settings->getValue( "token_string", value_string );
    p_settings->getValue( "token_vec",    value_vec );

    // modify the settings
    value_bool = value_bool ? false : true;
    value_int++;
    value_float  += 1.5f;
    value_string = value_string == "hello world" ? "bye bye world" : "hello world";
    value_vec    += Vec3f( 1.0f, 1.0f, 1.0f );

    // set the modified settings
    p_settings->setValue( "token_bool",   value_bool );
    p_settings->setValue( "token_int",    value_int );
    p_settings->setValue( "token_float",  value_float );
    p_settings->setValue( "token_string", value_string );
    p_settings->setValue( "token_vec",    value_vec );

    // store back changes ( into associated file )
    SettingsManager::get()->storeProfile( "gamesettings" );
*/
