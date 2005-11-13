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
 # the overall game configuration is handled here
 #  - loading
 #  - storing
 #  - value retrieval method
 #  - value modificaton method
 #
 #   date of creation:  04/02/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _YAF3D_CONFIGURATION_H_
#define _YAF3D_CONFIGURATION_H_

#include <ctd_base.h>
#include <ctd_singleton.h>
#include "ctd_settings.h"

namespace yaf3d
{

//! Profile name of game settings
#define YAF3D_GAMESETTING_PROFILENAME     "gamesettings"
//! File name for game settings storage
#define YAF3D_GAMESETTING_FILENAME        "gamesettings.cfg"

//! Game settings' token names
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
// player
#define YAF3D_GS_PLAYER_NAME              "playerName"
//   folder in media directory for player configuration files
#define YAF3D_GS_PLAYER_CONFIG_DIR        "playerConfigDir"
//   player configuration
#define YAF3D_GS_PLAYER_CONFIG            "playerConfig"
//   player key bindings
#define YAF3D_GS_KEY_MOVE_FORWARD         "moveForward"
#define YAF3D_GS_KEY_MOVE_BACKWARD        "moveBackward"
#define YAF3D_GS_KEY_MOVE_LEFT            "moveLeft"
#define YAF3D_GS_KEY_MOVE_RIGHT           "moveRight"
#define YAF3D_GS_KEY_JUMP                 "jump"
#define YAF3D_GS_KEY_CAMERAMODE           "cameraMode"
#define YAF3D_GS_KEY_CHATMODE             "chatMode"
// mouse settings
#define YAF3D_GS_MOUSESENS                "mouseSensitivity"
#define YAF3D_GS_MAX_MOUSESENS            3.0f                /* maximal mouse sensitivity */
#define YAF3D_GS_INVERTMOUSE              "mouseInverted"
// networking
#define YAF3D_GS_SERVER_NAME              "serverName"
#define YAF3D_GS_SERVER_IP                "serverIP"
#define YAF3D_GS_SERVER_PORT              "serverPort"

class Application;

//! Game configuration
class Configuration : public Singleton< Configuration >
{
    public:

        //! Given a setting name ( e.g. screen width YAF3D_GS_SCREENWIDTH ) return its value in 'value'
        template< typename TypeT >
        inline bool                             getSettingValue( const std::string& name, TypeT& value );

        //! Set value for given setting. Returns false if setting does not exist.
        template< class TypeT >
        inline bool                             setSettingValue( const std::string& name, const TypeT& value );

        //! Set setting given its name and its value as string.
        bool                                    setSettingValueAsString( const std::string& name, const std::string& value );

        //! Get the configuration settings as string pair < setting name, setting value >
        void                                    getConfigurationAsString( std::vector< std::pair< std::string, std::string > >& settings );

        //! Store the settings to file
        void                                    store();

    protected:

                                                Configuration();

        virtual                                 ~Configuration();
        
        //! Shutdown the network device.
        void                                    shutdown();

        //! Configuration settings
        Settings*                               _p_settings;

        //! Game settings
        unsigned int                            _screenWidth;

        unsigned int                            _screenHeight;

        unsigned int                            _colorBits;

        bool                                    _fullScreen;

        std::string                             _keyboardType;

        std::string                             _guiScheme;

        std::string                             _playerName;

        std::string                             _playerConfig;

        std::string                             _playerConfigDir;

        float                                   _mouseSensitivity;

        bool                                    _mouseInverted;

        std::string                             _serverName;

        std::string                             _serverIP;
        
        unsigned int                            _serverPort;

        std::string                             _moveForward;

        std::string                             _moveBackward;
        
        std::string                             _moveLeft;
        
        std::string                             _moveRight;

        std::string                             _jump;

        std::string                             _cameramode;

        std::string                             _chatmode;
    
    friend class Singleton< Configuration >;
    friend class Application;
};                                             

// inline methods

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

#endif //_YAF3D_CONFIGURATION_H_