#include "ddz_game_info.h"

namespace dooqu_server
{
	namespace ddz
	{
		ddz_game_info::ddz_game_info()
		{
			this->money_ = 0;
			this->experience_ = 0;
			this->money_modify_ = 0;
			this->experience_modify_ = 0;
			this->win_count_ = 0;
			this->fail_count_ = 0;
			this->win_count_modify_ = 0;
			this->fail_count_modify_ = 0;
			this->bit_count = 0;
			this->verify_code = 0;
			this->desk_ = NULL;
			this->is_ready_ = false;
			this->is_robot_mode_ = false;
			this->is_card_declared_ = false;
		}

		void ddz_game_info::reset(bool leave_desk)
		{
			this->is_ready_ = false;
			this->bit_count = 0;
			this->is_robot_mode_ = false;
			this->is_card_declared_ = false;

			if(leave_desk)
			{
				this->desk_ = NULL;
			}
		}

		void ddz_game_info::robot_mode(bool in_robot)
		{
			this->is_robot_mode_ = in_robot;
		}


		bool ddz_game_info::is_robot_mode()
		{
			return this->is_robot_mode_;
		}

		void ddz_game_info::update(int money_modi, int experience_modi, int win_modi, int fail_modi)
		{
			this->money_modify_ += money_modi;
			this->experience_modify_ += experience_modi;

			this->money_ += money_modi;
			this->experience_ += experience_modi;

			this->win_count_modify_ += win_modi;
			this->win_count_ += win_modi;

			this->fail_count_modify_ += fail_modi;
			this->fail_count_ += fail_modi;
		}

		ddz_game_info::~ddz_game_info()
		{
			//printf("~ddz_game_info");
		}
	}
}
