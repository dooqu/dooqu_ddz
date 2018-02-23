#include "ddz_desk.h"
#include <cstdlib>

namespace dooqu_server
{
	namespace ddz
	{
		const char* ddz_desk::POS_STRINGS[3] = { "0", "1", "2" };
		const char* ddz_desk::DESK_POKERS[54] = { "A1", "A2", "A3", "A4", "B1", "B2", "B3", "B4", "C1", "C2", "C3", "C4",
			"D1", "D2", "D3", "D4", "E1", "E2", "E3", "E4", "F1", "F2", "F3", "F4", "G1", "G2", "G3", "G4", "H1", "H2", "H3", "H4", "I1", "I2", "I3", "I4",
			"J1", "J2", "J3", "J4", "K1", "K2", "K3", "K4", "L1", "L2", "L3", "L4", "M1", "M2", "M3", "M4", "N1", "O1" };


		ddz_desk::ddz_desk(int desk_id) : ddz_desk()
		{
			sprintf(this->desk_id_, "%d", desk_id);
			this->desk_id_[2] = '\0';
		}


		ddz_desk::ddz_desk()
		{
			this->bid_pos_ = -1;

			this->verifi_code = 0;

			//玩家数组
			std::memset(this->clients_, 0, sizeof(this->clients_));
			std::memset(this->bid_info, 0, sizeof(this->bid_info));
			std::memset(this->pos_pokers_, 0, sizeof(this->pos_pokers_));
			this->pos_pokers_[0] = new pos_poker_list();
			this->pos_pokers_[1] = new pos_poker_list();
			this->pos_pokers_[2] = new pos_poker_list();

			this->reset();

			std::copy(ddz_desk::DESK_POKERS, ddz_desk::DESK_POKERS + 54, this->desk_pokers_);
		}


		void ddz_desk::reset()
		{
			this->is_running_ = false;
			this->landlord_ = NULL;
			this->temp_landlord = NULL;
			this->multiple_ = -1;
			this->bomb_count_ = 0;
			this->declare_count_ = 0;
			this->doublejoker_count_ = 0;
			this->curr_pos_ = -1;
			this->first_pos_ = -1;
			this->curr_poker_.zero();
			this->curr_shower_ = NULL;
			this->is_bided_ = false;

			last_actived_time_.restart();

			this->clear_pos_pokers();
		}

		void ddz_desk::clear_pos_pokers()
		{
			for (int i = 0; i < ddz_desk::DESK_CAPACITY; ++i)
			{
				this->pos_pokers_[i]->clear();
				this->bid_info[i] = 0;
			}
		}

		ws_client* ddz_desk::get_landlord()
		{
			return this->landlord_;
		}

		void ddz_desk::set_landlord(ws_client* client)
		{
			this->landlord_ = client;
		}

		void ddz_desk::active()
		{
			this->last_actived_time_.restart();
		}

		void ddz_desk::set(int pos_index, ws_client* client)
		{
			this->clients_[pos_index] = client;
		}

		void ddz_desk::set_start_pos(int pos_index)
		{
			this->first_pos_ = pos_index;
			this->curr_pos_ = pos_index;
		}

		bool ddz_desk::is_null(int pos_index)
		{
			return this->clients_[pos_index] == NULL;
		}

		ws_client* ddz_desk::pos_client(int pos_index)
		{
			return this->clients_[pos_index];
		}

		ws_client* ddz_desk::first()
		{
			if (this->first_pos_ == -1)
				return NULL;

			return this->clients_[this->first_pos_];
		}

		ws_client* ddz_desk::current()
		{
			if (this->curr_pos_ == -1)
				return NULL;

			return this->clients_[this->curr_pos_];
		}

		ws_client* ddz_desk::next()
		{
			return this->clients_[this->get_next_pos_index()];
		}

		ws_client* ddz_desk::last()
		{
			if (this->first_pos_ == -1)
				return NULL;

			return this->clients_[(this->first_pos_ + 2) % DESK_CAPACITY];
		}

		ws_client* ddz_desk::previous()
		{
			if (this->first_pos_ == -1)
				return NULL;

			int pre_index = this->curr_pos_ - 1;

			if (pre_index < 0)
			{
				pre_index = DESK_CAPACITY - 1;
			}

			return this->clients_[pre_index];
		}

		void ddz_desk::move_next()
		{
			this->curr_pos_ = this->get_next_pos_index();
		}

		int ddz_desk::get_next_pos_index()
		{
			return (this->curr_pos_ + 1) % DESK_CAPACITY;
		}

		int ddz_desk::any_null()
		{
			for (int i = 0; i < DESK_CAPACITY; ++i)
			{
				if (this->clients_[i] == NULL)
					return i;
			}

			return -1;
		}

		int ddz_desk::pos_index_of(ws_client* client)
		{
			for (int i = 0; i < ddz_desk::DESK_CAPACITY; ++i)
			{
				if (this->clients_[i] != NULL && this->clients_[i] == client)
				{
					return i;
				}
			}

			return -1;
		}

		int ddz_desk::declare_count()
		{
			return this->declare_count_;
		}

		void ddz_desk::increase_multiple()
		{
			++this->multiple_;
		}

		void ddz_desk::allocate_pokers()
		{
			int pos_i = 54;
			int rnd_i;
			const char* temp;

			while (--pos_i)
			{
				rnd_i = rand() % (pos_i + 1);
				temp = this->desk_pokers_[pos_i];
				this->desk_pokers_[pos_i] = this->desk_pokers_[rnd_i];
				this->desk_pokers_[rnd_i] = temp;
			}

			for (int poker_index = 0; poker_index < 51; ++poker_index)
			{
				this->pos_pokers_[poker_index % 3]->insert(this->desk_pokers_[poker_index]);
			}
		}

		pos_poker_list* ddz_desk::pos_pokers(int pos_index)
		{
			return this->pos_pokers_[pos_index];
		}


		// void ddz_desk::write_to_everyone(char* message)
		// {
		// 	for (int i = 0; i < ddz_desk::DESK_CAPACITY; ++i)
		// 	{
		// 		ws_client* curr_client = this->pos_client(i);

		// 		if (curr_client != NULL)
		// 		{
		// 			curr_client->write_frame(true, ws_framedata::opcode::TEXT, message);
		// 		}
		// 	}
		// }


		void ddz_desk::set_running(bool is_running)
		{
			this->is_running_ = is_running;
		}

		ddz_desk::~ddz_desk()
		{
			for (int i = 0; i < ddz_desk::DESK_CAPACITY; ++i)
			{
				this->pos_pokers_[i]->clear();
				delete this->pos_pokers_[i];
			}
		}
	}
}
