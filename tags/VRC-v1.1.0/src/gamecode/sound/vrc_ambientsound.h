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
 # entity ambient sound
 #
 #   date of creation:  03/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_AMBIENTSOUND_H_
#define _VRC_AMBIENTSOUND_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_AMBIENTSOUND    "AmbientSound"

//! 3DSound Entity
class EnAmbientSound : public yaf3d::BaseEntity
{
    public:
                                                    EnAmbientSound();

        virtual                                     ~EnAmbientSound();

        //! This entity does not need a transform node, which would be created by level manager on loading
        const bool                                  isTransformable() const { return false; }

        //! Initialize 
        void                                        initialize();

        //! Start / continue playing sound, set 'cont' to true in order to continue.
        void                                        startPlaying( bool cont = false );

        //! Stop playing sound, pass 'true' in order to pause only, otherwise the sound is stopped
        void                                        stopPlaying( bool pause = false );

        //! Set sound volume (0..1)
        void                                        setVolume( float volume );

        //! Get sound volume (0..1)
        float                                       getVolume();

    protected:

        // Handler system notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Sound file name
        std::string                                 _soundFile;

        //! Loop playing?
        bool                                        _loop;

        //! If true then the sound is immediately played after creation
        bool                                        _autoPlay;

        //! Sound volume [ 0..1 ]
        float                                       _volume;

        //! Sound ID
        unsigned int                                _soundID;

        //! Sound channel
        FMOD::Channel*                              _p_channel;
};

//! Entity type definition used for type registry
class AmbientSoundEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    AmbientSoundEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_AMBIENTSOUND, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~AmbientSoundEntityFactory() {}

        Macro_CreateEntity( EnAmbientSound );
};

}

#endif // _VRC_AMBIENTSOUND_H_
