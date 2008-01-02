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
 # link node used for building the node relationships
 #
 #   date of creation:  18/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _NODELINK_H_
#define _NODELINK_H_

#include <main.h>
#include <core/node.h>


namespace beditor
{

//! Link node
class NodeLink : public BaseNode
{
    //! Set the smart pointer class as friend
    DECLARE_SMARTPTR_ACCESS( NodeLink )

    public:

        //! Construct a link node
                                                    NodeLink();

        //! Set the source and destination nodes
        void                                        setSourceDestination( BaseNodePtr src, BaseNodePtr dest );

        //! Get the source and destination nodes
        void                                        getSourceDestination( BaseNodePtr& src, BaseNodePtr& dest );

        //! Check if the given node is source or destination
        bool                                        isAssociated( BaseNodePtr node ) const;

    protected:

        virtual                                     ~NodeLink();
};

//! Typedef for the smart pointer
typedef SmartPtr< NodeLink >   NodeLinkPtr;


//! Define the registry class
ELEMENT_REGISTRY_CLASS( RegistryLink, NodeLink )

} // namespace beditor

#endif // _NODELINK_H_
