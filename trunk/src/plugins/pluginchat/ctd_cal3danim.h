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
 # neoengine, Cal3D animation module 
 #
 # this module manages and synchronizes the character animation
 #  using Cal3D
 #
 #
 #   date of creation:  01/07/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  10/27/2003 boto       creation CTDCal3DAnim
 #
 #  10/30/2003 boto       implemented: initialization of cal3d anims 
 #                                      and control functions
 #
 #  01/07/2004 boto       port from fly3d to neoengine
 #
 ################################################################*/


#ifndef _CTD_CAL3D_ANIM_H_
#define _CTD_CAL3D_ANIM_H_

#include <cal3d.h>

// this animation handler reads cal3d files and 
//  executes animation commands such as walk and turn, etc.  
class CTDCal3DAnim 
{

	public:

													CTDCal3DAnim( NeoEngine::SceneNode* pkNode );

													~CTDCal3DAnim();

		// initialize the character and its animation
		bool										Initialize( const std::string &strCfgFile );

		void										UpdateAnim( const float &fDeltaTime );
		
        void										Render();

		// set level of detail [0.0 .. 1.0]
		void										SetLod( const float &fLod );

		// command interface for controlling animation
		void										Walk();
		
        void										WalkBack();
		
        void										Run();
		
        void										Jump();
		
        void										Landing();
		
        bool										IsLanded();		// rerurns true when landing is completed
		
        void										Stop();
		
        void										Idle();
		
        void										Turn();
		
        void										Look(const float &fAngleX, const float &fAngleY); // set the head orientation into look direction

	private:

		// cal3d config file parser
		bool										ParseModelConfiguration( const std::string& strFilename );

		// convertes cal3d geometry into neoengine buffers
		bool										SetupGeometry();

		std::vector< NeoEngine::Mesh* >				m_vpkMeshes;

		CalModel									m_calModel;
		CalCoreModel								m_calCoreModel;
		CalRenderer									*m_pCalRenderer;

		// is used to force an updating of polygon buffers ( this is needed after changing lod )
		bool										m_bUpdatePolygonBuffer;

		// core's node delivers info about transformation and rotation
		NeoEngine::SceneNode						*m_pkNode;

		float										m_fScale;
		// offset position and rotation
		NeoEngine::Quaternion						m_kRotation;
		NeoEngine::Vector3d							m_kPosition;

		// animation control related variables
		float										m_blendTime;
		float										m_fLandingTimer;
		float										m_fElapsedTime;

		int											m_IdLastAnimation;		// current animation id
		int											m_IdNewAnimation;		// new animation id

		// animation ids
		int											m_IdAnimIdle;
		int											m_IdAnimWalk;
		int											m_IdAnimRun;
		int											m_IdAnimAttack;
		int											m_IdAnimJump;
		int											m_IdAnimLand;
		int											m_IdAnimFreeze;

};

#endif  //_CTD_CAL3D_ANIM_H_
