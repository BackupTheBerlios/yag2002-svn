/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
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
 # keyframe animation tool
 #
 #
 #   date of creation:  10/16/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _ANIMUTIL_H_
#define _ANIMUTIL_H_

#include "base.h"


//! Keyframe animation utility class
/**
* This class allows manipulations of a NeoEngine keyframe animation file
   \author A. Botorabi (info@botorabi.de)
*/
class KeyFrameAnimUtil
{

    public:

                                                    KeyFrameAnimUtil();

                                                    ~KeyFrameAnimUtil();


        //! Load an 'nani' file
        bool                                        Load( NeoEngine::File* pkFile );

        //! Load an 'nani' file
        bool                                        Load( const std::string &strFileName );

        //! Store an 'nani' file
        bool                                        Store( NeoEngine::File* pkFile, bool bBinaryOutput = true );

        //! Store an 'nani' file
        bool                                        Store( const std::string &strFileName, bool bBinaryOutput = true );

        //! Get animated node object, valid after loading
        NeoEngine::AnimatedNode*                    GetAnimatedNode();

        //! Get node animation object, valid after loading
        NeoEngine::NodeAnimation*                   GetNodeAnimation();

        //! Set begining index of frames to be manipulated
        void                                        SetBeginFrame( size_t uiBeginFrame );
    
        //! Set end index of frames to be manipulated
        void                                        SetEndFrame( size_t uiEndFrame );

        //! Set new animation lenght ( in seconds )
        void                                        SetAnimLength( float fLength );

        //! Get animation lenght ( in seconds )
        float                                       GetAnimLength();

        //! Set node name of loaded animation
        void                                        SetAnimationName( const std::string& strName );

        //! Get node name of loaded animation
        const std::string&                          GetAnimationName();

        //! Get the total count of keyframes in loaded animation
        unsigned int                                GetNumKeyframes();

        //! Get animation id 
        unsigned int                                GetAnimID();

        //! Set animation id
        void                                        SetAnimID( unsigned int uiID );

        //! All following commands are processed on the keyframe sequence between m_uiBeginFrame and m_uiEndFrame

        //! Delete given frames
        void                                        CmdDeleteFrames( size_t uiBegin, size_t uiEnd );

        //! Rotate given keyframes about the given axis and amount
        void                                        CmdRotate( const NeoEngine::Vector3d& kRotationAxis, float fAngle );

        //! Set rotation
        void                                        CmdSetRotation( const NeoEngine::Vector3d& kRotation );

        //! Translate given keyframes
        void                                        CmdTranslate( const NeoEngine::Vector3d& kTranslate );

        //! Set translation
        void                                        CmdSetTranslation( const NeoEngine::Vector3d& kTranslation );

    protected:

        //! Begining index of frames to be manipulated
        size_t                                      m_uiBeginFrame;

        //! End index of frames to be manipulated
        size_t                                      m_uiEndFrame;

        //! Animated node ( valid after successfull reading of an anim file )
        NeoEngine::AnimatedNode*                   m_pkAnimatedNode;

        //! Node animation containing all necessary data ( valid after successfull reading of an anim file )
        NeoEngine::NodeAnimation*                   m_pkNodeAnim;

        //! Read the keyframes stored in an ".nani" file
        bool                                        ReadKeyframes( NeoEngine::File* pkFile );

        //! Write output file
        bool                                        WriteKeyframes( NeoEngine::File* pkFile, bool bBinaryOutput = true );

};


#endif // _ANIMUTIL_H_

