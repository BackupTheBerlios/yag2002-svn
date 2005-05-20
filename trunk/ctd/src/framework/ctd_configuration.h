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

#ifndef _CTD_CONFIGURATION_H_
#define _CTD_CONFIGURATION_H_

#include <ctd_base.h>
#include <ctd_singleton.h>
#include "ctd_settings.h"

namespace CTD
{

//! Profile name of game settings
#define CTD_GAMESETTING_PROFILENAME     "gamesettings"
//! File name for game settings storage
#define CTD_GAMESETTING_FILENAME        "gamesettings.cfg"

//! Game settings' token names
//keyboard type
#define CTD_GS_KEYBOARD                 "keyboard"
#define CTD_GS_KEYBOARD_ENGLISH         "english"
#define CTD_GS_KEYBOARD_GERMAN          "german"
// screen
#define CTD_GS_SCREENWIDTH              "screenWidth"
#define CTD_GS_SCREENHEIGHT             "screenHeight"
#define CTD_GS_FULLSCREEN               "fullScreen"

// gui
#define CTD_GS_GUISCHEME                "guiScheme"
// player
#define CTD_GS_PLAYERNAME               "playerName"
#define CTD_GS_MOUSESENS                "mouseSensitivity"
#define CTD_GS_MAX_MOUSESENS            3.0f                /* maximal mouse sensitivity */
#define CTD_GS_INVERTMOUSE              "mouseInverted"
//   player key bindings
#define CTD_GS_KEY_MOVE_FORWARD         "moveForward"
#define CTD_GS_KEY_MOVE_BACKWARD        "moveBackward"
#define CTD_GS_KEY_MOVE_LEFT            "moveLeft"
#define CTD_GS_KEY_MOVE_RIGHT           "moveRight"
#define CTD_GS_KEY_JUMP                 "jump"
#define CTD_GS_KEY_CAMERAMODE           "cameramode"
#define CTD_GS_KEY_CHATMODE             "chatmode"
// networking
#define CTD_GS_SERVER_NAME              "serverName"
#define CTD_GS_SERVER_IP                "serverIP"
#define CTD_GS_SERVER_PORT              "serverPort"

class Application;

//! Game configuration
class Configuration : public Singleton< Configuration >
{
    public:

                                                Configuration();

        virtual                                 ~Configuration();


        //! Given a setting name ( e.g. screen width CTD_GS_SCREENWIDTH ) return its value in 'value'
        template< typename TypeT >
        inline bool                             getSettingValue( const std::string& name, TypeT& value );

        //! Set setting value. Returns false if setting does not exist.
        template< class TypeT >
        inline bool                             setSettingValue( const std::string& name, const TypeT& value );

        //! Store the settings to file
        void                                    store();

    protected:

        //! Update the gui system including drawing
        void                                    update( float deltaTime );

        
        //! Shutdown the network device.
        void                                    shutdown();

        //! Avoid copy constructor
        Configuration&                          operator = ( const Configuration& config );

        //! Configuration settings
        Settings*                               _p_settings;

        //! Game settings
        unsigned int                            _screenWidth;

        unsigned int                            _screenHeight;

        bool                                    _fullScreen;

        std::string                             _keyboardType;

        std::string                             _guiScheme;

        std::string                             _playerName;

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

} // namespace CTD

#endif //_CTD_CONFIGURATION_H_
