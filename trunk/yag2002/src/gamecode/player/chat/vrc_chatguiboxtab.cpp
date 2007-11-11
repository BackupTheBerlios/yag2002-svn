/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 #  tabs for chat box gui
 #
 #   date of creation:  01/22/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_chatguibox.h"
#include "vrc_chatguiboxtab.h"
#include "../../sound/vrc_ambientsound.h"

namespace vrc
{

// layout prefix
#define CHATLAYOUTPANE_PREFIX       "chatboxpane_"

ChannelTabPane::ChannelTabPane( CEGUI::TabControl* p_tabcontrol, ChatGuiBox* p_guibox, bool isSystemIO ) :
_isSystemIO( isSystemIO ),
_whisperMode( false ),
_p_tabCtrl( p_tabcontrol ),
_p_guibox( p_guibox ),
_p_tabPane( NULL ),
_p_messagebox( NULL ),
_p_editbox( NULL ),
_p_listbox( NULL ),
_p_whisper( NULL )
{
    try
    {
        // every pane must have unique name
        std::string postfix;
        static unsigned int instnum = 0;
        std::stringstream   prefcounter;
        prefcounter << instnum++;
        postfix += prefcounter.str().c_str();

        _p_tabPane = static_cast< CEGUI::Window* >( CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", std::string( CHATLAYOUTPANE_PREFIX "tabpane" ) + postfix ) );
        _p_tabPane->setMetricsMode( CEGUI::Relative );
        _p_tabCtrl->addTab( _p_tabPane );

        // we must also set a unique id for later removal from tab control
        _p_tabPane->setID( instnum );
        _p_tabPane->subscribeEvent( CEGUI::Window::EventParentSized, CEGUI::Event::Subscriber( &vrc::ChannelTabPane::onSizeChanged, this ) );
        _p_tabPane->subscribeEvent( CEGUI::Window::EventShown, CEGUI::Event::Subscriber( &vrc::ChannelTabPane::onSelected, this ) );
        _p_tabPane->setFont( YAF3D_GUI_FONT8 );

        _p_messagebox = static_cast< CEGUI::MultiLineEditbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/MultiLineEditbox", std::string( CHATLAYOUTPANE_PREFIX "tabpane_msgbox" ) + postfix ) );
        _p_messagebox->setReadOnly( true );
        _p_messagebox->setMetricsMode( CEGUI::Absolute );
        _p_messagebox->setPosition( CEGUI::Point( GUI_PANE_SPACING, GUI_PANE_SPACING ) );
        // actual size is calculated in resize callback
        _p_messagebox->setSize( CEGUI::Size( 150.0f, 100.0f ) );
        _p_messagebox->setFont( YAF3D_GUI_FONT8 );
        _p_tabPane->addChildWindow( _p_messagebox );

        _p_editbox = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUTPANE_PREFIX "tabpane_editbox" ) + postfix ) );
        _p_editbox->subscribeEvent( CEGUI::MultiLineEditbox::EventCharacterKey, CEGUI::Event::Subscriber( &vrc::ChannelTabPane::onEditboxTextChanged, this ) );
        _p_editbox->setMetricsMode( CEGUI::Absolute );
        _p_tabPane->addChildWindow( _p_editbox );

        // nickname list, needed only for channel panes, not for system IO panes
        if ( !_isSystemIO )
        {
            // setup the body listbox
            _p_listbox = static_cast< CEGUI::Listbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Listbox", std::string( CHATLAYOUTPANE_PREFIX "tabpane_nicklist" ) + postfix ) );
            _p_listbox->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::ChannelTabPane::onListItemSelChanged, this ) );
            _p_listbox->setSortingEnabled( true );
            _p_listbox->setMetricsMode( CEGUI::Absolute );
            _p_listbox->setFont( YAF3D_GUI_FONT8 );
            _p_tabPane->addChildWindow( _p_listbox );

            // setup whisper checkbox
            _p_whisper = static_cast< CEGUI::Checkbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Checkbox", std::string( CHATLAYOUTPANE_PREFIX "tabpane_whisper" ) + postfix ) );;
            _p_whisper->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::ChannelTabPane::onWhisperChanged, this ) );
            _p_whisper->setText( "Whisper" );
            _p_whisper->setSelected( false );
            _p_whisper->setMetricsMode( CEGUI::Absolute );
            _p_whisper->setFont( YAF3D_GUI_FONT8 );
            _p_tabPane->addChildWindow( _p_whisper );

            _whisperMode = _p_whisper->isSelected();
        }

        // set initial pane size
        {
            CEGUI::EventArgs arg;
            onSizeChanged( arg );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "ChannelTabPane: problem creating a new tab pane" << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

ChannelTabPane::~ChannelTabPane()
{
    try
    {
        _p_tabCtrl->removeTab( _p_tabPane->getID() );
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_tabPane );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "~ChannelTabPane: problem cleaning up gui resources" << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void ChannelTabPane::updateMemberList( std::vector< std::string >& list )
{
    if ( _isSystemIO )
        return;

    // set selection background color
    CEGUI::ColourRect col(
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0.0f         , 0.8f ),
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0.0f         , 0.8f )
                          );
    // fill up the list
    _p_listbox->resetList();
    std::vector< std::string >::iterator p_beg = list.begin(), p_end = list.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( p_beg->c_str() );
        p_item->setSelectionColours( col );
        p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
        _p_listbox->insertItem( p_item, NULL );
    }

    //! TODO: select the last whispered persion!
    if ( list.size() > 0 )
    {
        _p_listbox->getListboxItemFromIndex( 0 )->setSelected( true );
    }
}

void ChannelTabPane::addMessage( const CEGUI::String& msg, const CEGUI::String& author )
{
    // let the user see that a message arrived when the chat box is hidden
    if ( _p_guibox->isHidden() )
    {
        _p_guibox->showMsgArrived( true );
        _p_guibox->getShortMsgBox()->setText( author + ": " + msg );
    }

    // mark also the pane head
    if ( !isSelected() )
        _p_tabPane->setText( _title + " *" );

    CEGUI::String buffer = _p_messagebox->getText();
    buffer += "[" + yaf3d::getFormatedTime() + "] " + author + ": " + msg;
    _p_messagebox->setText( buffer );
    // set carat position in order to trigger text scrolling after a new line has been added
    _p_messagebox->setCaratIndex( buffer.length() - 1 );
}

bool ChannelTabPane::onSelected( const CEGUI::EventArgs& /*arg*/ )
{
    // remove "new message" marker ( '*' )
    _p_tabPane->setText( _title );

    return true;
}

bool ChannelTabPane::onListItemSelChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );
    return true;
}

bool ChannelTabPane::onWhisperChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    _whisperMode = _p_whisper->isSelected();

    return true;
}

bool ChannelTabPane::onSizeChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // recalculate gui elements
    CEGUI::Size size = _p_tabPane->getSize( CEGUI::Absolute );

    _p_messagebox->setPosition( CEGUI::Point( GUI_PANE_SPACING, GUI_PANE_SPACING ) );
    _p_messagebox->setSize( CEGUI::Size( size.d_width - GUI_PANE_MSG_OFFSET_RIGHT, size.d_height - GUI_PANE_MSG_OFFSET_BUTTOM - GUI_PANE_SPACING ) );

    _p_editbox->setPosition( CEGUI::Point( GUI_PANE_SPACING, size.d_height - GUI_PANE_MSG_OFFSET_BUTTOM + GUI_PANE_SPACING ) );
    _p_editbox->setSize( CEGUI::Size( size.d_width - GUI_PANE_MSG_OFFSET_RIGHT, GUI_PANE_MSG_OFFSET_BUTTOM - 2.0f * GUI_PANE_SPACING ) );

    if ( !_isSystemIO )
    {
        _p_listbox->setPosition( CEGUI::Point( size.d_width - GUI_PANE_MSG_OFFSET_RIGHT + 2.0f * GUI_PANE_SPACING, GUI_PANE_SPACING ) );
        _p_listbox->setSize( CEGUI::Size( GUI_PANE_MSG_OFFSET_RIGHT - 3.0f * GUI_PANE_SPACING, size.d_height - GUI_PANE_MSG_OFFSET_BUTTOM - GUI_PANE_SPACING ) );

        _p_whisper->setPosition( CEGUI::Point( size.d_width - GUI_PANE_MSG_OFFSET_RIGHT + 2.0f * GUI_PANE_SPACING, size.d_height - GUI_PANE_MSG_OFFSET_BUTTOM - GUI_PANE_SPACING ) );
        _p_whisper->setSize( CEGUI::Size( GUI_PANE_MSG_OFFSET_RIGHT - 3.0f * GUI_PANE_SPACING, GUI_PANE_MSG_OFFSET_BUTTOM - 2.0f * GUI_PANE_SPACING ) );
    }

    return true;
}

bool ChannelTabPane::onEditboxTextChanged( const CEGUI::EventArgs& arg )
{
    // play sound
    gameutils::GuiUtils::get()->playSound( SND_NAME_TYPING );

    // check for 'Return' key
    CEGUI::KeyEventArgs& ke = static_cast< CEGUI::KeyEventArgs& >( const_cast< CEGUI::EventArgs& >( arg ) );
    if ( ke.codepoint == SDLK_RETURN )
    {
        assert( _configuration._p_protocolHandler && "invalid protocol handler!" );

        // skip empty lines
        if ( !_p_editbox->getText().length() )
            return true;

        // check if we are in whisper mode
        std::string recipient;
        if ( _whisperMode )
        {
            CEGUI::ListboxItem* p_sel = _p_listbox->getFirstSelectedItem();
            if ( p_sel && p_sel->getText().length() )
                recipient = p_sel->getText().c_str();
        }

        // send the msg over net
        _configuration._p_protocolHandler->send( _p_editbox->getText(), _configuration._channel, recipient );

        // add the msg to local chat box ( if it was not a command )
        if ( _p_editbox->getText().compare( 0, 1, "/" ) )
        {
            addMessage( _p_editbox->getText(), _configuration._nickname );
        }
        _p_editbox->setText( "" );
    }
    return true;
}

void ChannelTabPane::setTitle( const std::string& title )
{
    _title = title;
    _p_tabPane->setText( title );
}

const ChatConnectionConfig& ChannelTabPane::getConfiguration()
{
    return _configuration;
}

void ChannelTabPane::setConfiguration( const ChatConnectionConfig& conf )
{
    _configuration = conf;
}

void ChannelTabPane::setSelection()
{
    _p_tabCtrl->setSelectedTab( _p_tabPane->getID() );
}

bool ChannelTabPane::isSelected()
{
    return _p_tabCtrl->isTabContentsSelected( _p_tabCtrl->getTabContents( _p_tabPane->getID() ) );
}

void ChannelTabPane::setEditBoxFocus( bool en )
{
    if ( en )
    {
        _p_editbox->activate();
    }
    else
    {
        _p_editbox->deactivate();
    }
}

void ChannelTabPane::onReceive( const std::string& /*channel*/, const std::string& sender, const CEGUI::String& msg )
{
    addMessage( reinterpret_cast< const CEGUI::utf8* >( msg.c_str() ), sender );
}

void ChannelTabPane::onNicknameChanged( const std::string& newname, const std::string& oldname )
{
    // did _we_ change our nick name or someone else changed nickname?
    if ( newname == oldname )
    {
        // check for initial nick name notification
        if ( _configuration._nickname != newname )
        {
            // change nick name in internal list
            size_t numnicks = _nickNames.size();
            for ( size_t cnt = 0; cnt < numnicks; ++cnt )
            {
                if ( _nickNames[ cnt ] == _configuration._nickname )
                {
                    _nickNames[ cnt ] = newname;
                    break;
                }
            }

            addMessage( " you changed your nickname to '" + newname  + "'", "* " );
            _configuration._nickname = newname;
        }
    }
    else
    {
        // change nick name in internal list
        size_t numnicks = _nickNames.size();
        bool nickfound = false;
        for ( size_t cnt = 0; cnt < numnicks; ++cnt )
        {
            std::string realnick = _nickNames[ cnt ];
            if ( realnick[ 0 ] == '@' )
                realnick = realnick.erase( 0, 1 );

            if ( realnick == oldname )
            {
                nickfound = true;
                break;
            }
        }
        // post a message only if the user is in this channel
        if ( nickfound )
        {
            addMessage( "'" + oldname + "' changed nickname to '" + newname  + "'", "* " );
        }
    }
    // trigger updating the listbox
    _configuration._p_protocolHandler->requestMemberList( _configuration._channel );
}

void ChannelTabPane::onJoinedChannel( const ChatConnectionConfig& cfg )
{
    if ( _configuration._nickname != cfg._nickname )
        addMessage( cfg._nickname + " entered the chat room", "* " );

    // trigger updating the listbox
    _configuration._p_protocolHandler->requestMemberList( _configuration._channel );
}

void ChannelTabPane::onLeftChannel( const ChatConnectionConfig& cfg )
{
    // have _we_ left the channel ?
    if ( ( cfg._nickname == _configuration._nickname ) && ( cfg._channel == _configuration._channel ) )
    {
        // trigger removal of this tab pane
        _p_guibox->removeTabPane( this );
    }
    else
    {
        addMessage( cfg._nickname + " left the chat room", "* " );
        // trigger updating the listbox
        _configuration._p_protocolHandler->requestMemberList( _configuration._channel );
    }
}

void ChannelTabPane::onKicked( const std::string& channel, const std::string& kicker, const std::string& kicked )
{
    // were _we_ kicked ?
    if ( kicked == _configuration._nickname )
    {
        addMessage( "you have been kicked by " + kicker + " from channel " + channel, "* " );
    }
    else
    {
        addMessage( kicked + " has been kicked by " + kicker, "* " );
    }
}

void ChannelTabPane::onReceiveMemberList( const std::string& /*channel*/ )
{
    // update member list
    _nickNames.clear();
    _configuration._p_protocolHandler->getMemberList( _configuration._channel, _nickNames );
    updateMemberList( _nickNames );
}

} // namespace vrc
