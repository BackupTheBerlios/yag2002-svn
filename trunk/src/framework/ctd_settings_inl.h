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
 # neoengine, settings
 #
 # this class implements the management of game settings
 #
 #
 #   date of creation:  06/15/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/

#ifndef _CTD_SETTINGS_H_
#error "do not include this file! this is an inline header."
#endif


template< typename TypeT >
inline bool Settings::RegisterSetting( const std::string &strToken, const TypeT &Value )
{

    // check for existing token names, no duplicated are allowed
    size_t uiTokens = m_vpkSettings.size();
    for ( size_t uiCnt = 0; uiCnt < uiTokens; uiCnt++ ) {

        if ( m_vpkSettings[ uiCnt ]->GetTokenName() == strToken ) {

            return false;

        }

    }
    Setting< TypeT > *pkSetting = new Setting< TypeT >( strToken, Value );
    m_vpkSettings.push_back( pkSetting );
    
    return true;

}

template< class TypeT >
inline bool Settings::GetValue( const std::string &strToken, TypeT& Value )
{

    SettingBase *pkSetting = FindSetting( strToken );
    if ( pkSetting == NULL ) {

        return false;

    }
    Value = ( static_cast< Setting< TypeT >* >( pkSetting ) )->m_Value;
    return true;

}

template< class TypeT >
inline bool Settings::SetValue( const std::string &strToken, const TypeT& Value )
{

    SettingBase *pkSetting = FindSetting( strToken );
    if ( pkSetting == NULL ) {

        return false;

    }
    ( static_cast< Setting< TypeT >* >( pkSetting ) )->m_Value = Value;
    return true;

}

inline Settings::SettingBase* Settings::FindSetting( const std::string &strToken )
{

    for ( size_t uiCnt = 0; uiCnt < m_vpkSettings.size(); uiCnt++ ) {

        if ( m_vpkSettings[ uiCnt ]->GetTokenName() == strToken ) {

            return m_vpkSettings[ uiCnt ];

        }
    }

    return NULL;

}
