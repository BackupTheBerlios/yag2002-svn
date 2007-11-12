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
 # player's property gui implementation
 #
 #   date of creation:  12/11/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ****************************************************************/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_propgui.h"

#define PDLG_PREFIX                 "prop_"
#define PROPERTIES_GUI_LAYOUT       "gui/properties.xml"

namespace vrc
{

PropertyGui::PropertyGui() :
 _p_mainWnd( NULL ),
 _p_frame( NULL ),
 _p_btnOpen( NULL )
{
    setupGui();
}

PropertyGui::~PropertyGui()
{
    try
    {
        if ( _p_mainWnd )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_mainWnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "PropertyGui: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void PropertyGui::setupGui()
{

    _p_mainWnd = yaf3d::GuiManager::get()->loadLayout( PROPERTIES_GUI_LAYOUT, NULL, PDLG_PREFIX );
    if ( !_p_mainWnd )
    {
        log_error << "PropertyGui: cannot find layout: " << PROPERTIES_GUI_LAYOUT << std::endl;
        return;
    }

    try
    {
        _p_frame = static_cast< CEGUI::FrameWindow* >( _p_mainWnd->getChild( PDLG_PREFIX "frame" ) );

        // subscribe for getting on-close callbacks
        _p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedClose, this ) );
        CEGUI::PushButton* p_btnclose = static_cast< CEGUI::PushButton* >( _p_frame->getChild( PDLG_PREFIX "btn_close" ) );
        p_btnclose->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedClose, this ) );

        // setup property frame open button
        _p_btnOpen = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", PDLG_PREFIX "_btn_openbox_" ) );
        _p_btnOpen->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedOpen, this ) );
        _p_btnOpen->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::PropertyGui::onHoverOpen, this ) );
        _p_btnOpen->setStandardImageryEnabled( false );
        _p_btnOpen->setPosition( CEGUI::Point( 0.0f, 0.6f ) );
        _p_btnOpen->setSize( CEGUI::Size( 0.08f, 0.1f ) );
        _p_mainWnd->addChildWindow( _p_btnOpen );

        // set editbox open button images
        const CEGUI::Image* p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_HAND_NORMAL ); //! TODO: take a proper image
        CEGUI::RenderableImage* p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setPushedImage( p_rendImage );
        _p_btnOpen->setNormalImage( p_rendImage );
        delete p_rendImage;

        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_HAND_HOOVER ); //! TODO: take a proper image
        p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setHoverImage( p_rendImage );
        delete p_rendImage;
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "PropertyGui: cannot setup gui layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;

        _p_mainWnd->hide();
        _p_mainWnd->deactivate();

        return;
    }

    _p_mainWnd->activate();
    _p_mainWnd->show();
    _p_frame->hide();
}

bool PropertyGui::onHoverOpen( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse over sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_HOVER );
    return true;
}

bool PropertyGui::onClickedOpen( const CEGUI::EventArgs& /*arg*/ )
{
    _p_frame->show();
    _p_btnOpen->hide();
    return true;
}

bool PropertyGui::onClickedClose( const CEGUI::EventArgs& /*arg*/ )
{
    _p_frame->hide();
    _p_btnOpen->show();
    return true;
}

} // namespace vrc
