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
 #  07/28/2004 boto       creation of CTDSettings
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/

#ifndef _CTD_SETTINGS_H_
#define _CTD_SETTINGS_H_


#include "ctd_frbase.h"
#include <string>


namespace CTD
{


class Settings;

//! Class for managing game settings
class Settings
{

    public:

                                        Settings();

                                        ~Settings();

        /**
        * Load settings defined in file strCfgFileName.
        * \param strSettingsFile        File the settings are loaded from and stored to
        * \return                       false if something went wrong.
        */
        bool                            Load( const std::string &strSettingsFile );

        /**
        * Load settings defined in file strCfgFileName.
        * \param pkFile                 File instance the settings are loaded from and stored to. It is deleted in Shutdown method. Don't delete it yourself!
        * \return                       false if something went wrong.
        */
        bool                            Load( NeoEngine::File *pkFile );

        /**
        * Store loaded settings file
        * \param pkFile                 File for storing, if NULL then the same file for loading is used
        * \return                       false if something went wrong.
        */
        bool                            Store( NeoEngine::File *pkFile = NULL );

        /**
        * Shutdown the singleton instance of Settings.
        */
        void                            Shutdown();

        /**
        * Register a new setting
        * \param strToken               Token name
        * \param Value                  Token value
        * \return                       flase ist the token name already exsists.
        */
        template< typename TypeT >
        bool                            RegisterSetting( const std::string &strToken, const TypeT &Value );

        /**
        * Get setting's value
        * \param strToken               Token name
        * \param Value                  Token value
        * \return                       true if the setting token exists, otherwise false
        */
        template< class TypeT >
        bool                            GetValue( const std::string &strToken, TypeT& Value );

        /**
        * Set setting's value
        * \param strToken               Token name
        * \param Value                  Token value to be set
        * \return                       true if the setting token exists, otherwise false
        */
        template< class TypeT >
        bool                            SetValue( const std::string &strToken, const TypeT& Value );

    protected:


        //! Base class for all types of settings
        class SettingBase
        {

            public:
                
                                                SettingBase() {};

                //! Get setting's name
                const std::string&              GetTokenName()
                                                {
                                                    return m_strToken;
                                                }

                //! Retrieve the type info of value
                virtual const std::type_info&   GetTypeInfo() = 0;

            protected:

                //! Token name
                std::string                     m_strToken;

        };

        //! This class contains one setting with associated value
        template< class TypeT >
        class Setting : public SettingBase
        {

            public:

                typedef typename TypeT          TokenType;

                                                Setting( const std::string &strName, TokenType Value )
                                                {
                                                    m_strToken = strName;
                                                    m_Value    = Value;
                                                }

                                                ~Setting() {}

                const std::type_info&           GetTypeInfo()
                                                {
                                                    return typeid( TokenType );
                                                }

            protected:


                //! Setting's value
                TokenType                       m_Value;

            friend  class Settings;

        };

        //! Find the setting in settings list with given name
        SettingBase*                    FindSetting( const std::string &strToken );

        //! Helper method for writing one setting of type string
        bool                            Write( const std::string &strToken, const std::string &strValue );

        //! Helper method for writing one setting of type bool
        bool                            Write( const std::string &strToken, const bool &bValue );

        //! Helper method for writing one setting of type integer
        bool                            Write( const std::string &strToken, const int &iValue );

        //! Helper method for writing one setting of type float
        bool                            Write( const std::string &strToken, const float &fValue );

        //! Helper method for writing one setting of type Vector3d
        bool                            Write( const std::string &strToken, const NeoEngine::Vector3d &kValue );

        //! Helper method for reading given setting and its value of type string
        bool                            Read( const std::string &strToken, std::string &strValue );

        //! Helper method for reading given setting and its value of type bool
        bool                            Read( const std::string &strToken, bool &bValue );

        //! Helper method for reading given setting and its value of type integer
        bool                            Read( const std::string &strToken, int &iValue );

        //! Helper method for reading given setting and its value of type float
        bool                            Read( const std::string &strToken, float &fValue );

        //! Helper method for reading given setting and its value of type Vector3d
        bool                            Read( const std::string &strToken, NeoEngine::Vector3d &kValue );

        //! List of game settings
        std::vector< SettingBase* >     m_vpkSettings;

        //! File buffer
        std::string                     m_strFileBuffer;

        //! file name where the settings are loaded and stored
        std::string                     m_strSettingsFile;

        //! Shows that a setting file has been successfully loaded
        bool                            m_bLoaded;

        NeoEngine::File                 *m_pkFile;


};


#include "ctd_settings_inl.h"

} // namespace CTD

#endif // _CTD_SETTINGS_H_
