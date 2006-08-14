/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # class for testing the script base class BaseScript
 #
 #   date of creation:  04/15/2006
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#ifndef _VRC_TESTSCRIPT_H_
#define _VRC_TESTSCRIPT_H_

#include "vrc_script.h"

namespace vrc
{

#define ENTITY_NAME_TESTSCRIPT      "TestScript"

//! Class for a test script entity
class EnTestScript : public BaseScript< EnTestScript >, public yaf3d::BaseEntity
{
    public:

                                                EnTestScript();

        virtual                                 ~EnTestScript();

        //! Initialize
        void                                    initialize();

        //! This entity needs updating
        void                                    updateEntity( float deltaTime );

        //! test for an exposed method
        void                                    exposedMethod1( const Params& arguments, Params& returnvalues );

        //! test for an exposed method
        void                                    exposedMethod2( const Params& arguments, Params& returnvalues );

    protected:

        //! Handle notifications
        void                                    handleNotification( const yaf3d::EntityNotification& notification );

        //! Script file
        std::string                             _scriptFile;
};

//! Entity type definition used for type registry
class TestScriptEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    TestScriptEntityFactory() :
                                                     yaf3d::BaseEntityFactory
                                                     (
                                                       ENTITY_NAME_TESTSCRIPT,
                                                       yaf3d::BaseEntityFactory::Client | yaf3d::BaseEntityFactory::Standalone
                                                      )
                                                    {}

        virtual                                     ~TestScriptEntityFactory() {}

        Macro_CreateEntity( EnTestScript );
};

}

#endif //_VRC_TESTSCRIPT_H_
