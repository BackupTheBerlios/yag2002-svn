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
 # entity ambient sound
 #
 #   date of creation:  03/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_AMBIENTSOUND_H_
#define _CTD_AMBIENTSOUND_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_AMBIENTSOUND    "AmbientSound"

//! 3DSound Entity
class EnAmbientSound : public BaseEntity
{
    public:
                                                    EnAmbientSound();

        virtual                                     ~EnAmbientSound();

        //! This entity does not need a transform node, which would be created by level manager on loading
        const bool                                  isTransformable() const { return false; }

        //! Initialize 
        void                                        initialize();

        //! Start playing sound
        void                                        startPlaying();

        //! Stop playing sound
        void                                        stopPlaying();

        //! Set sound volume (0..1)
        void                                        setVolume( float volume );

        //! Get sound volume (0..1)
        float                                       getVolume();

    protected:

        //! Resource directory for searching for sound files
        std::string                                 _soundFileDir;

        //! Sound file name
        std::string                                 _soundFile;

        //! Loop playing?
        bool                                        _loop;

        //! If true then the sound is immediately played after creation
        bool                                        _autoPlay;

        //! Sound volume [ 0..1 ]
        float                                       _volume;

        osgAL::SoundNode*                           _p_soundNode;

        osg::ref_ptr< osgAL::SoundState >           _soundState;
};

//! Entity type definition used for type registry
class AmbientSoundEntityFactory : public BaseEntityFactory
{
    public:
                                                    AmbientSoundEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_AMBIENTSOUND, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~AmbientSoundEntityFactory() {}

        Macro_CreateEntity( EnAmbientSound );
};

}

#endif // _CTD_AMBIENTSOUND_H_
