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
 # key map: provides english and german key mapping
 #
 #   date of creation:  04/08/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_keymap.h"
#include "ctd_log.h"

namespace CTD
{

CTD_SINGLETON_IMPL( KeyMap );

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

    _keyNames.insert( std::make_pair( "CtrlL",      osgGA::GUIEventAdapter::KEY_Control_L   ) );
    _keyNames.insert( std::make_pair( "CtrlR",      osgGA::GUIEventAdapter::KEY_Control_R   ) );
    _keyNames.insert( std::make_pair( "SuperL",     osgGA::GUIEventAdapter::KEY_Super_L     ) );
    _keyNames.insert( std::make_pair( "SuperR",     osgGA::GUIEventAdapter::KEY_Super_R     ) );
    _keyNames.insert( std::make_pair( "AltL",       osgGA::GUIEventAdapter::KEY_Alt_L       ) );
    _keyNames.insert( std::make_pair( "AltR",       osgGA::GUIEventAdapter::KEY_Alt_R       ) );
    _keyNames.insert( std::make_pair( "Space",      osgGA::GUIEventAdapter::KEY_Space       ) );
    _keyNames.insert( std::make_pair( "Switch",     osgGA::GUIEventAdapter::KEY_Mode_switch ) );
    _keyNames.insert( std::make_pair( "Menu",       osgGA::GUIEventAdapter::KEY_Menu        ) );
    _keyNames.insert( std::make_pair( "ShiftL",     osgGA::GUIEventAdapter::KEY_Shift_L     ) );
    _keyNames.insert( std::make_pair( "ShiftR",     osgGA::GUIEventAdapter::KEY_Shift_R     ) );
    _keyNames.insert( std::make_pair( "Return",     osgGA::GUIEventAdapter::KEY_Return      ) );
    _keyNames.insert( std::make_pair( "Caps",       osgGA::GUIEventAdapter::KEY_Caps_Lock   ) );
    _keyNames.insert( std::make_pair( "Tab",        osgGA::GUIEventAdapter::KEY_Tab         ) );

    _keyNames.insert( std::make_pair( "A",          'a'                                     ) );
    _keyNames.insert( std::make_pair( "B",          'b'                                     ) );
    _keyNames.insert( std::make_pair( "C",          'c'                                     ) );
    _keyNames.insert( std::make_pair( "D",          'd'                                     ) );
    _keyNames.insert( std::make_pair( "E",          'e'                                     ) );
    _keyNames.insert( std::make_pair( "F",          'f'                                     ) );
    _keyNames.insert( std::make_pair( "G",          'g'                                     ) );
    _keyNames.insert( std::make_pair( "H",          'h'                                     ) );
    _keyNames.insert( std::make_pair( "I",          'i'                                     ) );
    _keyNames.insert( std::make_pair( "J",          'j'                                     ) );
    _keyNames.insert( std::make_pair( "K",          'k'                                     ) );
    _keyNames.insert( std::make_pair( "L",          'l'                                     ) );
    _keyNames.insert( std::make_pair( "M",          'm'                                     ) );
    _keyNames.insert( std::make_pair( "N",          'n'                                     ) );
    _keyNames.insert( std::make_pair( "O",          'o'                                     ) );
    _keyNames.insert( std::make_pair( "P",          'p'                                     ) );
    _keyNames.insert( std::make_pair( "Q",          'q'                                     ) );
    _keyNames.insert( std::make_pair( "R",          'r'                                     ) );
    _keyNames.insert( std::make_pair( "S",          's'                                     ) );
    _keyNames.insert( std::make_pair( "T",          't'                                     ) );
    _keyNames.insert( std::make_pair( "U",          'u'                                     ) );
    _keyNames.insert( std::make_pair( "V",          'v'                                     ) );
    _keyNames.insert( std::make_pair( "W",          'w'                                     ) );
    _keyNames.insert( std::make_pair( "X",          'x'                                     ) );
    _keyNames.insert( std::make_pair( "Y",          'y'                                     ) );
    _keyNames.insert( std::make_pair( "Z",          'z'                                     ) );

    _keyNames.insert( std::make_pair( ";",          ';'                                     ) );
    _keyNames.insert( std::make_pair( "[",          '['                                     ) );
    _keyNames.insert( std::make_pair( ">",          '>'                                     ) );
    _keyNames.insert( std::make_pair( "<",          '<'                                     ) );
    _keyNames.insert( std::make_pair( "_",          '_'                                     ) );
    _keyNames.insert( std::make_pair( "?",          '?'                                     ) );
    _keyNames.insert( std::make_pair( "(",          '('                                     ) );
    _keyNames.insert( std::make_pair( ")",          ')'                                     ) );
    _keyNames.insert( std::make_pair( "+",          '+'                                     ) );
    _keyNames.insert( std::make_pair( "*",          '*'                                     ) );
    _keyNames.insert( std::make_pair( "|",          '|'                                     ) );
    _keyNames.insert( std::make_pair( ">",          '>'                                     ) );
    _keyNames.insert( std::make_pair( "\\",         '\\'                                    ) );
    _keyNames.insert( std::make_pair( "&",          '&'                                     ) );
    _keyNames.insert( std::make_pair( "^",          '^'                                     ) );
    _keyNames.insert( std::make_pair( "#",          '#'                                     ) );
    _keyNames.insert( std::make_pair( "`",          '`'                                     ) );
    _keyNames.insert( std::make_pair( "~",          '~'                                     ) );
    _keyNames.insert( std::make_pair( "]",          ']'                                     ) );
    _keyNames.insert( std::make_pair( ":",          ':'                                     ) );
    _keyNames.insert( std::make_pair( "/",          '/'                                     ) );
    _keyNames.insert( std::make_pair( "{",          '{'                                     ) );
    _keyNames.insert( std::make_pair( "}",          '}'                                     ) );
    _keyNames.insert( std::make_pair( "-",          '-'                                     ) );
    _keyNames.insert( std::make_pair( "!",          '!'                                     ) );
    _keyNames.insert( std::make_pair( "\"",         '\"'                                    ) );
    _keyNames.insert( std::make_pair( "�",          '�'                                     ) );
    _keyNames.insert( std::make_pair( "%",          '%'                                     ) );
    _keyNames.insert( std::make_pair( "&",          '&'                                     ) );
    _keyNames.insert( std::make_pair( "�",          '�'                                     ) );
    _keyNames.insert( std::make_pair( "~",          '~'                                     ) );
    _keyNames.insert( std::make_pair( "=",          '='                                     ) );

    _keyNames.insert( std::make_pair( "1",          '1'                                     ) );
    _keyNames.insert( std::make_pair( "2",          '2'                                     ) );
    _keyNames.insert( std::make_pair( "3",          '3'                                     ) );
    _keyNames.insert( std::make_pair( "4",          '4'                                     ) );
    _keyNames.insert( std::make_pair( "5",          '5'                                     ) );
    _keyNames.insert( std::make_pair( "6",          '6'                                     ) );
    _keyNames.insert( std::make_pair( "7",          '7'                                     ) );
    _keyNames.insert( std::make_pair( "8",          '8'                                     ) );
    _keyNames.insert( std::make_pair( "9",          '9'                                     ) );
    _keyNames.insert( std::make_pair( "0",          '0'                                     ) );

    _keyNames.insert( std::make_pair( "Esc",        osgGA::GUIEventAdapter::KEY_Escape      ) );
    _keyNames.insert( std::make_pair( "F1",         osgGA::GUIEventAdapter::KEY_F1          ) );
    _keyNames.insert( std::make_pair( "F2",         osgGA::GUIEventAdapter::KEY_F2          ) );
    _keyNames.insert( std::make_pair( "F3",         osgGA::GUIEventAdapter::KEY_F3          ) );
    _keyNames.insert( std::make_pair( "F4",         osgGA::GUIEventAdapter::KEY_F4          ) );
    _keyNames.insert( std::make_pair( "F5",         osgGA::GUIEventAdapter::KEY_F5          ) );
    _keyNames.insert( std::make_pair( "F6",         osgGA::GUIEventAdapter::KEY_F6          ) );
    _keyNames.insert( std::make_pair( "F7",         osgGA::GUIEventAdapter::KEY_F7          ) );
    _keyNames.insert( std::make_pair( "F8",         osgGA::GUIEventAdapter::KEY_F8          ) );
    _keyNames.insert( std::make_pair( "F9",         osgGA::GUIEventAdapter::KEY_F9          ) );
    _keyNames.insert( std::make_pair( "F10",        osgGA::GUIEventAdapter::KEY_F1          ) );
    _keyNames.insert( std::make_pair( "F11",        osgGA::GUIEventAdapter::KEY_F11         ) );
    _keyNames.insert( std::make_pair( "F12",        osgGA::GUIEventAdapter::KEY_F12         ) );
    
    _keyNames.insert( std::make_pair( "Left",       osgGA::GUIEventAdapter::KEY_Left        ) );
    _keyNames.insert( std::make_pair( "Right",      osgGA::GUIEventAdapter::KEY_Right       ) );
    _keyNames.insert( std::make_pair( "Up",         osgGA::GUIEventAdapter::KEY_Up          ) );
    _keyNames.insert( std::make_pair( "Down",       osgGA::GUIEventAdapter::KEY_Down        ) );

    _keyNames.insert( std::make_pair( "Delete",     osgGA::GUIEventAdapter::KEY_Delete      ) );
    _keyNames.insert( std::make_pair( "End",        osgGA::GUIEventAdapter::KEY_End         ) );
    _keyNames.insert( std::make_pair( "PageDown",   osgGA::GUIEventAdapter::KEY_Page_Down   ) );
    _keyNames.insert( std::make_pair( "PageUp",     osgGA::GUIEventAdapter::KEY_Page_Up     ) );

    _keyNames.insert( std::make_pair( "Insert",     osgGA::GUIEventAdapter::KEY_Insert      ) );
    _keyNames.insert( std::make_pair( "Home",       osgGA::GUIEventAdapter::KEY_Home        ) );

    _keyNames.insert( std::make_pair( "Pause",      osgGA::GUIEventAdapter::KEY_Pause       ) );
    _keyNames.insert( std::make_pair( "ScrLk",      osgGA::GUIEventAdapter::KEY_Scroll_Lock ) );
    _keyNames.insert( std::make_pair( "PrtScrn",    osgGA::GUIEventAdapter::KEY_Print       ) );

    _keyNames.insert( std::make_pair( "NumLock",    osgGA::GUIEventAdapter::KEY_Num_Lock    ) );
    _keyNames.insert( std::make_pair( "KP_Del",     osgGA::GUIEventAdapter::KEY_KP_Delete   ) );
    _keyNames.insert( std::make_pair( "KP_Enter",   osgGA::GUIEventAdapter::KEY_KP_Enter    ) );
    _keyNames.insert( std::make_pair( "KP_Add",     osgGA::GUIEventAdapter::KEY_KP_Add      ) );
    _keyNames.insert( std::make_pair( "KP_Div",     osgGA::GUIEventAdapter::KEY_KP_Divide   ) );
    _keyNames.insert( std::make_pair( "KP_Mult",    osgGA::GUIEventAdapter::KEY_KP_Multiply ) );
    _keyNames.insert( std::make_pair( "KP_Minus",   osgGA::GUIEventAdapter::KEY_KP_Subtract ) );

    _keyNames.insert( std::make_pair( "KP_0",       osgGA::GUIEventAdapter::KEY_KP_Insert   ) );
    _keyNames.insert( std::make_pair( "KP_1",       osgGA::GUIEventAdapter::KEY_KP_End      ) );
    _keyNames.insert( std::make_pair( "KP_2",       osgGA::GUIEventAdapter::KEY_KP_Down     ) );
    _keyNames.insert( std::make_pair( "KP_3",       osgGA::GUIEventAdapter::KEY_KP_Page_Down) );
    _keyNames.insert( std::make_pair( "KP_4",       osgGA::GUIEventAdapter::KEY_KP_Left     ) );
    _keyNames.insert( std::make_pair( "KP_5",       osgGA::GUIEventAdapter::KEY_KP_Begin    ) );
    _keyNames.insert( std::make_pair( "KP_6",       osgGA::GUIEventAdapter::KEY_KP_Right    ) );
    _keyNames.insert( std::make_pair( "KP_7",       osgGA::GUIEventAdapter::KEY_KP_Home     ) );
    _keyNames.insert( std::make_pair( "KP_8",       osgGA::GUIEventAdapter::KEY_KP_Up       ) );
    _keyNames.insert( std::make_pair( "KP_9",       osgGA::GUIEventAdapter::KEY_KP_Page_Up  ) );

    // map german special keys:
    if ( _keyboardType == KeyMap::German )
    {
        _keyNames.insert( std::make_pair( "�",          59                                      ) );
        _keyNames.insert( std::make_pair( "�",          96                                      ) );
        _keyNames.insert( std::make_pair( "�",          39                                      ) );
        _keyNames.insert( std::make_pair( "�",          91                                      ) );
    }

    // create the key code / key name map
    std::map< std::string, unsigned int >::iterator p_key = _keyNames.begin(), p_end =  _keyNames.end();
    for ( ; p_key != p_end; p_key++ )
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
        _translationTable.insert( std::make_pair( 93  , getKeyCode( "�" ) ) );
        _translationTable.insert( std::make_pair( 42  , getKeyCode( "(" ) ) );
        _translationTable.insert( std::make_pair( 91  , getKeyCode( "\\" ) ) );
        _translationTable.insert( std::make_pair( 47  , getKeyCode( "#" ) ) );
        _translationTable.insert( std::make_pair( 61  , getKeyCode( "+" ) ) );
        _translationTable.insert( std::make_pair( 33  , getKeyCode( "!" ) ) );
        _translationTable.insert( std::make_pair( 64  , getKeyCode( "\"" ) ) );
        _translationTable.insert( std::make_pair( 35  , getKeyCode( "�" ) ) );
        _translationTable.insert( std::make_pair( 37  , getKeyCode( "%" ) ) );
        _translationTable.insert( std::make_pair( 94  , getKeyCode( "&" ) ) );
        _translationTable.insert( std::make_pair( 38  , getKeyCode( "/" ) ) );
        _translationTable.insert( std::make_pair( 42  , getKeyCode( "(" ) ) );
        _translationTable.insert( std::make_pair( 40  , getKeyCode( ")" ) ) );
        _translationTable.insert( std::make_pair( 41  , getKeyCode( "=" ) ) );
        _translationTable.insert( std::make_pair( 123 , getKeyCode( "?" ) ) );                                                    
        _translationTable.insert( std::make_pair( 92  , getKeyCode( "^" ) ) );                                                    
        _translationTable.insert( std::make_pair( 124 , getKeyCode( "�" ) ) );                                                    
    }                                              

}

KeyMap::~KeyMap()
{
}

unsigned int KeyMap::translateKey( unsigned int key )
{
    // translate key if it is in translate table
    std::map< unsigned int, unsigned int >::iterator p_tans = _translationTable.find( key );
    if ( p_tans != _translationTable.end() )
        return p_tans->second;

    return key;
}

unsigned int KeyMap::getKeyCode( const std::string& name )
{
    std::map< std::string, unsigned int >::iterator p_key = _keyNames.find( name );
    if ( p_key == _keyNames.end() ) // not-registered key?
        return 0;

    return p_key->second;
}

const std::string& KeyMap::getKeyName( unsigned int symb )
{
    std::map< unsigned int, std::string >::iterator p_key = _keyCodes.find( symb );
    if ( p_key == _keyCodes.end() ) // not-registered key?
    {
        static std::string nf;
        return nf;
    }
    return p_key->second;
}

}
