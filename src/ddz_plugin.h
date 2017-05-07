#ifndef __DDZ_PLUGIN_H__
#define __DDZ_PLUGIN_H__
#define _CRT_SECURE_NO_DEPRECATE 1

#include <iostream>
#include <cstring>
#include <vector>
#include <boost/pool/singleton_pool.hpp>
#include "dooqu_service.h"
#include "ddz_desk.h"
#include "poker_parser.h"
#include "poker_info.h"
#include "poker_finder.h"
#include "ddz_game_info.h"
#include "service_error.h"



namespace dooqu_server
{
	namespace ddz
	{
		using namespace dooqu_service::service;

		class dooqu_service::service::game_service;
		class dooqu_service::service::game_client;

		enum
		{
			SIZE_OF_CLIENT_ID_MAX = 10,
			SIZE_OF_CLIENT_NAME_NAME = 32,
			BUFFER_SIZE_SMALL = 16,
			BUFFER_SIZE_MIDDLE = 32,
			BUFFER_SIZE_LARGE = 64,
			BUFFER_SIZE_MAX = 128
		};


		class ddz_plugin : public game_plugin
		{
		protected:
			int desk_count_;
			int multiple_;
			//std::vector<ddz_desk*> desks_;
			ddz_desk_list<100> desks_;
			//boost::asio::deadline_timer timer_;
			int max_waiting_duration_;
			virtual void on_load();
			virtual void on_unload();
			virtual void on_update();
			virtual int on_befor_client_join(game_client* client);
			virtual void on_client_join(game_client* client);
			virtual void on_client_leave(game_client* client, int code);
			virtual void on_client_join_desk(game_client* client, ddz_desk* desk, int pos_index);
			virtual void on_client_leave_desk(game_client* client, ddz_desk* desk, int reaspon);
			virtual void on_client_bid(game_client* client, ddz_desk* desk, bool is_bid);
			virtual void on_client_show_poker(game_client* client, ddz_desk* desk, command* cmd);
			virtual void on_client_card_refuse(game_client* client, command*);
			virtual void on_game_start(ddz_desk*, int);
			virtual void on_game_bid(ddz_desk*, int);
			virtual void on_game_landlord(ddz_desk*, game_client*, int);
			virtual void on_robot_card(ddz_desk*, game_client*, int);

			virtual void on_game_stop(ddz_desk* desk, game_client* client, bool normal);
			virtual bool need_update_offline_client(game_client*, string&, string&);

			//client handle
			virtual void on_join_desk_handle(game_client*, command*);
			virtual void on_client_ready_handle(game_client*, command*);
			virtual void on_client_bid_handle(game_client*, command*);
			virtual void on_client_card_handle(game_client*, command*);
			virtual void on_client_msg_handle(game_client*, command*);
			virtual void on_client_ping_handle(game_client*, command*);
			virtual void on_client_robot_handle(game_client*, command*);
			virtual void on_client_declare_handle(game_client*, command*);
			virtual void on_client_bye_handle(game_client*, command*);

		public:
			ddz_plugin(game_service*, char* game_id, char* title, double frequence, int desk_count, int game_multiple, int max_waiting_duration);
			virtual ~ddz_plugin();
			const static char* POS_STRINGS[3];

			typedef boost::singleton_pool<ddz_game_info, sizeof(ddz_game_info)> ddz_game_info_pool;
		};
	}
}

using namespace dooqu_service::service;
using namespace dooqu_server::ddz;

extern "C"
{
     game_plugin* create_plugin(game_service* service,
                  char* game_id,
                  char* name,
                  int frequence,
                  int capacity,
                  std::map<const char*, const char*,char_key_op>* configs)
    {

        ddz_plugin* plugin = new ddz_plugin(service, game_id, name, frequence, capacity, 15, 21000);
        plugin->config(configs);
        return plugin;
    }

}


#endif
