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
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/08/2005 boto       creation of Settings
 #
 ################################################################*/

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <base.h>
#include <singleton.h>
#include <smartptr.h>


namespace yaf3d
{

class Application;
class Settings;

//! Typedef for the settings smart pointer
typedef SmartPtr< Settings >   SettingsPtr;

//! Settings manager ( look at the end of file for an usage example )
class SettingsManager : public Singleton< SettingsManager >
{
    public:

        //! Create a new named profile and its associated file.
        SettingsPtr                            createProfile( const std::string& profilename, const std::string& filename );

        //! Destroy given profile if it exists.
        void                                    destroyProfile( const std::string& profilename );

        //! Load an already created profile.
        bool                                    loadProfile( const std::string& profilename );

        //! Store an already created and loaded profile.
        bool                                    storeProfile( const std::string& profilename );

        //! Given a profile name return its instance, returns NULL if the profile does not exist.
        SettingsPtr                             getProfile( const std::string& profilename );

    protected:

                                                SettingsManager();

        virtual                                 ~SettingsManager();

        void                                    shutdown();

        std::map< std::string, SettingsPtr >    _profiles;

    friend class Singleton< SettingsManager >;
    friend class Application;
};

//! Class for managing game settings
class Settings : public RefCount< Settings >
{
    //! Set the smart pointer class as friend
    DECLARE_SMARTPTR_ACCESS( Settings )

    public:

        //! Base class for all types of settings
        class SettingBase
        {

            public:
                
                virtual                         ~SettingBase() {};

                //! Get setting's name
                const std::string&              getTokenName() const
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

                typedef TypeT                   SettingTokenType;

                                                Setting( const std::string& name, SettingTokenType value )
                                                {
                                                    _token = name;
                                                    _value = value;
                                                }

                virtual                         ~Setting() {}

                const std::type_info&           getTypeInfo()
                                                {
                                                    return typeid( SettingTokenType );
                                                }

            protected:


                //! Setting's value
                SettingTokenType                _value;

            friend  class Settings;

        };

        //! Constructor
                                                Settings();

        //! Set file name associated with this settings instance. If file name is set then the load method can be called with default arguments.
        void                                    setFileName( const std::string& filename );

        //! Register one single setting. Use this method for registering all tokens before calling 'load'.
        template< typename TypeT >
        bool                                    registerSetting( const std::string& token, const TypeT& value );

        //! Load settings from given file. If filename is empty the preset file name is used.
        //! If readonly is true then the file is loaded from virtual file system, and the settings cannot be written back.
        bool                                    load( bool readonly = false, const std::string& filename = "" );

        //! Store settings. If filename is empty the already loaded file is used for storing.
        //! NOTE: if the settings file was loaded in read-only mode then an attempt to store will fail!
        bool                                    store( const std::string& filename = "" );

        //! Get token value
        template< class TypeT >
        bool                                    getValue( const std::string& token, TypeT& value );

        //! Set token value
        template< class TypeT >
        bool                                    setValue( const std::string& token, const TypeT& value );

        //! Find the setting with given name in settings list 
        SettingBase*                            findSetting( const std::string& token );

        //! Get all storage objects for settings
        inline const std::vector< Settings::SettingBase* >& getAllSettingStorages() const;

    protected:

        virtual                                 ~Settings();

        //! Helper method for writing one setting of type string
        bool                                    write( const std::string& token, const std::string& value );

        //! Helper method for writing one setting of type bool
        bool                                    write( const std::string& token, const bool& value );

        //! Helper method for writing one setting of type integer
        bool                                    write( const std::string& token, const int& value );

        //! Helper method for writing one setting of type unsigned integer
        bool                                    write( const std::string& token, const unsigned int& value );

        //! Helper method for writing one setting of type float
        bool                                    write( const std::string& token, const float& value );

        //! Helper method for writing one setting of type Vec3f
        bool                                    write( const std::string& token, const osg::Vec3f& value );

        //! Helper method for writing one setting of type Vec2f
        bool                                    write( const std::string& token, const osg::Vec2f& value );

        //! Helper method for reading given setting and its value of type string
        bool                                    read( const std::string& token, std::string& value );

        //! Helper method for reading given setting and its value of type bool
        bool                                    read( const std::string& token, bool& value );

        //! Helper method for reading given setting and its value of type integer
        bool                                    read( const std::string& token, int& value );

        //! Helper method for reading given setting and its value of type unsigned integer
        bool                                    read( const std::string& token, unsigned int& value );

        //! Helper method for reading given setting and its value of type float
        bool                                    read( const std::string& token, float& value );

        //! Helper method for reading given setting and its value of type Vector3d
        bool                                    read( const std::string& token, osg::Vec3f& value );

        //! Helper method for reading given setting and its value of type Vector2d
        bool                                    read( const std::string& token, osg::Vec2f& value );

        //! List of settings
        std::vector< SettingBase* >             _settings;

        //! File buffer
        std::string                             _fileBuffer;

        //! file name where the settings are loaded and stored
        std::string                             _settingsFile;

        //! Shows that a setting file has been successfully loaded
        bool                                    _loaded;
};


#include "settings.inl"

} // namespace yaf3d

#endif // _SETTINGS_H_


/**
Usage example:

    SettingsPtr settings = SettingsManager::get()->createProfile( "gamesettings", rootpath + "settings.cfg" );
    bool   value_bool    = true;
    int    value_int     = 12345;
    float  value_float   = 0.98765f;
    string value_string  = string( "hello world" );
    Vec3f  value_vec     = Vec3f( 1.1f, 2.2f, 3.3f );

    // register token/default value settings
    settings->registerSetting( "token_bool",   value_bool   );
    settings->registerSetting( "token_int",    value_int    );
    settings->registerSetting( "token_float",  value_float  );
    settings->registerSetting( "token_string", value_string );
    settings->registerSetting( "token_vec",    value_vec    );
    
    // load profile
    SettingsManager::get()->loadProfile( "gamesettings" );
    
    // get loaded values
    settings->getValue( "token_bool",   value_bool );
    settings->getValue( "token_int",    value_int );
    settings->getValue( "token_float",  value_float );
    settings->getValue( "token_string", value_string );
    settings->getValue( "token_vec",    value_vec );

    // modify the settings
    value_bool = value_bool ? false : true;
    ++value_int;
    value_float  += 1.5f;
    value_string = value_string == "hello world" ? "bye bye world" : "hello world";
    value_vec    += Vec3f( 1.0f, 1.0f, 1.0f );

    // set the modified settings
    settings->setValue( "token_bool",   value_bool );
    settings->setValue( "token_int",    value_int );
    settings->setValue( "token_float",  value_float );
    settings->setValue( "token_string", value_string );
    settings->setValue( "token_vec",    value_vec );

    // store back changes ( into associated file )
    SettingsManager::get()->storeProfile( "gamesettings" );
*/
