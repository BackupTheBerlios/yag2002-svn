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
 # console command 'writeosgfile', use this to write out the current
 #  scene into a given osg file
 #
 #   date of creation:  08/11/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_CMDWRITEOSGFILE_H_
#define _VRC_CMDWRITEOSGFILE_H_

#include <vrc_main.h>

namespace vrc
{
// console command name
#define CMD_NAME_WRITEOSGFILE    "writeosgfile"
#define CMD_USAGE_WRITEOSGFILE   "write the current scene into given file. if option -i is given then the input file is read and written (converted) to output file.\n"\
                                 "use: writeosgfile [ -i < input file > ] < output file >"

class BaseConsoleCommand;

//! Class for command 'writeosgfile'
class CmdWriteOSGFile : public BaseConsoleCommand
{
    public:
                                                    CmdWriteOSGFile();

        virtual                                     ~CmdWriteOSGFile();

        const std::string&                          execute( const std::vector< std::string >& arguments );
};



} // namespace vrc

#endif // _VRC_CMDWRITEOSGFILE_H_
