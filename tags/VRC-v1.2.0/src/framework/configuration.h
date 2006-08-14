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
 # the framework configuration can be accessed here
 #
 #   date of creation:  04/02/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <base.h>
#include <singleton.h>
#include "settings.h"

namespace yaf3d
{

//! Profile name of game settings
#define YAF3D_GAMESETTING_PROFILENAME     "gamesettings"
//! File name for game settings storage
#define YAF3D_GAMESETTING_FILENAME        "gamesettings.cfg"

//! Settings' token names

// log level
#define YAF3D_GS_LOG_LEVEL                "logLevel"

//keyboard type
#define YAF3D_GS_KEYBOARD                 "keyboard"
#define YAF3D_GS_KEYBOARD_ENGLISH         "english"
#define YAF3D_GS_KEYBOARD_GERMAN          "german"

// screen
#define YAF3D_GS_SCREENWIDTH              "screenWidth"
#define YAF3D_GS_SCREENHEIGHT             "screenHeight"
#define YAF3D_GS_COLORBITS                "colorBits"
#define YAF3D_GS_FULLSCREEN               "fullScreen"

// gui
#define YAF3D_GS_GUISCHEME                "guiScheme"
#define YAF3D_GS_GUI_DEFUALT_SCHEME       "gui/schemes/TaharezLook.scheme"

// networking
#define YAF3D_GS_SERVER_NAME              "serverName"
#define YAF3D_GS_SERVER_IP                "serverIP"
#define YAF3D_GS_SERVER_PORT              "serverPort"
#define YAF3D_GS_PUBLIC_HOST              "publicHost"
#define YAF3D_GS_DEFAULT_SERVERPORT       32000

class Application;

//! Game configuration
class Configuration : public Singleton< Configuration >
{
    public:

        //! Load settings from file to all registered settings. Returns false on file io errors.
        bool                                    load();

        //! Store the settings to file. Returns false on file io errors.
        bool                                    store();

        //! Add a new setting name to configuration. Returns false if setting already exists.
        template< typename TypeT >
        inline bool                             addSetting( const std::string& name, TypeT& value );

        //! Given a setting name ( e.g. screen width YAF3D_GS_SCREENWIDTH ) return its value in 'value'. Returns false if setting does not exist.
        template< typename TypeT >
        inline bool                             getSettingValue( const std::string& name, TypeT& value );

        //! Set value for given setting. Returns false if setting does not exist.
        template< class TypeT >
        inline bool                             setSettingValue( const std::string& name, const TypeT& value );

        //! Set setting given its name and its value as string.
        bool                                    setSettingValueAsString( const std::string& name, const std::string& value );

        //! Get the configuration settings as string pair < setting name, setting value >
        void                                    getConfigurationAsString( std::vector< std::pair< std::string, std::string > >& settings );

    protected:

                                                Configuration();

        virtual                                 ~Configuration();
        
        //! Shutdown the network device.
        void                                    shutdown();

        //! Configuration settings
        Settings*                               _p_settings;

        //! Standard settings
        std::string                             _logLevel;

        unsigned int                            _screenWidth;

        unsigned int                            _screenHeight;

        unsigned int                            _colorBits;

        bool                                    _fullScreen;

        std::string                             _keyboardType;

        std::string                             _guiScheme;

        std::string                             _serverName;

        std::string                             _serverIP;
        
        unsigned int                            _serverPort;

        std::string                             _publicHost;

    friend class Singleton< Configuration >;
    friend class Application;
};                                             

// inline methods

template< typename TypeT >
inline bool Configuration::addSetting( const std::string& name, TypeT& value )
{
    return _p_settings->registerSetting( name, value );
}

template< typename TypeT >
inline bool Configuration::getSettingValue( const std::string& name, TypeT& value )
{
    return _p_settings->getValue( name, value );
}

template< typename TypeT >
inline bool Configuration::setSettingValue( const std::string& name, const TypeT& value )
{
    return _p_settings->setValue( name, value );
}

} // namespace yaf3d

#endif // _CONFIGURATION_H_
