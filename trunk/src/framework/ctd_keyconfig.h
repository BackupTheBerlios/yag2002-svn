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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  02/08/2004 boto       creation of CTDKeyConfig
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/

#ifndef _CTD_KEYCONFIG_H_
#define _CTD_KEYCONFIG_H_


#include "ctd_frbase.h"
#include <neoengine/input.h>
#include <string>

namespace CTD
{

typedef struct _key_binding
{

    int             m_iKeyCode;
    char            m_pcKeyString[32];

} tKeyBinding;

extern tKeyBinding g_akKeyBindings[];

// this class describes one key binding pair Name <-> Key Code
class KeyBinding
{
    public: 
    
        std::string     m_strKeyName;
        int             m_iKeyCode;

};

class KeyConfig
{

    public:

        /**
        * Retrieve a key code given its name. This function is also implemented in engine interface class.
        * \strKeyName   Name of key
        * \uiKeyCode    ASCII code of key
        * \return  false if something went wrong, in this case uiKeyCode is set to ZERO.
        */
        bool                            GetKeyCode( const std::string &strKeyName , int &iKeyCode );


    protected:

                                        KeyConfig();

                                        ~KeyConfig();

        /**
        * Set up the key configuration defined in file strCfgFileName.
        * \return  false if something went wrong.
        */
        bool                            Initialize( std::string &strCfgFileName );

        /**
        * List of Key bindings parsed from key config file
        */
        std::vector< KeyBinding >       m_vkKeyBindings;

        /**
        * Indicates that the key configuration is initialized.
        */
        bool                            m_bInitialized;


    friend  class   FrameworkImpl;

};

} // namespace CTD

#endif //_CTD_KEYCONFIG_H_
