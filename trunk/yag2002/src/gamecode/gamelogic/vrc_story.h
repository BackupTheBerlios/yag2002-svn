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
 # a story builds an interaction task with the player.
 #  it can be e.g. a quest.
 #
 #   date of creation:  05/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _VRC_STORY_H_
#define _VRC_STORY_H_

#include <vrc_main.h>
#include "vrc_storyevent.h"
#include <scripting/vrc_script.h>

namespace vrc
{

class StoryEngine;
class StoryBookLoader;

//! Story class responsible to process player events and progress the story state
class Story : public BaseScript< Story >, public yaf3d::RefCount< Story >
{
    public:

        //! Get the owner ID.
        unsigned int                                getOwnerID() const { return _ownerID; }

        //! Get story type
        const std::string&                          getType() const { return _type; }

        //! Get story name
        const std::string&                          getName() const { return _name; }

    protected:

        //! Construct a story with given type and type, this story will be stock element. Concrete stories are
        //   created by cloning.
        explicit                                    Story( const std::string type, const std::string name );

        virtual                                     ~Story();

        //! Setup the story given its script file
        bool                                        setup( const std::string& scriptfile );

        //! Create a concrete story as clone with given owner ID and name
        yaf3d::SmartPtr< Story >                    clone( unsigned int ownerID, const std::string& name );

        //! Process the event in story by calling script function.
        void                                        processEvent( unsigned int storyTime, const StoryEvent& event );

        //! Update the story
        void                                        update( float deltaTime );

        // Exposed methods to scripting
        // ############################

        //! Method for outputting to log system
        void                                        llog( const Params& arguments, Params& /*returnvalues*/ );

        //! Begin a new story
        void                                        lbeginStory( const Params& arguments, Params& /*returnvalues*/ );

        //! End an existing story
        void                                        lcloseStory( const Params& arguments, Params& /*returnvalues*/ );

        // ############################

        //! Story owner ID
        unsigned int                                _ownerID;

        //! Script file
        std::string                                 _scriptFile;

        //! Story type
        std::string                                 _type;

        //! Story name
        std::string                                 _name;

        //! The story engine
        StoryEngine*                                _p_storyEngine;

        //! Log object for story system output
        yaf3d::Log*                                 _p_log;

        //! This will be set if script errors detected, then event processing and updates are no longer done on the story scripts.
        bool                                        _freeze;

   friend class StoryEngine;
   friend class StoryBookLoader;
   friend class yaf3d::SmartPtr< Story >;
};

//! Type for story smart pointer
typedef yaf3d::SmartPtr< Story >    StoryPtr;

}

#endif // _VRC_STORY_H_
