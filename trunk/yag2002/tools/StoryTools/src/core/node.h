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
 # a node describes any kind of logical block
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _NODE_H_
#define _NODE_H_

#include <main.h>

namespace beditor
{

class BaseNode;
class NodeRenderer;

//! Typedef for the smart pointer
typedef SmartPtr< BaseNode >   BaseNodePtr;


//! Base class of any kind of nodes, an object of this class can be hold only in a smart pointer!
class BaseNode : public RefCount< BaseNode >
{
    DECLARE_SMARTPTR_ACCESS( BaseNode )

    public:

        //! Node type
        enum NoteType
        {
            eTypeStory     = 0x10,
            eTypeLink,
            eTypeExecution,
            eTypeOperation,
            eTypeDialog,
            eTypeEvent
        };

        //! Construct a node of given type
        explicit                                    BaseNode( unsigned int type );

        //! Set node name
        void                                        setName( const std::string& name );

        //! Get node name
        const std::string&                          getName() const;

        //! Get the node type, one of NoteType enums.
        unsigned int                                getType() const;

        //! Get the node type as string
        std::string                                 getTypeAsString() const;

        //! 2D hit test used for visualization. If a node vector is given then the node appends itself when the hit was successfull. Returns true when at least one hit detected.
        bool                                        hit( const Eigen::Vector2f& pos, std::vector< BaseNodePtr >* p_nodes = NULL );

        //! 3D hit test used for visualization. If a node vector is given then the node appends itself when the hit was successfull. Returns true when at least one hit detected.
        bool                                        hit( const Eigen::Vector3f& pos, std::vector< BaseNodePtr >* p_nodes = NULL );

        //! Add a child to node, returns false if the child already exists.
        bool                                        addChild( BaseNode* p_child );

        //! Remove given child, returns false if the child does not exist.
        bool                                        removeChild( BaseNode* p_child );

        //! Remove all children
        void                                        removeChildren();

        //! Add new parent
        bool                                        addParent( BaseNode* p_child );

        //! Remove given node from parent list, returns false if the node does not exist in parents.
        bool                                        removeParent( BaseNode* p_node );

        //! Get node children
        std::vector< BaseNodePtr >&                 getChildren();

        //! Get node parents
        std::vector< BaseNode* >&                   getParents();

        //! Set the node position on draw panel
        void                                        setPosition( const Eigen::Vector3f& pos );

        //! Get the node position on draw panel
        Eigen::Vector3f                             getPosition() const;

        //! Set the node scale on draw panel
        void                                        setScale( const Eigen::Vector3f& scale );

        //! Get the node scale on draw panel
        Eigen::Vector3f                             getScale() const;

        //! Highlight the node and its children which are in node list
        virtual void                                highlight( std::vector< BaseNodePtr >& nodes );

        //! Render the node shape, override it by derived class in order to implement custom shape(s). For a top node let p_parent be NULL.
        virtual void                                render( const Eigen::Matrix4f& view, BaseNode* p_parent = NULL );

    protected:

        //! The object can be destroyed only by the smart pointer
        virtual                                     ~BaseNode();

        //! Check if this node is contained in given list
        bool                                        findNode( std::vector< BaseNodePtr >& nodes );

        //! Node renderer
        NodeRenderer*                               _p_nodeRenderer;

        //! Node type
        unsigned int                                _type;

        //! Node name
        std::string                                 _name;

        //! Node children
        std::vector< BaseNodePtr >                  _children;

        //! Node parents
        std::vector< BaseNode* >                    _parents;
};

} // namespace beditor

#endif // _NODE_H_
