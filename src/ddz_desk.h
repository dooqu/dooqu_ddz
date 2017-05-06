#ifndef __DDZ_DESK_H__
#define __DDZ_DESK_H__

#include <set>
#include <mutex>
//#include <boost/thread/recursive_mutex.hpp>
#include <boost/noncopyable.hpp>
#include "poker_info.h"
#include "service/game_client.h"
#include "util/char_key_op.h"
#include "util/tick_count.h"

namespace dooqu_server
{
	namespace ddz
	{
		using namespace dooqu_service::service;
		using namespace dooqu_service::util;

		typedef std::set<const char*, char_key_op> pos_poker_list;

		class ddz_desk : boost::noncopyable
		{
		public:
			friend class ddz_plugin;
			enum{ DESK_CAPACITY = 3 };


		protected:
			char desk_id_[4];


			//桌子状态
			bool is_running_;


			//当前出牌的玩家
			game_client* curr_shower_;


			//当前身份为地主的玩家
			game_client* landlord_;


			//临时变量
			game_client* temp_landlord;


			//玩家数组
			game_client* clients_[DESK_CAPACITY];


			//最新一把出的牌型
			poker_info curr_poker_;


			//
			int ApplyHolderIndex;


			//激活时间戳
			tick_count last_actived_time_;


			//桌子状态锁
			std::recursive_mutex status_lock_;


			//记录炸弹牌型的出现次数
			int bomb_count_;


			//记录明牌的次数
			int declare_count_;



			int doublejoker_count_;


			//
			bool is_bided_;


			//当前的玩家
			int curr_pos_;


			//初始的第一个起始玩家
			int first_pos_;


			static const char* DESK_POKERS[54];


			const char* desk_pokers_[54];


			pos_poker_list* pos_pokers_[DESK_CAPACITY];

			//庄家的位置
			int bid_pos_;

			//当局游戏的倍数
			int multiple_;

			//叫地主的信息
			int bid_info[DESK_CAPACITY];

		public:
			static const char* POS_STRINGS[];


			int verifi_code;


			ddz_desk(int desk_id);


			ddz_desk();


			virtual ~ddz_desk();

			//当前焦点玩家的位置
			int curr_pos(){ return this->curr_pos_; }


			game_client* get_landlord();


			void set_landlord(game_client*);


			void active();

			//增加当前游戏的游戏倍数
			void increase_multiple();


			std::recursive_mutex& status_mutex(){ return this->status_lock_; }

			//获取当前游戏桌子的id
			char* id(){ return &this->desk_id_[0]; }

			void set_id(int i){ sprintf(this->desk_id_, "%d", i);  this->desk_id_[2] = 0; }

			//获取桌子上的玩家指针数组，默认指针指向位置索引为0的玩家
			game_client* clients(){ return this->clients_[0]; }

			//设置制定位置索引的玩家信息
			void set(int pos_index, game_client* client);


			//设置第一个玩家的起始位置
			void set_start_pos(int pos_index);


			//返回指定所谓位置上是否没有坐下玩家
			bool is_null(int pos_index);


			//当前游戏桌子是否在游戏中
			bool is_running(){ return this->is_running_; }


			//获取轮值叫牌玩家的位置
			int bid_pos(){ return this->bid_pos_; }

			//该桌子是否已经开始叫牌
			bool is_status_bided(){ return this->is_bided_; }

			//设置该桌的叫牌状态
			void set_status_bided(bool is_bided){ this->is_bided_ = is_bided; }


			//将焦点移动到下一个轮值叫牌玩家的位置上，并返回其索引位置
			int move_to_next_bid_pos(){ this->bid_pos_ = ++this->bid_pos_ % DESK_CAPACITY; return this->bid_pos_; }

			//设置游戏桌子是否是游戏中状态，该函数只修改状态值
			void set_running(bool is_running);

			//清空三个座位发的牌面列表
			void clear_pos_pokers();

			//给各玩家分牌
			void allocate_pokers();

			//如果游戏桌子任何一个位置玩家为空，那返回该位置索引，否则返回-1
			int any_null();

			//获取指定玩家在桌子上的位置索引
			int pos_index_of(game_client* client);

			//获取明牌的次数
			int declare_count();

			//获取指定索引位置上玩家目前手里的牌面
			pos_poker_list* pos_pokers(int pos_index);

			//获取制定索引位置的玩家信息
			game_client* pos_client(int pos_index);

			//当前游戏桌子轮值的第一个玩家
			game_client* first();


			//获取当前游戏桌子轮值的当前玩家
			game_client* current();


			//获取当前游戏桌子焦点玩家的下一个玩家
			game_client* next();


			//获取当前游戏桌子的最后一个焦点玩家
			game_client* last();


			//获取当前焦点玩家的前一个玩家
			game_client* previous();


			//将轮值焦点向下一个玩家移动
			void move_next();

			//获取当前轮值焦点的下一个焦点
			int get_next_pos_index();


			long long elapsed_active_time(){ return this->last_actived_time_.elapsed(); }


			//获取当前桌子的倍数
			int multiple(){ return this->multiple_; }


			void write_to_everyone(char* message);


			void reset();
		};


		template<int DESK_NUM>
		class ddz_desk_list
		{
		private:
			ddz_desk desks_[DESK_NUM];
		public:
			ddz_desk_list()
			{
				for (int i = 0; i < DESK_NUM; ++i)
				{
					this->desks_[i].set_id(i);
				}
			}

			ddz_desk* at(int index)
			{
				assert(index >= 0 && index < DESK_NUM);

				if (index < 0 || index >= DESK_NUM)
					return NULL;

				return &this->desks_[index];
			}

			int size()
			{
				return DESK_NUM;
			}
		};
	}
}

#endif
