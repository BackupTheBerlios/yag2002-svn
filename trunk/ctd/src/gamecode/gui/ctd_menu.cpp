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
#include "../visuals/ctd_camera.h"
#include "../visuals/ctd_skybox.h"
#include "../visuals/ctd_fog.h"

// used for starting server process
HANDLE _serverProcHandle = NULL;

using namespace std;

namespace CTD
{

// prefix for menu layout resources
#define MENU_PREFIX             "menu_"
#define OVERLAY_IMAGESET        MENU_PREFIX "loadingoverlay"
#define DEFAULT_LEVEL_LOADER    "gui/scene/loader"
#define MENU_SCENE              "gui/scene/menuscene.osg"
#define MENU_CAMERAPATH         "gui/scene/campath.osg"

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
_p_btnStartWT( NULL ),
_p_btnStartJoin( NULL ),
_p_btnStartServer( NULL ),
_p_btnReturn( NULL ),
_p_btnLeave( NULL ),
_p_loadingWindow( NULL ),
_p_loadingLevelPic( NULL ),
_p_loadingOverlayImage( NULL ),
_p_loadingOverly( NULL ),
_p_cameraControl( NULL ),
_menuConfig( "gui/menu.xml" ),
_settingsDialogConfig( "gui/settings.xml" ),
_levelSelectDialogConfig( "gui/levelselect.xml" ),
_introTexture( "gui/intro.tga" ),
_loadingOverlayTexture( "gui/loading.tga" ),
_buttonClickSound( "gui/sound/click.wav" ),
_buttonHoverSound( "gui/sound/hover.wav" ),
_introductionSound( "gui/sound/intro.wav" ),
_menuSceneFile( MENU_SCENE ),
_menuCameraPathFile( MENU_CAMERAPATH ),
_menuState( None ),
_levelSelectionState( ForStandalone ),
_levelLoaded( false ),
_p_fog( NULL ),
_p_skyBox( NULL )
{
    // register entity attributes
    _attributeManager.addAttribute( "menuConfig"                , _menuConfig               );
    _attributeManager.addAttribute( "settingsDialogConfig"      , _settingsDialogConfig     );
    _attributeManager.addAttribute( "levelSelectDialogConfig"   , _levelSelectDialogConfig  );
    _attributeManager.addAttribute( "intoTexture"               , _introTexture             );
    _attributeManager.addAttribute( "loadingOverlayTexture"     , _loadingOverlayTexture    );
    _attributeManager.addAttribute( "buttonClickSound"          , _buttonClickSound         );
    _attributeManager.addAttribute( "buttonHoverSound"          , _buttonHoverSound         );
    _attributeManager.addAttribute( "introductionSound"         , _introductionSound        );

    _attributeManager.addAttribute( "menuScene"                 , _menuSceneFile            );
    _attributeManager.addAttribute( "cameraPath"                , _menuCameraPathFile       );

    _attributeManager.addAttribute( "skyboxRight"               , _skyboxImages[ 0 ]        );
    _attributeManager.addAttribute( "skyboxLeft"                , _skyboxImages[ 1 ]        );
    _attributeManager.addAttribute( "skyboxFront"               , _skyboxImages[ 2 ]        );
    _attributeManager.addAttribute( "skyboxBack"                , _skyboxImages[ 3 ]        );
    _attributeManager.addAttribute( "skyboxUp"                  , _skyboxImages[ 4 ]        );
    _attributeManager.addAttribute( "skyboxDown"                , _skyboxImages[ 5 ]        );

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
}

void EnMenu::handleNotification( const EntityNotification& notify )
{
    // handle notifications
    switch( notify.getId() )
    {
        // for every subsequent level loading we must register outself again for getting updating
        case CTD_NOTIFY_NEW_LEVEL_INITIALIZED:
            break;

        // we have to trigger the deletion ourselves! ( we disabled auto-deletion for this entity )
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
        _clickSound.reset( setupSound( _buttonClickSound, 0.1f ) );

    if ( _buttonHoverSound.length() )
        _hoverSound.reset( setupSound( _buttonHoverSound, 0.1f ) );

    if ( _introductionSound.length() )
        _introSound.reset( setupSound( _introductionSound, 1.0f ) );

    // create a scene with camera path animation and world geometry
    createMenuScene();

    // load the menu layout
    try
    {
        _p_menuWindow = GuiManager::get()->loadLayout( _menuConfig, NULL, MENU_PREFIX );
        _p_menuWindow->hide();

        // set button callbacks
        _p_btnStartJoin = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_startjoin" ) );
        _p_btnStartJoin->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedJoin, this ) );
        _p_btnStartJoin->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );

        _p_btnStartServer = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_startserver" ) );
        _p_btnStartServer->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedServer, this ) );
        _p_btnStartServer->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );

        _p_btnStartWT = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_startwt" ) );
        _p_btnStartWT->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedWT, this ) );
        _p_btnStartWT->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );

        CEGUI::PushButton* p_btnGS = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_game_settings" ) );
        p_btnGS->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedGameSettings, this ) );
        p_btnGS->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );

        CEGUI::PushButton* p_btnQuit = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_quit" ) );
        p_btnQuit->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedQuit, this ) );
        p_btnQuit->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );

        _p_btnReturn = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_return" ) );
        _p_btnReturn->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedReturnToLevel, this ) );
        _p_btnReturn->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );

        _p_btnLeave = static_cast< CEGUI::PushButton* >( _p_menuWindow->getChild( MENU_PREFIX "btn_leave" ) );
        _p_btnLeave->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( EnMenu::onClickedLeave, this ) );
        _p_btnLeave->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( EnMenu::onButtonHover, this ) );

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
        _p_loadingLevelPic->setSize( CEGUI::Size( 1.0, 1.0f ) );
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
        _p_loadingOverly->setSize( CEGUI::Size( 0.5f, 0.2f ) );
        _p_loadingOverly->setPosition( CEGUI::Point( 0.25f, 0.75f ) );
    }
    catch ( CEGUI::Exception e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** Menu: cannot find layout: " << _menuConfig << endl;
        log << "      reason: " << e.getMessage().c_str() << endl;
        return;
    }

    // setup dialog for editing game settings
    _settingsDialog = auto_ptr< DialogGameSettings >( new DialogGameSettings( this ) );
    if ( !_settingsDialog->initialize( _settingsDialogConfig ) )
        return;
    // set the click sound object
    _settingsDialog->setClickSound( _clickSound.get() );

    // setup dialog for selecting a level
    _levelSelectDialog = auto_ptr< DialogLevelSelect >( new DialogLevelSelect( this ) );
    if ( !_levelSelectDialog->initialize( _levelSelectDialogConfig ) )
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

    // this entity needs updates initially for getting the intro running
    EntityManager::get()->registerUpdate( this, true );

    // we register outself to get notifications. interesting one is the shutdown notification
    //  as we have to trigger the destruction outself -- because of setAutoDelete( false )
    EntityManager::get()->registerNotification( this, true );
}

void EnMenu::createMenuScene()
{
    // load the menu scene and camera path
    osg::Node* p_scenenode = LevelManager::get()->loadMesh( _menuSceneFile );
    osg::Node* p_animnode  = LevelManager::get()->loadMesh( _menuCameraPathFile );
    if ( p_scenenode && p_animnode )
    {
        _menuAnimationPath = new osg::Group();
        _menuAnimationPath->addChild( p_animnode );
    }
    else
    {
        log << Log::LogLevel( Log::L_WARNING ) << "*** EnMenu: cannot setup scene; either menu scene or camera animation is missing" << endl;
        return;
    }
    _menuScene = new osg::Group;
    _menuScene->setName( "_menuScene_" );
    _menuScene->addChild( p_scenenode );
    p_scenenode->setName( "_menuSceneNode_" );
    _menuScene->addChild( _menuAnimationPath.get() );

    // create and setup camera
    EnCamera* p_camEntity = static_cast< EnCamera* >( EntityManager::get()->createEntity( ENTITY_NAME_CAMERA, "_menuCam_" ) );
    assert( p_camEntity && "cannot create camera entity!" );
    _p_cameraControl = p_camEntity;
    // our camera must be persistent, as it must survive every subsequent level loading
    _p_cameraControl->setPersistent( true );

    osg::Vec3f bgcolor( 0.0f, 0.0f, 0.0f ); // black background
    float fov = 60.0f;
    p_camEntity->getAttributeManager().setAttributeValue( "backgroundColor", bgcolor );
    p_camEntity->getAttributeManager().setAttributeValue( "fov",             fov     );
    osg::Quat rotoffset( -M_PIF / 2.0f, osg::Vec3f( 1, 0, 0 ) );
    p_camEntity->setCameraOffsetRotation( rotoffset );    
    EntityManager::get()->addToScene( p_camEntity );    
    p_camEntity->initialize();
    p_camEntity->postInitialize();

    // create and setup skybox
    EnSkyBox* p_skyboxEntity = static_cast< EnSkyBox* >( EntityManager::get()->createEntity( ENTITY_NAME_SKYBOX, "_menuSkybox_" ) );
    assert( p_skyboxEntity && "cannot create skybox entity!" );
    _p_skyBox = p_skyboxEntity;
    p_skyboxEntity->getAttributeManager().setAttributeValue( "persistence", true                );
    p_skyboxEntity->getAttributeManager().setAttributeValue( "enable",      false               );
    p_skyboxEntity->getAttributeManager().setAttributeValue( "right",      _skyboxImages[ 0 ]   );
    p_skyboxEntity->getAttributeManager().setAttributeValue( "left",       _skyboxImages[ 1 ]   );
    p_skyboxEntity->getAttributeManager().setAttributeValue( "back",       _skyboxImages[ 2 ]   );
    p_skyboxEntity->getAttributeManager().setAttributeValue( "front",      _skyboxImages[ 3 ]   );
    p_skyboxEntity->getAttributeManager().setAttributeValue( "up",         _skyboxImages[ 4 ]   );
    p_skyboxEntity->getAttributeManager().setAttributeValue( "down",       _skyboxImages[ 5 ]   );
    EntityManager::get()->addToScene( p_skyboxEntity );
    p_skyboxEntity->initialize();
    p_skyboxEntity->postInitialize();

    EnFog* p_fogEntity = static_cast< EnFog* >( EntityManager::get()->createEntity( ENTITY_NAME_FOG, "_menuFog_" ) );
    assert( p_fogEntity && "cannot create fog entity!" );
    _p_fog = p_fogEntity;
    p_fogEntity->getAttributeManager().setAttributeValue( "persistence",    true                            );
    p_fogEntity->getAttributeManager().setAttributeValue( "density",        0.5f                            );
    p_fogEntity->getAttributeManager().setAttributeValue( "start",          200.0f                          );
    p_fogEntity->getAttributeManager().setAttributeValue( "end",            1000.0f                         );
    p_fogEntity->getAttributeManager().setAttributeValue( "color",          osg::Vec3f( 0.5f, 0.5f, 0.5f )  );
    EntityManager::get()->addToScene( p_fogEntity );
    p_fogEntity->initialize();
    p_fogEntity->postInitialize();
}
   
EnAmbientSound* EnMenu::setupSound( const std::string& filename, float volume )
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
    p_ent->getAttributeManager().setAttributeValue( "volume",      volume   );
    
    // init entity
    p_ent->initialize();
    p_ent->postInitialize();
    
    return p_ent;
}

bool EnMenu::onButtonHover( const CEGUI::EventArgs& arg )
{
    if ( _hoverSound.get() )
        _hoverSound->startPlaying();
    
    return true;
}

bool EnMenu::onClickedGameSettings( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    _settingsDialog->show( true );
    _p_menuWindow->disable();

    return true;
}

bool EnMenu::onClickedQuit( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    Application::get()->stop();

    return true;
}

bool EnMenu::onClickedReturnToLevel( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    leave();

    return true;
}

bool EnMenu::onClickedLeave( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    // ask user before leaving
    {
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

//! TODO: show a dialog with all found servers and let the user choose one
//   currently only the local net is checked and the first found server is used
bool EnMenu::onClickedJoin( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    string url;
    Configuration::get()->getSettingValue( CTD_GS_SERVER_IP, url );
    string clientname;
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_NAME, clientname );
    NodeInfo nodeinfo( "", clientname );
    unsigned int channel;
    Configuration::get()->getSettingValue( CTD_GS_SERVER_PORT, channel );

    _p_menuWindow->disable();

    // try to join
    if ( !NetworkDevice::get()->setupClient( url, channel, nodeinfo ) )
    {
        log << Log::LogLevel( Log::L_WARNING ) << "cannot setup client networking" << endl;

        MessageBoxDialog* p_msg = new MessageBoxDialog( "Attention", "Cannot connect to server!", MessageBoxDialog::OK, true );
        // create a call back for Ok button of messagebox
        class MsgOkClick: public MessageBoxDialog::ClickCallback
        {
        public:

                                    MsgOkClick( EnMenu* p_menu ) : _p_menu( p_menu ) {}

            virtual                 ~MsgOkClick() {}

            void                    onClicked( unsigned int btnId )
                                    {
                                        _p_menu->_p_menuWindow->enable();

                                        // play click sound
                                        if ( _p_menu->_clickSound.get() )
                                            _p_menu->_clickSound->startPlaying();

                                    }

            EnMenu*                 _p_menu;
        };    
        p_msg->setClickCallback( new MsgOkClick( this ) );    
        p_msg->show();

        return true;
    }
    // set the game mode to Client before loading the level
    GameState::get()->setMode( GameState::Client );
    // now prepare loading level
    _queuedLevelFile = CTD_LEVEL_CLIENT_DIR + NetworkDevice::get()->getNodeInfo()->getLevelName();

    _menuState = BeginLoadingLevel;

    return true;
}

bool EnMenu::onClickedServer( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    _levelSelectDialog->changeSearchDirectory( CTD_LEVEL_SERVER_DIR );
    _p_menuWindow->disable();
    _levelSelectDialog->show( true );

    // set level loading state
    _levelSelectionState = ForServer;

    return true;
}

bool EnMenu::onClickedWT( const CEGUI::EventArgs& arg )
{
    if ( _clickSound.get() )
        _clickSound->startPlaying();

    GameState::get()->setMode( GameState::Standalone );
    _levelSelectDialog->changeSearchDirectory( CTD_LEVEL_SALONE_DIR );
    _p_menuWindow->disable();
    _levelSelectDialog->show( true );

    // set level loading state
    _levelSelectionState = ForStandalone;

    _p_menuWindow->disable();

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
            LevelManager::get()->unloadLevel( true, true );
            LevelManager::get()->loadLevel( _queuedLevelFile );
            _queuedLevelFile = ""; // reset the queue

            // now load the player
            string playerCfgFile = getPlayerConfig( GameState::get()->getMode() );
            std::vector< BaseEntity* > entities;
            LevelManager::get()->loadEntities( playerCfgFile );

            // complete level loading
            LevelManager::get()->finalizeLoading();

            // store level scene's static mesh for later switching
            _levelScene = LevelManager::get()->getStaticMesh();

            // hide the loading window
            _p_loadingWindow->hide();

            // set flag that we have loaded a level; some menu oprions depend on this flag
            _levelLoaded = true;

            // leave the menu system
            leave();

            GuiManager::get()->showMousePointer( true ); // let the mouse appear again

            // now start client networking when we joined to a session
            if ( GameState::get()->getMode() == GameState::Client )
            {
                if ( !NetworkDevice::get()->startClient() )
                {
                    MessageBoxDialog* p_msg = new MessageBoxDialog( "Attention", "Problem starting client!", MessageBoxDialog::OK, true );
                    p_msg->show();
                }
            }
        }
        break;

        case UnloadLevel:
        {
            LevelManager::get()->unloadLevel();
            _levelLoaded = false;
            switchMenuScene( true );
            _menuState = Visible;
        }
        break;

        case Hidden:
            break;

        case Visible:
        {
            _settingsDialog->update( deltaTime );

            if ( _menuAnimationPath.get() )
            {
                // play the camera animation during the user is in menu            
                TransformationVisitor tv( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ); // see ctd_utils.h in framework
                _menuAnimationPath->accept( tv );
                const osg::Matrixf&  mat = tv.getMatrix();
                osg::Quat rot;
                mat.get( rot );
                osg::Vec3f pos = mat.getTrans();
                _p_cameraControl->setCameraTranslation( pos, rot );
            }
        }
        break;

        default:
            assert( NULL && "invalid menu state!" );

    }
}

std::string EnMenu::getPlayerConfig( unsigned int mode )
{
    string playercfgdir;
    string playercfgfile;
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_CONFIG_DIR, playercfgdir );
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_CONFIG, playercfgfile );
    // assemble full path of player cfg file
    string cfg = Application::get()->getMediaPath() + playercfgdir + "/" + playercfgfile;

    // load player config
    string profile( cfg );
    Settings* p_settings = SettingsManager::get()->createProfile( profile, cfg );
    if ( !p_settings )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "Menu: cannot find player settings: " << cfg << endl;
        MessageBoxDialog* p_msg = new MessageBoxDialog( "Attention", "Error loading player!", MessageBoxDialog::OK, true );
        p_msg->show();
        return "";
    }

    string key, value;
    switch ( mode )
    {
        case GameState::Standalone:
            key = "standaloneConfig";
            break;

        case GameState::Client:
            key = "clientConfig";
            break;

        default:
            assert( NULL && "unknown game mode for player configuration in Menu!" );
    }

    p_settings->registerSetting( key, value );
    SettingsManager::get()->loadProfile( profile );
    p_settings->getValue( key, value );
    SettingsManager::get()->destroyProfile( profile );

    return value;
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

void EnMenu::enableSkybox( bool en )
{
    _p_skyBox->enable( en );
}

void EnMenu::enableFog( bool en )
{
    _p_fog->enable( en );
}

void EnMenu::enter()
{
    if ( _menuState == Visible )
        return;

    // send notification to all notification-registered entities about entering menu
    EntityManager::get()->sendNotification( EntityNotification( CTD_NOTIFY_MENU_ENTER ) );

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

    // set menu state
    _menuState = Visible;
}

void EnMenu::leave()
{
    if ( _menuState == Hidden )
        return;

    // send notification to all notification-registered entities about leaving menu
    EntityManager::get()->sendNotification( EntityNotification( CTD_NOTIFY_MENU_LEAVE ) );

    _p_menuWindow->hide();
    _p_menuWindow->enable();

    _settingsDialog->show( false );
    _levelSelectDialog->show( false );

    // reset the input handler
    _p_inputHandler->reset( true );

    // deactivate the menu scene
    switchMenuScene( false );

    // set menu state
    _menuState = Hidden;
}

void EnMenu::switchMenuScene( bool tomenu )
{
    if ( tomenu )
    {
        // replace the level scene node by menu scene node
        // store the static world node for later switching to level scene
        LevelManager::get()->setStaticMesh( _menuScene.get() );
        _p_cameraControl->setEnable( true );
        enableSkybox( true );
        enableFog( true );
    }
    else
    {
        // replace the menu scene node by level scene node
        LevelManager::get()->setStaticMesh( _levelScene.get() );
        _p_cameraControl->setEnable( false );
        enableSkybox( false );
        enableFog( false );
    }
}

void EnMenu::onSettingsDialogClose()
{
    _p_menuWindow->enable();
    _settingsDialog->show( false );
}

void EnMenu::onLevelSelectCanceled()
{    
    _p_menuWindow->enable();
}

// called by DialogLevelSelect when a level has been selected by user
void EnMenu::onLevelSelected( string levelfile, CEGUI::Image* p_img )
{
    // if we have already started a server then return without starting a new one
    if ( _serverProcHandle )
        return;

    if ( _levelSelectionState == ForStandalone ) 
    {
        // prepare the level loading; the actual loading is done in update method
        _menuState = BeginLoadingLevel;
        _queuedLevelFile = CTD_LEVEL_SALONE_DIR + levelfile;

        _p_loadingLevelPic->setImage( p_img );

        _p_loadingWindow->show();
        _p_menuWindow->hide();

        // set game mode to standalone
        GameState::get()->setMode( GameState::Standalone );
    }
    else if ( _levelSelectionState == ForServer ) 
    {
        // disable the start server button
        _p_btnStartServer->disable();

        // get the full binary path
        string cmd = Application::get()->getFullBinPath();
        string arg1( "-server" );
        string arg2( "-level" );
        string arg3( levelfile );

        // use utility function to start the server
        string args = arg1 + "  " + arg2 + "  " + arg3;
        _serverProcHandle = spawnApplication( cmd, args );
    }
    else
    {
        assert( NULL && "invalid level select state!" );
    }

    _p_menuWindow->enable();
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
    if ( !_levelLoaded )
        return;

    _menuState = UnloadLevel;
    _p_btnStartJoin->show();
    _p_btnStartWT->show();
    _p_btnStartServer->show();
    _p_btnReturn->hide();
    _p_btnLeave->hide();

    // end networking
    if ( GameState::get()->getMode() == GameState::Client || GameState::get()->getMode() == GameState::Server )
        NetworkDevice::get()->disconnect();

    // reset the game state
    GameState::get()->setMode( GameState::None );
}

} // namespace CTD
