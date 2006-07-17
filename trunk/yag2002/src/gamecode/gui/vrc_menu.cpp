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
 # entity game menu
 #
 #   date of creation:  04/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_menu.h"
#include "vrc_dialogsettings.h"
#include "vrc_dialoglevelselect.h"
#include "vrc_intro.h"
#include "../sound/vrc_ambientsound.h"
#include "../visuals/vrc_camera.h"
#include "../visuals/vrc_skybox.h"
#include "../visuals/vrc_fog.h"

// used for starting server process
static YAF3D_SPAWN_PROC_ID _serverProcHandle = static_cast< YAF3D_SPAWN_PROC_ID >( 0 );

namespace vrc
{

// prefix for menu layout resources
#define MENU_PREFIX                     "menu_"
#define OVERLAY_IMAGESET                MENU_PREFIX "loadingoverlay"

// some default resources
#define MENU_CAMERAPATH                 "gui/scene/campath.osg"

// menu background sound related parameters
#define BCKRGD_SND_FADEIN_PERIOD        2.0f
#define BCKRGD_SND_FADEOUT_PERIOD       1.5f

//! Input handler class for menu, it handles ESC key press and toggles the menu gui
class MenuInputHandler : public vrc::gameutils::GenericInputHandler< EnMenu >
{
    public:

        explicit                            MenuInputHandler( EnMenu* p_menu ) :
                                             vrc::gameutils::GenericInputHandler< EnMenu >( p_menu ),
                                             _menuActive( true ),
                                             _lockEsc( false )
                                            {}

        virtual                             ~MenuInputHandler() {}

        //! Handle input events
        bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
                                            {
                                                const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
                                                assert( p_eventAdapter && "invalid event adapter received" );

                                                unsigned int eventType   = p_eventAdapter->getEventType();
                                                int          key         = p_eventAdapter->getKey();
                                                unsigned int buttonMask  = p_eventAdapter->getButtonMask();

                                                // check for idling, i.e. no user interaction while in menu
                                                if ( ( eventType != osgGA::GUIEventAdapter::FRAME ) && ( eventType != 0xffff ) )
                                                    getUserObject()->triggerUserInteraction();

                                                // during intro we abort it when mouse clicked
                                                if ( buttonMask == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
                                                {
                                                    if ( getUserObject()->_menuState == EnMenu::Intro )
                                                    {
                                                        getUserObject()->stopIntro();
                                                        getUserObject()->enter();
                                                        return false;
                                                    }
                                                }

                                                if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
                                                {
                                                    if ( !_lockEsc && ( key == SDLK_ESCAPE ) )
                                                    {
                                                        _lockEsc = true;

                                                        // are we in intro, if so then abort it
                                                        if ( getUserObject()->_menuState == EnMenu::Intro )
                                                            getUserObject()->stopIntro();

                                                        if ( _menuActive )
                                                        {
                                                            getUserObject()->enter();
                                                        }
                                                        else
                                                        {
                                                            getUserObject()->leave();
                                                        }
                                                    }
                                                }
                                                else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
                                                {
                                                    if ( key == SDLK_ESCAPE )
                                                        _lockEsc = false;
                                                }

                                                return false;
                                            }

        // reset the input handler to activate or deactivate the menu on next esc key press
        void                                reset( bool active ) { _lockEsc = false; _menuActive = active; }

    protected:

        bool                                _menuActive;

        bool                                _lockEsc;
};

//! Implement and register the menu entity factory
YAF3D_IMPL_ENTITYFACTORY( MenuEntityFactory )

EnMenu::EnMenu() :
_menuConfig( "gui/menu.xml" ),
_settingsDialogConfig( "gui/settings.xml" ),
_levelSelectDialogConfig( "gui/levelselect.xml" ),
_introTexture( "gui/intro.tga" ),
_loadingOverlayTexture( "gui/loading.tga" ),
_introductionSound( "gui/sound/intro.wav" ),
_introductionSoundVolume( 1.0f ),
_backgroundSound( "gui/sound/background.wav" ),
_backgroundSoundVolume( 0.6f ),
_menuCameraPathFile( MENU_CAMERAPATH ),
_menuIdleTimeout( 15.0f ),
_cameraBackgroundColor( osg::Vec3f( 0.0f, 0.0f, 0.0f ) ),
_cameraFov( 45.0f ),
_cameraNearClip( 0.5f ),  
_cameraFarClip( 3000.0f ),
_menuState( None ),
_menuSoundState( SoundStopped ),
_levelSelectionState( ForStandalone ),
_p_cameraControl( NULL ),
_p_introSound( NULL ),
_p_backgrdSound( NULL ),
_backgrdSoundVolume( 1.0f ),
_p_menuWindow( NULL ),
_p_loadingWindow( NULL ),
_p_btnStartJoin( NULL ),
_p_btnStartServer( NULL ),
_p_btnStartWT( NULL ),
_p_btnReturn( NULL ),
_p_btnLeave( NULL ),
_p_loadingOverly( NULL ),
_p_loadingLevelPic( NULL ),
_p_clientLevelFiles( NULL ),
_p_inputHandler( NULL ),
_soundFadingCnt( 0.0f ),
_idleCounter( 0.0f ),
_levelLoaded( false )
{
    // register entity attributes
    getAttributeManager().addAttribute( "menuConfig"                , _menuConfig               );
    getAttributeManager().addAttribute( "settingsDialogConfig"      , _settingsDialogConfig     );
    getAttributeManager().addAttribute( "levelSelectDialogConfig"   , _levelSelectDialogConfig  );
    getAttributeManager().addAttribute( "intoTexture"               , _introTexture             );
    getAttributeManager().addAttribute( "loadingOverlayTexture"     , _loadingOverlayTexture    );
    getAttributeManager().addAttribute( "introductionSound"         , _introductionSound        );
    getAttributeManager().addAttribute( "introductionSoundVolume"   , _introductionSoundVolume  );
    getAttributeManager().addAttribute( "backgroundSound"           , _backgroundSound          );
    getAttributeManager().addAttribute( "backgroundSoundVolume"     , _backgroundSoundVolume    );
    getAttributeManager().addAttribute( "idleTimeout"               , _menuIdleTimeout          );

    getAttributeManager().addAttribute( "cameraPath"                , _menuCameraPathFile       );
    getAttributeManager().addAttribute( "cameraBackgroundColor"     , _cameraBackgroundColor    );
    getAttributeManager().addAttribute( "cameraFov"                 , _cameraFov                );
    getAttributeManager().addAttribute( "cameraNearClip"            , _cameraNearClip           );
    getAttributeManager().addAttribute( "cameraFarClip"             , _cameraFarClip            );
}

EnMenu::~EnMenu()
{
    try
    {
        if ( _p_menuWindow )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_menuWindow );

        if ( _p_loadingWindow )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_loadingWindow );

        if ( _p_loadingOverly )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_loadingOverly );

        CEGUI::ImagesetManager::getSingleton().destroyImageset( OVERLAY_IMAGESET );

        if ( _p_clientLevelFiles )
            delete _p_clientLevelFiles;
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnMenu: problem cleaning up entity." << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    // destroy self-created entities
    // note: these entities are created via yaf3d::EntityManager with "no add to pool" option,
    //       thus we must not use yaf3d::EntityManager's detroyEntity method to destroy them, we have to delete them manually instead!
    if ( _p_introSound )
        delete _p_introSound;

    if ( _p_backgrdSound )
        delete _p_backgrdSound;

    // destroy the input handler
    if ( _p_inputHandler )
        _p_inputHandler->destroyHandler();
}

void EnMenu::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        // for every subsequent level loading we must register outself again for getting updating
        case YAF3D_NOTIFY_NEW_LEVEL_INITIALIZED:
            break;

        // other entities can also request for unloading a level too ( e.g. the console ), handle this here
        case YAF3D_NOTIFY_UNLOAD_LEVEL:
            leaveLevel();
            break;

        // we have to trigger the deletion ourselves! ( we disabled auto-deletion for this entity )
        case YAF3D_NOTIFY_SHUTDOWN:
            yaf3d::EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnMenu::initialize()
{
    // setup sounds
    if ( _introductionSound.length() )
        _p_introSound = setupSound( _introductionSound, _introductionSoundVolume, false );

    if ( _backgroundSound.length() )
    {
        _p_backgrdSound = setupSound( _backgroundSound, _backgroundSoundVolume, true );

        bool musicenabled = true;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_MUSIC_ENABLE, musicenabled );
        if ( !musicenabled )
            _backgrdSoundVolume = 0.0f;
        else
            yaf3d::Configuration::get()->getSettingValue( VRC_GS_MUSIC_VOLUME, _backgrdSoundVolume );
        
        setBkgMusicVolume( _backgrdSoundVolume );
    }

    // create a scene with camera path animation and world geometry
    createMenuScene();

    // load the menu layout
    try
    {
        _p_menuWindow = yaf3d::GuiManager::get()->loadLayout( _menuConfig, NULL, MENU_PREFIX );
        _p_menuWindow->hide();

        // set button callbacks
        _p_btnStartJoin = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_startjoin" ) );
        _p_btnStartJoin->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::EnMenu::onClickedJoin, this ) );
        _p_btnStartJoin->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::EnMenu::onButtonHover, this ) );
        _p_btnStartJoin->setFont( YAF3D_GUI_FONT8 );

        _p_btnStartServer = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_startserver" ) );
        _p_btnStartServer->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::EnMenu::onClickedServer, this ) );
        _p_btnStartServer->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::EnMenu::onButtonHover, this ) );
        _p_btnStartServer->setFont( YAF3D_GUI_FONT8 );

        _p_btnStartWT = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_startwt" ) );
        _p_btnStartWT->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::EnMenu::onClickedWT, this ) );
        _p_btnStartWT->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::EnMenu::onButtonHover, this ) );
        _p_btnStartWT->setFont( YAF3D_GUI_FONT8 );

        CEGUI::PushButton* p_btnGS = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_game_settings" ) );
        p_btnGS->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::EnMenu::onClickedGameSettings, this ) );
        p_btnGS->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::EnMenu::onButtonHover, this ) );
        p_btnGS->setFont( YAF3D_GUI_FONT8 );

        CEGUI::PushButton* p_btnQuit = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_quit" ) );
        p_btnQuit->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::EnMenu::onClickedQuit, this ) );
        p_btnQuit->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::EnMenu::onButtonHover, this ) );
        p_btnQuit->setFont( YAF3D_GUI_FONT8 );

        _p_btnReturn = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_return" ) );
        _p_btnReturn->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::EnMenu::onClickedReturnToLevel, this ) );
        _p_btnReturn->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::EnMenu::onButtonHover, this ) );
        _p_btnReturn->setFont( YAF3D_GUI_FONT8 );

        _p_btnLeave = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_leave" ) );
        _p_btnLeave->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::EnMenu::onClickedLeave, this ) );
        _p_btnLeave->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::EnMenu::onButtonHover, this ) );
        _p_btnLeave->setFont( YAF3D_GUI_FONT8 );

        // setup loading window
        _p_loadingWindow = static_cast< CEGUI::Window* >( CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", ( MENU_PREFIX "wnd_loading" ) ) );
        _p_loadingWindow->hide();
        yaf3d::GuiManager::get()->getRootWindow()->addChildWindow( _p_loadingWindow );
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
        std::string materialName( OVERLAY_IMAGESET );
        CEGUI::Texture*  p_texture = yaf3d::GuiManager::get()->getGuiRenderer()->createTexture( _loadingOverlayTexture, "MenuResources" );
        CEGUI::Imageset* p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( materialName, p_texture );        
        if ( !p_imageSet->isImageDefined( _loadingOverlayTexture ) )
        {
            p_imageSet->defineImage( materialName, CEGUI::Point( 0.0f, 0.0f ), CEGUI::Size( p_texture->getWidth(), p_texture->getHeight() ), CEGUI::Point( 0.0f,0.0f ) );
        }
        CEGUI::Image* p_loadingOverlayImage = &const_cast< CEGUI::Image& >( p_imageSet->getImage( materialName ) );
        _p_loadingOverly->setImage( p_loadingOverlayImage );
        _p_loadingOverly->setSize( CEGUI::Size( 0.5f, 0.2f ) );
        _p_loadingOverly->setPosition( CEGUI::Point( 0.25f, 0.75f ) );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** Menu: cannot find layout: " << _menuConfig << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
        return;
    }

    // setup dialog for editing game settings
    _settingsDialog = std::auto_ptr< DialogGameSettings >( new DialogGameSettings( this ) );
    if ( !_settingsDialog->initialize( _settingsDialogConfig ) )
        return;

    // setup intro layout
    _intro = std::auto_ptr< IntroControl >( new IntroControl );
    if ( !_intro->initialize( _introTexture ) )
        return;
    // set the click and intro sound objects
    _intro->setIntroSound( _p_introSound );

    // create input handler
    _p_inputHandler = new MenuInputHandler( this );

    // setup level select dialog
    _levelSelectDialog = std::auto_ptr< DialogLevelSelect >( new DialogLevelSelect( this ) );
    if ( !_levelSelectDialog->initialize( _levelSelectDialogConfig ) )
    {
        log_error << "EnMenu: cannot setup level select dialog" << std::endl;
        return;
    }

    // start intro
    beginIntro();

    // this entity needs updates initially for getting the intro running
    yaf3d::EntityManager::get()->registerUpdate( this, true );

    // we register ourself to get notifications. interesting one is the shutdown notification
    //  as we have to trigger the destruction ourself as this entity is persistent.
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

void EnMenu::createMenuScene()
{
    // load the menu scene and camera path
    osg::Node* p_animnode  = yaf3d::LevelManager::get()->loadMesh( _menuCameraPathFile );
    if ( p_animnode )
    {
        _menuAnimationPath = new osg::Group();
        _menuAnimationPath->addChild( p_animnode );
    }
    else
    {
        log_error << "*** EnMenu: cannot setup scene as camera animation is missing" << std::endl;
        return;
    }
    // add the animation node to root scene
    yaf3d::Application::get()->getSceneRootNode()->addChild( _menuAnimationPath.get() );

    // get loaded menu scene ( this is the static world specified by MAP element in level file )
    _menuScene = yaf3d::LevelManager::get()->getStaticMesh();

    // create and setup camera
    log_debug << "creating menu camera entity '_menuCam_'" << std::endl;
    EnCamera* p_camEntity = static_cast< EnCamera* >( yaf3d::EntityManager::get()->createEntity( ENTITY_NAME_CAMERA, "_menuCam_" ) );
    assert( p_camEntity && "cannot create camera entity!" );
    _p_cameraControl = p_camEntity;
    // our camera must be persistent, as it must survive every subsequent level loading
    _p_cameraControl->setPersistent( true );
    osg::Vec3f campos( 0.0f, 0.0f, -10000.0f );
    p_camEntity->getAttributeManager().setAttributeValue( "position",        campos                 );
    p_camEntity->getAttributeManager().setAttributeValue( "backgroundColor", _cameraBackgroundColor );
    p_camEntity->getAttributeManager().setAttributeValue( "fov",             _cameraFov             );
    p_camEntity->getAttributeManager().setAttributeValue( "nearClip",        _cameraNearClip        );
    p_camEntity->getAttributeManager().setAttributeValue( "farClip",         _cameraFarClip         );
    osg::Quat rotoffset( -osg::PI / 2.0f, osg::Vec3f( 1, 0, 0 ) );
    p_camEntity->setCameraOffsetRotation( rotoffset );    
    p_camEntity->initialize();
    p_camEntity->postInitialize();
}

EnAmbientSound* EnMenu::setupSound( const std::string& filename, float volume, bool loop ) const
{
    // manually create an entity of type AmbientSound without adding it to pool as we use the entity locally
    //  and do not need managed destruction or searchable ability for the entity
    EnAmbientSound* p_ent = dynamic_cast< EnAmbientSound* >( yaf3d::EntityManager::get()->createEntity( ENTITY_NAME_AMBIENTSOUND, filename, false ) );
    if ( !p_ent )
    {
        log_error << "*** EnMenu: cannot create sound entity of type '" << ENTITY_NAME_AMBIENTSOUND << "'" << std::endl;
        return NULL;
    }

    // all menu sounds are of type Common
    std::string sndgroup( "Common" );

    // setup entity parameters
    p_ent->getAttributeManager().setAttributeValue( "soundFile",   filename );
    p_ent->getAttributeManager().setAttributeValue( "soundGroup",  sndgroup );
    p_ent->getAttributeManager().setAttributeValue( "loop",        loop     );
    p_ent->getAttributeManager().setAttributeValue( "autoPlay",    false    );
    p_ent->getAttributeManager().setAttributeValue( "volume",      volume   );

    // init entity
    p_ent->initialize();
    p_ent->postInitialize();
    // this entity instance needs no notification handling
    yaf3d::EntityManager::get()->registerNotification( p_ent, false );

    return p_ent;
}

bool EnMenu::onButtonHover( const CEGUI::EventArgs& arg )
{
    // play mouse over sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_HOVER );    
    return true;
}

bool EnMenu::onClickedGameSettings( const CEGUI::EventArgs& arg )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );    

    _settingsDialog->show( true );
    _p_menuWindow->disable();

    return true;
}

bool EnMenu::onClickedQuit( const CEGUI::EventArgs& arg )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );    

    yaf3d::Application::get()->stop();

    return true;
}

bool EnMenu::onClickedReturnToLevel( const CEGUI::EventArgs& arg )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );    

    leave();

    return true;
}

bool EnMenu::onClickedLeave( const CEGUI::EventArgs& arg )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );    

    // ask user before leaving
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "", "You really want to leave the level?", yaf3d::MessageBoxDialog::YES_NO, true );

        // create a call back for yes/no buttons of messagebox
        class MsgYesNoClick: public yaf3d::MessageBoxDialog::ClickCallback
        {
            public:

                explicit                MsgYesNoClick( EnMenu* p_menu ) : _p_menu( p_menu ) {}

                virtual                 ~MsgYesNoClick() {}

                void                    onClicked( unsigned int btnId )
                                        {
                                            // did the user clicked yes? if so then store settings
                                            if ( btnId == yaf3d::MessageBoxDialog::BTN_YES )                                                    
                                                _p_menu->leaveLevel();

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

bool EnMenu::onClickedJoin( const CEGUI::EventArgs& arg )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    std::string url;
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SERVER_IP, url );
    std::string clientname;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, clientname );
    yaf3d::NodeInfo nodeinfo( "", clientname );
    unsigned int channel = 0;
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SERVER_PORT, channel );

    // try to join
    try
    {
        yaf3d::NetworkDevice::get()->setupClient( url, channel, nodeinfo );
    }
    catch ( const yaf3d::NetworkException& e )
    {
        log_warning << "cannot setup client networking" << std::endl;

        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", e.what(), yaf3d::MessageBoxDialog::OK, true );
        // create a call back for Ok button of messagebox
        class MsgOkClick: public yaf3d::MessageBoxDialog::ClickCallback
        {
            public:

                explicit                MsgOkClick( EnMenu* p_menu ) : _p_menu( p_menu ) {}

                virtual                 ~MsgOkClick() {}

                void                    onClicked( unsigned int btnId )
                                        {
                                            _p_menu->_p_menuWindow->enable();

                                            // play mouse click sound
                                            vrc::gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );    
                                        }

                EnMenu*                 _p_menu;
        };
        p_msg->setClickCallback( new MsgOkClick( this ) );
        p_msg->show();

        // play attention sound
        vrc::gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_ATTENTION );    

        return true;
    }

    // set the game mode to Client before loading the level
    yaf3d::GameState::get()->setMode( yaf3d::GameState::Client );
    // now prepare loading level
    std::string levelfilename = yaf3d::NetworkDevice::get()->getNodeInfo()->getLevelName();
    _queuedLevelFile = YAF3D_LEVEL_CLIENT_DIR + levelfilename;

    // release level files object
    if ( _p_clientLevelFiles )
        delete _p_clientLevelFiles;

    _p_clientLevelFiles = new gameutils::LevelFiles( YAF3D_LEVEL_CLIENT_DIR );
    CEGUI::Image* p_img = _p_clientLevelFiles->getImage( levelfilename );

    _p_loadingLevelPic->setImage( p_img );
    _p_loadingWindow->show();
    _p_menuWindow->hide();

    _menuState = BeginLoadingLevel;

    return true;
}

bool EnMenu::onClickedServer( const CEGUI::EventArgs& arg )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );    

    _p_menuWindow->disable();

    // change search folder in dialog for selecting a level
    _levelSelectDialog->setSearchDirectory( YAF3D_LEVEL_SERVER_DIR );
    _levelSelectDialog->enable( true );

    // set level loading state
    _levelSelectionState = ForServer;

    return true;
}

bool EnMenu::onClickedWT( const CEGUI::EventArgs& arg )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );    

    yaf3d::GameState::get()->setMode( yaf3d::GameState::Standalone );
    _p_menuWindow->disable();

    // change search folder in dialog for selecting a level
    _levelSelectDialog->setSearchDirectory( YAF3D_LEVEL_SALONE_DIR );
    _levelSelectDialog->enable( true );

    // set level loading state
    _levelSelectionState = ForStandalone;

    _p_menuWindow->disable();

    return true;
}

void EnMenu::updateEntity( float deltaTime )
{
    // main control state machine
    switch ( _menuState )
    {
        case None:
            break;

        case BeginIntro:
        {
            // let the mouse disappear for intro
            gameutils::GuiUtils::get()->showMousePointer( false ); 
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
            gameutils::GuiUtils::get()->showMousePointer( false ); // let the mouse disappear 
            _menuState = PrepareLoadingLevel;
        }
        break;

        case PrepareLoadingLevel:
        {
            // unload level, don't keep physics and entities
            // note: the menu entity is persistent anyway, it handles the level switch itself!
            yaf3d::LevelManager::get()->unloadLevel( true, true );
            _menuState  = LoadingLevel;
        }
        break;

        case LoadingLevel:
        {
            yaf3d::LevelManager::get()->loadLevel( _queuedLevelFile );
            _queuedLevelFile = ""; // reset the queue

            // now load the player and its other entities
            std::string playerCfgFile;
            vrc::gameutils::PlayerUtils::get()->getPlayerConfig( yaf3d::GameState::get()->getMode(), false, playerCfgFile );
            yaf3d::LevelManager::get()->loadEntities( playerCfgFile );

            // complete level loading
            yaf3d::LevelManager::get()->finalizeLoading();

            // set flag that we have loaded a level; some menu oprions depend on this flag
            _levelLoaded = true;

            // store level scene's static mesh for later switching
            _levelScene = yaf3d::LevelManager::get()->getStaticMesh();

            // free up the client level file object which was previously used for getting the loading pic
            if ( _p_clientLevelFiles )
                delete _p_clientLevelFiles;
            _p_clientLevelFiles = NULL;

            // now start client networking when we joined to a session
            if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
            {
                // try to start client
                try
                {
                    yaf3d::NetworkDevice::get()->startClient();
                }
                catch ( const yaf3d::NetworkException& e )
                {

                    yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", e.what(), yaf3d::MessageBoxDialog::OK, true );
                    p_msg->show();

                    _menuState = UnloadLevel;

                    // play attention sound
                    vrc::gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_ATTENTION );

                    return;
                }
            }

            // leave the menu system
            leave();
            _menuState = Hidden;

            // disable level select dialog now freeing up the resources ( texures )
            _levelSelectDialog->enable( false );
        }
        break;

        case UnloadLevel:
        {
            yaf3d::LevelManager::get()->unloadLevel();
            // release the level scene node
            _levelScene.release();
            _levelLoaded = false;
            switchMenuScene( true );
            _menuState   = Visible;
        }
        break;

        // currently we do nothing in hidden state, just idle
        case Hidden:
            break;

        case Visible:
        {
            _settingsDialog->update( deltaTime );

            if ( _menuAnimationPath.get() )
            {
                // play the camera animation during the user is in menu            
                vrc::gameutils::TransformationVisitor tv( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ); // see vrc_utils.h in framework
                _menuAnimationPath->accept( tv );
                const osg::Matrixf&  mat = tv.getMatrix();
                osg::Quat rot;
                mat.get( rot );
                osg::Vec3f pos = mat.getTrans();
                _p_cameraControl->setCameraTransformation( pos, rot );
            }

            // handle menu idling
            {
                if ( _idleCounter >= 0.0 )
                {
                    _idleCounter -= deltaTime;
                }

                if ( _idleCounter < 0.0 )
                {
                    // start idle mode
                    _p_menuWindow->hide();
                    yaf3d::GuiManager::get()->showMousePointer( false );
                }
            }
        }
        break;

        default:
            assert( NULL && "invalid menu state!" );

    }

    // control the menu background sound on entering and leaving menu
    switch ( _menuSoundState )
    {
        case SoundStopped:
            break;

        case SoundFadeIn:
        {   
            _p_backgrdSound->startPlaying( true );
            _menuSoundState = SoundFadingIn;
        }
        break;

        case SoundFadingIn:
        {
            _soundFadingCnt += deltaTime;
            if ( _soundFadingCnt > BCKRGD_SND_FADEIN_PERIOD )
            {
                _soundFadingCnt = 0.0f;
                _menuSoundState = SoundStopped;
                break;
            }

            float volume = std::min( _soundFadingCnt / BCKRGD_SND_FADEIN_PERIOD, _backgrdSoundVolume );
            _p_backgrdSound->setVolume( volume );
        }
        break;

        case SoundFadeOut:
        {
            _soundFadingCnt += deltaTime;
            if ( _soundFadingCnt > BCKRGD_SND_FADEIN_PERIOD )
            {
                _p_backgrdSound->stopPlaying( true );
                _soundFadingCnt = 0.0f;
                _menuSoundState = SoundStopped;
                break;
            }

            float volume = std::max( _backgrdSoundVolume * ( 1.0f - _soundFadingCnt / BCKRGD_SND_FADEOUT_PERIOD ), 0.0f );
            _p_backgrdSound->setVolume( volume );
        }
        break;

        default:
            assert( NULL && "unknown menu sound state!" );
    }
}

void EnMenu::beginIntro()
{
    _p_menuWindow->hide();
    _p_loadingWindow->hide();
    _settingsDialog->show( false );
    _intro->start();
    _menuState = Intro;
}

void EnMenu::stopIntro()
{
    // let the mouse appear
    gameutils::GuiUtils::get()->showMousePointer( false ); 
    _intro->stop();
}

void EnMenu::triggerUserInteraction()
{
    // reset idle counter
    _idleCounter = _menuIdleTimeout;
    if ( _menuState == Visible )
    {
        _p_menuWindow->show();
        yaf3d::GuiManager::get()->showMousePointer( true );
    }
}

void EnMenu::enter()
{
    if ( _menuState == Visible )
        return;

    // send notification to all notification-registered entities about entering menu
    yaf3d::EntityManager::get()->sendNotification( yaf3d::EntityNotification( YAF3D_NOTIFY_MENU_ENTER ) );

    // set the right text for the multi-function buttons
    if ( _levelLoaded )
    {
        _p_btnStartJoin->hide();
        _p_btnStartWT->hide();
        _p_btnStartServer->hide();

        _p_btnReturn->show();
        _p_btnLeave->show();
    }
    else
    {
        _p_btnStartJoin->show();
        _p_btnStartWT->show();
        _p_btnStartServer->show();

        _p_btnReturn->hide();
        _p_btnLeave->hide();
    }

    // reset the input handler
    _p_inputHandler->reset( true );

    // show menu layout
    _p_menuWindow->enable();
    _p_menuWindow->show();

    // activate the menu scene
    switchMenuScene( true );

    // trigger fading in the menu sound, if music is enabled
    bool musicenable;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_MUSIC_ENABLE, musicenable );
    if ( musicenable )
        _menuSoundState = SoundFadeIn;

    // set menu state
    _menuState = Visible;
}

void EnMenu::leave()
{
    if ( _menuState == Hidden )
        return;

    // send notification to all notification-registered entities about leaving menu
    yaf3d::EntityManager::get()->sendNotification( yaf3d::EntityNotification( YAF3D_NOTIFY_MENU_LEAVE ) );

    _p_menuWindow->hide();
    _p_menuWindow->enable();

    _settingsDialog->show( false );

    // reset the input handler
    _p_inputHandler->reset( true );

    // deactivate the menu scene
    switchMenuScene( false );

    // trigger fading out the menu sound, if music is enabled
    bool musicenable;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_MUSIC_ENABLE, musicenable );
    if ( musicenable )
        _menuSoundState = SoundFadeOut;

    // set menu state
    _menuState = Hidden;

    // hide the loading window
    _p_loadingWindow->hide();
}

void EnMenu::switchMenuScene( bool tomenu )
{
    if ( tomenu )
    {
        // set the proper game state
        yaf3d::GameState::get()->setState( yaf3d::GameState::Menu );

        // replace the level scene node by menu scene node
        yaf3d::LevelManager::get()->setStaticMesh( NULL ); // remove the current mesh node from scene graph
        yaf3d::LevelManager::get()->setStaticMesh( _menuScene.get() );

        _p_cameraControl->setEnable( true );
        gameutils::GuiUtils::get()->showMousePointer( true );
    }
    else
    {
        // set the proper game state
        yaf3d::GameState::get()->setState( yaf3d::GameState::Running );

        // replace the menu scene node by level scene node
        yaf3d::LevelManager::get()->setStaticMesh( NULL ); // remove the current mesh node from scene graph
        yaf3d::LevelManager::get()->setStaticMesh( _levelScene.get() );

        _p_cameraControl->setEnable( false );
    }
}

void EnMenu::onSettingsDialogClose()
{
    _p_menuWindow->enable();
    _settingsDialog->show( false );
}

void EnMenu::setBkgMusicEnable( bool en )
{
    if ( _p_backgrdSound )
    {
        // use pausing and unpausing instead of stopping / unstopping sound
        //  stopping a sound can cause a lost of channel group affilication!
        if ( en )
            _p_backgrdSound->startPlaying( true );
        else
            _p_backgrdSound->stopPlaying( true );
    }
}

void EnMenu::setBkgMusicVolume( float volume )
{
    if ( _p_backgrdSound )
        _p_backgrdSound->setVolume( volume );

    _backgrdSoundVolume = volume;
}

void EnMenu::onLevelSelectCanceled()
{    
    _p_menuWindow->enable();

    // disable dialog, frees up the resources ( texures ) and hides the dialog
    _levelSelectDialog->enable( false );
}

// called by DialogLevelSelect when a level has been selected by user
void EnMenu::onLevelSelected( std::string levelfile, CEGUI::Image* p_img )
{
    if ( _levelSelectionState == ForStandalone ) 
    {
        // prepare the level loading; the actual loading is done in update method
        _menuState = BeginLoadingLevel;
        _queuedLevelFile = YAF3D_LEVEL_SALONE_DIR + levelfile;

        _p_loadingLevelPic->setImage( p_img );

        _p_loadingWindow->show();
        _p_menuWindow->hide();

        // set game mode to standalone
        yaf3d::GameState::get()->setMode( yaf3d::GameState::Standalone );
    }
    else if ( _levelSelectionState == ForServer ) 
    {
        // disable the start server button
        _p_btnStartServer->disable();

        // get the full binary path and set the server app name
        std::string cmd = yaf3d::Application::get()->getFullBinPath();
        cmd += "/" + std::string( VRC_SERVER_APP_NAME );

        std::string arg1( "-server" );
        std::string arg2( "-level" );
        std::string arg3( levelfile );

        // use utility function to start the server
        std::string args = arg1 + "  " + arg2 + "  " + arg3;
        _serverProcHandle = yaf3d::spawnApplication( cmd, args );
    }
    else
    {
        assert( NULL && "invalid level select state!" );
    }

    _p_menuWindow->enable();
}

void EnMenu::loadLevel( std::string levelfile, CEGUI::Image* p_img )
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
    if ( !_levelLoaded )
        return;

    // set the proper game state
    yaf3d::GameState::get()->setState( yaf3d::GameState::Leaving );

    _menuState = UnloadLevel;

    _p_btnStartJoin->show();
    _p_btnStartWT->show();
    _p_btnStartServer->show();
    _p_btnReturn->hide();
    _p_btnLeave->hide();

    // end networking
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client || yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
        yaf3d::NetworkDevice::get()->disconnect();

    // reset the game state
    yaf3d::GameState::get()->setMode( yaf3d::GameState::UnknownMode );
}

} // namespace vrc
