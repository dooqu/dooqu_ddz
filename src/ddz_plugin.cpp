// ddz_plugin.cpp : 定义 DLL 应用程序的导出函数。
//

#include <assert.h>
#include <stdio.h>
#include "ddz_plugin.h"
#include "service/game_service.h"



//#define SERVER_STATUS_DEBUG

namespace dooqu_server
{
	namespace ddz
	{
		void log(char* content)
		{
			FILE* fp = fopen("error.log", "at");

			if (fp != NULL)
			{
				fputs(content, fp);
				fputc((int)'\n', fp);

				fclose(fp);
			}
		}

		const char* ddz_plugin::POS_STRINGS[3] = { "0", "1", "2" };

		ddz_plugin::ddz_plugin(game_service* service, char* game_id, char* title, double frequence, int desk_count, int game_multiple, int max_waiting_duration) : game_plugin(service, game_id, title, frequence)//, desk_count_(desk_count), multiple_(multiple), /*timer_(service->get_io_service()),*/ max_waiting_duration_(21000)
		{
			this->desk_count_ = desk_count;
			this->multiple_ = game_multiple;
			this->max_waiting_duration_ = max_waiting_duration;
		}

		ddz_plugin::~ddz_plugin()
		{
			//回收游戏桌子资源
		}


		void ddz_plugin::on_load()
		{
			this->regist_handle("IDK", make_handler(ddz_plugin::on_join_desk_handle));
			this->regist_handle("RDY", make_handler(ddz_plugin::on_client_ready_handle));
			this->regist_handle("BID", make_handler(ddz_plugin::on_client_bid_handle));
			this->regist_handle("DLA", make_handler(ddz_plugin::on_client_declare_handle));
			this->regist_handle("CAD", make_handler(ddz_plugin::on_client_card_handle));
			this->regist_handle("MSG", make_handler(ddz_plugin::on_client_msg_handle));
			this->regist_handle("PNG", make_handler(ddz_plugin::on_client_ping_handle));
			this->regist_handle("RBT", make_handler(ddz_plugin::on_client_robot_handle));
			this->regist_handle("BYE", make_handler(ddz_plugin::on_client_bye_handle));

			game_plugin::on_load();
		}


		void ddz_plugin::on_unload()
		{
			game_plugin::on_unload();
		}


		void ddz_plugin::on_update()
		{
			if (this->is_onlined() == false)
				return;

			for (int i = 0; i < this->desks_.size(); ++i)
			{
				ddz_desk* desk = this->desks_.at(i);

				//thread_status::log("start->ddz_plugin::on_update.desk_status");
				std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());
				//thread_status::log("end->ddz_plugin::on_update.desk_status");


				if (desk->is_running() == true)
				{
					if (desk->elapsed_active_time() > this->max_waiting_duration_)
					{
						if (desk->current() == NULL)
							continue;

						if (desk->get_landlord() == NULL)
						{
							this->on_client_bid(desk->current(), desk, false);
						}
						else
						{
							//return;
							int pos_index = desk->pos_index_of(desk->current());

#ifdef SERVER_STATUS_DEBUG
							if (pos_index == -1)
							{
								log("110");
							}
							assert(pos_index != -1);
#endif

							if (pos_index == -1)
								continue;

							//如果还有扑克
							if (desk->pos_pokers(pos_index)->size() > 0)
							{
								if (desk->curr_poker_.type == poker_info::ZERO)
								{
									char command_buffer[7];
									command_buffer[sprintf(command_buffer, "CAD %s", (*desk->pos_pokers(pos_index)->begin()))] = 0;
									desk->current()->simulate_on_command(command_buffer, false);
								}
								else
								{
									desk->current()->simulate_on_command("CAD 0", true);
								}
							}
						}
					}
				}
				else
				{
					//游戏桌子如果没有运行，需要让在桌子上坐下而一直不准备的玩家起立
				}
			}
			game_plugin::on_update();
		}


		int ddz_plugin::on_befor_client_join(game_client* client)
		{
			//房间人满
			if (this->clients_.size() >= this->desk_count_ * ddz_desk::DESK_CAPACITY)
			{
				return service_error::GAME_IS_FULL;
			}

			return game_plugin::on_befor_client_join(client);
		}


		void ddz_plugin::on_client_join(game_client* client)
		{
			//printf("ddz_plugin::on_client_join\n");
			ddz_game_info* gameinfo = new ddz_game_info();

			gameinfo->money_ = 10000;
			gameinfo->experience_ = 10000;

			client->set_game_info(gameinfo);

			game_plugin::on_client_join(client);

			client->write("LOG %s %s %d%c", this->game_id(), client->id(), this->max_waiting_duration_, 0);
		}

		//client 离开，会发送BYE命令；
		//BYE命令会先到game_plugin::on_command中，先触发ddz_plugin中对BYE命令已经注册的handle，也就是on_client_bye_handle
		//然后，在on_command写，特殊的对BYE做了处理，他会调用remove_client函数
		void ddz_plugin::on_client_leave(game_client* client, int code)
		{
			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();

			if (game_info != NULL)
			{
				ddz_desk* desk = game_info->get_desk();
				if (desk != NULL)
				{
					//thread_status::log("start->ddz_plugin::on_client_leave.desk_status");
					std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());
					//thread_status::log("end->ddz_plugin::on_client_leave.desk_status");
					this->on_client_leave_desk(client, desk, client->error_code());
				}

				client->set_game_info(NULL);
				delete game_info;
			}
			game_plugin::on_client_leave(client, code);
		}


		//当玩家加入桌子后的事件
		void ddz_plugin::on_client_join_desk(game_client* client, ddz_desk* desk, int pos_index)
		{
			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();
			game_info->desk_ = desk;

			//先通知用户进入桌子
			client->write("IDK %s %d%c", desk->id(), pos_index, 0);

			//后进来用户的游戏币信息
			//char client_in_money[BUFFER_SIZE_SMALL] = { 0 };
			//sprintf(client_in_money, "%d", game_info->money());


			//房间里每个玩家的游戏币信息
			//char client_curr_money[BUFFER_SIZE_SMALL];

			for (int i = 0; i < ddz_desk::DESK_CAPACITY; ++i)
			{
				if (desk->pos_client(i) == NULL)
					continue;

				ddz_game_info* curr_game_info = ((ddz_game_info*)desk->pos_client(i)->get_game_info());

				//填充当前玩家的游戏币信息
				//std::memset(client_curr_money, 0, sizeof(client_curr_money));
				//sprintf(client_curr_money, "%d", curr_game_info->money());

				//向进入桌子的玩家通知当前桌子的玩家列表
				client->write("LSD %s %s %s %c %d%c", desk->pos_client(i)->id(), desk->pos_client(i)->name(), ddz_plugin::POS_STRINGS[i], (curr_game_info->is_ready()) ? '1' : '0', curr_game_info->money(), 0);

				//向已有的玩家通知进来玩家的信息
				if (desk->pos_client(i) != client)
				{
					desk->pos_client(i)->write("JDK %s %s %s %c %d%c", client->id(), client->name(), ddz_plugin::POS_STRINGS[pos_index], '0', game_info->money(), 0);
				}
			}
		}


		//当玩家离开桌子的事件
		void ddz_plugin::on_client_leave_desk(game_client* client, ddz_desk* desk, int reaspon)
		{
			//printf("ddz_plugin::on_client_leave_desk:%s\n", client->id());
			//找到这个用户在游戏桌子上的位置索引
			int pos_index = desk->pos_index_of(client);

#ifdef SERVER_STATUS_DEBUG
			if (pos_index == -1)
			{
				log("242");
			}
			assert(pos_index != -1);
#endif

			if (pos_index < 0)
				return;

			//将这个位置置空
			desk->set(pos_index, NULL);

			//如果玩家还在线，通知他已经成功离开
			if (client->available())
			{
				client->write("ODK %s %s%c", desk->id(), ddz_desk::POS_STRINGS[pos_index], NULL);
			}

			//通知每个人LDK的消息
			for (int i = 0; i < ddz_desk::DESK_CAPACITY; ++i)
			{
				game_client* curr_client = desk->pos_client(i);

				if (curr_client != NULL)
				{
					curr_client->write("LDK %s %s%c", client->id(), ddz_desk::POS_STRINGS[pos_index], NULL);
				}
			}

			((ddz_game_info*)client->get_game_info())->reset(true);

			//如果当前桌子在游戏中， 那么处理离开的逻辑
			if (desk->is_running())
			{
				desk->set_running(false);

				if (reaspon != service_error::SERVER_CLOSEING)
				{
					this->on_game_stop(desk, client, false);
					desk->reset();
				}
			}
		}


		//当游戏桌子开始新一局游戏
		void ddz_plugin::on_game_start(ddz_desk* desk, int verifi_code)
		{
			if (desk == NULL)
				return;

			std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());

#ifdef SERVER_STATUS_DEBUG
			if (desk->verifi_code != verifi_code)
			{
				log("292");
			}

			assert(desk->verifi_code == verifi_code);
#endif

			if (desk->verifi_code != verifi_code)
			{
				//不能让桌子的状态死锁
				return;
			}

			if (desk->is_running() == false || desk->any_null() != -1)
			{
				return;
			}

			//清空已经发的牌面
			desk->clear_pos_pokers();
			//分配新牌面
			desk->allocate_pokers();

			for (int i = 0; i < ddz_desk::DESK_CAPACITY; ++i)
			{
				ddz_game_info* curr_game_info = (ddz_game_info*)desk->pos_client(i)->get_game_info();
				curr_game_info->reset(false);

				pos_poker_list::iterator poker_list = desk->pos_pokers(i)->begin();

				desk->pos_client(i)->write("STT %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s%c",
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*++poker_list),
					(*poker_list), NULL);
			}

			++desk->verifi_code;

			this->zone_->queue_task(std::bind(&ddz_plugin::on_game_bid, this, desk, desk->verifi_code), 4300);
			//废弃this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_game_bid), desk, NULL, desk->verifi_code, 4300);
		}


		//如果玩家发送明牌的信息
		void ddz_plugin::on_client_declare_handle(game_client* client, command* cmd)
		{
			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();

			if (game_info == NULL)
				return;

			ddz_desk* desk = game_info->get_desk();

			if (desk == NULL)
				return;
			//thread_status::log("start->ddz_plugin::on_client_declare_handle.desk_status");
			std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());
			//thread_status::log("end->ddz_plugin::on_client_declare_handle.desk_status");

			if (desk->is_running() == false)
				return;

			if (game_info->is_card_declared())
				return;

			int client_pos = desk->pos_index_of(client);

			if (client_pos == -1)
				return;

			if (desk->is_status_bided())
			{
				if (desk->get_landlord() == NULL ||
					desk->get_landlord() != client ||
					desk->current() != client ||
					desk->multiple() == -1 ||
					desk->pos_pokers(client_pos)->size() != 20)
				{
					return;
				}
			}
			else
			{
				if (desk->multiple() != -1)
					return;
			}

			if (desk->declare_count() == 0)
				desk->bid_pos_ = client_pos;

			desk->declare_count_++;
			game_info->set_card_declared(true);

			char poker_buffer[66] = { 0 };
			int cp_pos = sprintf(poker_buffer, "DLA %d ", client_pos);

			pos_poker_list* poker_list = desk->pos_pokers(client_pos);
			pos_poker_list::iterator poker_finder = poker_list->begin();

			for (; poker_finder != poker_list->end(); ++poker_finder)
			{
				cp_pos += sprintf(&poker_buffer[cp_pos], "%s ", *poker_finder);
			}
			poker_buffer[59] = 0;

			desk->write_to_everyone(poker_buffer);
		}


		//当游戏开始叫牌
		void ddz_plugin::on_game_bid(ddz_desk* desk, int verifi_code)
		{

#ifdef SERVER_STATUS_DEBUG
			if (desk->verifi_code != verifi_code)
			{
				log("349");
			}
			assert(desk->verifi_code == verifi_code);
#endif

			if (desk->verifi_code != verifi_code)
				return;
			//thread_status::log("start->ddz_plugin::on_game_bid.desk_status");
			std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());
			//thread_status::log("end->ddz_plugin::on_game_bid.desk_status");

			if (desk->is_running() == false || desk->any_null() >= 0)
			{
				return;
			}

			desk->active();

			if (desk->declare_count() == 0)
			{
				desk->move_to_next_bid_pos();
			}

			desk->set_start_pos(desk->bid_pos());

			game_client* curr_client = desk->current();

			if (curr_client != NULL)
			{
				desk->set_status_bided(true);

				char buffer[BUFFER_SIZE_SMALL];
				int n = sprintf(buffer, "BID %d %d", desk->curr_pos(), desk->multiple());
				buffer[n] = 0;

				for (int i = 0; i < ddz_desk::DESK_CAPACITY; ++i)
				{
					desk->pos_client(i)->write(buffer);
				}

				//机器人模式逻辑，如果是机器人模式，那么自动叫、抢地主
				ddz_game_info* curr_game_info = (ddz_game_info*)curr_client->get_game_info();
				if (curr_game_info->is_robot_mode())
				{
					curr_client->simulate_on_command("BID 1", true);
				}
			}
		}


		//当前游戏桌子的地主已经决定
		void ddz_plugin::on_game_landlord(ddz_desk* desk, game_client* landlord, int verifi_code)
		{
			//thread_status::log("start->ddz_plugin::on_game_landlord.desk_status");
			std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());
			//thread_status::log("end->ddz_plugin::on_game_landlord.desk_status");

			//	game_client* landlord = (game_client*)landlord_;

#ifdef SERVER_STATUS_DEBUG
			if (desk->verifi_code != verifi_code)
				log("397");

			assert(desk->verifi_code == verifi_code);
#endif

			if (desk->verifi_code != verifi_code)
				return;

			int pos_index = desk->pos_index_of(landlord);

#ifdef SERVER_STATUS_DEBUG
			if (pos_index == -1)log("407");
			if (desk->is_running() == false)log("408");
			if (desk->landlord_ == NULL)log("409");
			if (desk->landlord_ != landlord)log("410");

			//pos_index != -1 其实可以去掉的， 因为-1是正常的
			assert(pos_index != -1);
			assert(desk->is_running());
			assert(desk->landlord_ != NULL);
			assert(desk->landlord_ == landlord);
#endif

			if (desk->is_running() == false || desk->landlord_ == NULL ||
				desk->landlord_ != landlord || pos_index == -1)
				return;

			desk->set_start_pos(pos_index);

			//把底牌分给地主
			desk->pos_pokers(pos_index)->insert(&desk->desk_pokers_[51], &desk->desk_pokers_[54]);

			char buffer[BUFFER_SIZE_SMALL] = { 0 };
			sprintf(buffer, "LRD %s %s %s %s", ddz_desk::POS_STRINGS[pos_index],
				desk->desk_pokers_[51], desk->desk_pokers_[52], desk->desk_pokers_[53]);

			desk->write_to_everyone(buffer);

			ddz_game_info* lord_game_info = (ddz_game_info*)desk->current()->get_game_info();

			//如果地主目前处于机器人模式，那么延迟1000毫秒自动帮他出牌
			if (lord_game_info->is_robot_mode())
			{
				++lord_game_info->verify_code;

				this->zone_->queue_task(std::bind(&ddz_plugin::on_robot_card, this, desk, landlord, lord_game_info->verify_code), 1000);

				//废弃:this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_robot_card), desk, landlord, lord_game_info->verify_code, 1000);
			}
		}



		void ddz_plugin::on_client_show_poker(game_client* client, ddz_desk* desk, command* cmd)
		{
			desk->active();
			int pos_index = desk->pos_index_of(client);

#ifdef SERVER_STATUS_DEBUG
			if (pos_index == -1)log("441");

			assert(pos_index != -1);
#endif
			if (pos_index == -1)
				return;

			char buffer[76] = { 0 };
			int p_size = sprintf(buffer, "CAD %s ", ddz_desk::POS_STRINGS[pos_index]);

			for (int i = 0; i < cmd->param_size(); ++i)
			{
				p_size += sprintf(&buffer[p_size], (i == 0) ? "%s" : ",%s", cmd->params(i));
				desk->pos_pokers(pos_index)->erase(cmd->params(i));
				//考虑下玩家弃牌的情况，还erase？
			}

			if (desk->pos_pokers(pos_index)->size() == 0)
			{
				desk->write_to_everyone(buffer);
				desk->set_running(false);
				this->on_game_stop(desk, client, true);
				desk->reset();
			}
			else
			{
				desk->move_next();
				sprintf(&buffer[p_size], " %s,%c", ddz_desk::POS_STRINGS[desk->curr_pos()], (desk->curr_poker_.type == poker_info::ZERO) ? '1' : '0');
				desk->write_to_everyone(buffer);

				ddz_game_info* curr_game_info = (ddz_game_info*)desk->current()->get_game_info();
				//如果下家处于机器人模式，那么1000毫秒之后自动帮他出牌
				if (curr_game_info->is_robot_mode())
				{
					++curr_game_info->verify_code;

					this->zone_->queue_task(std::bind(&ddz_plugin::on_robot_card, this, desk, desk->current(), curr_game_info->verify_code), 1000);

					//废弃this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_robot_card), desk, desk->current(), curr_game_info->verify_code, 1000);
				}
			}
		}


		void ddz_plugin::on_game_stop(ddz_desk* desk, game_client* client, bool normal)
		{
			//printf("\nddz_plugin::on_game_stop");

			int game_tax = 0;
			char cmd_buffer[BUFFER_SIZE_SMALL] = { 0 };

			sprintf(cmd_buffer, "STP %s %d", desk->id(), (normal)? 1 : 0);
			//sprintf(cmd_buffer, (normal) ? 1 : 0 );
			desk->write_to_everyone(cmd_buffer);

			if (normal)
			{
				//先将剩余的牌面信息同步出去
				char buffer[BUFFER_SIZE_LARGE];
				for (int curr_index = 0; curr_index < ddz_desk::DESK_CAPACITY; ++curr_index)
				{
					pos_poker_list* pos_pokers = desk->pos_pokers(curr_index);

					//重置准备状态
					ddz_game_info* curr_game_info = (ddz_game_info*)desk->pos_client(curr_index)->get_game_info();
					curr_game_info->is_ready_ = false;

					if (pos_pokers->size() == 0)
						continue;

					std::memset(buffer, 0, sizeof(buffer));
					std::strncpy(buffer, "LFC", 3);

					int n = 0;
					for (pos_poker_list::iterator plist = pos_pokers->begin(); plist != pos_pokers->end(); ++plist, ++n)
					{
						if (plist == pos_pokers->begin())
						{
							buffer[3] = ' ';
							buffer[4] = *ddz_desk::POS_STRINGS[curr_index];
						}

						buffer[5 + (n * 3)] = ' ';
						buffer[5 + (n * 3) + 1] = (*plist)[0];
						buffer[5 + (n * 3) + 2] = (*plist)[1];
					}

					//将牌面信息发送给当前curr_pos的下家和下下家
					desk->pos_client((curr_index + 1) % 3)->write(buffer);
					desk->pos_client((curr_index + 2) % 3)->write(buffer);
				}

				//同步比分信息
				//底分,叫分倍数,炸弹,王炸,明牌 金钱,金钱变动,经验,经验变动
				char res_buffer[256];
				int n = std::sprintf(res_buffer, "RST %d,%d,%d,%d,%d", this->multiple_, desk->multiple(), desk->declare_count(), desk->bomb_count_, desk->doublejoker_count_);
				res_buffer[n] = 0;

				int total_multiple = this->multiple_ * (int)std::pow(2, (double)((desk->multiple() == -1) ? 0 : desk->multiple())) * (int)std::pow(2, (double)desk->bomb_count_) * (int)std::pow(2, (double)desk->doublejoker_count_) * (int)std::pow(3, (double)desk->declare_count());
				desk->write_to_everyone(res_buffer);

				bool is_landlord_win = (client == desk->landlord_);
				if (is_landlord_win)
				{
					//如果是正常游戏结束 && 地主赢了
					int money_landlord_win = 0;
					for (int curr_index = 0; curr_index < ddz_desk::DESK_CAPACITY; ++curr_index)
					{
						game_client* curr_client = desk->pos_client(curr_index);
						if (curr_client == NULL || curr_client == desk->landlord_)
							continue;

						ddz_game_info* curr_game_info = (ddz_game_info*)curr_client->get_game_info();

						//int exp_lost = total_multiple;
						//当前玩家理论上应该输掉的值
						int curr_money_lost = total_multiple * 2 + game_tax;

						//如果玩家输掉的值已经>玩家目前手里的钱
						if (curr_money_lost > curr_game_info->money())
						{
							curr_money_lost = curr_game_info->money();
						}

						//系统回收的当前输掉玩家的钱，还要刨除当前这个玩家应该缴纳的桌费
						int curr_money_lost_to_winner = curr_money_lost - game_tax;
						//如果赢的钱剪掉桌费后<0，说明赢的太少了， 那么给地主的钱就没有了，优先刨除掉桌费
						if (curr_money_lost_to_winner < 0)
						{
							curr_money_lost_to_winner = 0;
						}
						//累加输给地主的总金额
						money_landlord_win += curr_money_lost_to_winner;
						//更新当前玩家的资料
						curr_game_info->update(curr_money_lost, total_multiple, 0, 1);

						std::memset(res_buffer, 0, sizeof(res_buffer));
						//      %s       %s   %s        %d   %d       %d      %d      %d         %d        %d       %d
						//UPD clientID nick pos_index role iswin money_modi money exper_modi experience wincount failcount.
						int n = sprintf(res_buffer, "UPD %s %s %d 0 0 %d %d %d %d %d %d",
							curr_client->id(),
							curr_client->name(),
							curr_index,
							0 - curr_money_lost,
							curr_game_info->money(),
							0 - total_multiple,
							curr_game_info->experience(),
							curr_game_info->win_count(),
							curr_game_info->fail_count());

						res_buffer[n] = 0;
						desk->write_to_everyone(res_buffer);
					}

					ddz_game_info* landlord_game_info = (ddz_game_info*)desk->landlord_->get_game_info();

					money_landlord_win -= game_tax;
					if (money_landlord_win < 0 && (landlord_game_info->money() + money_landlord_win) < 0)
					{
						money_landlord_win = 0 - landlord_game_info->money();
					}

					//std::memset(res_buffer, 0, sizeof(res_buffer));
					int n = sprintf(res_buffer, "UPD %s %s %d 1 1 %d %d %d %d %d %d",
						desk->landlord_->id(),
						desk->landlord_->name(),
						desk->pos_index_of(desk->landlord_),
						money_landlord_win,
						landlord_game_info->money(),
						total_multiple,
						landlord_game_info->experience(),
						landlord_game_info->win_count(),
						landlord_game_info->fail_count());

					res_buffer[n] = 0;
					desk->write_to_everyone(res_buffer);
				}
				else
				{
					//如果是正常游戏结束 && 地主输了
					int exp_landlord_lost = total_multiple * 2;
					int money_landlord_lost = exp_landlord_lost * 2 - game_tax;
					//扣掉的游戏费用 + 桌费
					ddz_game_info* landlord_game_info = (ddz_game_info*)desk->landlord_->get_game_info();

					//assert(landlord_game_info != NULL);

					//如果要扣的钱都比地主手里的存款还多， 那么只能把存款扣到零，金钱不能为负
					if (money_landlord_lost > landlord_game_info->money())
					{
						money_landlord_lost = landlord_game_info->money();
					}

					//输给农民的要在输钱的基础上，刨去系统桌费，这个要系统优先回收。
					int money_lost_to_peasants = money_landlord_lost - game_tax;

					//如果输给农民的钱扣掉桌费都<0了的话， 那农民得不到分，还是优先系统回收桌费。
					if (money_lost_to_peasants < 0)
					{
						money_lost_to_peasants = 0;
					}

					//更新服务器内存数据中的数据
					landlord_game_info->update(0 - money_landlord_lost, 0 - exp_landlord_lost, 0, 1);

					int n = sprintf(res_buffer, "UPD %s %s %d 1 1 %d %d %d %d %d %d",
						desk->landlord_->id(),
						desk->landlord_->name(),
						desk->pos_index_of(desk->landlord_),
						0 - money_landlord_lost,
						landlord_game_info->money(),
						0 - exp_landlord_lost,
						landlord_game_info->experience(),
						landlord_game_info->win_count(),
						landlord_game_info->fail_count());

					res_buffer[n] = 0;
					desk->write_to_everyone(res_buffer);


					for (int curr_index = 0; curr_index < ddz_desk::DESK_CAPACITY; ++curr_index)
					{
						game_client* curr_client = desk->pos_client(curr_index);

#ifdef SERVER_STATUS_DEBUG
						if (curr_client == NULL)
							log("646");

						assert(curr_client != NULL);
#endif

						if (curr_client == NULL || curr_client == desk->landlord_)
							continue;

						ddz_game_info* curr_game_info = (ddz_game_info*)curr_client->get_game_info();

#ifdef SERVER_STATUS_DEBUG
						if (curr_game_info == NULL)
							log("655");

						assert(curr_game_info != NULL);
#endif

						int exp_peasant_win = total_multiple;
						int money_peasant_win = money_lost_to_peasants / 2;

						//扣掉桌费到手的
						int money_peasant_hold = money_peasant_win - game_tax;
						if (money_peasant_hold < 0)
						{
							if ((curr_game_info->money() + money_peasant_hold) < 0)
							{
								money_peasant_hold = 0 - curr_game_info->money();
							}
						}

						curr_game_info->update(money_peasant_hold, exp_peasant_win, 1, 0);

						int n = sprintf(res_buffer, "UPD %s %s %d 0 0 %d %d %d %d %d %d",
							curr_client->id(),
							curr_client->name(),
							curr_index,
							money_peasant_hold,
							curr_game_info->money(),
							exp_peasant_win,
							curr_game_info->experience(),
							curr_game_info->win_count(),
							curr_game_info->fail_count());

						res_buffer[n] = 0;
						desk->write_to_everyone(res_buffer);
					}
				}
			}
			else
			{
			}
			desk->reset();
		}



		//当用户触发进入桌子的命令
		void ddz_plugin::on_join_desk_handle(game_client* client, command* cmd)
		{
			ddz_game_info* game_info = (ddz_game_info*)(client->get_game_info());
			ddz_desk* last_desk = game_info->get_desk();

			if (last_desk != NULL)
			{
				//thread_status::log("start->ddz_plugin::on_join_desk_handle.desk_status");
				//记得上锁
				std::lock_guard<std::recursive_mutex> lock(last_desk->status_mutex());
				//thread_status::log("end->ddz_plugin::on_join_desk_handle.desk_status");

				this->on_client_leave_desk(client, last_desk, service_error::CLIENT_EXIT);
			}

			for (int free_count_required = 1; free_count_required < 4; free_count_required++)
			{
				for (int i = 0; this->is_onlined() && i < this->desks_.size(); i++)
				{
					//获取当前要查询的桌子的指针引用
					ddz_desk* curr_desk = this->desks_.at(i);
					//如果当前桌子是上次坐的桌子，那么跳过
					if (curr_desk == last_desk)
					{
						continue;
					}

					//当前桌子上锁
					std::lock_guard<std::recursive_mutex> lock(curr_desk->status_mutex());

					//如果当前游戏桌子的状态是正在游戏，也要跳过
					if (curr_desk->is_running())
					{
						continue;
					}

					int free_count = 0;
					int free_pos = -1;
					for (int pos_index = 0; pos_index < ddz_desk::DESK_CAPACITY; pos_index++)
					{
						if (curr_desk->pos_client(pos_index) == NULL)
						{
							free_count++;
							//如果当前空闲的位置大于目标数量，那么这个桌子跳过
							if (free_count > free_count_required)
							{
								break;
							}

							if (free_pos == -1)
							{
								free_pos = pos_index;
							}
						}
					}

					if (free_pos != -1 && free_count == free_count_required)
					{
						this->desks_.at(i)->set(free_pos, client);
						this->on_client_join_desk(client, curr_desk, free_pos);
						return;
					}
				}
			}

			client->write("IDK -1%c", NULL);
		}


		//当用户触发准备命令
		void ddz_plugin::on_client_ready_handle(game_client* client, command* cmd)
		{
			if (cmd->param_size() != 0)
				return;

			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();

			//如果玩家已经准备了， 返回
			if (game_info->is_ready())
				return;

			ddz_desk* desk = game_info->get_desk();

			//如果玩家还没加入到任何桌子，返回
			if (desk == NULL)
				return;
//			thread_status::log("start->ddz_plugin::on_client_ready_handle.desk_status");
			//加锁
			std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());
			//thread_status::log("end->ddz_plugin::on_client_ready_handle.desk_status");
			if (desk->is_running())
				return;

			//如果座位上没找到玩家，返回
			int pos_index = desk->pos_index_of(client);

#ifdef SERVER_STATUS_DEBUG
			if (pos_index == -1) log("792");

			assert(pos_index != -1);
#endif

			if (pos_index == -1)
				return;

			game_info->is_ready_ = true;

			bool is_all_ready = true;
			for (int i = 0; i < ddz_desk::DESK_CAPACITY; i++)
			{
				game_client* curr_client = desk->pos_client(i);

				if (curr_client == NULL)
				{
					is_all_ready = false;
				}
				else
				{
					if (((ddz_game_info*)curr_client->get_game_info())->is_ready() == false)
					{
						is_all_ready = false;
					}
					curr_client->write("RDY %s %s%c", client->id(), ddz_desk::POS_STRINGS[pos_index], NULL);
				}
			}

			if (is_all_ready)
			{
				//更新桌子的激活时间
				desk->active();

				//重置桌子的信息
				desk->reset();

				//设定游戏状态为开始游戏
				desk->set_running(true);

				if (this->zone_ != NULL)
				{
					++desk->verifi_code;

					this->zone_->queue_task(std::bind(&ddz_plugin::on_game_start, this, desk, desk->verifi_code), 500);
					//this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_game_start), desk, NULL, desk->verifi_code, 500);
				}
			}
		}


		void ddz_plugin::on_client_bid(game_client* client, ddz_desk* desk, bool is_bid)
		{
			//如果(当前桌子不在游戏状态} || 当前地主还没有就位，那么返回 || 如果不是当前玩家叫牌
			if (desk->is_running() == false || desk->get_landlord() != NULL || desk->current() != client)
				return;

			int pos_index = desk->pos_index_of(client);

#ifdef SERVER_STATUS_DEBUG
			if (pos_index == -1)
				log("848");

			assert(pos_index != -1);
#endif

			if (pos_index == -1)
				return;

			desk->active();
			desk->bid_info[pos_index] = (is_bid) ? 1 : -1;

			//##倍数是个不固定值，需要注意，防止缓冲区溢出!##
			char buffer[BUFFER_SIZE_SMALL] = { 0 };
			//std::memset(buffer, 0, sizeof(buffer));
			int mess_size = sprintf(buffer, "RBD %s %c %d", ddz_desk::POS_STRINGS[pos_index], (is_bid) ? '1' : '0', desk->multiple());
			desk->write_to_everyone(buffer);//

			if (desk->multiple() == -1)
			{
				if (is_bid)
				{
					desk->increase_multiple();
					desk->temp_landlord = client;
					desk->set_start_pos(desk->get_next_pos_index());

					for (int curr_pos = desk->curr_pos(), count = 0; count < ddz_desk::DESK_CAPACITY; count++)
					{
						if (count < 2)
						{
							if (desk->bid_info[curr_pos] == -1)
							{
								desk->move_next();
							}
							else
							{
								std::memset(buffer, 0, sizeof(buffer));
								//sprintf(&buffer[mess_size], " %s %d", ddz_desk::POS_STRINGS[desk->curr_pos()], desk->multiple());
								sprintf(buffer, "BID %s %d", ddz_desk::POS_STRINGS[desk->curr_pos()], desk->multiple());
								desk->write_to_everyone(buffer);

								goto ROBOT_BID;
								//return;
							}
						}
						else
						{
							//desk->write_to_everyone(buffer);
							desk->landlord_ = desk->temp_landlord;
							++desk->verifi_code;

							this->zone_->queue_task(std::bind(&ddz_plugin::on_game_landlord, this, desk, desk->landlord_, desk->verifi_code), 500);
							//废弃this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_game_landlord), desk, desk->landlord_, desk->verifi_code, 500);
							return;
						}
					}
				}
				else
				{
					if (desk->next() == desk->first())
					{
						//desk->write_to_everyone(buffer);
						//如果没有人明牌过
						if (desk->declare_count() == 0)
						{
							++desk->verifi_code;
							this->zone_->queue_task(std::bind(&ddz_plugin::on_game_start, this, desk, desk->verifi_code), 500);
							//废弃this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_game_start), desk, NULL, desk->verifi_code, 500);
						}
						else
						{
							desk->landlord_ = desk->pos_client(desk->bid_pos());
							++desk->verifi_code;

							this->zone_->queue_task(std::bind(&ddz_plugin::on_game_landlord, this, desk, desk->landlord_, desk->verifi_code), 500);
							//废弃this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_game_landlord), desk, desk->landlord_, desk->verifi_code, 500);
						}
						return;
					}
					else
					{
						desk->move_next();
						std::memset(buffer, 0, sizeof(buffer));
						sprintf(buffer, "BID %s %d", ddz_desk::POS_STRINGS[desk->curr_pos()], desk->multiple());
						desk->write_to_everyone(buffer);

						goto ROBOT_BID;
					}
				}
			}
			else
			{
				if (is_bid)
				{
					desk->increase_multiple();
					desk->temp_landlord = client;
				}

				for (int i = 0; i < 2; i++)
				{
					if (desk->next() == desk->first())
					{
						desk->landlord_ = desk->temp_landlord;
						//回应代码
						//desk->write_to_everyone(buffer);
						++desk->verifi_code;

						this->zone_->queue_task(std::bind(&ddz_plugin::on_game_landlord, this, desk, desk->landlord_, desk->verifi_code), 500);
						//this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_game_landlord), desk, desk->landlord_, desk->verifi_code, 500);
						return;
					}
					else
					{
						desk->move_next();

						if (desk->bid_info[desk->curr_pos()] == -1)
						{
							continue;
						}
						else
						{
							if (desk->current() == desk->temp_landlord)
							{
								desk->landlord_ = desk->temp_landlord;
								//desk->write_to_everyone(buffer);
								++desk->verifi_code;

								this->zone_->queue_task(std::bind(&ddz_plugin::on_game_landlord, this, desk, desk->landlord_, desk->verifi_code), 500);

								//废弃this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_game_landlord), desk, desk->landlord_, desk->verifi_code, 500);

								return;
							}
							else
							{
								std::memset(buffer, 0, sizeof(buffer));
								sprintf(buffer, "BID %s %d", ddz_desk::POS_STRINGS[desk->curr_pos()], desk->multiple());
								desk->write_to_everyone(buffer);

								goto ROBOT_BID;
								//return;
							}
						}
					}
				}
			}
			return;


		ROBOT_BID:
			ddz_game_info* curr_game_info = (ddz_game_info*)desk->current()->get_game_info();

#ifdef SERVER_STATUS_DEBUG
			assert(curr_game_info != NULL);
#endif

			if (curr_game_info->is_robot_mode())
			{
				desk->current()->simulate_on_command("BID 1", true);
			}
		}


		//当用户出发叫牌命令
		void ddz_plugin::on_client_bid_handle(game_client* client, command* cmd)
		{
			if (cmd->param_size() != 1)
				return;

			bool is_bid = (std::strcmp(cmd->params(0), "1") == 0);
			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();
			//if(game_info->is_robot_mode())
			//	return;

			ddz_desk* desk = game_info->get_desk();
			if (desk == NULL)
				return;
			//thread_status::log("start->ddz_plugin::on_client_bid_handle.desk_status");
			std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());
			//thread_status::log("end->ddz_plugin::on_client_bid_handle.desk_status");
			this->on_client_bid(client, desk, is_bid);
		}



		void ddz_plugin::on_client_card_handle(game_client* client, command* cmd)
		{
			if (cmd->param_size() <= 0)
				return;

			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();
			////如果是机器人模式
			//if(game_info->is_robot_mode())
			//	return;

			ddz_desk* desk = game_info->get_desk();

#ifdef SERVER_STATUS_DEBUG
			if (desk == NULL)
				log("1005");

			assert(desk != NULL);
#endif

			if (desk == NULL)
				return;
			//thread_status::log("start->ddz_plugin::on_client_card_handle.desk_status");
			std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());
			//thread_status::log("end->ddz_plugin::on_client_card_handle.desk_status");

			if (desk->is_running() == false || desk->landlord_ == NULL)
			{
				return;
			}

			int pos_index = desk->pos_index_of(client);

#ifdef SERVER_STATUS_DEBUG
			if (pos_index == -1)
				log("1020");

			assert(pos_index != -1);
#endif

			if (pos_index == -1)
				return;

			if (desk->current() != client)
			{
				//可能因为网络延迟，牌面会被退回
				if (std::strcmp(cmd->params(0), "0") != 0)
				{
					this->on_client_card_refuse(client, cmd);
				}
				return;
			}

			//如果玩家是要过牌
			if (cmd->param_size() == 1 && std::strcmp(cmd->params(0), "0") == 0)
			{
				//如果是游戏的第一把牌，或者是自己带牌权出牌， 是不允许“过牌”的。
				if (desk->curr_shower_ == NULL || desk->curr_shower_ == desk->current())
				{
					char buffer[32] = { 0 };
					sprintf(buffer, "NOT %s", client->id());
					client->write(buffer);
					return;
				}

				if (desk->next() == desk->curr_shower_)
				{
					desk->curr_poker_.zero();
				}

				this->on_client_show_poker(client, desk, cmd);
			}
			else
			{
				char poker_chars[21] = { 0 };

				bool all_finded = true;
				for (int i = 0; i < cmd->param_size(); i++)
				{
					//检查是否为正确的牌型
					if (poker_parser::is_poker(cmd->params(i)) == false)
					{
						return;
					}

					//检查该玩家是否拥有这些牌面
					pos_poker_list::iterator poker_list = desk->pos_pokers(pos_index)->find(cmd->params(i));
					if (poker_list == desk->pos_pokers(pos_index)->end())
					{
						all_finded = false;
						break;
					}

					//将牌面值记录到char[]中，供后续的parser使用
					poker_chars[i] = cmd->params(i)[0];
				}

				if (all_finded == false)
				{
					this->on_client_card_refuse(client, cmd);
					return;
				}

				poker_info pokerinfo;
				poker_parser::parse(poker_chars, cmd->param_size(), pokerinfo);

				if (pokerinfo.type == poker_info::ERR)
				{
					this->on_client_card_refuse(client, cmd);
					return;
				}

				if (poker_parser::is_large(pokerinfo, desk->curr_poker_) == false)
				{
					this->on_client_card_refuse(client, cmd);
					return;
				}

				switch (pokerinfo.type)
				{
				case poker_info::BOMB:
					++desk->bomb_count_;
					break;

				case poker_info::DOUBLE_JOKER:
					break;
				}

				desk->curr_poker_ = pokerinfo;
				desk->curr_shower_ = client;

				this->on_client_show_poker(client, desk, cmd);
			}
		}



		void ddz_plugin::on_client_card_refuse(game_client* client, command* cmd)
		{
			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();

			ddz_desk* desk = game_info->get_desk();

			pos_poker_list* pokers = desk->pos_pokers(desk->pos_index_of(client));

			//如果送过来的牌面大于20张，说明是有问题的
			if (cmd->param_size() > 20)
				return;

			char buffer[BUFFER_SIZE_LARGE] = { 0 };
			//std::memset(buffer, 0, sizeof(buffer));

			int p_size = std::sprintf(buffer, "RFS");

			for (int i = 0; i < cmd->param_size(); i++)
			{
				//要检查玩家送过来的牌面信息是否正确，否则如果伪造，这里容易缓冲区溢出
				if (poker_parser::is_poker(cmd->params(i)) == false)
					return;

				p_size += std::sprintf(&buffer[p_size], " %s", cmd->params(i));
			}
			client->write(buffer);
		}


		//
		void ddz_plugin::on_robot_card(ddz_desk* desk, game_client* client, int verify_code)
		{
			//ddz_desk* desk = (ddz_desk*)arg_desk;
			std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());

			if (desk->is_running() == false)
				return;

			//	game_client* client = (game_client*)arg_client;

			if (desk->current() != client || desk->current() == NULL)
				return;

			int client_pos = desk->pos_index_of(client);

			//如果未找到该玩家
			if (client_pos == -1)
				return;

			//如果扑克牌为空
			if (desk->pos_pokers(client_pos)->size() == 0)
				return;

			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();

#ifdef SERVER_STATUS_DEBUG
			assert(game_info->verify_code == verify_code);
#endif

			if (game_info == NULL
				|| game_info->is_robot_mode() == false
				|| game_info->verify_code != verify_code)
				return;


			//if(desk->current() == client)
			//{
			poker_array pokers_finded;
			poker_finder::find_big_pokers(desk->curr_poker_, *desk->pos_pokers(client_pos), pokers_finded);

			if (pokers_finded.size() > 0)
			{
				char buffer[BUFFER_SIZE_MAX] = { 0 };
				int n = sprintf(buffer, "CAD");

				for (int i = 0; i < pokers_finded.size(); i++)
				{
					n += sprintf(&buffer[n], " %s", pokers_finded.at(i));
				}

				client->simulate_on_command(buffer, false);
			}
			else
			{
				client->simulate_on_command("CAD 0", true);
			}
			/*	}*/
		}



		void ddz_plugin::on_client_msg_handle(game_client* client, command* cmd)
		{
			if (cmd->param_size() != 2)
				return;

			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();
			ddz_desk* desk = game_info->get_desk();

			//如果是正常消息
			if (std::strcmp(cmd->params(0), "0") == 0)
			{
				if (desk != NULL)
				{
				}
				else
				{
				}
			}
			else if (std::strcmp(cmd->params(0), "1") == 0)
			{

			}
		}



		void ddz_plugin::on_client_ping_handle(game_client* client, command* cmd)
		{
			//用户发送PNG命令时将被调用
		}


		void ddz_plugin::on_client_robot_handle(game_client* client, command* cmd)
		{
			if (cmd->param_size() != 1)
			{
				return;
			}

			int mode = std::atoi(cmd->params(0));

			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();
			ddz_desk* desk = game_info->get_desk();
			std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());

			if (desk->is_running() == false)
				return;

			int client_pos = desk->pos_index_of(client);

			if (client_pos == -1)
				return;


			//RBT index ret
			char buffer[8];
			buffer[7] = 0;

			if (mode == 1 && game_info->is_robot_mode() == false)
			{
				game_info->is_robot_mode_ = true;
				sprintf(buffer, "RBT %s %d", ddz_desk::POS_STRINGS[client_pos], mode);
				desk->write_to_everyone(buffer);

				if (desk->landlord_ == NULL)
				{
					if (desk->current() == client)
					{
						client->simulate_on_command("BID 1", true);
					}
				}
				else
				{
					if (desk->current() == client)
					{
						this->on_robot_card(desk, client, game_info->verify_code);
					}
				}
				//if(desk->current() == client)
				//{
				//	this->zone_->queue_task(this, make_plugin_handle(ddz_plugin::on_robot_card), desk, client, 0, 500);
				//}
			}
			else if (mode == 0 && game_info->is_robot_mode())
			{
				//重置verify_code，使目前已经投递的延迟请求无效。
				game_info->verify_code = 0;
				game_info->is_robot_mode_ = false;
				sprintf(buffer, "RBT %s %d", ddz_desk::POS_STRINGS[client_pos], mode);
				desk->write_to_everyone(buffer);
			}
		}


		//client 离开，会发送BYE命令；
		//BYE命令会先到on_command中，先触发ddz_plugin中对BYE命令已经注册的handle，也就是on_client_bye_handle
		//然后，在on_command写，特殊的对BYE做了处理，他会调用remove_client函数
		void ddz_plugin::on_client_bye_handle(game_client* client, command* cmd)
		{
			//printf("ddz_plugin::on_client_bye_handle:%s", client->id());
			ddz_game_info* game_info = (ddz_game_info*)client->get_game_info();
			int reason = (cmd->param_size() > 0) ? std::atoi(cmd->params(0)) : service_error::CLIENT_EXIT;

			ddz_desk* desk = game_info->get_desk();
			if (desk != NULL)
			{
				std::lock_guard<std::recursive_mutex> lock(desk->status_mutex());
				this->on_client_leave_desk(client, desk, client->error_code());
			}
		}

		bool ddz_plugin::need_update_offline_client(game_client* c, string& serverURL, string& path)
		{
			return false;
			serverURL = "127.0.0.1";
			char c_buf[64] = { 0 };
			sprintf(c_buf, "/offline.aspx?id={%s}", c->id());
			path = c_buf;
			return true;
		}
	}
}
