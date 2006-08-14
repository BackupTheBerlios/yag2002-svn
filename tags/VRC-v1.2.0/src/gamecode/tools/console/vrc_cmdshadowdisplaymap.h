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
 # console command for enabling / disabling the shadow map display
 #
 #   date of creation:  07/14/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CMDSHADOWDISPLAYMAP_H_
#define _VRC_CMDSHADOWDISPLAYMAP_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_SHADOWSHOWMAP    "shadow.display.map"
#define CMD_USAGE_SHADOWSHOWMAP   "enable / disable the display of current shadow map.\n"\
                                  "use: shadow.display.map <true / false>"

class BaseConsoleCommand;

//! Class for command 'shadow.display.map'
class CmdDisplayShadowMap : public BaseConsoleCommand
{
    public:
                                                    CmdDisplayShadowMap();

        virtual                                     ~CmdDisplayShadowMap();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace vrc

#endif // _VRC_CMDSHADOWDISPLAYMAP_H_
