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
 #   last change:		12/16/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 # 
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  09/10/2002 boto       added the feature of text buffering
 #
 #  11/01/2002 boto       added some usefull macros 
 #
 #  11/14/2002 boto       small bug fix in INIT macro
 #
 #  01/30/2003 boto       added macro for console printfs 'CTDCONSOLE_PRINT'
 #
 #  12/16/2003 boto       ported to neoengine (0.6.2)
 #
 ################################################################*/

/*
  usage:

	1. set the upper left corner of debug output region and the font size with the contructor or 'Init'
	2. use 'AddOutput' to add text into output queue
	3. call 'Print' in your plugin's fly_message function when CTD_PLUGIN_MSG_RENDER_2D is 
	    comming in (every frame)

    4. you must set 'CTDPRINTF' as preprocessor define to include all debug output stuff

  alternatively, you can use the macros included in header for your comfort.

  you can disable or enable the output with the functions 'Disable' and 'Enable'
  when no place is available on the screen for an output, the module prints an attention message with the
   number of lines which has been rejected

*/


#ifndef _CTD_PRINTF_
#define _CTD_PRINTF_

#include <vector>
#include <string>


// some usefull macros
//#######################################################################################################//

// console printfs can be done by the following macro
static std::string con_00234_str;
#define CTD_STR	"  CTD "
#define CTDCONSOLE_PRINT(msg_type, str)	{ con_00234_str = CTD_STR; con_00234_str += str; NeoEngine::neolog << msg_type << (con_00234_str) << std::endl; }	


// some usefull macros
#ifdef	 CTDPRINTF

// initialize the debug printer
#define  CTDPRINTF_INIT(dbg_obj, x, y, buffering)\
		dbg_obj.Init((x), (y));\
		dbg_obj.SetBuffering(buffering);

// activate/deactivate text abuffering (accumulation), the default is disabled
#define  CTDPRINTF_BUFFERING(dbg_obj, buf)	dbg_obj.SetBuffering((buf));
// use DEBUG_PRINT to add your text for next print update
#define	 CTDPRINTF_PRINT(dbg_obj, str)	dbg_obj.AddOutput((str));
// use this to print out whole the text; a good place for call this is in CTD_PLUGIN_MSG_RENDER_2D section
#define	 CTDPRINTF_UPDATE(dbg_obj)	dbg_obj.Print();
// enables the printing on screen
#define  CTDPRINTF_ENABLE(dbg_obj)	dbg_obj.Enable((true));
// disables the printing on screen, the content of buffer remains if buffering enabled before
#define  CTDPRINTF_DISABLE(dbg_obj)	dbg_obj.Enable((false));
// flush the text buffer, usefull if you enabled buffering before and would like to clear text in buffer
#define  CTDPRINTF_FLUSH(dbg_obj)	dbg_obj.FlushBuffer();

#else

#define  CTDPRINTF_INIT(dbg_obj, x, y, buffering)
#define  CTDPRINTF_BUFFERING(dbg_obj, buf)
#define  CTDPRINTF_PRINT(dbg_obj, str)
#define	 CTDPRINTF_UPDATE(dbg_obj)
#define  CTDPRINTF_ENABLE(dbg_obj)
#define  CTDPRINTF_DISABLE(dbg_obj)
#define  CTDPRINTF_FLUSH(dbg_obj)

#endif


//#######################################################################################################//


// helper class to conviniently print status and info into screen
class CTDPrintf {

public:

	CTDPrintf(unsigned int x = 10, unsigned int y = 10);
	~CTDPrintf();

	void	Init(unsigned int x, unsigned int y);

	void	Enable(bool en) {m_bEnable = en;}


	// activate/deactivate the text buffering, when activated the text for every print is
	//  accumulated, i.e. it is not cleared after every 'Print' call
	void	SetBuffering(bool buffering) { m_bMsgBuffering = buffering; }
	
	// flush the text buffer
	void	FlushBuffer() { m_OutputBuffer.clear(); }

	bool	AddOutput(const std::string&);
	void	Print();


private:

	std::vector<std::string>		m_OutputBuffer;		// output buffer, it is flushed when calling Print per frame

	bool							m_bNullPrintf;		// is true in server mode, then all printfs are without effect
	bool							m_bEnable;			// enables/disables the output
	bool							m_bMsgBuffering;	// enables the buffering of messages, i.e. no clearing per print call!
	unsigned int					m_nFontSize;
	unsigned int					m_nX;				// upper left X corner,  begin of the output section
	unsigned int					m_nY;				// upper left Y corner,  begin of the output section

	unsigned int					m_nOffsetY;			// used for incerementing lines 
	unsigned int					m_nRejectedLines;	// number of rejected outputs because of few screen space

	unsigned int					m_uiScreenSizeY;	// screen's height, this is used to rejects printfs when there is no place on screen anymore

	NeoEngine::Font					*m_pkFont;

};

#endif // _CTD_PRINTF_
