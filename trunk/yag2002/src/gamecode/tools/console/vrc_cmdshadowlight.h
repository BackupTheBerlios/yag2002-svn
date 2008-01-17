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
 # console command for setting and displaying shadow light source
 #
 #   date of creation:  01/06/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_CMDSHADOWLIGHT_H_
#define _VRC_CMDSHADOWLIGHT_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_SHADOWSHOWLIGHT    "shadow.light"
#define CMD_USAGE_SHADOWSHOWLIGHT   "set/get dynamic shadow light source parameters.\n"\
                                    "use:\n"\
                                    "  shadow.light get            get current light parameters\n"\
                                    "  shadow.light pos \"x y z\"    set light source position\n"

class BaseConsoleCommand;

//! Class for command 'shadow.light'
class CmdShadowLight : public BaseConsoleCommand
{
    public:
                                                    CmdShadowLight();

        virtual                                     ~CmdShadowLight();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace vrc

#endif // _VRC_CMDSHADOWLIGHT_H_