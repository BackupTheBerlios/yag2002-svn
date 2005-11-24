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
 # player's chat box gui
 #
 #   date of creation:  04/19/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_chatguibox.h"
#include "../../sound/vrc_ambientsound.h"

namespace vrc
{
// layout prefix
#define CHATLAYOUT_PREFIX           "chatbox_"
#define FADE_TIME                   1.0f

#define VRC_IMAGE_SET               "VRCImageSet"
#define VRC_IMAGE_SET_FILE          "gui/imagesets/VRCImageSet.imageset"

// some layout related constants
#define GUI_TABCTRL_OFFSETX         25.0f
#define GUI_TABCTRL_OFFSETY         35.0f
#define GUI_TABCTRL_SIZEY           35.0f
#define GUI_TABCTRL_TAB_HEIGHT      25.0f

#define GUI_PANE_SPACING            5.0f
#define GUI_PANE_MSG_OFFSET_RIGHT   100.0f
#define GUI_PANE_MSG_OFFSET_BUTTOM  45.0f

#define GUI_CLOSE_BTN_WIDTH         70.0f
#define GUI_CLOSE_BTN_HEIGHT        20.0f

#define GUI_IRCCONNECT_BTN_WIDTH    110.0f
#define GUI_IRCCONNECT_BTN_HEIGHT   20.0f

// sound stuff
#define SND_CLOSE_CHANNEL           "sound/chatgui/closechannel.wav"
#define SND_CONNECT                 "sound/chatgui/connect.wav"
#define SND_TYPING                  "sound/chatgui/typing.wav"
#define SND_VOL_CLOSE_CHANNEL       0.2f
#define SND_VOL_CONNECT             0.2f
#define SND_VOL_TYPING              0.5f
#define SND_NAME_CLOSE_CHANNEL      "cgui_clc"
#define SND_NAME_CONNECT            "cgui_con"
#define SND_NAME_TYPING             "cgui_typ"

ChatGuiBox::ChannelTabPane::ChannelTabPane( CEGUI::TabControl* p_tabcontrol, ChatGuiBox* p_guibox ) :
_p_tabCtrl( p_tabcontrol ),
_p_guibox( p_guibox ),
_p_tabPane( NULL ),
_p_messagebox( NULL ),
_p_editbox( NULL ),
_p_listbox( NULL )
{
    try
    {
        // every pane must have unique name
        std::string postfix;
        static unsigned int instnum = 0;
        std::stringstream   prefcounter;
        prefcounter << instnum++;
        postfix += prefcounter.str().c_str();

        _p_tabPane = static_cast< CEGUI::Window* >( CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", std::string( CHATLAYOUT_PREFIX "tabpane" ) + postfix ) );
        _p_tabPane->setMetricsMode( CEGUI::Relative );
        _p_tabCtrl->addTab( _p_tabPane );

        // we must also set a unique id for later removal from tab control
        _p_tabPane->setID( instnum );
        _p_tabPane->subscribeEvent( CEGUI::Window::EventParentSized, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::ChannelTabPane::onSizeChanged, this ) );
        _p_tabPane->subscribeEvent( CEGUI::Window::EventShown, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::ChannelTabPane::onSelected, this ) );
        _p_tabPane->setFont( YAF3D_GUI_FONT8 );

        _p_messagebox = static_cast< CEGUI::MultiLineEditbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/MultiLineEditbox", std::string( CHATLAYOUT_PREFIX "tabpane_msgbox" ) + postfix ) );
        _p_messagebox->setReadOnly( true );
        _p_messagebox->setMetricsMode( CEGUI::Absolute );
        _p_messagebox->setPosition( CEGUI::Point( GUI_PANE_SPACING, GUI_PANE_SPACING ) );
        // actual size is calculated in resize callback
        _p_messagebox->setSize( CEGUI::Size( 150.0f, 100.0f ) );
        _p_messagebox->setFont( YAF3D_GUI_FONT8 );
        _p_tabPane->addChildWindow( _p_messagebox );

        _p_editbox = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "tabpane_editbox" ) + postfix ) );
        _p_editbox->subscribeEvent( CEGUI::MultiLineEditbox::EventCharacterKey, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::ChannelTabPane::onEditboxTextChanged, this ) );
        _p_editbox->setMetricsMode( CEGUI::Absolute );
        _p_tabPane->addChildWindow( _p_editbox );

        // nickname list
        _p_listbox = static_cast< CEGUI::Listbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Listbox", std::string( CHATLAYOUT_PREFIX "tabpane_nicklist" ) + postfix ) );
        _p_listbox->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::ChannelTabPane::onListItemSelChanged, this ) );
        _p_listbox->setSortingEnabled( true );
        _p_listbox->setMetricsMode( CEGUI::Absolute );
        _p_listbox->setFont( YAF3D_GUI_FONT8 );
        _p_tabPane->addChildWindow( _p_listbox );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "ChannelTabPane: problem creating a new tab pane" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

ChatGuiBox::ChannelTabPane::~ChannelTabPane()
{
    try
    {
        _p_tabCtrl->removeTab( _p_tabPane->getID() );
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_tabPane );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "~ChannelTabPane: problem cleaning up gui resources" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void ChatGuiBox::ChannelTabPane::updateMemberList( std::vector< std::string >& list )
{
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

    if ( list.size() > 0 )
    {
        _p_listbox->getListboxItemFromIndex( 0 )->setSelected( true );
    }
}

void ChatGuiBox::ChannelTabPane::addMessage( const CEGUI::String& msg, const CEGUI::String& author )
{
    // let the user see that a message arrived when the chat box is hidden
    if ( _p_guibox->isHidden() )
    {
        _p_guibox->showMsgArrived( true );
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

bool ChatGuiBox::ChannelTabPane::onSelected( const CEGUI::EventArgs& arg )
{
    // remove "new message" marker ( '*' )
    _p_tabPane->setText( _title );

    return true;
}

bool ChatGuiBox::ChannelTabPane::onListItemSelChanged( const CEGUI::EventArgs& arg )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );
    return true;
}

bool ChatGuiBox::ChannelTabPane::onSizeChanged( const CEGUI::EventArgs& arg )
{
    // recalculate gui elements

    CEGUI::Size size = _p_tabPane->getSize( CEGUI::Absolute );

    _p_messagebox->setPosition( CEGUI::Point( GUI_PANE_SPACING, GUI_PANE_SPACING ) );
    _p_messagebox->setSize( CEGUI::Size( size.d_width - GUI_PANE_MSG_OFFSET_RIGHT, size.d_height - GUI_PANE_MSG_OFFSET_BUTTOM - GUI_PANE_SPACING ) );

    _p_editbox->setPosition( CEGUI::Point( GUI_PANE_SPACING, size.d_height - GUI_PANE_MSG_OFFSET_BUTTOM + GUI_PANE_SPACING ) );
    _p_editbox->setSize( CEGUI::Size( size.d_width - GUI_PANE_MSG_OFFSET_RIGHT, GUI_PANE_MSG_OFFSET_BUTTOM - 2.0f * GUI_PANE_SPACING ) );

    _p_listbox->setPosition( CEGUI::Point( size.d_width - GUI_PANE_MSG_OFFSET_RIGHT + 2.0f * GUI_PANE_SPACING, GUI_PANE_SPACING ) );
    _p_listbox->setSize( CEGUI::Size( GUI_PANE_MSG_OFFSET_RIGHT - 3.0f * GUI_PANE_SPACING, size.d_height - 2.0f * GUI_PANE_SPACING ) );

    return true;
}

bool ChatGuiBox::ChannelTabPane::onEditboxTextChanged( const CEGUI::EventArgs& arg )
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
        
        // send the msg over net
        _configuration._p_protocolHandler->send( _p_editbox->getText().c_str(), _configuration._channel ); 

        // add the msg to local chat box ( if it was not a command )
        if ( _p_editbox->getText().compare( 0, 1, "/" ) )
        {
            addMessage( _p_editbox->getText(), _configuration._nickname );
        }
        _p_editbox->setText( "" );
    }
    return true;
}

void ChatGuiBox::ChannelTabPane::setTitle( const std::string& title )
{
    _title = title;
    _p_tabPane->setText( title );
}

const ChatConnectionConfig& ChatGuiBox::ChannelTabPane::getConfiguration()
{
    return _configuration;
}

void ChatGuiBox::ChannelTabPane::setConfiguration( const ChatConnectionConfig& conf )
{
    _configuration = conf;
}

void ChatGuiBox::ChannelTabPane::setSelection()
{
    _p_tabCtrl->setSelectedTab( _p_tabPane->getID() );
}

bool ChatGuiBox::ChannelTabPane::isSelected()
{       
    return _p_tabCtrl->isTabContentsSelected( _p_tabCtrl->getTabContents( _p_tabPane->getID() ) );
}

void ChatGuiBox::ChannelTabPane::setEditBoxFocus( bool en )
{
    if ( en )
        _p_editbox->activate();
    else
        _p_editbox->deactivate();
}

void ChatGuiBox::ChannelTabPane::onReceive( const std::string& channel, const std::string& sender, const std::string& msg )
{
    addMessage( msg, sender );
}

void ChatGuiBox::ChannelTabPane::onNicknameChanged( const std::string& newname, const std::string& oldname )
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

void ChatGuiBox::ChannelTabPane::onJoinedChannel( const ChatConnectionConfig& cfg )
{
    if ( _configuration._nickname != cfg._nickname )
        addMessage( cfg._nickname + " entered the chat room", "* " );

    // trigger updating the listbox
    _configuration._p_protocolHandler->requestMemberList( _configuration._channel );
}

void ChatGuiBox::ChannelTabPane::onLeftChannel( const ChatConnectionConfig& cfg )
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

void ChatGuiBox::ChannelTabPane::onKicked( const std::string& channel, const std::string& kicker, const std::string& kicked )
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

void ChatGuiBox::ChannelTabPane::onReceiveMemberList( const std::string& channel )
{
    // update member list
    _nickNames.clear();
    _configuration._p_protocolHandler->getMemberList( _configuration._channel, _nickNames );
    updateMemberList( _nickNames );
}

//------------------------

ChatGuiBox::ChatGuiBox() :
_boxState( BoxHidden ),
_connectionState( ConnectionIdle ),
_p_chatMgr( NULL ),
_p_frame( NULL ),
_p_btnCloseChannel( NULL ),
_p_btnConnectIRC( NULL ),
_p_btnOpen( NULL ),
_p_btnMsgArrived( NULL ),
_p_tabCtrl( NULL ),
_modeEdit( false ),
_fadeTimer( 0 ),
_frameAlphaValue( 1.0f ),
_p_connectionDialog( NULL )
{
}

ChatGuiBox::~ChatGuiBox()
{
    try
    {
        if ( _p_connectionDialog )
            delete _p_connectionDialog;

        // delete all tab panes
        ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
        for ( ; p_beg != p_end; ++p_beg )
            delete p_beg->second;

        CEGUI::WindowManager::getSingleton().destroyWindow( _p_btnOpen );
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_frame );
        CEGUI::ImagesetManager::getSingleton().destroyImageset( VRC_IMAGE_SET );

    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "ChatGuiBox: problem cleaning up gui resources" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void ChatGuiBox::initialize( ChatManager* p_chatMgr )
{
    _p_chatMgr = p_chatMgr;
    // init nickname with player name
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, _nickname );

    // as this class can be instatiated several times it is important to generated unique prefix for every instance
    std::string prefix( CHATLAYOUT_PREFIX );
    static unsigned int instnum = 0;
    std::stringstream   prefcounter;
    prefcounter << instnum++;
    prefix += prefcounter.str().c_str();

    CEGUI::Window* p_wnd = gameutils::GuiUtils::get()->getMainGuiWindow();
    try
    {
        _p_frame = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", CHATLAYOUT_PREFIX "_charboxframe_" ) );
		_p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::onCloseFrame, this ) );
        // note: a minimum size must exist, otherwise cegui may hang during some internal calculations!
        _p_frame->setMinimumSize( CEGUI::Size( 0.2f, 0.2f ) );
        _p_frame->hide();
        _p_frame->setText( "chatbox" );
        _p_frame->setAlpha( 0.8f );
        _p_frame->setPosition( CEGUI::Point( 0.08f, 0.7f ) );
        _p_frame->setSize( CEGUI::Size( 0.86f, 0.28f ) );

        p_wnd->addChildWindow( _p_frame );

        // create close channel button
        _p_btnCloseChannel = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", std::string( CHATLAYOUT_PREFIX "_charboxframe_close_" ) ) );
        _p_btnCloseChannel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::onClickedCloseChannelPane, this ) );
        _p_btnCloseChannel->setMetricsMode( CEGUI::Absolute );
        // actual size is calculated in resize callback
        _p_btnCloseChannel->setSize( CEGUI::Size( 50.0f, 20.0f ) );
        _p_btnCloseChannel->setText( "close" );
        _p_btnCloseChannel->setFont( YAF3D_GUI_FONT8 );
        _p_btnCloseChannel->show();
        _p_frame->addChildWindow( _p_btnCloseChannel );

        // create irc connection button
        _p_btnConnectIRC = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", "_chatctrl_irc_" ) );
        _p_btnConnectIRC->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::onClickedConnectIRC, this ) );
        _p_btnConnectIRC->setMetricsMode( CEGUI::Absolute );
        // actual size and position are calculated in resize callback
        _p_btnConnectIRC->setSize( CEGUI::Size( 100.0f, 20.0f ) );
        _p_btnConnectIRC->setText( "IRC connect" );
        _p_btnConnectIRC->setFont( YAF3D_GUI_FONT8 );
        _p_btnConnectIRC->show();
        _p_frame->addChildWindow( _p_btnConnectIRC );

        _boxFrameSize = osg::Vec2f( _p_frame->getSize().d_width, _p_frame->getSize().d_height );
        _frameAlphaValue = _p_frame->getAlpha();
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** error setting up chat box gui" << std::endl;
        log << "   reason: " << e.getMessage().c_str() << std::endl;
        return;
    }

    try
    {
        // setup chat box hide button with ctd specific image set
        _p_btnOpen = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", CHATLAYOUT_PREFIX "_btn_openbox_" ) );
        _p_btnOpen->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::onClickedOpen, this ) );
        _p_btnOpen->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::onHoverOpen, this ) );
        _p_btnOpen->setStandardImageryEnabled( false );
        _p_btnOpen->setPosition( CEGUI::Point( 0.0f, 0.7f ) );
        _p_btnOpen->setSize( CEGUI::Size( 0.08f, 0.1f ) );
        p_wnd->addChildWindow( _p_btnOpen );

        // load our custom imageset
        CEGUI::Imageset* p_imageSet = NULL;
        if ( CEGUI::ImagesetManager::getSingleton().isImagesetPresent( VRC_IMAGE_SET ) )
        {
            p_imageSet = CEGUI::ImagesetManager::getSingleton().getImageset( VRC_IMAGE_SET );
        }
        else
        {
            p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( VRC_IMAGE_SET_FILE );
        }

        // set editbox open button images
        const CEGUI::Image* p_image = &p_imageSet->getImage( "HandNormal" );
        CEGUI::RenderableImage* p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setPushedImage( p_rendImage );
        _p_btnOpen->setNormalImage( p_rendImage );
        delete p_rendImage;

        p_image = &p_imageSet->getImage( "HandHoover" );
        p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setHoverImage( p_rendImage );
        delete p_rendImage;

        _p_btnMsgArrived = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", prefix + "btn_post" ) );
        _p_btnMsgArrived->setPosition( _p_btnOpen->getPosition() );
        _p_btnMsgArrived->setAlpha( 0.9f );
        _p_btnMsgArrived->setAlwaysOnTop( true );
        _p_btnMsgArrived->setPosition( CEGUI::Relative, CEGUI::Point( 0.01f, 0.7f ) );
        _p_btnMsgArrived->setSize( CEGUI::Size( 0.03f, 0.03f ) );
        _p_btnMsgArrived->setBackgroundEnabled( false );
        _p_btnMsgArrived->setFrameEnabled( false );
        p_wnd->addChildWindow( _p_btnMsgArrived );
        p_image = &p_imageSet->getImage( "Post" );
        _p_btnMsgArrived->setImage( p_image );
        _p_btnMsgArrived->hide();

        // create tab control
        _p_tabCtrl = static_cast< CEGUI::TabControl* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/TabControl", "_chatbox_channeltab_" ) );
        _p_tabCtrl->setMetricsMode( CEGUI::Absolute );
        _p_tabCtrl->setPosition( CEGUI::Point( GUI_TABCTRL_OFFSETX, GUI_TABCTRL_OFFSETY ) );
        // some initial size, the actual one are calculated in resize callback
        _p_tabCtrl->setSize( CEGUI::Size( 200.0f, 200.0f ) );
        _p_tabCtrl->setTabHeight( GUI_TABCTRL_TAB_HEIGHT );
        _p_tabCtrl->subscribeEvent( CEGUI::Window::EventParentSized, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::onSizeChanged, this ) );
        _p_tabCtrl->subscribeEvent( CEGUI::TabControl::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::ChatGuiBox::onChannelTabChanged, this ) );

        _p_frame->addChildWindow( _p_tabCtrl );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** error setting up chat box frame" << std::endl;
        log << "   reason: " << e.getMessage().c_str() << std::endl;
        return;
    }

    // setup sounds
    osg::ref_ptr< osgAL::SoundState > snd;
    
    snd = gameutils::GuiUtils::get()->getSound( SND_NAME_CLOSE_CHANNEL );
    if ( !snd.valid() )
        gameutils::GuiUtils::get()->createSound( SND_NAME_CLOSE_CHANNEL, SND_CLOSE_CHANNEL, SND_VOL_CLOSE_CHANNEL );

    snd = gameutils::GuiUtils::get()->getSound( SND_NAME_CONNECT );
    if ( !snd.valid() )
        gameutils::GuiUtils::get()->createSound( SND_NAME_CONNECT, SND_CONNECT, SND_VOL_CONNECT );

    snd = gameutils::GuiUtils::get()->getSound( SND_NAME_TYPING );
    if ( !snd.valid() )
        gameutils::GuiUtils::get()->createSound( SND_NAME_TYPING, SND_TYPING, SND_VOL_TYPING );
}

void ChatGuiBox::showMsgArrived( bool show )
{
    if ( show )
        _p_btnMsgArrived->show();
    else
        _p_btnMsgArrived->hide();
}

ChatGuiBox::ChannelTabPane* ChatGuiBox::getTabPane( const ChatConnectionConfig& cfg )
{
    ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( p_beg->first == cfg )
            return p_beg->second;

    return NULL;
}

void ChatGuiBox::destroyChannelPane( const ChatConnectionConfig& cfg )
{
    // close all tab panes
    ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( cfg._channel == p_beg->first._channel )
            break;

    // tollerate when a pane cannot be found
    if ( p_beg != p_end )
    {
        // deregister callback first
        p_beg->second->getConfiguration()._p_protocolHandler->deregisterProtocolCallback( p_beg->second );
        delete p_beg->second;
        _tabpanes.erase( p_beg );
    }
}

ChatGuiBox::ChannelTabPane* ChatGuiBox::getOrCreateChannelPane( const ChatConnectionConfig& cfg )
{
    // check if already have this pane
    ChannelTabPane* p_pane = getTabPane( cfg );
    if ( p_pane )
        return p_pane;

    // create new tab pane
    p_pane = new ChannelTabPane( _p_tabCtrl, this );
    p_pane->setTitle( cfg._protocol + " " + cfg._channel );

    // append the new channel to list
    _tabpanes.push_back( std::make_pair( cfg, p_pane ) );

    // register callback
    p_pane->setConfiguration( cfg );
    cfg._p_protocolHandler->registerProtocolCallback( p_pane, cfg._channel );

    // set tab selection to new created or exsiting pane
    p_pane->setSelection();

    return p_pane;
}

void ChatGuiBox::setEditBoxFocus( bool en )
{
    if ( !_p_tabCtrl )
        return;

    // search active pane and set selection to its edit box field
    if ( ( _p_tabCtrl->getTabCount() > 0 ) && ( _tabpanes.size() > 0 ) )
    {
        TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( p_beg->second->isSelected() )
            {
                // set focus to edit field of active pane
                p_beg->second->setEditBoxFocus( en );
                break;
            }
        }
    }
}

void ChatGuiBox::update( float deltaTime )
{
    switch( _boxState )
    {
        case BoxVisible:
            break;

        case BoxHidden:
            break;

        case BoxFadeIn:
        {
            _p_frame->show();
            if ( _fadeTimer > FADE_TIME )
            {
                _fadeTimer = 0;
                // restore the initial size
                CEGUI::Size size( _boxFrameSize.x(), _boxFrameSize.y() ); 
                _p_frame->setAlpha( _frameAlphaValue );
                _p_frame->setSize( size );
                _p_btnOpen->hide();
                setEditBoxFocus( true );
                _boxState = BoxVisible;
                break;
            }
            _fadeTimer += deltaTime;
            // fade in the box
            float fadefac = _fadeTimer / FADE_TIME;
            CEGUI::Size size( _boxFrameSize.x() * fadefac, _boxFrameSize.y() * fadefac ); 
            _p_frame->setSize( size );
            _p_frame->setAlpha( fadefac * _frameAlphaValue );
            _p_btnOpen->setAlpha( std::max( 0.0f, ( 1.0f - ( _fadeTimer / FADE_TIME ) ) * _frameAlphaValue ) );
        }
        break;

        case BoxFadeOut:
        {
            _p_btnOpen->show();
            if ( _fadeTimer > FADE_TIME )
            {
                _fadeTimer = 0;
                // set size to zero
                CEGUI::Size size( 0, 0 ); 
                _p_frame->setSize( size );
                _p_frame->hide();
                setEditBoxFocus( false );
                _boxState = BoxHidden;
                break;
            }
            _fadeTimer += deltaTime;
            // fade in the box
            float fadefac = std::max( 0.0f, 1.0f - ( _fadeTimer / FADE_TIME ) );
            CEGUI::Size size( _boxFrameSize.x() * fadefac, _boxFrameSize.y() * fadefac ); 
            _p_frame->setSize( size );
            _p_frame->setAlpha( fadefac * _frameAlphaValue );
            _p_btnOpen->setAlpha( ( _fadeTimer / FADE_TIME ) * _frameAlphaValue );
        }
        break;

        default:
            assert( NULL && "invalid chat gui state!" );

    }

    // check the tab pane removal queue
    while ( _queueRemoveTabPane.size() )
    { 
        ChannelTabPane* p_pane = _queueRemoveTabPane.front();
        destroyChannelPane( p_pane->getConfiguration() );
        _queueRemoveTabPane.pop();

        // if all tab panes ( except VRC ) are closed then close also all open connections to chat servers
        // we should work out a better control for disconnecting from chat servers!
        if ( ( _tabpanes.size() == 0 ) || ( ( _tabpanes.size() == 1 ) && ( _tabpanes[ 0 ].first._protocol == VRC_PROTOCOL_NAME ) ) )
            _p_chatMgr->closeConnections();
    }
}

void ChatGuiBox::show( bool visible )
{
    if ( visible )
    {
        _p_btnOpen->show();
        if ( _boxState != BoxHidden )
            _p_frame->show();
    }
    else
    {
        _p_btnOpen->hide();
        _p_frame->hide();
    }
}

bool ChatGuiBox::onCloseFrame( const CEGUI::EventArgs& arg )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    fadeChatbox( true );
    return true;
}

bool ChatGuiBox::onClickedCloseChannelPane( const CEGUI::EventArgs& arg )
{
    // play sound
    gameutils::GuiUtils::get()->playSound( SND_NAME_CLOSE_CHANNEL );

    // search for currently active pane
    ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( p_beg->second->isSelected() )
            break;

    // is any pane active?
    if ( p_beg == p_end )
        return true;

    // we don't close VRC pane!
    if ( p_beg->first._protocol == VRC_PROTOCOL_NAME )
        return true;

    // trigger leaving the channel in the same way as the user would do via command
    p_beg->first._p_protocolHandler->send( "/part", p_beg->first._channel );

    return true;
}

// this is called when clicking on connection button
bool ChatGuiBox::onClickedConnectIRC( const CEGUI::EventArgs& arg )
{
    // play sound
    gameutils::GuiUtils::get()->playSound( SND_NAME_CONNECT );

    // check if we are already connecting
    if ( _connectionState == Connecting )
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", "Already trying to connect to a chat server.", yaf3d::MessageBoxDialog::OK, true );
        p_msg->show();                
        return true;
    }

    _connectionState = Connecting;

    if ( !_p_connectionDialog )
        _p_connectionDialog = new ConnectionDialog< ChatGuiBox >( this );

    // set IRC as protocol
    ChatConnectionConfig& conf = _p_connectionDialog->getConfiguration();
    conf._protocol = "IRC";
    //! TODO: read in these settings from a history file
    conf._port     = 6667;
    conf._serverURL = "localhost";
    conf._channel  = "#vrc";
    conf._nickname = "boto";

    _p_connectionDialog->setConfiguration( conf );

    // show up the dialog
    _p_connectionDialog->setTitle( "IRC Connection yaf3d::Settings" );
    _p_connectionDialog->show( true );

    return true;
}

// this is called by connection dialog instance
void ChatGuiBox::onConnectionDialogClickedConnect( const ChatConnectionConfig& conf )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    _p_connectionDialog->show( false );

    // try to connect
    try
    {
        _p_chatMgr->createConnection( conf );
    }
    catch( const ChatExpection& e )
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Connection error", "Could not connect to server.\n" + e.what(), yaf3d::MessageBoxDialog::OK, true );
        p_msg->show();                

        log_error << "exception occured trying to connect to a chat server" << std::endl;
        log << "   reason: " << e.what() << std::endl;
    }
        
    _connectionState = ConnectionIdle;
}

void ChatGuiBox::onConnectionDialogClickedCancel()
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    // set connection state
    _connectionState = ConnectionIdle;
    _p_connectionDialog->show( false );
}

bool ChatGuiBox::onSizeChanged( const CEGUI::EventArgs& arg )
{
    CEGUI::Size size = _p_frame->getSize( CEGUI::Absolute );

    _p_tabCtrl->setSize( CEGUI::Size( size.d_width - 2.0f * GUI_TABCTRL_OFFSETX, size.d_height - GUI_TABCTRL_SIZEY - GUI_TABCTRL_OFFSETY ) );
    // it's curious, we have to set the tab height here, otherwise the height is changed ( although we have absolute metric mode )
    _p_tabCtrl->setTabHeight( GUI_TABCTRL_TAB_HEIGHT );

    _p_btnCloseChannel->setPosition( CEGUI::Point( 5.0f * GUI_PANE_SPACING, size.d_height - 1.5f * GUI_PANE_SPACING - GUI_CLOSE_BTN_HEIGHT ) );
    _p_btnCloseChannel->setSize( CEGUI::Size( GUI_CLOSE_BTN_WIDTH, GUI_CLOSE_BTN_HEIGHT ) );

    _p_btnConnectIRC->setPosition( CEGUI::Point( size.d_width - GUI_IRCCONNECT_BTN_WIDTH - 5.0f * GUI_PANE_SPACING, size.d_height - 1.5f * GUI_PANE_SPACING - GUI_IRCCONNECT_BTN_HEIGHT ) );
    _p_btnConnectIRC->setSize( CEGUI::Size( GUI_IRCCONNECT_BTN_WIDTH, GUI_IRCCONNECT_BTN_HEIGHT ) );

    return true;
}

bool ChatGuiBox::onChannelTabChanged( const CEGUI::EventArgs& arg )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );    
    return true;
}

bool ChatGuiBox::onHoverOpen( const CEGUI::EventArgs& arg )
{
    // play mouse over sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_HOVER );    
    return true;
}

bool ChatGuiBox::onClickedOpen( const CEGUI::EventArgs& arg )
{
    // are we already in fading action?
    if ( ( _boxState == BoxFadeIn ) || ( _boxState == BoxFadeOut ) )
        return true;

    // play click sound
    gameutils::GuiUtils::get()->playSound( SND_NAME_CLOSE_CHANNEL );

    fadeChatbox( false );

    showMsgArrived( false );

    return true;
}

void ChatGuiBox::fadeChatbox( bool fadeout )
{
    if ( ( fadeout && ( _boxState == BoxHidden ) ) ||
         ( !fadeout && ( _boxState == BoxVisible ) ) )
        return;

    if ( fadeout )
    {
        // store the current size for later fade-out
        _boxFrameSize = osg::Vec2f( _p_frame->getSize().d_width, _p_frame->getSize().d_height );
        _boxState = BoxFadeOut;
    }
    else
    {
        _boxState = BoxFadeIn;
        _p_btnMsgArrived->hide();
    }
}

void ChatGuiBox::setupChatIO( const ChatConnectionConfig& config )
{
    ChannelTabPane* p_pane = getOrCreateChannelPane( config );
    assert( p_pane && "could not create new pane for chat io!" );
}

void ChatGuiBox::outputText( const std::string& channel, const std::string& msg )
{
    // output the system message to currently selected pane
    ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( p_beg->second->isSelected() )
        {
            p_beg->second->addMessage( msg, "* " );
            break;
        }
    }
}

} // namespace vrc
