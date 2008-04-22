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
 # operation node
 #
 #   date of creation:  08/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _NODEOPERATION_H_
#define _NODEOPERATION_H_

#include <main.h>
#include <core/node.h>

namespace beditor
{

//! Operation node describing AND, OR
class NodeOperation : public BaseNode
{
    //! Set the smart pointer class as friend
    DECLARE_SMARTPTR_ACCESS( NodeOperation )

    public:

        //! Operation type
        enum OpType
        {
            eTypeAND = 0x100,
            eTypeOR
        };

        //! Construct an operation node of given type
        explicit                                NodeOperation( unsigned int type );

        //! Get the operation type, one of OpType enums.
        unsigned int                            getOperationType() const;


    protected:

        virtual                                 ~NodeOperation();

        //! Operation type
        unsigned int                            _opType;
};

//! Typedef for the smart pointer
typedef SmartPtr< NodeOperation >   NodeOperationPtr;

} // namespace beditor

#endif // _NODEOPERATION_H_
