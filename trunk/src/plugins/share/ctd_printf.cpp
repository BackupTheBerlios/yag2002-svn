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
 # neoengine, debug output module
 #
 #
 #   date of creation:  05/30/2002
 #   last change:       12/16/2002
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#include <base.h>
#include "ctd_printf.h"

#include <string>

using namespace std;

CTDPrintf::CTDPrintf(unsigned int x, unsigned int y) 
{

    m_bNullPrintf       = false;
    m_bEnable           = true;
    m_nFontSize         = 10;
    m_nX                = x;
    m_nY                = y;
    m_nOffsetY          = 0;
    m_nRejectedLines    = 0;
    m_bMsgBuffering     = false;    // default is no text buffering

    m_pkFont            = NULL;

}

CTDPrintf::~CTDPrintf() 
{

}

void CTDPrintf::Init(unsigned int x, unsigned int y)
{

    m_bNullPrintf       = ( CTD::Framework::Get()->GetGameMode() == CTD::stateSERVER );

    if ( m_bNullPrintf ) {

        return;

    }

    m_nX = x;
    m_nY = y;

    m_pkFont        = NeoEngine::Core::Get()->GetConsole()->GetDefaultFont();
    // get screen' height
    m_uiScreenSizeY = NeoEngine::Core::Get()->GetRenderDevice()->GetHeight();
    // set font size
    m_nFontSize     = m_pkFont->GetLineHeight();

}

bool CTDPrintf::AddOutput(const std::string &strOutput) 
{

    // if disabled not output is desired
    if ( ( m_bEnable == false ) || ( m_bNullPrintf ) ) {

        return true;

    }

    m_nOffsetY += m_nFontSize;

    // check whether there is enough space for printing a new debug line in Print() later
    if (m_nOffsetY > ( m_uiScreenSizeY - m_nFontSize ) )
    {

        m_nRejectedLines++;

        return false;

    }
    
    m_OutputBuffer.push_back(strOutput);
    return true;

}


void CTDPrintf::Print() 
{

    // if disabled not output is desired
    if ( ( m_bEnable == false ) || ( m_bNullPrintf ) ) {

        return;

    }

    int lineoffset = m_nY;

    for (unsigned int num = 0; num < m_OutputBuffer.size(); num++) 
    {

        m_pkFont->Printf( m_nX, lineoffset, m_OutputBuffer[num].c_str() );
        lineoffset += m_nFontSize;

    }

    // check whether any lines have been rejected due to limited screen size
    if (m_nRejectedLines != 0) 
    {
    
        std::string str;
        char pcBuff[10];

        str = "Attention: no space for debug output on the screen. \n"
              "Increase screen size!\n";
        
        str += itoa(m_nRejectedLines, pcBuff, 10);
        str += " lines rejected";
        
        m_pkFont->Printf( 0, 0, str.c_str() );

    }

    // clear the text buffer for next frame
    m_nOffsetY = 0;
    m_nRejectedLines = 0;

    if (m_bMsgBuffering == false) {
        m_OutputBuffer.clear();
    }

}

