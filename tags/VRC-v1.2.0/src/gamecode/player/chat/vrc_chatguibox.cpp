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
#include "vrc_chatguiboxtab.h"
#include "../../sound/vrc_ambientsound.h"

namespace vrc
{
// layout prefix
#define CHATLAYOUT_PREFIX           "chatbox_"
#define FADE_TIME                   1.0f

// delay in seconds for editbox activation
#define EDITBOX_ACTIVATION_DELAY    0.7f

// delay in seconds for deleting the short messages when chatbox is closed
#define SHORTMESSAGE_DELETION_DELAY 20.0f

// some layout related constants
#define GUI_TABCTRL_OFFSETX         25.0f
#define GUI_TABCTRL_OFFSETY         35.0f
#define GUI_TABCTRL_SIZEY           35.0f
#define GUI_TABCTRL_TAB_HEIGHT      25.0f

#define GUI_CLOSE_BTN_WIDTH         120.0f
#define GUI_CLOSE_BTN_HEIGHT        20.0f

#define GUI_IRCCONNECT_BTN_WIDTH    110.0f
#define GUI_IRCCONNECT_BTN_HEIGHT   20.0f


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
_shortMsgBox( NULL ),
_shortMsgBoxAcceccCounter( SHORTMESSAGE_DELETION_DELAY ),
_editBoxActivationCounter( 0.0f ),
_editBoxActivationTab( NULL ),
_fadeTimer( 0.0f ),
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

        if ( _shortMsgBox )
            CEGUI::WindowManager::getSingleton().destroyWindow( _shortMsgBox );
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
        _p_btnCloseChannel->setSize( CEGUI::Size( GUI_CLOSE_BTN_WIDTH, GUI_CLOSE_BTN_HEIGHT ) );
        _p_btnCloseChannel->setText( "close channel" );
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

        // set editbox open button images
        const CEGUI::Image* p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_HAND_NORMAL );
        CEGUI::RenderableImage* p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setPushedImage( p_rendImage );
        _p_btnOpen->setNormalImage( p_rendImage );
        delete p_rendImage;

        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_HAND_HOOVER );
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

        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_POST );
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

        // create a message area for posting messages when the box is closed
        CEGUI::Window* p_rootwnd = yaf3d::GuiManager::get()->getRootWindow();
        _shortMsgBox = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", "_chatbox_msgbox_" ) );
        _shortMsgBox->setPosition( CEGUI::Point( 0.0f, 0.9f ) );
        _shortMsgBox->setSize( CEGUI::Size( 0.9f, 0.1f ) );
        _shortMsgBox->setFont( YAF3D_GUI_FONT8 );
        _shortMsgBox->setBackgroundEnabled( false );
        _shortMsgBox->setFrameEnabled( false );
        _shortMsgBox->show();
        p_rootwnd->addChildWindow( _shortMsgBox );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** error setting up chat box frame" << std::endl;
        log << "   reason: " << e.getMessage().c_str() << std::endl;
        return;
    }

    // setup sounds
    unsigned int sndID = 0;

    sndID = gameutils::GuiUtils::get()->getSoundID( SND_NAME_CLOSE_CHANNEL );
    if ( sndID == 0 )
        gameutils::GuiUtils::get()->createSound( SND_NAME_CLOSE_CHANNEL, SND_CLOSE_CHANNEL, SND_VOL_CLOSE_CHANNEL );

    sndID = gameutils::GuiUtils::get()->getSoundID( SND_NAME_CONNECT );
    if ( sndID == 0 )
        gameutils::GuiUtils::get()->createSound( SND_NAME_CONNECT, SND_CONNECT, SND_VOL_CONNECT );

    sndID = gameutils::GuiUtils::get()->getSoundID( SND_NAME_TYPING );
    if ( sndID == 0 )
        gameutils::GuiUtils::get()->createSound( SND_NAME_TYPING, SND_TYPING, SND_VOL_TYPING );
}

void ChatGuiBox::showMsgArrived( bool show )
{
    if ( show )
        _p_btnMsgArrived->show();
    else
        _p_btnMsgArrived->hide();
}

ChannelTabPane* ChatGuiBox::getTabPane( const ChatConnectionConfig& cfg )
{
    ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( p_beg->first == cfg )
            return p_beg->second;

    return NULL;
}

CEGUI::StaticText* ChatGuiBox::getShortMsgBox()
{
    _shortMsgBoxAcceccCounter = SHORTMESSAGE_DELETION_DELAY;
    return _shortMsgBox;
}

void ChatGuiBox::destroyChannelPane( const ChatConnectionConfig& cfg )
{
    // find tab panes
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

ChannelTabPane* ChatGuiBox::getOrCreateChannelPane( const ChatConnectionConfig& cfg, bool isSystemIO )
{
    // check if already have this pane
    ChannelTabPane* p_pane = getTabPane( cfg );
    if ( p_pane )
        return p_pane;

    // create new tab pane
    p_pane = new ChannelTabPane( _p_tabCtrl, this, isSystemIO );
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

    // if the chat box is not open so do not activate it
    if ( en && ( _boxState == BoxHidden ) )
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
                if ( !en )
                {
                    p_beg->second->setEditBoxFocus( false );
                    _editBoxActivationCounter = -1.0f;
                }
                else
                {
                    // activation of edit box is delayed a little bit ( because of continuing of walk state, avoid inappropriate key press sensing )
                    _editBoxActivationTab     = p_beg->second;
                    _editBoxActivationCounter = EDITBOX_ACTIVATION_DELAY;
                }
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
                // let the short message box disappear
                _shortMsgBox->hide();
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
                // let the short message box appear
                _shortMsgBox->show();
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

    // check for edit box activation counter
    if ( _editBoxActivationCounter > 0.0f )
    {
        _editBoxActivationCounter -= deltaTime;
        if ( _editBoxActivationCounter < 0.0 )
        {
            assert( _editBoxActivationTab && "tab is not set, set it together with tab activation counter!" );
            _editBoxActivationTab->setEditBoxFocus( true );
        }
    }

    // check for short message box time out and delete the text when time out occured
    if ( _shortMsgBoxAcceccCounter > 0.0f )
    {
        _shortMsgBoxAcceccCounter -= deltaTime;
        if ( _shortMsgBoxAcceccCounter < 0.0 )
        {
            _shortMsgBox->setText( "" );
        }
    }

}

void ChatGuiBox::removeTabPane( ChannelTabPane* p_pane )
{
    _queueRemoveTabPane.push( p_pane );
}

void ChatGuiBox::show( bool visible )
{
    if ( visible )
    {
        _p_btnOpen->show();
        if ( _boxState != BoxHidden )
        {
            _p_frame->show();
            _shortMsgBox->hide();
        }
        else if ( _boxState == BoxHidden )
        {
            _shortMsgBox->show();
        }
    }
    else
    {
        _p_btnOpen->hide();
        _p_frame->hide();
        _shortMsgBox->hide();
    }
}

bool ChatGuiBox::onCloseFrame( const CEGUI::EventArgs& /*arg*/ )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    fadeChatbox( true );
    return true;
}

bool ChatGuiBox::onClickedCloseChannelPane( const CEGUI::EventArgs& /*arg*/ )
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

    // check if a server connection pane is to be closed
    // server connection panes have channel and server url both set to server url
    ChatConnectionConfig conf = p_beg->second->getConfiguration();
    if ( conf._channel == conf._serverURL )
    {
        // remove all channel panes related to that server
        ChatGuiBox::TabPanePairList::iterator p_chanbeg = _tabpanes.begin(), p_chanend = _tabpanes.end();
        for ( ; p_chanbeg != p_chanend; ++p_chanbeg )
        {
            if ( p_chanbeg->second->getConfiguration()._serverURL == conf._serverURL )
                removeTabPane( p_chanbeg->second );
        }

        // now close the connection to server
        p_beg->first._p_protocolHandler->destroyConnection();
    }
    else
    {
        // trigger leaving the channel in the same way as the user would do via command
        p_beg->first._p_protocolHandler->send( "/part", p_beg->first._channel );
    }

    return true;
}

// this is called when clicking on connection button
bool ChatGuiBox::onClickedConnectIRC( const CEGUI::EventArgs& /*arg*/ )
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

    // take the player name as nickname
    std::string nickname;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, nickname );

    // set IRC as protocol
    ChatConnectionConfig& conf = _p_connectionDialog->getConfiguration();
    conf._protocol  = "IRC";
    //! TODO: read in these settings from a history file
    conf._port      = 6667;
    conf._serverURL = "irc.freenode.net";
    conf._channel   = "#vrc";
    conf._nickname  = nickname;

    _p_connectionDialog->setConfiguration( conf );

    // show up the dialog
    _p_connectionDialog->setTitle( "IRC Connection Settings" );
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
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Connection error", std::string( "Could not connect to server.\n" ) + e.what(), yaf3d::MessageBoxDialog::OK, true );
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

bool ChatGuiBox::onSizeChanged( const CEGUI::EventArgs& /*arg*/ )
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

bool ChatGuiBox::onChannelTabChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );
    return true;
}

bool ChatGuiBox::onHoverOpen( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse over sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_HOVER );
    return true;
}

bool ChatGuiBox::onClickedOpen( const CEGUI::EventArgs& /*arg*/ )
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

void ChatGuiBox::setupChatIO( const ChatConnectionConfig& config, bool systemIO )
{
    ChannelTabPane* p_pane = getOrCreateChannelPane( config, systemIO );
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
            p_beg->second->addMessage( channel + ": " + msg, "* " );
            break;
        }
    }
}

} // namespace vrc
