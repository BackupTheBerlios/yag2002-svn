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
 # Entiy for player's picking funtion
 #
 #   date of creation:  07/16/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_playerimpl.h"
#include "../storage/vrc_storageclient.h"
#include "../visuals/vrc_camera.h"
#include "vrc_playeranim.h"
#include "vrc_playerpicker.h"


//! Gui related defines
#define PLAYERINFO_DLG_PREFIX       "pinfo_"
#define PLAYERINFO_GUI_LAYOUT       "gui/userprofile.xml"

//! Player highlighting by color
#define HIGHLIGHT_COL_PERIOD        0.5f  /* color animation period in secods */
#define HIGHLIGHT_COL_STRENGTH      0.35f  /* color strength for animation */


namespace vrc
{

//! Implement and register the player picker entity factory
YAF3D_IMPL_ENTITYFACTORY( PlayerPickerEntityFactory )


//! Implementation of input handler
EnPlayerPicker::PickerInputHandler::PickerInputHandler( EnPlayerPicker* p_entity ) :
 _p_entity( p_entity ),
 _enable( true )
{
}

bool EnPlayerPicker::PickerInputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/ )
{
    // first check if picking is locked
    unsigned int ctrlmodes = gameutils::PlayerUtils::get()->getPlayerControlModes();
    if ( ctrlmodes & gameutils::PlayerUtils::eLockPicking )
        return false;

    if ( !_p_entity->_p_pickedPlayer || !_enable )
        return false;

    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );

    unsigned int eventType  = p_eventAdapter->getEventType();
    unsigned int mouseBtn   = p_eventAdapter->getButton();
    bool mouseButtonRelease = ( eventType == osgGA::GUIEventAdapter::RELEASE );

    if ( mouseButtonRelease )
    {
        if ( mouseBtn == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
            _p_entity->playerPicked( _p_entity->_p_pickedPlayer );
    }

    // let the event further process by other handlers
    return false;
}

//! Implementation of player picker
EnPlayerPicker::EnPlayerPicker() :
 _p_inputHandler( NULL ),
 _maxPickDistance( 10.0f ),
 _highlightByMesh( false ),
 _detectionAngle( 20.0f ),
 _viewAngle( 0.0f ),
 _pickCheckTimer( 0.0f ),
 _maxPickDistance2( 100.0f ),
 _p_pickedPlayer( NULL ),
 _p_frame( NULL ),
 _p_editboxNickName( NULL ),
 _p_editboxMemberSince( NULL ),
 _p_editboxOnlineTime( NULL ),
 _p_editboxStatus( NULL ),
 _p_editboxAboutMe( NULL )
{
    log_debug << "creating player picker entity"  << getInstanceName() << std::endl;

    getAttributeManager().addAttribute( "maxPickDistance" , _maxPickDistance );
    getAttributeManager().addAttribute( "detectionAngle"  , _detectionAngle  );
    getAttributeManager().addAttribute( "signMeshFile"    , _signMeshFile    );
    getAttributeManager().addAttribute( "highlightByMesh" , _highlightByMesh );
}

EnPlayerPicker::~EnPlayerPicker()
{
    log_debug << "destroying player picker entity '"  << getInstanceName() << std::endl;

    // cleanup the sign mesh's smart pointer
    _signMesh = NULL;

    if ( _p_inputHandler )
        _p_inputHandler->destroyHandler();

    // dergister from callback
    gameutils::PlayerUtils::get()->registerCallbackPlayerListChanged( this, false );

    try
    {
        if ( _p_frame )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_frame );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnPlayerPicker: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void EnPlayerPicker::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle the menu entering/leaving for property GUI
    switch ( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
            if ( _p_inputHandler )
                _p_inputHandler->enable( false );
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _p_inputHandler )
                _p_inputHandler->enable( true );
        }
        break;

        default:
            ;
    }
}

void EnPlayerPicker::initialize()
{
    // get the sign mesh
    _signMesh = yaf3d::LevelManager::get()->loadMesh( _signMeshFile, true );
    if ( _highlightByMesh && !_signMesh.get() )
    {
        log_warning << "*** cannot get the sign mesh " << _signMeshFile << ", deactivating entity" << std::endl;
        return;
    }

    //! setup view angle
    _viewAngle = cos( osg::DegreesToRadians( _detectionAngle ) );

    // calc the square of max distance (run-time speed optimization)
    _maxPickDistance2 = _maxPickDistance * _maxPickDistance;

    // create input handler
    _p_inputHandler = new EnPlayerPicker::PickerInputHandler( this );

    // setup the gui
    setupGui();

    // register for getting system notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
    // register entity in order to get updated per simulation step.
    yaf3d::EntityManager::get()->registerUpdate( this, true );
    // register for getting notified when players leave
    gameutils::PlayerUtils::get()->registerCallbackPlayerListChanged( this, true );
}

void EnPlayerPicker::updateEntity( float deltaTime )
{
    if ( _pickCheckTimer > PLAYERPICKING_UPDATE_PERIOD )
    {
        _pickCheckTimer = 0.0f;
        checkPicking();
    }
    else
    {
        _pickCheckTimer += deltaTime;
    }

    if ( !_highlightByMesh && _p_pickedPlayer )
    {
        // animate the highlight color
        static float coltime = 0.0f;
        coltime += deltaTime;
        float col = sinf( coltime * ( 2.0f / HIGHLIGHT_COL_PERIOD ) ) / ( ( 1.0f / HIGHLIGHT_COL_STRENGTH ) * osg::PI );
        osg::Vec3f color( col, col, col );

        EnPlayerAnimation* p_anim = _p_pickedPlayer->getPlayerImplementation()->getPlayerAnimation();
        p_anim->setHighlightColor( color );
    }
}

void EnPlayerPicker::onPlayerListChanged( bool /*localplayer*/, bool joining, yaf3d::BaseEntity* p_entity )
{
    if ( !joining && ( _p_pickedPlayer == p_entity ) )
        _p_pickedPlayer = NULL;
}

void EnPlayerPicker::checkPicking()
{
    EnPlayer* p_player = dynamic_cast< EnPlayer* >( vrc::gameutils::PlayerUtils::get()->getLocalPlayer() );
    if ( !p_player )
        return;

    // check for ego and player control mode
    unsigned int ctrlmodes = gameutils::PlayerUtils::get()->getPlayerControlModes();
    if ( ( p_player->getPlayerImplementation()->getCameraMode() != BasePlayerImplementation::Ego ) ||
         ( ctrlmodes & gameutils::PlayerUtils::eLockPicking ) )
    {
        if ( _p_pickedPlayer )
        {
            if ( _highlightByMesh )
            {
                _p_pickedPlayer->getTransformationNode()->removeChild( _signMesh.get() );
            }
            else
            {
                // reset the animation color
                osg::Vec3f color( 0.0f, 0.0f, 0.0f );
                EnPlayerAnimation* p_anim = _p_pickedPlayer->getPlayerImplementation()->getPlayerAnimation();
                p_anim->setHighlightColor( color );
            }
            _p_pickedPlayer = NULL;
        }
        return;
    }

    EnCamera*   p_playercamera    = p_player->getPlayerImplementation()->getPlayerCamera();
    const osg::Vec3f& campos      = p_playercamera->getCameraPosition() + p_playercamera->getCameraOffsetPosition();
    const osg::Quat&  camrotlocal = p_playercamera->getLocalRotation();
    const osg::Quat&  camrot      = p_playercamera->getCameraRotation();

    // calculate the player look direction
    osg::Vec3f lookdir( 0.0f, 1.0f, 0.0f );
    lookdir = camrotlocal * camrot * lookdir;

    // a line between a remote client and camera
    osg::Vec3f  line;
    osg::Vec3f  maxdist( 1000000.0f, 0.0f, 0.0f );

    //! find nearest player in front of local player
    yaf3d::BaseEntity* p_playerinfront = NULL;
    const std::vector< yaf3d::BaseEntity* >& remoteplayers = vrc::gameutils::PlayerUtils::get()->getRemotePlayers();
    std::vector< yaf3d::BaseEntity* >::const_iterator p_beg = remoteplayers.begin(), p_end = remoteplayers.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        line = ( *p_beg )->getPosition() - campos;

        // consider the max distance
        if ( line.length2() > _maxPickDistance2 )
            continue;

        osg::Vec3f  dir( line );
        dir.normalize();

        // check if the player is in our view
        if ( ( dir * lookdir ) > _viewAngle )
        {
            // store the nearest distance ( sorting )
            if ( maxdist.length2() > line.length2() )
            {
                maxdist = line;
                p_playerinfront = *p_beg;
            }
        }
    }

    if ( p_playerinfront )
    {
        EnPlayer* p_playerentity = dynamic_cast< EnPlayer* >( p_playerinfront );
        assert( p_playerentity && "wrong object type: EnPlayer expected!" );

        // change the pick sign to new picked player
        if ( _p_pickedPlayer )
        {
            _p_pickedPlayer->getTransformationNode()->removeChild( _signMesh.get() );
        }
        _p_pickedPlayer = p_playerentity;

        if ( _highlightByMesh )
            _p_pickedPlayer->getTransformationNode()->addChild( _signMesh.get() );
    }
    else
    {
        if ( _p_pickedPlayer )
        {
            if ( _highlightByMesh )
            {
                _p_pickedPlayer->getTransformationNode()->removeChild( _signMesh.get() );
            }
            else
            {
                // reset the animation color
                osg::Vec3f color( 0.0f, 0.0f, 0.0f );
                EnPlayerAnimation* p_anim = _p_pickedPlayer->getPlayerImplementation()->getPlayerAnimation();
                p_anim->setHighlightColor( color );
            }

            _p_pickedPlayer = NULL;
        }
    }
}

void EnPlayerPicker::playerPicked( EnPlayer* p_player )
{
    std::string playername = p_player->getPlayerName();
    StorageClient::get()->requestPublicAccountInfo( playername, this );
}

void EnPlayerPicker::accountInfoResult( tAccountInfoData& info )
{
    if ( _p_frame )
        updateGui( info );
}

void EnPlayerPicker::setupGui()
{
    try
    {
        CEGUI::Window* p_maingui = gameutils::GuiUtils::get()->getMainGuiWindow();
        CEGUI::Window* p_layout  = yaf3d::GuiManager::get()->loadLayout( PLAYERINFO_GUI_LAYOUT, p_maingui, PLAYERINFO_DLG_PREFIX );

        _p_frame = static_cast< CEGUI::FrameWindow* >( p_layout );
        if ( !_p_frame )
        {
            log_error << "EnPlayerPicker: missing main gui frame with name 'frame'" << std::endl;
            return;
        }

        _p_frame->hide();

        // subscribe for getting on-close callbacks
        _p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::EnPlayerPicker::onClickedClose, this ) );
        CEGUI::PushButton* p_btnclose = static_cast< CEGUI::PushButton* >( _p_frame->getChild( PLAYERINFO_DLG_PREFIX "btn_ok" ) );
        p_btnclose->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::EnPlayerPicker::onClickedClose, this ) );

        _p_editboxNickName    = static_cast< CEGUI::Editbox* >( _p_frame->getChild( PLAYERINFO_DLG_PREFIX "eb_nickname" ) );
        _p_editboxMemberSince = static_cast< CEGUI::Editbox* >( _p_frame->getChild( PLAYERINFO_DLG_PREFIX "eb_membersince" ) );
        _p_editboxOnlineTime  = static_cast< CEGUI::Editbox* >( _p_frame->getChild( PLAYERINFO_DLG_PREFIX "eb_onlinetime" ) );
        _p_editboxStatus      = static_cast< CEGUI::Editbox* >( _p_frame->getChild( PLAYERINFO_DLG_PREFIX "eb_status" ) );
        _p_editboxAboutMe     = static_cast< CEGUI::MultiLineEditbox* >( _p_frame->getChild( PLAYERINFO_DLG_PREFIX "eb_aboutme" ) );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnPlayerPicker: cannot setup gui layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
        return;
    }
}

void EnPlayerPicker::updateGui( tAccountInfoData& info )
{
    assert( _p_editboxNickName );
    assert( _p_editboxMemberSince );
    assert( _p_editboxOnlineTime );
    assert( _p_editboxAboutMe );

    // format the online time and registration date
    std::vector< std::string > fields;
    std::string regdate, onlinetime, status;

    yaf3d::explode( info._p_onlineTime, ":", &fields );
    if ( fields.size() > 0 )
    {
        onlinetime += fields[ 0 ];

        // set the status depending on online time
        std::stringstream str;
        str << fields[ 0 ];
        unsigned int hours = 0;
        str >> hours;
        if ( hours < 1 )
        {
            status = "Fresh meat";
        }
        else if ( hours < 10 )
        {
            status = "Knows some";
        }
        else
        {
            status = "Stone-Washed";
        }
    }
    if ( fields.size() > 1 )
    {
        onlinetime += ":" + fields[ 1 ];
    }

    fields.clear();
    yaf3d::explode( info._p_registrationDate, " ", &fields );
    if ( fields.size() > 0 )
        regdate = fields[ 0 ];

    //! TODO: there are also other priviledge flags such as admin, moderator, banned etc.!
    //if ( info._priviledges )
    //    status = "Moderator";

    _p_editboxNickName->setText( info._p_nickName );
    _p_editboxMemberSince->setText( regdate );
    _p_editboxOnlineTime->setText( onlinetime );
    _p_editboxStatus->setText( status );
    _p_editboxAboutMe->setText( info._p_userDescription );


    // lock the player control
    unsigned int ctrlmodes = gameutils::PlayerUtils::get()->getPlayerControlModes();
    ctrlmodes |= ( gameutils::PlayerUtils::eLockPicking | gameutils::PlayerUtils::eLockCameraSwitch | gameutils::PlayerUtils::eLockLooking | gameutils::PlayerUtils::eLockMovement );
    gameutils::PlayerUtils::get()->setPlayerControlModes( ctrlmodes );

    // make the mouse pointer visible
    gameutils::GuiUtils::get()->showMousePointer( true );

    // show up the gui
    _p_frame->show();
}

bool EnPlayerPicker::onClickedClose( const CEGUI::EventArgs& /*arg*/ )
{
    _p_frame->hide();

    // unlock the player control
    unsigned int ctrlmodes = gameutils::PlayerUtils::get()->getPlayerControlModes();
    ctrlmodes &= ~( gameutils::PlayerUtils::eLockPicking | gameutils::PlayerUtils::eLockCameraSwitch | gameutils::PlayerUtils::eLockLooking | gameutils::PlayerUtils::eLockMovement );
    gameutils::PlayerUtils::get()->setPlayerControlModes( ctrlmodes );

    // hide the mouse pointer
    gameutils::GuiUtils::get()->showMousePointer( false );

    return true;
}

} // namespace vrc
