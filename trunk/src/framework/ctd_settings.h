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

// settings tokens
#define CTD_STOKEN_PLAYERNAME       "playername"
#define CTD_STOKEN_LEVEL            "level"
#define CTD_BTOKEN_NETWORKING       "networking"
#define CTD_BTOKEN_SERVER           "server"
#define CTD_BTOKEN_CLIENT           "client"
#define CTD_STOKEN_SERVERNAME       "servername"
#define CTD_STOKEN_CLIENTNAME       "clientname"
#define CTD_STOKEN_SERVERIP         "serverip"
#define CTD_ITOKEN_CLIENTPORT       "clientport"
#define CTD_ITOKEN_SERVERPORT       "serverport"


class Settings;

//! Singleton class for managing the game settings
class Settings
{

    public:

        /**
        * Retrieve the value of given setting which must be of type bool.
        * \param strSettingName         name of setting
        * \param bValue                 boolean value of setting
        * \return                       false if setting name and type do not match.
        */
        bool                            GetValue( const std::string &strSettingName, bool &bValue );

        /**
        * Set the value of given setting which must be of type bool.
        * \param strSettingName         name of setting
        * \param bValue                 boolean value of setting
        * \return                       false if setting name and type do not match.
        */
        bool                            SetValue( const std::string &strSettingName, const bool &bValue );

        /**
        * Retrieve the value of given setting which must be of type string.
        * \param strSettingName         name of setting
        * \param strValue               string value of setting
        * \return                       false if setting name and type do not match.
        */
        bool                            GetValue( const std::string &strSettingName, std::string &strValue );

        /**
        * Set the value of given setting which must be of type string.
        * \param strSettingName         name of setting
        * \param strValue               string value of setting
        * \return                       false if setting name and type do not match.
        */
        bool                            SetValue( const std::string &strSettingName, const std::string &strValue );

        /**
        * Retrieve the value of given setting which must be of type integer.
        * \param strSettingName         name of setting
        * \param iValue                 integer value of setting
        * \return                       false if setting name and type do not match.
        */
        bool                            GetValue( const std::string &strSettingName, int &iValue );

        /**
        * Set the value of given setting which must be of type integer.
        * \param strSettingName         name of setting
        * \param iValue                 integer value of setting
        * \return                       false if setting name and type do not match.
        */
        bool                            SetValue( const std::string &strSettingName, const int &iValue );

        /**
        * Create / retrieve an instance of this class.
        * \return                        Pointer to a valid instace of this class.
        */
        static Settings*                Get() 
                                        {                                       
                                            if( s_SingletonSettings == NULL ) s_SingletonSettings = new Settings;
                                            return s_SingletonSettings;
                                        }


    protected:

                                        Settings();

                                        ~Settings();

        static Settings*                s_SingletonSettings;

        /**
        * Load settings defined in file strCfgFileName.
        * \return  false if something went wrong.
        */
        bool                            Load( const std::string &strCfgFileName );

        /**
        * Store settings defined in file strCfgFileName.
        * \return                      false if something went wrong.
        */
        bool                            Store( const std::string &strCfgFileName );

        /**
        * Shutdown the singleton instance of Settings.
        */
        void                            Shutdown();

        //! Helper function for writing one setting of type string
        bool                            WriteString( const std::string &strToken, const std::string &strValue );

        //! Helper function for writing one setting of type bool
        bool                            WriteBool( const std::string &strToken, const bool &bValue );

        //! Helper function for writing one setting of type integer
        bool                            WriteInt( const std::string &strToken, const int &iValue );

        //! Helper function for reading given setting and its value of type string
        bool                            ReadString( const std::string &strToken, std::string &strValue );

        //! Helper function for reading given setting and its value of type bool
        bool                            ReadBool( const std::string &strToken, bool &bValue );

        //! Helper function for reading given setting and its value of type integer
        bool                            ReadInt( const std::string &strToken, int &iValue );

        //! This class contains one setting with associated value
        class Setting
        {

            public:


                                        Setting( const std::string &strName, bool bValue ) : m_iValue( 0 )
                                        {

                                            m_strToken          = strName;
                                            m_bValue            = bValue;
                                            m_eType             = eBOOL;

                                        }


                                        Setting( const std::string &strName, const std::string &strValue ) : m_bValue( false ), m_iValue( 0 )
                                        {

                                            m_strToken          = strName;
                                            m_strValue          = strValue;
                                            m_eType             = eSTRING;

                                        }

                                        Setting( const std::string &strName, int iValue  ) : m_bValue( false )
                                        {

                                            m_strToken          = strName;
                                            m_iValue            = iValue;
                                            m_eType             = eINT;

                                        }

                                        ~Setting() {}

                enum { eSTRING, eBOOL, eINT } m_eType;

                std::string             m_strToken;

                std::string             m_strValue;

                bool                    m_bValue;

                int                     m_iValue;

        };

        //! Find the setting in settings list with given name
        Setting*                        FindSetting( const std::string &strToken );

        //! List of game settings
        std::vector< Setting* >         m_vpkSettings;

        //! File buffer
        std::string                     m_strFileBuffer;

    friend  class   FrameworkImpl;

};


#include "ctd_settings_inl.h"

} // namespace CTD

#endif // _CTD_SETTINGS_H_
