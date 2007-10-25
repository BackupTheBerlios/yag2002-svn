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
 # networking for storage
 #
 # this class implements the networking functionality for storage
 #
 #
 #   date of creation:  09/25/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_STORAGENETWORKING_H_
#define _VRC_STORAGENETWORKING_H_

#include <vrc_main.h>
#include "networkingRoles/_RO_StorageNetworking.h"

namespace vrc
{

class StorageNetworking : _RO_DO_PUBLIC_RO( StorageNetworking )
{
    public:

        explicit                                   StorageNetworking();

        virtual                                    ~StorageNetworking();

        //! Object can now be initialized in scene
        void                                        PostObjectCreate();

        //! Callback class for account info retrieval.
        //!  Note: only one request can by handled at the same time.
        class CallbackAccountInfoResult
        {
            public:

                //! If granted is true then the authentification was successfull.
                virtual void                        accountInfoResult( tAccountInfoData& info ) = 0;
        };

        //! Request the server for account information, used by client. The callback is called when the result arrives.
        void                                        requestAccountInfo( unsigned int userID, CallbackAccountInfoResult* p_callback );

    protected:

        // Internal RN Overrides, do not use these methods!
        //-----------------------------------------------------------------------------------//

        //! Request the server for account info.
        void                                        RPC_RequestAccountInfo( tAccountInfoData info );

        //! Result of account info called on client.
        void                                        RPC_AccountInfoResult( tAccountInfoData info );

        //! Account info callback
        CallbackAccountInfoResult*                  _p_accountInfoCallback;

    friend class _MAKE_RO( StorageNetworking );
};

} // namespace vrc

#endif //_VRC_STORAGENETWORKING_H_
