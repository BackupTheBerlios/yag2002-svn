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


inline bool Settings::GetValue( const std::string &strSettingName, bool &bValue )
{

    Setting *pkSetting = FindSetting( strSettingName );

    if ( pkSetting == NULL ) {

        return false;

    }

    bValue = pkSetting->m_bValue;

    return true;

}

inline bool Settings::SetValue( const std::string &strSettingName, const bool &bValue )
{

    Setting *pkSetting = FindSetting( strSettingName );

    if ( pkSetting == NULL ) {

        return false;

    }

    pkSetting->m_bValue = bValue;

    return true;

}

inline bool Settings::GetValue( const std::string &strSettingName, std::string &strValue )
{

    Setting *pkSetting = FindSetting( strSettingName );

    if ( pkSetting == NULL ) {

        return false;

    }

    strValue = pkSetting->m_strValue;

    return true;

}

inline bool Settings::SetValue( const std::string &strSettingName, const std::string &strValue )
{

    Setting *pkSetting = FindSetting( strSettingName );

    if ( pkSetting == NULL ) {

        return false;

    }

    pkSetting->m_strValue = strValue;

    return true;

}

inline bool Settings::GetValue( const std::string &strSettingName, int &iValue )
{

    Setting *pkSetting = FindSetting( strSettingName );

    if ( pkSetting == NULL ) {

        return false;

    }

    iValue = pkSetting->m_iValue;

    return true;

}

inline bool Settings::SetValue( const std::string &strSettingName, const int &iValue )
{

    Setting *pkSetting = FindSetting( strSettingName );

    if ( pkSetting == NULL ) {

        return false;

    }

    pkSetting->m_iValue = iValue;

    return true;

}

inline Settings::Setting* Settings::FindSetting( const std::string &strToken )
{

    for ( size_t uiCnt = 0; uiCnt < m_vpkSettings.size(); uiCnt++ ) {

        if ( m_vpkSettings[ uiCnt ]->m_strToken == strToken ) {

            return m_vpkSettings[ uiCnt ];

        }
    }

    return NULL;

}
