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
 # networking for mail transfer
 #
 # this class implements the networking functionality for the mailbox
 #
 #
 #   date of creation:  29/07/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_mailboxclient.h"
#include "vrc_mailboxnetworking.h"
#include "vrc_storageserver.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>


using namespace RNReplicaNet;

#define VRC_MAILPROT_SEPARATOR      "\255"

namespace vrc
{

MailboxNetworking::MailboxNetworking( BaseMailboxStorage* p_storage ) :
 _p_mailboxStorage( p_storage ),
 _p_mailboxResponseCallback( NULL )
{
    // the storage object on clients is NULL, but not on server
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        assert( p_storage && "invalid storage for mailbox server!" );
    }
}

MailboxNetworking::~MailboxNetworking()
{
    // unset the networking object in mailbox client
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
        MailboxClient::get()->setNetworking( NULL );
}

void MailboxNetworking::PostObjectCreate()
{
    // set the networking object in mailbox client
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
        MailboxClient::get()->setNetworking( this );
}

void MailboxNetworking::setMailboxResponseCallback( CallbackMailboxNetwrokingResponse* p_cb )
{
    _p_mailboxResponseCallback = p_cb;
}

void MailboxNetworking::getMailFolders()
{
    assert( _p_mailboxResponseCallback && "invalid callback object" );

    tMailData request;
    request._header._cmd     = eMailboxCmdGetFolders;
    request._header._mailID  = 0;
    request._header._status  = 0;
    request._header._dataLen = 0;
    MASTER_FUNCTION_CALL( RPC_RequestMailCommand( request ) );
}

void MailboxNetworking::getMailHeaders( unsigned int attribute, const std::string& folder )
{
    assert( _p_mailboxResponseCallback && "invalid callback object" );

    tMailData request;
    request._header._cmd     = eMailboxCmdGetHeaders;
    request._header._mailID  = 0;
    request._header._status  = 0;
    request._header._dataLen = std::min( folder.size(), static_cast< unsigned int >( VRC_MAILBOXDATA_MAXLEN ) );
    strncpy( request._p_data, folder.c_str(), request._header._dataLen );
    MASTER_FUNCTION_CALL( RPC_RequestMailCommand( request ) );
}

void MailboxNetworking::getMail( unsigned int mailID )
{
    tMailData request;
    request._header._cmd     = eMailboxCmdGetMail;
    request._header._mailID  = mailID;
    request._header._status  = MailboxContent::eRecvMail;
    request._header._dataLen = 0;
    MASTER_FUNCTION_CALL( RPC_RequestMailCommand( request ) );
}

void MailboxNetworking::sendMail( const MailboxContent& mailcontent )
{
    //! TODO
MailboxContent content;
_p_mailboxResponseCallback->mailboxNetworkingResponse( content, MailboxContent::eSendMail, "OK - send mail" );
}

void MailboxNetworking::deleteMail( unsigned int mailID )
{
    //! TODO
//MailboxContent content;
//_p_mailboxResponseCallback->mailboxNetworkingResponse( content, MailboxContent::eDeleteMail, "OK - send mail" );
}

void MailboxNetworking::moveMail( unsigned int mailID, const std::string& destfolder )
{
    //! TODO
}

void MailboxNetworking::createMailFolder( const std::string& folder )
{
    //! TODO
}

void MailboxNetworking::deleteMailFolder( const std::string& folder )
{
    //! TODO
}

unsigned int MailboxNetworking::packData( const std::vector< std::string >& elements, const std::string& separator, char* p_buffer )
{
    // fill in the data buffer with semicolon separated elements
    p_buffer[ 0 ] = 0;
    unsigned int len = 0;
    for ( std::size_t cnt = 0; cnt < elements.size(); cnt++ )
    {
        len += elements[ cnt ].length();
        // just to be paranoid a little
        if ( len < VRC_MAILBOXDATA_MAXLEN )
        {
            strncat( p_buffer, elements[ cnt ].c_str(), VRC_MAILBOXDATA_MAXLEN );
            strncat( p_buffer, separator.c_str(), VRC_MAILBOXDATA_MAXLEN );
        }
        else
        {
            len -= elements[ cnt ].length();
            return len;
        }
    }

    return len;
}

unsigned int MailboxNetworking::unpackData( std::vector< std::string >& elements, const std::string& separator, char* p_buffer, unsigned int buffersize )
{
    p_buffer[ std::min( buffersize, static_cast< unsigned int >( VRC_MAILBOXDATA_MAXLEN ) ) ] = 0;
    yaf3d::explode( p_buffer, separator, &elements );

    return elements.size();
}

void MailboxNetworking::RPC_RequestMailCommand( tMailData request )
{ // called on server only

    assert( ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server ) && "invalid mailbox rpc call!" );
    assert( _p_mailboxStorage && "invalid storage object!" );

    // validate the request
    int          sessionID = GetProcessingDataBlocksFromSessionID();
    unsigned int userID    = StorageServer::get()->getUserID( sessionID );
    if ( !userID )
    {
        // ignore the request
        log_warning << "MailboxNetworking: user with session id " << sessionID << " requests for mail operation, but user is not logged in!" << std::endl;
        return;
    }

    switch ( request._header._cmd )
    {
        case eMailboxCmdGetFolders:
        {
            std::vector< std::string > folders;
            if ( !_p_mailboxStorage->getMailFolders( userID, folders ) )
            {
                request._header._status |= MailboxContent::eError;
                strncpy( request._p_data, "ERROR - receive folders, cannot get user folders", sizeof( request._p_data ) );
                request._header._dataLen = strnlen( request._p_data, sizeof( request._p_data ) );
                NOMINATED_REPLICAS_FUNCTION_CALL( 1, &sessionID, RPC_Response( request ) );
                break;
            }

            // setup the response packet and send it to client
            request._header._mailID = 0;
            request._header._status = MailboxContent::eRecvFolders | MailboxContent::eOk;

            // pack the data with semicolon separation
            unsigned int len = packData( folders, VRC_MAILPROT_SEPARATOR, request._p_data );
            request._p_data[ len + 1 ] = 0;
            request._header._dataLen = len + 1;

            // call the response rpc on client
            NOMINATED_REPLICAS_FUNCTION_CALL( 1, &sessionID, RPC_Response( request ) );
        }
        break;

        case eMailboxCmdGetHeaders:
        {
            // prepare the network packet for sending
            request._header._cmd    = eMailboxCmdGetHeaders;
            request._header._mailID = 0;
            request._header._status = MailboxContent::eRecvHeaders;

            // get the folder name
            std::string folder;
            request._p_data[ ( request._header._dataLen >= VRC_MAILBOXDATA_MAXLEN ) ? VRC_MAILBOXDATA_MAXLEN - 1 : request._header._dataLen ] = 0;
            folder = request._p_data;

            if ( !folder.length() )
            {
                request._header._status |= MailboxContent::eError;
                strncpy( request._p_data, "ERROR - receive headers, no folder selected", sizeof( request._p_data ) );
                request._header._dataLen = strnlen( request._p_data, sizeof( request._p_data ) );
                NOMINATED_REPLICAS_FUNCTION_CALL( 1, &sessionID, RPC_Response( request ) );
                break;
            }

            // get the headers of given folder
            std::vector< MailboxHeader > headers;
            if ( !_p_mailboxStorage->getMailHeaders( userID, MailboxHeader::eAll, folder, headers ) )
            {
                request._header._status |= MailboxContent::eError;
                strncpy( request._p_data, "ERROR - cannot get mail headers in requested folder '", sizeof( request._p_data ) );
                folder += "'";
                strncat( request._p_data, folder.c_str(), sizeof( request._p_data ) );
                request._header._dataLen = strnlen( request._p_data, sizeof( request._p_data ) );
                NOMINATED_REPLICAS_FUNCTION_CALL( 1, &sessionID, RPC_Response( request ) );
                break;
            }

            // setup the response packet and send it to client

            // assemble the headers into strings
            std::vector< std::string > packheaders;
            std::vector< MailboxHeader >::iterator p_header = headers.begin(), p_end = headers.end();
            for ( ; p_header != p_end; ++p_header )
            {
                std::string       subject;
                std::stringstream mailid, attributes;

                // convert the numbers to strings
                mailid << p_header->_id;
                attributes << p_header->_attributes;

                // limit the subject length
                subject = p_header->_subject;
                if ( subject.length() > 16 )
                {
                    subject [ 15 ] = 0;
                    subject += "...";
                }

                packheaders.push_back( mailid.str() + ", " + attributes.str() + "," + p_header->_date + "," + p_header->_from + "," + subject );
            }

            // pack the data with semicolon separation
            unsigned int len = packData( packheaders, VRC_MAILPROT_SEPARATOR, request._p_data );
            request._p_data[ len + 1 ] = 0;
            request._header._dataLen = len + 1;
            request._header._status  |= MailboxContent::eOk;
            // call the response rpc on client
            NOMINATED_REPLICAS_FUNCTION_CALL( 1, &sessionID, RPC_Response( request ) );
        }
        break;

        case eMailboxCmdGetMail:
        {
            request._header._status = MailboxContent::eRecvMail;

            // try to get the mail with given id
            MailboxContent content;
            if ( !_p_mailboxStorage->getMail( userID, request._header._mailID, content ) )
            {
                request._header._status |= MailboxContent::eError;
                strncpy( request._p_data, "ERROR - get mail, cannot get requested mail", sizeof( request._p_data ) );
                request._header._dataLen = strnlen( request._p_data, sizeof( request._p_data ) );
                NOMINATED_REPLICAS_FUNCTION_CALL( 1, &sessionID, RPC_Response( request ) );
                break;
            }

            // convert the numbers to strings
            std::stringstream mailid, attributes;
            mailid     << content._header._id;
            attributes << content._header._attributes;

            // assemble the data buffer string for a mail content
            request._p_data[ 0 ] = 0;
            std::vector< std::string > fields;
            fields.push_back( mailid.str() );
            fields.push_back( attributes.str() );

            fields.push_back( content._header._from );
            fields.push_back( content._header._to );
            fields.push_back( content._header._cc );
            fields.push_back( content._header._date );
            fields.push_back( content._header._subject );
            fields.push_back( content._body.c_str() );

            unsigned int len = packData( fields, VRC_MAILPROT_SEPARATOR, request._p_data );
            request._p_data[ len + 1 ] = 0;
            request._header._dataLen = len + 1;
            request._header._status  |= MailboxContent::eOk;

            // send out the mail content to client
            NOMINATED_REPLICAS_FUNCTION_CALL( 1, &sessionID, RPC_Response( request ) );
        }
        break;

        case eMailboxCmdSendMail:
        {
            //! TODO
        }
        break;

        case eMailboxCmdDeleMail:
        {
            //! TODO
        }
        break;

        case eMailboxCmdMoveMail:
        {
            //! TODO
        }
        break;

        case eMailboxCmdCreateFolder:
        {
            //! TODO
        }
        break;

        case eMailboxCmdDeleFolder:
        {
            //! TODO
        }
        break;

        default:
            log_error << "MailboxNetworking: unknown command received: " << request._header._cmd << std::endl;
    }
}

void MailboxNetworking::RPC_Response( tMailData response )
{ // called on client

    assert( ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client ) && "invalid mailbox rpc call!" );

    switch ( response._header._cmd )
    {
        case eMailboxCmdGetFolders:
        {
            response._p_data[ std::min( response._header._dataLen, static_cast< unsigned int >( VRC_MAILBOXDATA_MAXLEN ) ) ] = 0;

            MailboxContent content;

            if ( response._header._status & MailboxContent::eError )
            {
                _p_mailboxResponseCallback->mailboxNetworkingResponse( content, response._header._status, response._p_data );
                log_warning << "MailboxNetworking: error occured on getting mail folders '" << response._p_data << "'" << std::endl;
                break;
            }

            content._status = response._header._status;
            std::vector< std::string > folders;
            // unpack the data and fill the fodlers in mail content
            unpackData( folders, VRC_MAILPROT_SEPARATOR, response._p_data, response._header._dataLen );
            for ( std::size_t cnt = 0; cnt < folders.size(); cnt++ )
                content._folders.push_back( folders[ cnt ] );

            _p_mailboxResponseCallback->mailboxNetworkingResponse( content, MailboxContent::eRecvFolders, "OK - get folders" );
        }
        break;

        case eMailboxCmdGetHeaders:
        {
            response._p_data[ std::min( response._header._dataLen, static_cast< unsigned int >( VRC_MAILBOXDATA_MAXLEN ) ) ] = 0;

            MailboxContent content;
            content._status = response._header._status;

            if ( response._header._status & MailboxContent::eError )
            {
                _p_mailboxResponseCallback->mailboxNetworkingResponse( content, response._header._status, response._p_data );
                log_warning << "MailboxNetworking: error occured on getting mail headers '" << response._p_data << "'" << std::endl;
                break;
            }

            // unpack the data and fill the fodlers in mail content
            std::vector< std::string > packheaders;
            unpackData( packheaders, VRC_MAILPROT_SEPARATOR, response._p_data, response._header._dataLen );

            // assemble the mail content out of the unpacked data
            std::vector< std::string >::iterator p_header = packheaders.begin(), p_end = packheaders.end();
            for ( ; p_header != p_end; ++p_header )
            {
                MailboxHeader header;
                std::vector< std::string > fields;

                // extract the fields and check their count
                yaf3d::explode( *p_header, ",", &fields );
                if ( fields.size() < 5 )
                {
                    log_error << "MailboxNetworking: bad mailbox response for headers received" << std::endl;
                    break;
                }
                // upack the data
                std::stringstream mailid, attributes;
                mailid << fields[ 0 ];
                mailid >> header._id;
                attributes << fields[ 1 ];
                attributes >> fields[ 1 ];
                header._date    = fields[ 2 ];
                header._from    = fields[ 3 ];
                header._subject = fields[ 4 ];

                content._headers.push_back( header );
            }

            _p_mailboxResponseCallback->mailboxNetworkingResponse( content, MailboxContent::eRecvHeaders, "OK - get headers" );
        }
        break;

        case eMailboxCmdGetMail:
        {
            response._p_data[ std::min( response._header._dataLen, static_cast< unsigned int >( VRC_MAILBOXDATA_MAXLEN ) ) ] = 0;

            MailboxContent content;
            content._status = response._header._status;

            if ( response._header._status & MailboxContent::eError )
            {
                _p_mailboxResponseCallback->mailboxNetworkingResponse( content, response._header._status, response._p_data );
                log_warning << "MailboxNetworking: error occured on getting mail '" << response._p_data << "'" << std::endl;
                break;
            }

            // extract the mail content into fields
            std::vector< std::string > fields;
            unpackData( fields, VRC_MAILPROT_SEPARATOR, response._p_data, response._header._dataLen );
            if ( fields.size() < 8 )
            {
                log_error << "MailboxNetworking: bad mailbox response for mail content received" << std::endl;
                break;
            }

            // convert the strings to numbers
            std::stringstream mailid, attributes;
            mailid     << fields[ 0 ];
            mailid     >> content._header._id;
            attributes << fields[ 1 ];
            attributes >> content._header._attributes;

            content._header._from    = fields[ 2 ];
            content._header._to      = fields[ 3 ];
            content._header._cc      = fields[ 4 ];
            content._header._date    = fields[ 5 ];
            content._header._subject = fields[ 6 ];
            content._body            = fields[ 7 ];

            _p_mailboxResponseCallback->mailboxNetworkingResponse( content, MailboxContent::eRecvMail, "OK - get mail" );
        }
        break;

        case eMailboxCmdSendMail:
        {
            //! TODO
        }
        break;

        case eMailboxCmdDeleMail:
        {
            //! TODO
        }
        break;

        case eMailboxCmdMoveMail:
        {
            //! TODO
        }
        break;

        case eMailboxCmdCreateFolder:
        {
            //! TODO
        }
        break;

        case eMailboxCmdDeleFolder:
        {
            //! TODO
        }
        break;

        default:
            log_error << "MailboxNetworking: unknown command response received: " << response._header._cmd << std::endl;
    }
}

} // namespace vrc
