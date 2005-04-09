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

#ifndef _CTD_KEYMAP_H_
#define _CTD_KEYMAP_H_

#include <ctd_base.h>
#include <ctd_singleton.h>
#include <ctd_application.h>

namespace CTD
{
//! Key mapper
/**
* Singleton for mapping key codes supporting english and german keyboards
*/
class KeyMap : public Singleton< KeyMap >
{
    public:

        //! Keyboard type, currently only English and German are supported
        enum KeyboardType
        {
            English,
            German
        };

        //! Setup keyboard for english or german type. Use this method to switch the keyboard type.
        void                                    setup( KeyMap::KeyboardType type );

        //! Given a key name (e.g. "A" or "SPACE") return its keycode
        unsigned int                            getKeyCode( const std::string& name );

        //! Given a key code return its name
        const std::string&                      getKeyName( unsigned int symb );

        //! Translate key code into target code.
        unsigned int                            translateKey( unsigned int key );

    protected:

                                                KeyMap();

        /**
        * Avoid the usage of copy constructor.
        */
                                                KeyMap( KeyMap& );

        /**
        * Avoid assigning this singleton
        */
        KeyMap&                                 operator = ( const KeyMap& );

        virtual                                 ~KeyMap();

        //! Mapping of key names to key codes
        std::map< std::string, unsigned int >   _keyNames;

        //! Mapping of key codes to key names
        std::map< unsigned int, std::string >   _keyCodes;

        //! Translation map
        std::map< unsigned int, unsigned int >  _translationTable;

        //! Keyboad type ( en, ger, etc. )
        unsigned int                            _keyboardType;

    friend class Singleton< KeyMap >;
    friend class Application;
};


} // namespace CTD

#endif //_CTD_KEYMAP_H_
