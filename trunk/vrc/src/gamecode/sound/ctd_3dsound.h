/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # entity 3D sound
 #
 #   date of creation:  03/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_3DSOUND_H_
#define _CTD_3DSOUND_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>

namespace CTD
{

#define ENTITY_NAME_3DSOUND    "3DSound"

//! 3DSound Entity
class En3DSound : public BaseEntity
{
    public:
                                                    En3DSound();

        virtual                                     ~En3DSound();

        //! Initialize 
        void                                        initialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Start playing sound
        void                                        startPlaying();

        //! Stop playing sound
        void                                        stopPlaying();

    protected:

        //! Resource directory for searching for sound files
        std::string                                 _soundFileDir;

        //! Sound file name
        std::string                                 _soundFile;

        //! Loop playing?
        bool                                        _loop;

        //! Sound position
        osg::Vec3f                                  _position;

        //! If true then the sound is immediately played after creation
        bool                                        _autoPlay;

        //! Sound volume [ 0..1 ]
        float                                       _volume;

        //! Reference distance
        float                                       _referenceDist;

        //! Rolloff factor
        float                                       _rolloffFac;

        //! Used for visualizing sound sources
        bool                                        _showSource;

        osgAL::SoundNode*                           _p_soundNode;

        osgAL::SoundState*                          _soundState;
};

//! Entity type definition used for type registry
class ThreeDSoundEntityFactory : public BaseEntityFactory
{
    public:
                                                    ThreeDSoundEntityFactory() : BaseEntityFactory(ENTITY_NAME_3DSOUND) {}

        virtual                                     ~ThreeDSoundEntityFactory() {}

        Macro_CreateEntity( En3DSound );
};

}

#endif // _CTD_3DSOUND_H_
