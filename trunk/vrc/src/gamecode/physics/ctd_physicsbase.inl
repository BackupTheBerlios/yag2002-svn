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
 # some base functionalities are gathered here useful for physics
 #  entities
 #
 #   date of creation:  04/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

namespace CTD
{

inline void PhysicsSound::updateSound( float deltaTime )
{
    _pastTime += deltaTime;
}

//! Contact process callback function called when the body collides with level or other entities
template< class EntityT >
int entityContactProcessLevel( const NewtonMaterial* p_material, const NewtonContact* p_contact )
{
    // set right parameters for predfined materials
    Physics::levelContactProcess( p_material, p_contact );

    // determine which body is the cylinder entity
	NewtonBody*        p_body = s_entityColStruct->_p_body1;
    EntityT*           p_ent  = NULL;
    void*              p_userData = NewtonBodyGetUserData( p_body );
    if ( p_userData ) // check for entity collision
    {
        BaseEntity* p_entity = reinterpret_cast< BaseEntity* >( p_userData );
        p_ent  = dynamic_cast< EntityT* >( p_entity );
        if ( !p_ent )
        {
            p_ent = static_cast< EntityT* >( NewtonBodyGetUserData( s_entityColStruct->_p_body0 ) );
        }

	} 
    else // level collision
    {
        p_ent = static_cast< EntityT* >( NewtonBodyGetUserData( s_entityColStruct->_p_body0 ) );
    }

    // let the phyiscs core calculate the tangent and normal speeds
    Physics::genericContactProcess( p_material, p_contact );
    CollisionStruct* p_colStruct = Physics::getCollisionStruct();
    p_ent->_contactMaxNormalSpeed  = p_colStruct->_contactMaxNormalSpeed;
    p_ent->_contactMaxTangentSpeed = p_colStruct->_contactMaxTangentSpeed;

    // play appropriate sound only if the cylinder bumps to something
    if ( p_colStruct->_contactMaxNormalSpeed > p_ent->_playThreshold )
    {
        unsigned int attribute = ( unsigned int )( NewtonMaterialGetContactFaceAttribute( p_material ) );
        unsigned int materialType = attribute & 0xFF;
        switch ( materialType )
        {
            case 0: // only entities have face attribute 0
                // for a sofisticated design here a sound callback function of the colising entity could be called.
                //  but for now we play the default sound.
                p_ent->playSoundColMetal();
                break;

            case Physics::MAT_DEFAULT:
            case Physics::MAT_METALL:
                p_ent->playSoundColMetal();
                break;

            case Physics::MAT_WOOD:
                p_ent->playSoundColWood();
                break;

            case Physics::MAT_STONE:
                p_ent->playSoundColStone();
                break;

            case Physics::MAT_GRASS:
                p_ent->playSoundColGrass();
                break;

            // handle collision with all other material types
            default:
                p_ent->playSoundColMetal();
        }
    }

    return 1;
}

} // namespace CTD
