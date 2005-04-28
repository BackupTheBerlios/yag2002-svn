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
 # entity game menu
 #
 #   date of creation:  04/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_menu.h"
#include "ctd_dialogsettings.h"
#include "ctd_dialoglevelselect.h"
#include "ctd_intro.h"
#include "../sound/ctd_ambientsound.h"

using namespace std;

namespace CTD
{

// prefix for menu layout resources
#define MENU_PREFIX     "menu_"

//! Input handler class for menu, it handles ESC key press and toggles the menu gui
class MenuInputHandler : public GenericInputHandler< EnMenu >
{
    public:

                                            MenuInputHandler( EnMenu* p_menu ) :
                                             _p_menu( p_menu ),
                                             _toggleMenu( true ),
                                             _lockEsc( false )
                                            {}
                                            
        virtual                             ~MenuInputHandler() {}

        //! Handle input events
        bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
                                            {
                                                unsigned int eventType   = ea.getEventType();
                                                int          key         = ea.getKey();
                                                unsigned int buttonMask  = ea.getButtonMask();

                                                // during intro we abort it when mouse clicked
                                                if ( buttonMask == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
                                                {
                                                    if ( _p_menu->_menuState == EnMenu::Intro )
                                                    {
                                                        _p_menu->stopIntro();
                                                        _p_menu->enter();
                                                        _toggleMenu = !_toggleMenu;
                                                    }
                                                }

                                                if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
                                                {
                                                    if ( !_lockEsc && ( key == osgGA::GUIEventAdapter::KEY_Escape ) )
                                                    {
                                                        _lockEsc = true;

                                                        // are we in intro, if so then abort it
                                                        if ( _p_menu->_menuState == EnMenu::Intro )
                                                            _p_menu->stopIntro();

                                                        if ( _toggleMenu )
                                                           _p_menu->enter();
                                                        else
                                                            _p_menu->leave();
                                                    
                                                        _toggleMenu = !_toggleMenu;
                                                    }
                                                }
                                                else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
                                                {
                                                    if ( key == osgGA::GUIEventAdapter::KEY_Escape )
                                                        _lockEsc = false;
                                                }

                                                return false;
                                            }

    protected:

        EnMenu*                             _p_menu;

        bool                                _toggleMenu;

        bool                                _lockEsc;
};

//! Implement and register the menu entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( MenuEntityFactory );

EnMenu::EnMenu() :
_p_menuWindow( NULL ),
_menuConfig( "gui/menu.xml" ),
_settingsDialogConfig( "gui/settings.xml" ),
_levelSelectDialogConfig( "gui/levelselect.xml" ),
_introTexture( "gui/intro.tga" ),
_buttonClickSound( "gui/sounds/click.wav" ),
_buttonHoverSound( "gui/sounds/hover.wav" ),
_introductionSound( "gui/sounds/intro.wav" ),
_beginHover( false ),
_menuState( None )
{
    // make this entity persistent ( thus on level changing this entity will not be deleted by entity manager )
    setAutoDelete( false );
    // this entity needs updates initially for getting the intro running
    EntityManager::get()->registerUpdate( this, true );

    // we register outself to get notifications. interesting one is the shutdown notification
    //  as we have to trigger the destruction outself -- because of setAutoDelete( false )
    EntityManager::get()->registerNotification( this, true );

    // register entity attributes
    _attributeManager.addAttribute( "menuConfig"              , _menuConfig                 );
    _attributeManager.addAttribute( "settingsDialogConfig"    , _settingsDialogConfig       );
    _attributeManager.addAttribute( "levelSelectDialogConfig" , _levelSelectDialogConfig    );
    _attributeManager.addAttribute( "intoTexture"             , _introTexture               );
    _attributeManager.addAttribute( "buttonClickSound"        , _buttonClickSound           );
    _attributeManager.addAttribute( "buttonHoverSound"        , _buttonHoverSound           );
    _attributeManager.addAttribute( "introductionSound"       , _introductionSound          );
}

EnMenu::~EnMenu()
{
    if ( _p_menuWindow )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_menuWindow );

    _p_inputHandler->destroyHandler();

    // deregister from getting notification
    EntityManager::get()->registerNotification( this, false );
}

void EnMenu::handleNotification( EntityNotification& notify )
{
    // handle notifications
    switch( notify.getId() )
    {
        // we have to trigger the deletion ourself! ( we disabled auto-deletion for this entity )
        case CTD_NOTIFY_SHUTDOWN:
            EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnMenu::initialize()
{
    // setup sounds
    if ( _buttonClickSound.length() )
        _clickSound.reset( setupSound( _buttonClickSound ) );

    if ( _buttonHoverSound.length() )
        _hoverSound.reset( setupSound( _buttonHoverSound ) );

    if ( _introductionSound.length() )
        _introSound.reset( setupSound( _introductionSound ) );

    // load the main menu layout
    try
    {
        _p_menuWindow = GuiManager::get()->loadLayout( _menuConfig, NULL, MENU_PREFIX );
        _p_menuWindow->hide();

        // set button callbacks
        CEGUI::PushButton* p_btnGS = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_game_settings" ) );
        p_btnGS->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedGameSettings, this ) );
        p_btnGS->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );
        p_btnGS->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonLostHover, this ) );

        CEGUI::PushButton* p_btnquit = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_quit" ) );
        p_btnquit->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedQuit, this ) );
        p_btnquit->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );
        p_btnquit->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonLostHover, this ) );

        CEGUI::PushButton* p_btnstart = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_start" ) );
        p_btnstart->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedStart, this ) );
        p_btnstart->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );
        p_btnstart->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonLostHover, this ) );
    }
    catch ( CEGUI::Exception e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** Menu: cannot find layout: " << _menuConfig << endl;
        log << "      reason: " << e.getMessage().c_str() << endl;
        return;
    }

    // setup dialog for editing game settings
    _settingsDialog = auto_ptr< DialogGameSettings >( new DialogGameSettings );
    if ( !_settingsDialog->initialize( _settingsDialogConfig, _p_menuWindow ) )
        return;
    // set the click sound object
    _settingsDialog->setClickSound( _clickSound.get() );

    // setup dialog for selecting a level
    _levelSelectDialog = auto_ptr< DialogLevelSelect >( new DialogLevelSelect( this ) );
    if ( !_levelSelectDialog->initialize( _levelSelectDialogConfig, _p_menuWindow ) )
        return;
    // set the click sound object
    _levelSelectDialog->setClickSound( _clickSound.get() );

    // setup intro layout
    _intro = auto_ptr< IntroControl >( new IntroControl );
    if ( !_intro->initialize( _introTexture ) )
        return;
    // set the click and intro sound objects
    _intro->setClickSound( _clickSound.get() );
    _intro->setIntroSound( _introSound.get() );

    // create input handler
    _p_inputHandler = new MenuInputHandler( this );

    // start intro
    beginIntro();
}

EnAmbientSound* EnMenu::setupSound( const std::string& filename )
{
    // manually create an entity of type AmbientSound without adding it to pool as we use the entity locally
    //  and do not need managed destruction or searchable ability for the entity
    EnAmbientSound* p_ent = static_cast< EnAmbientSound* >( EntityManager::get()->createEntity( ENTITY_NAME_AMBIENTSOUND, filename, false ) );
    if ( !p_ent )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** EnMenu: cannot create sound entity of type '" << ENTITY_NAME_AMBIENTSOUND << "'" << endl;
        return NULL;
    }

    // setup entity parameters
    string dir  = extractPath( filename );
    string file = extractFileName( filename );
    p_ent->getAttributeManager().setAttributeValue( "resourceDir", dir      );
    p_ent->getAttributeManager().setAttributeValue( "soundFile",   file     );
    p_ent->getAttributeManager().setAttributeValue( "loop",        false    );
    p_ent->getAttributeManager().setAttributeValue( "autoPlay",    false    );
    
    // init entity
    p_ent->initialize();
    p_ent->postInitialize();

    return p_ent;
}

bool EnMenu::onButtonHover( const CEGUI::EventArgs& arg )
{
    if ( !_beginHover )
    {
        _beginHover = true;
        if ( _hoverSound.get() )
            _hoverSound->startPlaying();
    }
    return true;
}

bool EnMenu::onButtonLostHover( const CEGUI::EventArgs& arg )
{
    _beginHover = false;
    return true;
}

bool EnMenu::onClickedGameSettings( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    _settingsDialog->show( true );

    return true;
}

bool EnMenu::onClickedQuit( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    Application::get()->stop();
    return true;
}

bool EnMenu::onClickedStart( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    _levelSelectDialog->show( true );
    return true;
}

void EnMenu::updateEntity( float deltaTime )
{
    switch ( _menuState )
    {
        case None:
            break;

        case BeginIntro:
        {     
            _intro->start();
            _menuState = Intro;
        }
        break;

        case Intro:
        {            
            _intro->update( deltaTime );
        }
        break;

        case Hidden:

            break;

        case Visible:

            _settingsDialog->update( deltaTime );
            break;

        default:
            assert( NULL && "invalid menu state!" );

    }
}

void EnMenu::beginIntro()
{
    _p_menuWindow->hide();
    _settingsDialog->show( false );
    _levelSelectDialog->show( false );
    _intro->start();
    _menuState = Intro;
}

void EnMenu::stopIntro()
{
    _intro->stop();
    _menuState = Visible;
}

void EnMenu::enter()
{
    // send notification to all notification-registered entities about entering menu
    EntityManager::get()->sendNotification( EntityNotification( CTD_NOTIFY_MENU_ENTER, this ) );

    _p_menuWindow->show();

    _menuState = Visible;

    // register for updating ( we need it so long we are in menu )
    EntityManager::get()->registerUpdate( this, true );
}

void EnMenu::leave()
{
    // send notification to all notification-registered entities about leaving menu
    EntityManager::get()->sendNotification( EntityNotification( CTD_NOTIFY_MENU_LEAVE, this ) );

    _p_menuWindow->hide();
    _settingsDialog->show( false );
    _levelSelectDialog->show( false );

    _menuState = Hidden;

    // de-register updating ( this entity needs not updates when we are not in menu )
    EntityManager::get()->registerUpdate( this, false );
}

} // namespace CTD
