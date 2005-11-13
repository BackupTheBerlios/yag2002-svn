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
 # key map: provides english and german key mapping
 #
 #   date of creation:  04/08/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _YAF3DKEYMAP_H_
#define _YAF3DKEYMAP_H_

#include <ctd_base.h>
#include <ctd_singleton.h>
#include <ctd_application.h>

namespace yaf3d
{
//! Key mapper
/**
* Singleton for mapping key codes supporting english and german keyboards.
* It provides also mouse buttons' name and key code access.
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

        //! Given a key name (e.g. "A" or "SPACE") return its keycode. 
        unsigned int                            getKeyCode( const std::string& name);

        //! Given a key code return its name.
        const std::string&                      getKeyName( unsigned int code );

        //! Given a mouse button name return its code.
        unsigned int                            getMouseButtonCode( const std::string& name );

        //! Given a mouse button code return its name.
        const std::string&                      getMouseButtonName( unsigned int code );

        //! Given a key or mouse button name return its code.
        unsigned int                            getCode( const std::string& name );

        //! Translate key code into target code.
        unsigned int                            translateKey( unsigned int code );

    protected:

                                                KeyMap();

        virtual                                 ~KeyMap();

        //! Shutdown singleton
        void                                    shutdown();

        //! Mapping of key names to key codes
        std::map< std::string, unsigned int >   _keyNames;

        //! Mapping of key codes to key names
        std::map< unsigned int, std::string >   _keyCodes;

        //! Translation map
        std::map< unsigned int, unsigned int >  _translationTable;

        //! Mapping of mouse button names to key codes
        std::map< std::string, unsigned int >   _mouseBtnNames;

        //! Mapping of key codes to mouse button names
        std::map< unsigned int, std::string >   _mouseBtnCodes;

        //! Keyboad type ( en, ger, etc. )
        unsigned int                            _keyboardType;

    friend class Singleton< KeyMap >;
    friend class Application;
};


} // namespace yaf3d

#endif //_YAF3DKEYMAP_H_
