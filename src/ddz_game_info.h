#ifndef __DDZ_GAME_INFO_H__
#define __DDZ_GAME_INFO_H__

#include <set>
#include "ddz_desk.h"
#include "service/game_info.h"
#include "util/char_key_op.h"
#include "util/tick_count.h"

//#define get_game_info(game_info_) (ddz_game_info*)game_info_

namespace dooqu_server
{
	namespace ddz
	{
		using namespace dooqu_service::service;
		using namespace dooqu_service::util;

		class ddz_game_info : public game_info
		{
		protected:
			friend class ddz_plugin;

			//玩家的游戏金币数量
			int money_;

			//玩家的游戏经验值
			int experience_;


			int money_modify_;


			int experience_modify_;


			int win_count_;


			int fail_count_;


			int win_count_modify_;


			int fail_count_modify_;


			//玩家当前所在的游戏桌子
			ddz_desk* desk_;

			//玩家进入桌子后，是否已经准备
			bool is_ready_;

			bool is_robot_mode_;

			//玩家进入桌子的时间戳
			tick_count in_desk_time_;

			bool is_card_declared_;

			//long verify_code_;

		public:
			ddz_game_info();
			virtual ~ddz_game_info();
			int bit_count;
			long verify_code;
			ddz_desk* get_desk(){ return this->desk_; }
			int money(){ return this->money_; }
			int experience(){ return this->experience_; }
			int win_count(){ return this->win_count_; }
			int fail_count(){ return this->fail_count_; }
			bool is_ready(){ return this->is_ready_; }
			bool is_card_declared(){ return this->is_card_declared_; }
			void set_card_declared(bool is_declared){ this->is_card_declared_ = is_declared; }
			void reset(bool leave_desk);
			void robot_mode(bool in_robot);
			bool is_robot_mode();
			void update(int money_modi, int experience_modi, int win_modi, int fail_modi);
		};
	}
}
#endif
