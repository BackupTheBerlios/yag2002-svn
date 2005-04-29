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
#define MENU_PREFIX             "menu_"
#define OVERLAY_IMAGESET        MENU_PREFIX "loadingoverlay"
#define DEFAULT_LEVEL_LOADER    "scenes/loader"

//! Input handler class for menu, it handles ESC key press and toggles the menu gui
class MenuInputHandler : public GenericInputHandler< EnMenu >
{
    public:

                                            MenuInputHandler( EnMenu* p_menu ) :
                                             _p_menu( p_menu ),
                                             _menuActive( true ),
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
                                                        return false;
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

                                                        if ( _menuActive )
                                                        {
                                                           _p_menu->enter();
                                                        }
                                                        else
                                                        {
                                                            _p_menu->leave();
                                                        }
                                                    }
                                                }
                                                else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
                                                {
                                                    if ( key == osgGA::GUIEventAdapter::KEY_Escape )
                                                        _lockEsc = false;
                                                }

                                                return false;
                                            }

        // reset the input handler to activate or deactivate the menu on next esc key press
        void                                reset( bool active ) { _lockEsc = false; _menuActive = active; }

    protected:

        EnMenu*                             _p_menu;

        bool                                _menuActive;

        bool                                _lockEsc;
};

//! Implement and register the menu entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( MenuEntityFactory );

EnMenu::EnMenu() :
_p_menuWindow( NULL ),
_p_btnStart( NULL ),
_p_btnQuit( NULL ),
_p_loadingWindow( NULL ),
_p_loadingLevelPic( NULL ),
_p_loadingOverlayImage( NULL ),
_p_loadingOverly( NULL ),
_menuConfig( "gui/menu.xml" ),
_settingsDialogConfig( "gui/settings.xml" ),
_levelSelectDialogConfig( "gui/levelselect.xml" ),
_introTexture( "gui/intro.tga" ),
_loadingOverlayTexture( "gui/loading.tga" ),
_buttonClickSound( "gui/sounds/click.wav" ),
_buttonHoverSound( "gui/sounds/hover.wav" ),
_introductionSound( "gui/sounds/intro.wav" ),
_beginHover( false ),
_menuState( None ),
_levelLoaded( false )
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
    _attributeManager.addAttribute( "loadingOverlayTexture"   , _loadingOverlayTexture    );
    _attributeManager.addAttribute( "buttonClickSound"        , _buttonClickSound           );
    _attributeManager.addAttribute( "buttonHoverSound"        , _buttonHoverSound           );
    _attributeManager.addAttribute( "introductionSound"       , _introductionSound          );
}

EnMenu::~EnMenu()
{
    if ( _p_menuWindow )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_menuWindow );

    if ( _p_loadingWindow )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_loadingWindow );

    if ( _p_loadingOverly )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_loadingOverly );

    if ( _p_loadingLevelPic )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_loadingLevelPic );

    CEGUI::ImagesetManager::getSingleton().destroyImageset( OVERLAY_IMAGESET );

    // destroy the input handler
    _p_inputHandler->destroyHandler();

    // deregister from getting notification
    EntityManager::get()->registerNotification( this, false );
}

void EnMenu::handleNotification( EntityNotification& notify )
{
    // handle notifications
    switch( notify.getId() )
    {
        // for every subsequent level loading we must register outself again for getting updating
        case CTD_NOTIFY_NEW_LEVEL_INITIALIZED:
            EntityManager::get()->registerUpdate( this, true );
            break;

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
    // note: this entity is persistent; on every level loading it is initialized again.
    //       we need only the first initialization, so skip the subsequent ones!
    static s_alreadyInitialized = false;
    if ( s_alreadyInitialized )
        return;
    else
        s_alreadyInitialized = true;

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

        _p_btnQuit = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_quit" ) );
        _p_btnQuit->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedQuitReturnToLevel, this ) );
        _p_btnQuit->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );
        _p_btnQuit->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonLostHover, this ) );

        _p_btnStart = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_start" ) );
        _p_btnStart->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedStartLeave, this ) );
        _p_btnStart->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );
        _p_btnStart->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonLostHover, this ) );

        // setup loading window
        _p_loadingWindow = static_cast< CEGUI::Window* >( CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", ( MENU_PREFIX "wnd_loading" ) ) );
        _p_loadingWindow->hide();
        GuiManager::get()->getRootWindow()->addChildWindow( _p_loadingWindow );
        // ------------------
        // we support a level preview pic and an overlay ( e.g. with text "level loading" )
        _p_loadingOverly = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", ( MENU_PREFIX "img_loadingoverlay" ) ) );
        _p_loadingOverly->setSize( CEGUI::Size( 1.0f, 1.0f ) );
        _p_loadingOverly->setAlpha( 0.8f );
        _p_loadingOverly->setAlwaysOnTop( true );
        _p_loadingOverly->setBackgroundEnabled( false );
        _p_loadingOverly->setFrameEnabled( false );
        _p_loadingWindow->addChildWindow( _p_loadingOverly );

        _p_loadingLevelPic = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", ( MENU_PREFIX "img_loadingpic" ) ) );
        _p_loadingLevelPic->setSize( CEGUI::Size( 1.0f, 1.0f ) );
        _p_loadingLevelPic->setImage( NULL ); // this image will be delivered by level selector later
        _p_loadingLevelPic->setBackgroundEnabled( false );
        _p_loadingLevelPic->setFrameEnabled( false );
        _p_loadingWindow->addChildWindow( _p_loadingLevelPic );

        // create a new imageset for loading pic
        string materialName( OVERLAY_IMAGESET );
        CEGUI::Texture*  p_texture = GuiManager::get()->getGuiRenderer()->createTexture( _loadingOverlayTexture, "MenuResources" );
        CEGUI::Imageset* p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( materialName, p_texture );        
        if ( !p_imageSet->isImageDefined( _loadingOverlayTexture ) )
        {
            p_imageSet->defineImage( materialName, CEGUI::Point( 0.0f, 0.0f ), CEGUI::Size( p_texture->getWidth(), p_texture->getHeight() ), CEGUI::Point( 0.0f,0.0f ) );
        }
        CEGUI::Image* _p_loadingOverlayImage = &const_cast< CEGUI::Image& >( p_imageSet->getImage( materialName ) );
        _p_loadingOverly->setImage( _p_loadingOverlayImage );
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

bool EnMenu::onClickedQuitReturnToLevel( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    if ( !_levelLoaded )
    {
        Application::get()->stop();
    }
    else
    {
        leave();
    }
    return true;
}

bool EnMenu::onClickedStartLeave( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    if ( !_levelLoaded )
    {
        _levelSelectDialog->show( true );
    }
    else
    {
        // ask user before leaving
        MessageBoxDialog* p_msg = new MessageBoxDialog( "", "You want to leave the level?", MessageBoxDialog::YES_NO, true );

        // create a call back for yes/no buttons of messagebox
        class MsgYesNoClick: public MessageBoxDialog::ClickCallback
        {
        public:

                                    MsgYesNoClick( EnMenu* p_menu ) : _p_menu( p_menu ) {}

            virtual                 ~MsgYesNoClick() {}

            void                    onClicked( unsigned int btnId )
                                    {
                                        // did the user clicked yes? if so then store settings
                                        if ( btnId == MessageBoxDialog::BTN_YES )                                                    
                                            _p_menu->leaveLevel();
                                        else
                                            _p_menu->leave();

                                        _p_menu->_p_menuWindow->enable();
                                    }

            EnMenu*                 _p_menu;
        };
        p_msg->setClickCallback( new MsgYesNoClick( this ) );    
        p_msg->show();
        _p_menuWindow->disable();
    }

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

        // in order to show up the loading pic we have to activate it
        //  and do the actual level loading one step later, as loading a level blocks the main loop :-(
        case BeginLoadingLevel:
        {
            _p_loadingWindow->show();
            // show up the loading window           
            GuiManager::get()->showMousePointer( false ); // let the mouse disappear 
            _menuState = LoadingLevel;
        }
        break;

        case LoadingLevel:
        {
            // load a new level, don't keep physics and entities
            // note: the menu entity is persistent anyway, it handles the level switch itself!
            LevelManager::get()->load( _queuedLevelFile, false, false );
            _queuedLevelFile = ""; // reset the queue

            // hide the loading window
            _p_loadingWindow->hide();

            // set flag that we have loaded a level; some menu oprions depend on this flag
            _levelLoaded = true;

            // leave the menu system
            leave();

            GuiManager::get()->showMousePointer( true ); // let the mouse appear again 
        }
        break;

        case BeginLeavingLevel:
        {
            GuiManager::get()->showMousePointer( false ); // let the mouse disappear 
            _menuState = LeavingLevel;        
        }
        break;

        case LeavingLevel:
        {
            LevelManager::get()->load( _queuedLevelFile, false, false );
            _queuedLevelFile = ""; // reset the queue
            GuiManager::get()->showMousePointer( true ); // let the mouse disappear 
            _levelLoaded = false;
            enter();
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
    _p_loadingWindow->hide();
    _settingsDialog->show( false );
    _levelSelectDialog->show( false );
    _intro->start();    
    _menuState = Intro;
}

void EnMenu::stopIntro()
{
    _intro->stop();
}

void EnMenu::enter()
{
    if ( _menuState == Visible )
        return;

    // send notification to all notification-registered entities about entering menu
    EntityManager::get()->sendNotification( EntityNotification( CTD_NOTIFY_MENU_ENTER, this ) );

    _p_menuWindow->show();

    _menuState = Visible;

    if ( _levelLoaded )
    {
        _p_btnStart->setText( "leave level" );
        _p_btnQuit->setText( "return" );
    }
    else
    {
        _p_btnStart->setText( "start" );
        _p_btnQuit->setText( "quit" );
    }

    // reset the input handler
    _p_inputHandler->reset( true );
}

void EnMenu::leave()
{
    if ( _menuState == Hidden )
        return;

    // send notification to all notification-registered entities about leaving menu
    EntityManager::get()->sendNotification( EntityNotification( CTD_NOTIFY_MENU_LEAVE, this ) );

    _p_menuWindow->hide();
    _settingsDialog->show( false );
    _levelSelectDialog->show( false );

    _menuState = Hidden;

    // reset the input handler
    _p_inputHandler->reset( true );
}

void EnMenu::loadLevel( string levelfile, CEGUI::Image* p_img )
{
    // prepare the level loading; the actual loading is done in update method
    _menuState = BeginLoadingLevel;
    _queuedLevelFile = levelfile;

    _p_loadingLevelPic->setImage( p_img );

    _p_loadingWindow->show();
    _p_menuWindow->hide();
}

void EnMenu::leaveLevel()
{
    // prepare the level loading; the actual loading is done in update method
    _menuState = BeginLeavingLevel;
    _queuedLevelFile = DEFAULT_LEVEL_LOADER;
}

} // namespace CTD
