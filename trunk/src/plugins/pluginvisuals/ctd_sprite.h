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
 # neoengine, sprite class
 #
 # this class implements a simple sprite
 #
 #
 #   date of creation:  03/21/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/21/2004 boto       creation of CTDSprite
 #
 #
 ################################################################*/

#ifndef _CTD_SPRITE_H_
#define _CTD_SPRITE_H_


#include <base.h>

#include <neoengine/sprite.h>
#include <neoengine/callback.h>

#include <string>


namespace CTD_IPluginVisuals {


// name of this entity
#define CTD_ENTITY_NAME_Sprite "Sprite"


class CTDSprite : public CTD::BaseEntity, public NeoEngine::FrameCallback
{

	public:

													CTDSprite();
		virtual										~CTDSprite();

		//******************************* override some functions **********************************//

		/**
		* Initializing function
		*/
		void										Initialize();

		/**
		* Messaging function.
		* \param  iMsgId                            Message ID
		* \param  pMsgStruct                        Message specific data structure
		* \return                                   This return value is message specific.
		*/
		int											Message( int iMsgId, void *pMsgStruct );

		//*******************************************************************************************//

		int											ParameterDescription( int param_index, CTD::ParameterDescriptor *pd );

		//******************************************************************************************//

		/**
		* Frame callback method for rendering the sprite without lighting
		* \param eType                              Callback type
		* \param pData                              Data pointer (type specific)
		*/
		void                                        FrameEvent( NeoEngine::FrameCallback::FRAMECALLBACKTYPE eType, void *pData );
		//******************************************************************************************//

		// entity parameters
		NeoEngine::Vector3d							m_kPosition;
		float										m_fSizeU;
		float										m_fSizeV;
		std::string									m_strMatFile;
		NeoEngine::Vector3d							m_kColor;
		// this flas determines wether the sprite should be activated ( rendered ) after initialization
		//  if you intend to use this sprite in other entities set this parameter to false ( default true )
		bool										m_bAutoActivate;

	private:
	
		// mesh entity for rendering the sprite
		NeoEngine::Sprite							*m_pkSprite;

};


// descriptor for sprite entity
class CTDSpriteDesc : public CTD::EntityDescriptor
{

	public:
													CTDSpriteDesc() { };
													~CTDSpriteDesc() { }
			
		const std::string&							GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_Sprite ) }
		CTD::BaseEntity*							CreateEntityInstance() { return (CTD::BaseEntity*) new CTDSprite; };

};

// global instance of sprite entity is used in dll interface
extern CTDSpriteDesc g_pkSpriteEntity_desc;

} // namespace CTD_IPluginVisuals 

#endif //_CTD_SPRITE_H_
