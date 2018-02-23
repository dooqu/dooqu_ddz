#ifndef __DDZ_PLUGIN_H__
#define __DDZ_PLUGIN_H__
#define _CRT_SECURE_NO_DEPRECATE 1

#include <iostream>
#include <cstring>
#include <vector>
#include <boost/pool/singleton_pool.hpp>
#include "service/game_plugin.h"
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

class dooqu_service::basic::ws_service;
class dooqu_service::basic::ws_client;

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
    ddz_desk_list desks_;
    int max_waiting_duration_;
    virtual void on_load();
    virtual void on_unload();
    virtual void on_update();

    virtual int on_befor_client_join(ws_client* client);
    virtual void on_client_join(ws_client* client);
    virtual void on_client_leave(ws_client* client, int code);
    virtual void on_client_join_desk(ws_client* client, ddz_desk* desk, int pos_index);
    virtual void on_client_leave_desk(ws_client* client, ddz_desk* desk, int reaspon);
    virtual void on_client_bid(ws_client* client, ddz_desk* desk, bool is_bid);
    virtual void on_client_show_poker(ws_client* client, ddz_desk* desk, command* cmd);
    virtual void on_client_card_refuse(ws_client* client, command*);
    virtual void on_game_start(ddz_desk*, int);
    virtual void on_game_bid(ddz_desk*, int);
    virtual void on_game_landlord(ddz_desk*, ws_client*, int);
    virtual void on_robot_card(ddz_desk*, ws_client*, int);

    virtual void on_game_stop(ddz_desk* desk, ws_client* client, bool normal);
    virtual bool need_update_offline_client(ws_client*, string&, string&);

    //client handle
    virtual void on_join_desk_handle(ws_client*, command*);
    virtual void on_client_ready_handle(ws_client*, command*);
    virtual void on_client_bid_handle(ws_client*, command*);
    virtual void on_client_card_handle(ws_client*, command*);
    virtual void on_client_msg_handle(ws_client*, command*);
    virtual void on_client_ping_handle(ws_client*, command*);
    virtual void on_client_robot_handle(ws_client*, command*);
    virtual void on_client_declare_handle(ws_client*, command*);
    virtual void on_client_bye_handle(ws_client*, command*);

public:
    ddz_plugin(ws_service*, char* game_id, char* title, int capacity);
    virtual ~ddz_plugin();
    virtual void config(plugin_config_map& configs);
    const static char* POS_STRINGS[3];
};
}
}

using namespace dooqu_service::service;
using namespace dooqu_server::ddz;

extern "C"
{
    game_plugin* create_plugin(ws_service* service,
                               char* game_id,
                               char* name,
                               int capacity)
    {

        ddz_plugin* plugin = new ddz_plugin(service, game_id, name, capacity);
        return plugin;
    }

    void destroy_plugin(game_plugin* plugin)
    {
        if(plugin != NULL)
            delete plugin;
    }
}


#endif
