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
 # neoengine, key configuration
 #
 # this class implements the key configuration basing on a config file
 #
 #
 #   date of creation:  02/08/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "ctd_frbase.h"
#include "ctd_keyconfig.h"

using namespace NeoEngine;
using namespace std;

namespace CTD
{

#define END_OF_LIST     "EndOfList"
tKeyBinding g_akKeyBindings[] = 
{
    KC_SPACE          , "KC_SPACE",
    KC_0              , "KC_0",   
    KC_1              , "KC_1",        
    KC_2              , "KC_2",       
    KC_3              , "KC_3",       
    KC_4              , "KC_4",         
    KC_5              , "KC_5",         
    KC_6              , "KC_6",         
    KC_7              , "KC_7",         
    KC_8              , "KC_8",         
    KC_9              , "KC_9",         
    KC_A              , "KC_A",         
    KC_B              , "KC_B",         
    KC_C              , "KC_C",         
    KC_D              , "KC_D",         
    KC_E              , "KC_E",         
    KC_F              , "KC_F",         
    KC_G              , "KC_G",         
    KC_H              , "KC_H",         
    KC_I              , "KC_I",         
    KC_J              , "KC_J",         
    KC_K              , "KC_K",         
    KC_L              , "KC_L",         
    KC_M              , "KC_M",         
    KC_N              , "KC_N",         
    KC_O              , "KC_O",         
    KC_P              , "KC_P",         
    KC_Q              , "KC_Q",         
    KC_R              , "KC_R", 
    KC_S              , "KC_S",   
    KC_T              , "KC_T",   
    KC_U              , "KC_U",   
    KC_V              , "KC_V",   
    KC_W              , "KC_W",   
    KC_X              , "KC_X",   
    KC_Y              , "KC_Y",   
    KC_Z              , "KC_Z",   

    KC_RETURN         , "KC_RETURN",    
    KC_ESCAPE         , "KC_ESCAPE",  
    KC_BACKSPACE      , "KC_BACKSPACE", 
    KC_UP             , "KC_UP",
    KC_DOWN           , "KC_DOWN",      
    KC_LEFT           , "KC_LEFT",    
    KC_RIGHT          , "KC_RIGHT",    
    KC_F1             , "KC_F1",        
    KC_F2             , "KC_F2",        
    KC_F3             , "KC_F3",        
    KC_F4             , "KC_F4",        
    KC_F5             , "KC_F5",        
    KC_F6             , "KC_F6",        
    KC_F7             , "KC_F7",        
    KC_F8             , "KC_F8",        
    KC_F9             , "KC_F9",        
    KC_F10            , "KC_F10",       
    KC_F11            , "KC_F11",       
    KC_F12            , "KC_F12",       
    KC_F13            , "KC_F13",       
    KC_F14            , "KC_F14",       
    KC_F15            , "KC_F15",       

    KC_DOT               , "KC_DOT"             ,  // .
    KC_COMMA             , "KC_COMMA"           ,  // ,
    KC_COLON             , "KC_COLON"           ,  // :
    KC_SEMICOLON         , "KC_SEMICOLON"       ,  // ;
    KC_SLASH             , "KC_SLASH"           ,  // /
    KC_BACKSLASH         , "KC_BACKSLASH"       ,  // backslash ( \ ) 
    KC_PLUS              , "KC_PLUS"            ,  // +
    KC_MINUS             , "KC_MINUS"           ,  // -
    KC_ASTERISK          , "KC_ASTERISK"        ,  // *
    KC_EXCLAMATION       , "KC_EXCLAMATION"     ,  // !
    KC_QUESTION          , "KC_QUESTION"        ,  // ?
    KC_QUOTEDOUBLE       , "KC_QUOTEDOUBLE"     ,  // "
    KC_QUOTE             , "KC_QUOTE"           ,  // '
    KC_EQUAL             , "KC_EQUAL"           ,  // =
    KC_HASH              , "KC_HASH"            ,  // #
    KC_PERCENT           , "KC_PERCENT"         ,  // %
    KC_AMPERSAND         , "KC_AMPERSAND"       ,  // &
    KC_UNDERSCORE        , "KC_UNDERSCORE"      ,  // _
    KC_LEFTPARENTHESIS   , "KC_LEFTPARENTHESIS" ,  // (
    KC_RIGHTPARENTHESIS  , "KC_RIGHTPARENTHESIS",  // )
    KC_LEFTBRACKET       , "KC_LEFTBRACKET"     ,  // [
    KC_RIGHTBRACKET      , "KC_RIGHTBRACKET"    ,  // ]
    KC_LEFTCURL          , "KC_LEFTCURL"        ,  // {
    KC_RIGHTCURL         , "KC_RIGHTCURL"       ,  // }
    KC_DOLLAR            , "KC_DOLLAR"          ,  // $
    KC_POUND             , "KC_POUND"           ,  // £
    KC_EURO              , "KC_EURO"            ,  // $
    KC_LESS              , "KC_LESS"            ,  // <
    KC_GREATER           , "KC_GREATER"         ,  // >
    KC_BAR               , "KC_BAR"             ,  // |
    KC_GRAVE             , "KC_GRAVE"           ,
    KC_TILDE             , "KC_TILDE"           ,  // ~
    KC_AT                , "KC_AT"              ,  // @

    KC_KP_0              , "KC_KP_0",       
    KC_KP_1              , "KC_KP_1",       
    KC_KP_2              , "KC_KP_2",       
    KC_KP_3              , "KC_KP_3",       
    KC_KP_4              , "KC_KP_4",       
    KC_KP_5              , "KC_KP_5",       
    KC_KP_6              , "KC_KP_6",       
    KC_KP_7              , "KC_KP_7",       
    KC_KP_8              , "KC_KP_8",       
    KC_KP_9              , "KC_KP_9", 
    KC_KP_PLUS           , "KC_KP_PLUS",
    KC_KP_MINUS          , "KC_KP_MINUS",   
    KC_KP_DECIMAL        , "KC_KP_DECIMAL", 
    KC_KP_DIVIDE         , "KC_KP_DIVIDE",  
    KC_KP_ASTERISK       , "KC_KP_ASTERISK",
    KC_KP_NUMLOCK        , "KC_KP_NUMLOCK", 
    KC_KP_ENTER          , "KC_KP_ENTER",   

    KC_TAB               , "KC_TAB",       
    KC_CAPSLOCK          , "KC_CAPSLOCK",  
    KC_LSHIFT            , "KC_LSHIFT",    
    KC_LCTRL             , "KC_LCTRL",     
    KC_LALT              , "KC_LALT",      
    KC_LWIN              , "KC_LWIN",      
    KC_RSHIFT            , "KC_RSHIFT",    
    KC_RCTRL             , "KC_RCTRL",     
    KC_RALT              , "KC_RALT",      
    KC_RWIN              , "KC_RWIN",      
    KC_INSERT            , "KC_INSERT",    
    KC_DELETE            , "KC_DELETE",    
    KC_HOME              , "KC_HOME",      
    KC_END               , "KC_END",       
    KC_PAGEUP            , "KC_PAGEUP",    
    KC_PAGEDOWN          , "KC_PAGEDOWN",  
    KC_SCROLLLOCK        , "KC_SCROLLLOCK",
    KC_PAUSE             , "KC_PAUSE",     

    KC_LAST_BUILTIN      , END_OF_LIST

};


KeyConfig::KeyConfig()
{

    m_bInitialized = false;

}

KeyConfig::~KeyConfig()
{
}

// setup key bindings described in given key configuration file
bool KeyConfig::Initialize( std::string &strCfgFileName )
{

    neolog << LogLevel( INFO ) << "CTD configuring key maps ... " << endl;

    File    kFile;
    if ( !kFile.Open( "", strCfgFileName, std::ios_base::binary | std::ios_base::in ) ) {

        neolog << LogLevel( WARNING ) << " *** CTD could not find key configuration file '" << strCfgFileName.c_str() << "'" << endl;
        return false;

    }

    // read in the config file for further processing
    unsigned int uiFileSize = kFile.GetSize();
    char    *pcInitialBuffer = new char[ uiFileSize ];
    if ( !kFile.Read( pcInitialBuffer, uiFileSize ) ) {

        neolog << LogLevel( WARNING ) << " *** CTD error reading key configuration file '" << strCfgFileName.c_str() << "'" << endl;
        delete pcInitialBuffer;
        return false;

    }

    char            pcLineBuffer[256];
    char            pcKeyName[64];
    char            pcKeyCode[64];
    char            *pcBuffer = pcInitialBuffer;
    KeyBinding  kKB;
    bool            bReadLine = true;

    while ( bReadLine == true ) {

        // copy one line into line buffer
        unsigned int uiC;
        for ( uiC = 0; ( pcLineBuffer[ uiC ] = *pcBuffer++ ) != '\n'; uiC++ ) {

            // save check for end of buffer
            if ( ( unsigned int )( pcBuffer - pcInitialBuffer ) > uiFileSize ) {
            
                bReadLine = false;
                break;

            }
                        
        }
        // terminate the line buffer string
        pcLineBuffer[ uiC ] = 0;

        // skip comments
        if ( ( pcLineBuffer[0] == '#') || ( pcLineBuffer[0] == '\n' ) ) {
        
            continue;

        }

        // skip invalid line formats
        if ( sscanf( pcLineBuffer, "%s = %s", pcKeyName, pcKeyCode ) != 2 ) {
            continue;
        }

    
        // go through all key code names and find the requested one
        unsigned int uiKCCnt = 0;
        bool         bKeyFound = false;
        while ( strcmp( g_akKeyBindings[ uiKCCnt ].m_pcKeyString, END_OF_LIST ) ) {

            // if key code found then store its name and key code into list
            if ( !strcmp( g_akKeyBindings[ uiKCCnt ].m_pcKeyString, pcKeyCode ) ) {

                kKB.m_strKeyName = pcKeyName;
                kKB.m_iKeyCode   = g_akKeyBindings[ uiKCCnt ].m_iKeyCode;
        
                m_vkKeyBindings.push_back( kKB );

                neolog << LogLevel( INFO ) << "   configuring key: '" << 
                    kKB.m_strKeyName.c_str() << "' : '" << 
                    kKB.m_iKeyCode << "'" << endl;

                bKeyFound = true;
                break;

            }

            uiKCCnt++;

        }

        if ( bKeyFound == false ) {

            neolog << LogLevel( WARNING ) << endl;
            neolog << LogLevel( WARNING ) << "   *** invalid requested key code: '" << pcKeyCode << "', skip binding!" << endl;
            neolog << LogLevel( WARNING ) << endl;

        }

    }

    delete pcInitialBuffer;

    m_bInitialized = true;
    
    return true;

}

bool KeyConfig::GetKeyCode( const std::string &strKeyName , int &iKeyCode ) 
{

    // find the requested key name in key list
    size_t uiNumKeys = m_vkKeyBindings.size();
    for ( size_t uiKeyCnt = 0; uiKeyCnt < uiNumKeys; uiKeyCnt++ ) {

        if ( m_vkKeyBindings[ uiKeyCnt ].m_strKeyName == strKeyName ) {

            iKeyCode = m_vkKeyBindings[ uiKeyCnt ].m_iKeyCode;
            return true;

        }
    }

    return false;

}

} // namespace CTD
