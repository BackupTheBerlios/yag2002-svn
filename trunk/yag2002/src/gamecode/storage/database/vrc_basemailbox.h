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
 # base class for mailbox functionality used by storage managers
 #
 #   date of creation:  07/29/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ################################################################*/

#ifndef _VRC_BASEMAILBOX_H_
#define _VRC_BASEMAILBOX_H_

#include <vrc_main.h>


namespace vrc
{

//! Default mail box folders
#define VRC_DEFAULT_MAILFOLDER_INBOX      "Inbox"
#define VRC_DEFAULT_MAILFOLDER_SENT       "Sent"

class BaseMailbox
{
    public:

        //! Class for transfering mail content
        class Content
        {
            public:
                                        Content() :
                                         _attributes( 0 ),
                                         _id( 0 )
                                        {
                                        }

                virtual                 ~Content() {}

                //! Sender
                std::string             _from;

                //! Comma separated recipients
                std::string             _to;

                //! Comma separated copy recipients
                std::string             _cc;

                //! Timestamp of receipt or send
                std::string             _date;

                //! Mail subject
                std::string             _subject;

                //! Mail body
                std::string             _body;

                enum Attributes
                {
                    //! New mail, still not read
                    eNotRead        = 0x01,
                    //! Normal priority mail
                    ePriorityNormal = 0x04,
                    //! High priority mail
                    ePriorityHeigh  = 0x02
                };

                //! Mail attributes, see Attributes
                unsigned int            _attributes;

                //! Unique mail ID
                unsigned int            _id;
        };

    public:

                                                BaseMailbox();

        virtual                                 ~BaseMailbox();

        //! Get mail headers with given attribute (see Content::Attributes), they are stored in 'headers' without body.
        virtual bool                             getMailHeaders( unsigned int userID, unsigned int attribute, const std::string& folder, std::vector< Content >& headers ) = 0;

        //! Get mail with given ID.
        virtual bool                             getMail( unsigned int userID, unsigned int mailID, Content& mailcontent ) = 0;

        //! Send mail
        virtual bool                             sendMail( unsigned int userID, const Content& mailcontent ) = 0;

        //! Delete mail with given ID.
        virtual bool                             deleteMail( unsigned int userID, unsigned int mailID ) = 0;

        //! Move mail with given ID to 'destfolder'. The destination folder must exist.
        virtual bool                             moveMail( unsigned int userID, unsigned int mailID, const std::string& destfolder ) = 0;

        //! Create a new mail folder.
        virtual bool                             createMailFolder( unsigned int userID, const std::string& folder ) = 0;

        //! Delete the given mail folder.
        virtual bool                             deleteMailFolder( unsigned int userID, const std::string& folder ) = 0;
};

} // namespace vrc

#endif  // _VRC_BASEMAILBOX_H_
