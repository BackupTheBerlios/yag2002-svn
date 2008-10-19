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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
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

                                                    StorageNetworking();

        virtual                                    ~StorageNetworking();

        //! Object has been created.
        void                                        PostObjectCreate();

        //! Callback class for account info retrieval.
        class CallbackAccountInfoResult
        {
            public:

                                                    CallbackAccountInfoResult() {}

                virtual                             ~CallbackAccountInfoResult() {}

                //! Get the account information. Used when requestPublicAccountInfo called before.
                virtual void                        accountInfoResult( tAccountInfoData& info ) {}

                //! Get the user contact list. Used when requestContacts called before. If 'success is false then something went wrong.
                virtual void                        contactsResult( bool success, const std::vector< std::string >& contacts ) {}
        };

        //! Request the server for account information, used by client. The callback is called when the result arrives.
        void                                        requestAccountInfo( unsigned int userID, CallbackAccountInfoResult* p_callback );

        //! Request the server for public account information of given user.
        void                                        requestPublicAccountInfo( const std::string& username, class CallbackAccountInfoResult* p_callback );

        //! Update the user account info, used by client. Note: currently, only the user description can be updated.
        void                                        updateAccountInfo( unsigned int userID, const tAccountInfoData& info );

        //! Request for the contact list. 'contactsResult' of callback object will be called when result arrives.
        void                                        requestContacts( unsigned int userID, CallbackAccountInfoResult* p_callback );

        //! Update the contact list.
        void                                        updateContacts( unsigned int userID, const std::vector< std::string >& contacts );

    protected:

        // Internal RN Overrides, do not use these methods!
        //-----------------------------------------------//

        //! Request the server for account info.
        void                                        RPC_RequestAccountInfo( tAccountInfoData info );

        //! Request the server for updating account info.
        void                                        RPC_RequestUpdateAccountInfo( tAccountInfoData info );

        //! Result of account info called on client.
        void                                        RPC_AccountInfoResult( tAccountInfoData info );

        //! Request for user contacts.
        void                                        RPC_RequestContacts( tUserContacts data );

        //! User contacts request result.
        void                                        RPC_ContactsResult( tUserContacts data );

        //! Account info callback
        CallbackAccountInfoResult*                  _p_accountInfoCallback;

        //! Account's public info callback
        CallbackAccountInfoResult*                  _p_accountPublicInfoCallback;

        //! Contact list result callback
        CallbackAccountInfoResult*                  _p_contactsCallback;

    friend class _MAKE_RO( StorageNetworking );
};

} // namespace vrc

#endif //_VRC_STORAGENETWORKING_H_
