/*
 * CActionFormat.h
 *
 *  Created on: 12.08.2010
 *      Author: gerstrong
 *
 *  This special class is used for now in Galaxy and will hold the actions to perform
 *  when certain (sprites) does an action. As there is a location to that action we read it in
 *  and extend with member function will get the proper action.
 *
 *  Special thanks to lemm for his hints!
 *
 */

#ifndef CACTIONFORMAT_H_
#define CACTIONFORMAT_H_

#include <stdint.h>

class CActionFormat {
public:
	CActionFormat();
	virtual ~CActionFormat();

private:
	int16_t Left_sprite;         // 124-400
	int16_t Right_sprite;        // 124-400
	int16_t Movement_parameter;  // 0-3
	int16_t Change_h;            // 0/1
	int16_t Change_v;            // 0/1
	int16_t Delay;				// (Anim speed)  0-8000
	int16_t H_anim_move_amount;  // +- any value
	int16_t V_anim_move_amount;  // +- any value
	int16_t Behavior;            // [Start of behavior codes only]
	int16_t Behavior_segment;    // [Segment values only]
	int16_t Check_sprites;       // [Start of check sprite codes only]
	int16_t Check_segment;       // [Segment values only]
	int16_t Check_tiles;         // [Start of check tile code only]
	int16_t Check_segment2;       // [Segment values only]
	int16_t Next_action;         // [Valid actions only]
};

#endif /* CACTIONFORMAT_H_ */
