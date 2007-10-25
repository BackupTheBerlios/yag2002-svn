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
 # key map: provides english and german key mapping
 #
 #   date of creation:  04/08/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "log.h"
#include "keymap.h"

namespace yaf3d
{

YAF3D_SINGLETON_IMPL( KeyMap )

//! Implementation of KeyMap
KeyMap::KeyMap() :
_keyboardType( KeyMap::English )
{
    setup( KeyMap::English );
}

void KeyMap::setup( KeyMap::KeyboardType type )
{
    _keyboardType = type;
    _keyNames.clear();
    _keyCodes.clear();
    _mouseBtnNames.clear();
    _mouseBtnCodes.clear();

    // mouse buttons' names and codes
    _mouseBtnNames.insert( std::make_pair( "LMB",   osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON   ) );
    _mouseBtnNames.insert( std::make_pair( "RMB",   osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON  ) );
    _mouseBtnNames.insert( std::make_pair( "MMB",   osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON ) );
    _mouseBtnCodes.insert( std::make_pair( osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON,    "LMB"  ) );
    _mouseBtnCodes.insert( std::make_pair( osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON,   "RMB"  ) );
    _mouseBtnCodes.insert( std::make_pair( osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON , "MMB"  ) );

    // keyboard's key names and codes
    _keyNames.insert( std::make_pair( "CtrlL",      SDLK_LCTRL     ) );
    _keyNames.insert( std::make_pair( "CtrlR",      SDLK_RCTRL     ) );
    _keyNames.insert( std::make_pair( "SuperL",     SDLK_LSUPER    ) );
    _keyNames.insert( std::make_pair( "SuperR",     SDLK_RSUPER    ) );
    _keyNames.insert( std::make_pair( "AltL",       SDLK_LALT      ) );
    _keyNames.insert( std::make_pair( "AltR",       SDLK_RALT      ) );
    _keyNames.insert( std::make_pair( "Space",      SDLK_SPACE     ) );
    _keyNames.insert( std::make_pair( "Switch",     SDLK_MODE      ) );
    _keyNames.insert( std::make_pair( "Menu",       SDLK_MENU      ) );
    _keyNames.insert( std::make_pair( "ShiftL",     SDLK_LSHIFT    ) );
    _keyNames.insert( std::make_pair( "ShiftR",     SDLK_RSHIFT    ) );
    _keyNames.insert( std::make_pair( "Return",     SDLK_RETURN    ) );
    _keyNames.insert( std::make_pair( "BackQuote",  SDLK_BACKQUOTE ) );
    _keyNames.insert( std::make_pair( "Caps",       SDLK_CAPSLOCK  ) );
    _keyNames.insert( std::make_pair( "Tab",        SDLK_TAB       ) );

    _keyNames.insert( std::make_pair( "A",          'a' ) );
    _keyNames.insert( std::make_pair( "B",          'b' ) );
    _keyNames.insert( std::make_pair( "C",          'c' ) );
    _keyNames.insert( std::make_pair( "D",          'd' ) );
    _keyNames.insert( std::make_pair( "E",          'e' ) );
    _keyNames.insert( std::make_pair( "F",          'f' ) );
    _keyNames.insert( std::make_pair( "G",          'g' ) );
    _keyNames.insert( std::make_pair( "H",          'h' ) );
    _keyNames.insert( std::make_pair( "I",          'i' ) );
    _keyNames.insert( std::make_pair( "J",          'j' ) );
    _keyNames.insert( std::make_pair( "K",          'k' ) );
    _keyNames.insert( std::make_pair( "L",          'l' ) );
    _keyNames.insert( std::make_pair( "M",          'm' ) );
    _keyNames.insert( std::make_pair( "N",          'n' ) );
    _keyNames.insert( std::make_pair( "O",          'o' ) );
    _keyNames.insert( std::make_pair( "P",          'p' ) );
    _keyNames.insert( std::make_pair( "Q",          'q' ) );
    _keyNames.insert( std::make_pair( "R",          'r' ) );
    _keyNames.insert( std::make_pair( "S",          's' ) );
    _keyNames.insert( std::make_pair( "T",          't' ) );
    _keyNames.insert( std::make_pair( "U",          'u' ) );
    _keyNames.insert( std::make_pair( "V",          'v' ) );
    _keyNames.insert( std::make_pair( "W",          'w' ) );
    _keyNames.insert( std::make_pair( "X",          'x' ) );
    _keyNames.insert( std::make_pair( "Y",          'y' ) );
    _keyNames.insert( std::make_pair( "Z",          'z' ) );

    _keyNames.insert( std::make_pair( ";",          ';' ) );
    _keyNames.insert( std::make_pair( "[",          '[' ) );
    _keyNames.insert( std::make_pair( ">",          '>' ) );
    _keyNames.insert( std::make_pair( "<",          '<' ) );
    _keyNames.insert( std::make_pair( "_",          '_' ) );
    _keyNames.insert( std::make_pair( "?",          '?' ) );
    _keyNames.insert( std::make_pair( "(",          '(' ) );
    _keyNames.insert( std::make_pair( ")",          ')' ) );
    _keyNames.insert( std::make_pair( "+",          '+' ) );
    _keyNames.insert( std::make_pair( "*",          '*' ) );
    _keyNames.insert( std::make_pair( "|",          '|' ) );
    _keyNames.insert( std::make_pair( ">",          '>' ) );
    _keyNames.insert( std::make_pair( "\\",         '\\') );
    _keyNames.insert( std::make_pair( "&",          '&' ) );
    _keyNames.insert( std::make_pair( "^",          '^' ) );
    _keyNames.insert( std::make_pair( "#",          '#' ) );
    _keyNames.insert( std::make_pair( "`",          '`' ) );
    _keyNames.insert( std::make_pair( "~",          '~' ) );
    _keyNames.insert( std::make_pair( "]",          ']' ) );
    _keyNames.insert( std::make_pair( ":",          ':' ) );
    _keyNames.insert( std::make_pair( "/",          '/' ) );
    _keyNames.insert( std::make_pair( "{",          '{' ) );
    _keyNames.insert( std::make_pair( "}",          '}' ) );
    _keyNames.insert( std::make_pair( "-",          '-' ) );
    _keyNames.insert( std::make_pair( "!",          '!' ) );
    _keyNames.insert( std::make_pair( "\"",         '\"') );
    _keyNames.insert( std::make_pair( "§",          '§' ) );
    _keyNames.insert( std::make_pair( "%",          '%' ) );
    _keyNames.insert( std::make_pair( "&",          '&' ) );
    _keyNames.insert( std::make_pair( "´",          '´' ) );
    _keyNames.insert( std::make_pair( "~",          '~' ) );
    _keyNames.insert( std::make_pair( "=",          '=' ) );

    _keyNames.insert( std::make_pair( "1",          '1' ) );
    _keyNames.insert( std::make_pair( "2",          '2' ) );
    _keyNames.insert( std::make_pair( "3",          '3' ) );
    _keyNames.insert( std::make_pair( "4",          '4' ) );
    _keyNames.insert( std::make_pair( "5",          '5' ) );
    _keyNames.insert( std::make_pair( "6",          '6' ) );
    _keyNames.insert( std::make_pair( "7",          '7' ) );
    _keyNames.insert( std::make_pair( "8",          '8' ) );
    _keyNames.insert( std::make_pair( "9",          '9' ) );
    _keyNames.insert( std::make_pair( "0",          '0' ) );

    _keyNames.insert( std::make_pair( "Esc",        SDLK_ESCAPE ) );
    _keyNames.insert( std::make_pair( "F1",         SDLK_F1     ) );
    _keyNames.insert( std::make_pair( "F2",         SDLK_F2     ) );
    _keyNames.insert( std::make_pair( "F3",         SDLK_F3     ) );
    _keyNames.insert( std::make_pair( "F4",         SDLK_F4     ) );
    _keyNames.insert( std::make_pair( "F5",         SDLK_F5     ) );
    _keyNames.insert( std::make_pair( "F6",         SDLK_F6     ) );
    _keyNames.insert( std::make_pair( "F7",         SDLK_F7     ) );
    _keyNames.insert( std::make_pair( "F8",         SDLK_F8     ) );
    _keyNames.insert( std::make_pair( "F9",         SDLK_F9     ) );
    _keyNames.insert( std::make_pair( "F10",        SDLK_F1     ) );
    _keyNames.insert( std::make_pair( "F11",        SDLK_F11    ) );
    _keyNames.insert( std::make_pair( "F12",        SDLK_F12    ) );
    
    _keyNames.insert( std::make_pair( "Left",       SDLK_LEFT   ) );
    _keyNames.insert( std::make_pair( "Right",      SDLK_RIGHT  ) );
    _keyNames.insert( std::make_pair( "Up",         SDLK_UP     ) );
    _keyNames.insert( std::make_pair( "Down",       SDLK_DOWN   ) );

    _keyNames.insert( std::make_pair( "Delete",     SDLK_DELETE   ) );
    _keyNames.insert( std::make_pair( "End",        SDLK_END      ) );
    _keyNames.insert( std::make_pair( "PageDown",   SDLK_PAGEDOWN ) );
    _keyNames.insert( std::make_pair( "PageUp",     SDLK_PAGEUP   ) );

    _keyNames.insert( std::make_pair( "Insert",     SDLK_INSERT   ) );
    _keyNames.insert( std::make_pair( "Home",       SDLK_HOME     ) );

    _keyNames.insert( std::make_pair( "Pause",      SDLK_PAUSE     ) );
    _keyNames.insert( std::make_pair( "ScrLk",      SDLK_SCROLLOCK ) );
    _keyNames.insert( std::make_pair( "PrtScrn",    SDLK_PRINT     ) );

    _keyNames.insert( std::make_pair( "NumLock",    SDLK_NUMLOCK     ) );
    _keyNames.insert( std::make_pair( "KP_Del",     SDLK_DELETE      ) );
    _keyNames.insert( std::make_pair( "KP_Enter",   SDLK_KP_ENTER    ) );
    _keyNames.insert( std::make_pair( "KP_Add",     SDLK_KP_PLUS     ) );
    _keyNames.insert( std::make_pair( "KP_Div",     SDLK_KP_DIVIDE   ) );
    _keyNames.insert( std::make_pair( "KP_Mult",    SDLK_KP_MULTIPLY ) );
    _keyNames.insert( std::make_pair( "KP_Minus",   SDLK_KP_MINUS    ) );

    _keyNames.insert( std::make_pair( "KP_0",       SDLK_KP0 ) );
    _keyNames.insert( std::make_pair( "KP_1",       SDLK_KP1 ) );
    _keyNames.insert( std::make_pair( "KP_2",       SDLK_KP2 ) );
    _keyNames.insert( std::make_pair( "KP_3",       SDLK_KP3 ) );
    _keyNames.insert( std::make_pair( "KP_4",       SDLK_KP4 ) );
    _keyNames.insert( std::make_pair( "KP_5",       SDLK_KP5 ) );
    _keyNames.insert( std::make_pair( "KP_6",       SDLK_KP6 ) );
    _keyNames.insert( std::make_pair( "KP_7",       SDLK_KP7 ) );
    _keyNames.insert( std::make_pair( "KP_8",       SDLK_KP8 ) );
    _keyNames.insert( std::make_pair( "KP_9",       SDLK_KP9 ) );

    // map german special keys:
    if ( _keyboardType == KeyMap::German )
    {
        _keyNames.insert( std::make_pair( "ü",      59 ) );
        _keyNames.insert( std::make_pair( "ö",      96 ) );
        _keyNames.insert( std::make_pair( "ä",      39 ) );
        _keyNames.insert( std::make_pair( "ß",      91 ) );
    }

    // create the key code / key name map
    std::map< std::string, unsigned int >::iterator p_key = _keyNames.begin(), p_end =  _keyNames.end();
    for ( ; p_key != p_end; ++p_key )
        _keyCodes.insert( std::make_pair( p_key->second, p_key->first ) );

    // fill in the translation table
    if ( _keyboardType == KeyMap::German )
    {
        //223; // s-zet
        //228; // ae
        //246; // oe
        //252; // ue
        //196; // Ae
        //214; // Oe
        //220; // Ue

        // map german special characters
        _translationTable.insert( std::make_pair( 91  ,  223 ) ); // s-zet
        _translationTable.insert( std::make_pair( 96  ,  246 ) ); // oe
        _translationTable.insert( std::make_pair( 39  ,  228 ) ); // ae
        _translationTable.insert( std::make_pair( 59  ,  252 ) ); // ue

        _translationTable.insert( std::make_pair( 126 ,  214 ) ); // Oe
        _translationTable.insert( std::make_pair( 34  ,  196 ) ); // Ae
        _translationTable.insert( std::make_pair( 58  ,  220 ) ); // Ue

        // remap characters from english to german
        _translationTable.insert( std::make_pair( 62  , getKeyCode( ":" ) ) );
        _translationTable.insert( std::make_pair( 60  , getKeyCode( ";" ) ) );
        _translationTable.insert( std::make_pair( 95  , getKeyCode( "_" ) ) );
        _translationTable.insert( std::make_pair( 40  , getKeyCode( ")" ) ) );
        _translationTable.insert( std::make_pair( 93  , getKeyCode( "´" ) ) );
        _translationTable.insert( std::make_pair( 42  , getKeyCode( "(" ) ) );
        _translationTable.insert( std::make_pair( 91  , getKeyCode( "\\" ) ) );
        _translationTable.insert( std::make_pair( 47  , getKeyCode( "#" ) ) );
        _translationTable.insert( std::make_pair( 61  , getKeyCode( "+" ) ) );
        _translationTable.insert( std::make_pair( 33  , getKeyCode( "!" ) ) );
        _translationTable.insert( std::make_pair( 64  , getKeyCode( "\"" ) ) );
        _translationTable.insert( std::make_pair( 35  , getKeyCode( "§" ) ) );
        _translationTable.insert( std::make_pair( 37  , getKeyCode( "%" ) ) );
        _translationTable.insert( std::make_pair( 94  , getKeyCode( "&" ) ) );
        _translationTable.insert( std::make_pair( 38  , getKeyCode( "/" ) ) );
        _translationTable.insert( std::make_pair( 42  , getKeyCode( "(" ) ) );
        _translationTable.insert( std::make_pair( 40  , getKeyCode( ")" ) ) );
        _translationTable.insert( std::make_pair( 41  , getKeyCode( "=" ) ) );
        _translationTable.insert( std::make_pair( 123 , getKeyCode( "?" ) ) );                                                    
        _translationTable.insert( std::make_pair( 92  , getKeyCode( "^" ) ) );                                                    
        _translationTable.insert( std::make_pair( 124 , getKeyCode( "°" ) ) );                                                    
    }                                              
}

KeyMap::~KeyMap()
{
    log_info << "KeyMap: shutting down" << std::endl;
}

void KeyMap::shutdown()
{
    _translationTable.clear();
    _mouseBtnNames.clear();
    _mouseBtnCodes.clear();
    _keyNames.clear();
    _keyCodes.clear();

    destroy();
}

unsigned int KeyMap::translateKey( unsigned int code )
{
    // translate key if it is in translate table
    std::map< unsigned int, unsigned int >::iterator p_tans = _translationTable.find( code );
    if ( p_tans != _translationTable.end() )
        return p_tans->second;

    return code;
}

unsigned int KeyMap::getKeyCode( const std::string& name )
{
    std::map< std::string, unsigned int >::iterator p_key = _keyNames.find( name );
    if ( p_key == _keyNames.end() ) // non-registered key?
        return 0;

    return  p_key->second;
}

const std::string& KeyMap::getKeyName( unsigned int code )
{
    std::map< unsigned int, std::string >::iterator p_key = _keyCodes.find( code );
    if ( p_key == _keyCodes.end() ) // non-registered key?
    {
        static std::string nf;
        return nf;
    }

    return p_key->second;
}

unsigned int KeyMap::getMouseButtonCode( const std::string& name )
{
    std::map< std::string, unsigned int >::iterator p_key = _mouseBtnNames.find( name );
    if ( p_key == _mouseBtnNames.end() ) // non-registered key?
        return 0;

    return  p_key->second;
}

const std::string& KeyMap::getMouseButtonName( unsigned int code )
{
    std::map< unsigned int, std::string >::iterator p_key = _mouseBtnCodes.find( code );
    if ( p_key == _mouseBtnCodes.end() ) // non-registered key?
    {
        static std::string nf;
        return nf;
    }

    return p_key->second;
}

unsigned int KeyMap::getCode( const std::string& name )
{
    unsigned int code = 0;
    if ( ( code = getMouseButtonCode( name ) ) != 0 )
        return code;
    
    return getKeyCode( name );
}

} // namespace yaf3d
