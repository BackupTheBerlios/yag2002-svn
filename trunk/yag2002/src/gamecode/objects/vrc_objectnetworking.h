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
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_OBJECTNETWORKING_H_
#define _VRC_OBJECTNETWORKING_H_

#include <vrc_main.h>
#include "networkingRoles/_RO_ObjectNetworking.h"

namespace vrc
{

class BaseObject;

class ObjectNetworking : _RO_DO_PUBLIC_RO( ObjectNetworking )
{
    public:

        //! Create network object with given object entity (used on server). When this object is replicated on clients then the object entity is created.
        explicit                                   ObjectNetworking( BaseObject* p_objectEntity = NULL );

        virtual                                    ~ObjectNetworking();

        //! Callback class for getting action results.
        //!  Note: only one request can by handled at the same time.
        class CallbackActionResult
        {
            public:

                //! If granted is true then the authentification was successfull.
                virtual void                        actionResult( tActionData& result ) = 0;
        };

        //! Client requests the server for performing an action. Use the callback in order to get notified when the result comes in on client.
        //! Returns false if a request is already in progress.
        bool                                        RequestAction( tActionData& action, CallbackActionResult* p_cb );

    protected:

        // Internal RN Overrides, do not use these methods!
        //-----------------------------------------------------------------------------------//

        //! Object can now be initialized in scene
        void                                        PostObjectCreate();

        //! Client requests the server to perform an action
        void                                        RPC_RequestAction( tActionData action );

        //! Action result sent by server
        void                                        RPC_ActionResult( tActionData action );

        //-----------------------------------------------------------------------------------//

        //! Unique object ID set on server
        unsigned int                                _objectID;

        //! Unique object reference ID set on server
        unsigned int                                _objectInstanceID;

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

        //! Base object
        BaseObject*                                 _p_objectEntity;

        //! Result callback object used by client
        CallbackActionResult*                       _p_cbResult;

    friend class _MAKE_RO( ObjectNetworking );
};

} // namespace vrc

#endif //_VRC_OBJECTNETWORKING_H_
