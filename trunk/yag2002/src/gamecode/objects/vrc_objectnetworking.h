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
 # networking for interactive objects; this is a server side object.
 #
 #   date of creation:  11/01/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_OBJECTNETWORKING_H_
#define _VRC_OBJECTNETWORKING_H_

#include <vrc_main.h>
#include "networkingRoles/_RO_ObjectNetworking.h"

namespace vrc
{

class ObjectNetworking : _RO_DO_PUBLIC_RO( ObjectNetworking )
{
    public:

        //! Create network object with given ID. When this object is replicated on clients then the ID will be set automatically.
        explicit                                   ObjectNetworking( unsigned int ID = 0 );

        virtual                                    ~ObjectNetworking();

        //! Set position, used by server on creation
        void                                        setPosition( const osg::Vec3f& pos );

        //! Set rotation about Z axis, used by server on creation
        void                                        setRotation( float rotZ );

        //! Set the object mesh file
        void                                        setMeshFile( const std::string& meshFile );

        //! Set the max heighlight distance
        void                                        setMaxHeighlightDistance( float maxHeighlightDistance );

        //! Set the max pick distance
        void                                        setMaxPickDistance( float maxPickDistance );

    protected:

        // Internal RN Overrides, do not use these methods!
        //-----------------------------------------------------------------------------------//

        //! Object can now be initialized in scene
        void                                        PostObjectCreate();

        //! Respawn object
        void                                        RPC_Respawn( tObjectData info );

        //! Use an object, e.g. picking it up
        void                                        RPC_Use( tObjectData info );

        //-----------------------------------------------------------------------------------//

        //! Unique object ID set on server
        unsigned int                                _objectID;

        //! Object mesh file
        char                                        _p_meshFile[ 64 ];

        //! Object position X
        float                                       _positionX;

        //! Object position Y
        float                                       _positionY;

        //! Object position Z
        float                                       _positionZ;

        //! Object rotation about Z
        float                                       _rotationZ;

        //! Max distance for being able to pick the object
        float                                       _maxPickDistance;

        //! Max distance for heighlighting the object
        float                                       _maxHeighlightDistance;

    friend class _MAKE_RO( ObjectNetworking );
};

} // namespace vrc

#endif //_VRC_OBJECTNETWORKING_H_
