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
 # console command 'rendeing.mode'
 #
 #   date of creation:  02/15/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_CMDRENDERINGMODE_H_
#define _VRC_CMDRENDERINGMODE_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_RENDERINGMODE    "rendering.mode"
#define CMD_USAGE_RENDERINGMODE   "change rendering mode to 'line', 'point', or 'fill'.\n"\
                                  "use: rendering.mode [ l[ine] | p[oint] | f[ill] ]"

class BaseConsoleCommand;

//! Class for command 'rendering.mode'
class CmdRenderingMode : public BaseConsoleCommand
{
    public:
                                                    CmdRenderingMode();

        virtual                                     ~CmdRenderingMode();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};

} // namespace vrc

#endif // _VRC_CMDRENDERINGMODE_H_
