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
 # neoengine, base particle class
 #
 # this class implements common functions of a particle system
 #  it uses the library of David K. McAllister
 #
 #
 #   date of creation:  03/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/23/2004 boto       creation of CTDBaseParticle
 #
 #
 ################################################################*/

#ifndef _CTD_BASEPARTICLE_H_
#define _CTD_BASEPARTICLE_H_

#include <base.h>

namespace CTD_IPluginVisuals {

// base class for all kinds of particle systems
class CTDBaseParticle
{

	public:

													CTDBaseParticle();
													~CTDBaseParticle();

		
		/**
		* create a particle group
		* \param kOrigin                           the origin of particle system in world space
		* \param iPopulation                       max particles allocated for the group
		* \param pkPrimitive                       template primitive used for rendering
		* \return                                  handle of new created group
		*/
		int										   CreateGroup( const NeoEngine::Vector3d &kOrigin, int iPopulation );

		/**
		* set new origin for particle group
		* \param kOrigin                           the origin of particle system in world space
		*/
		void									   SetGroupOrigin( const NeoEngine::Vector3d &kOrigin ) { m_kOrigin = kOrigin; }

		/**
		* create an action list for a particle group ( first use CreateGroup function )
		* \return                                  handle of new created action list
		*/
		int										   NewActionList();

		/**
		* end an action list for a particle group ( first use NewActionList function )
		*/
		void									   EndActionList();



		/**
		* Update particle states
		* \param fDeltaTime                        Time passed since last update
		*/
		void									   UpdateParticles( float fDeltaTime );

		/**
		* Render particles
		* \param pkPrimitive                       primitive used for rendering
		*/
		void									   RenderParticles( NeoEngine::RenderPrimitive *pkPrimitive );

	private:

		// particle system's origin
		NeoEngine::Vector3d							m_kOrigin;

		int											m_hParticleHandle;
		int											m_hActionHandle;

};

} // namespace CTD_IPluginVisuals 

#endif //_CTD_BASEPARTICLE_H_
