/*
 * CDiveSuit.cpp
 *
 *  Created on: 25.06.2011
 *      Author: gerstrong
 */

#include "CDiveSuit.h"
#include "../../common/ai/CPlayerLevel.h"
#include "../../common/dialog/CMessageBoxBitmapGalaxy.h"

namespace galaxy {

CDiveSuit::CDiveSuit(CMap *pmap, const Uint16 foeID, Uint32 x, Uint32 y, const int sprVar) :
CGalaxySpriteObject(pmap, foeID, x, y, sprVar),
taken(false)
{    
    mSpriteIdx = 306;
    calcBoundingBoxes();
    alignToTile();
}

void CDiveSuit::process()
{}

void CDiveSuit::getTouchedBy(CSpriteObject &theObject)
{
	if(taken)
		return;

	// When Keen touches the Council Member exit the level and add one to the council list
	if( CPlayerLevel *player = dynamic_cast<CPlayerLevel*>(&theObject) )
	{
		bool &swimsuit = player->m_Inventory.Item.m_special.ep4.swimsuit;

		gSound.playSound( SOUND_GET_WETSUIT, PLAY_PAUSEALL );
		taken = swimsuit = true;

        auto evExit = new EventExitLevel(mp_Map->getLevel(), true, false, mSprVar);
        evExit->playSound = true;


        showMsgWithBmp( gBehaviorEngine.getString("SWIM_SUIT_TEXT"), "KEENTHUMBSUP", LEFT, evExit);

        player->m_Inventory.Item.m_gem.clear();
	}
}


} /* namespace galaxy */
