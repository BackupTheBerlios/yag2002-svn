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
 # Class for managing game settings
 #
 #   date of creation:  03/08/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_SETTINGS_H_
#error "do not include this file! this is an inline header."
#endif

inline void Settings::setFileName( const std::string& filename )
{
    _settingsFile = filename;
}

template< typename TypeT >
inline bool Settings::registerSetting( const std::string& token, const TypeT& value )
{
    // check for existing token name, no duplicates are allowed
    size_t tokens = _settings.size();
    for ( size_t cnt = 0; cnt < tokens; cnt++ ) 
        if ( _settings[ cnt ]->getTokenName() == token ) 
            return false;

    Setting< TypeT >* p_setting = new Setting< TypeT >( token, value );
    _settings.push_back( p_setting );
    return true;
}

template< class TypeT >
inline bool Settings::getValue( const std::string& token, TypeT& value )
{
    SettingBase* p_setting = findSetting( token );
    if ( p_setting == NULL ) 
        return false;

    value = ( static_cast< Setting< TypeT >* >( p_setting ) )->_value;
    return true;
}

template< class TypeT >
inline bool Settings::setValue( const std::string &token, const TypeT& value )
{
    SettingBase* p_setting = findSetting( token );
    if ( p_setting == NULL ) 
        return false;

    ( static_cast< Setting< TypeT >* >( p_setting ) )->_value = value;
    return true;
}
        
inline const std::vector< Settings::SettingBase* >& Settings::getAllSettingStorages() const
{
    return _settings;
}

inline Settings::SettingBase* Settings::findSetting( const std::string& token )
{
    std::vector< SettingBase* >::iterator pp_setting = _settings.begin(), pp_settingEnd = _settings.end();
    for ( ; pp_setting != pp_settingEnd; pp_setting++ )
        if ( ( *pp_setting )->getTokenName() == token )
            return *pp_setting;

    return NULL;
}
