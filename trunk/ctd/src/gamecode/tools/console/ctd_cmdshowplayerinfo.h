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
 # console command for showing / hiding player info display
 #
 #   date of creation:  06/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDSHOWPLAYERINFO_H_
#define _CTD_CMDSHOWPLAYERINFO_H_

#include <ctd_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_SHOWPLAYERINFO    "show.playerinfo"
#define CMD_USAGE_SHOWPLAYERINFO   "use: show.playerinfo <true / false>"

class BaseConsoleCommand;
class EnPlayerInfoDisplay;

//! Class for command 'show.playerinfo'
class CmdShowPlayerInfo : public BaseConsoleCommand
{
    public:
                                                    CmdShowPlayerInfo();

        virtual                                     ~CmdShowPlayerInfo();

        const std::string&                          execute( const std::vector< std::string >& arguments );

    protected:
        
        EnPlayerInfoDisplay*                        _p_playerInfo;
};



} // namespace vrc

#endif // _CTD_CMDSHOWPLAYERINFO_H_
