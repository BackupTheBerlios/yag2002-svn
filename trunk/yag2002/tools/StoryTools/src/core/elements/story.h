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
 # story node containing all kinds of story elements
 #
 #   date of creation:  13/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _NODESTORY_H_
#define _NODESTORY_H_

#include <main.h>
#include <core/node.h>

namespace beditor
{

//! Story node
class NodeStory : public BaseNode
{
    //! Set the smart pointer class as friend
    DECLARE_SMARTPTR_ACCESS( NodeStory )

    public:

        //! Construct a story node
                                                    NodeStory();

    protected:

        virtual                                     ~NodeStory();
};

//! Typedef for the smart pointer
typedef SmartPtr< NodeStory >   NodeStoryPtr;


//! Define the registry class
ELEMENT_REGISTRY_CLASS( RegistryStory, NodeStory )

} // namespace beditor

#endif // _NODESTORY_H_
