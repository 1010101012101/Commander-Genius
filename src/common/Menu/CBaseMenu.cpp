/*
 * CBaseMenu.cpp
 *
 *  Created on: 28.11.2009
 *      Author: gerstrong
 */

#include <base/CInput.h>
#include "CBaseMenu.h"
#include "common/Menu/CMenuController.h"
#include "gui/CGUIBitmap.h"

CBaseMenu::CBaseMenu(const GsRect<float>& rect) :
mpMenuDialog( new CGUIDialog(rect, CGUIDialog::EXPAND) )
{
	std::string closeString;
	CGUIButton*	pButton;

    mpMenuDialog->updateBackground();

	if(g_pBehaviorEngine->getEngine() == ENGINE_GALAXY)
	{
        GsRect<float> rect(0.30f, 0.282f, 0.5f, 0.5f);
		mpMenuDialog->setRect(rect);
		closeString = "x";
		pButton	= new CGUIButton( closeString, new CloseMenuEvent(), CGUIButton::GALAXY );
		mpMenuDialog->addControl( pButton, GsRect<float>(-0.1f, 0.1f, 0.03f/rect.w, 0.03f/rect.h) );
	}
	else
	{
		const char closeChar = 0x1F;
		closeString = closeChar;
		pButton	= new CGUIButton( closeString, new CloseMenuEvent(), CGUIButton::NONE );
		mpMenuDialog->addControl( pButton, GsRect<float>(0.0f, 0.0f, 0.06f/rect.w, 0.06f/rect.h) );
	}

	pButton->setDown(true);
	mpReturnButton = pButton;
}


void CBaseMenu::select(const size_t value)
{
	if(value >= mpMenuDialog->getControlList().size())
		return;

	for(size_t c=0 ; c<value ; c++)
		mpMenuDialog->sendEvent( new CommandEvent(IC_DOWN) );
}


void CBaseMenu::setMenuLabel(const std::string &label)
{
	if(g_pBehaviorEngine->getEngine() == ENGINE_GALAXY)
	{
		GsRect<float> rect(-0.08f, -0.08f, 1.0f, 1.0f);
        CGUIBitmap* control = new CGUIBitmap(label);
		mpMenuDialog->addControl(control, rect);
        control->mEnabled = false;
	}
}


void CBaseMenu::ponder()
{
	// Command (Keyboard/Joystick) are handled here
	for( int cmd = IC_LEFT ; cmd < MAX_COMMANDS ; cmd++ )
	{
		if( g_pInput->getPressedCommand(cmd) )
		{
		    std::shared_ptr<CEvent> command(new CommandEvent( static_cast<InputCommands>(cmd) ));
            sendEvent(command);
		    break;
		}
	}

	mpMenuDialog->processLogic();

}


void CBaseMenu::render(const float deltaT)
{
    mpMenuDialog->processRendering();
}
