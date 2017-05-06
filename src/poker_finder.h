#ifndef __POKER_FINDER_H__
#define __POKER_FINDER_H__

#include <cstdlib>
#include <cstring>
#include <set>
#include <map>
#include <vector>
#include <assert.h>
#include "poker_info.h"
#include "util/char_key_op.h"

namespace dooqu_server
{
	namespace ddz
	{
		//using namespace dooqu_service::service;
		using namespace dooqu_service::util;

		typedef std::vector<const char*> poker_array;
		//typedef std::vector<std::vector<poker_array*>*> archived_pokers;
		typedef std::set<const char*, char_key_op> poker_list;

		class archived_pokers
		{
			enum{ARCHIVED_TYPE_NUM = 4};
		protected:
			std::vector<poker_array> archived_list[ARCHIVED_TYPE_NUM];
		public:
			inline std::vector<poker_array>* at(int p_index)
			{
				assert(p_index < ARCHIVED_TYPE_NUM && p_index >= 0);
				return &archived_list[p_index];
			}
			int size()
			{
				return ARCHIVED_TYPE_NUM;
			}
		};

		class poker_finder
		{
		public:
			static poker_array* find_type_value(archived_pokers& hash_pokers, int type_num, char value);
			static void find_single_chain_inner(poker_array& user_poker_list, char poker_value, int length, poker_array& finded_poker_list);
			static void find_double_chain_inner(poker_array& user_poker_list, char poker_value, int length, poker_array& finded_poker_list);
			//static void release_archived_array(archived_pokers& pokers_archived);
			static void release_three_types_map(std::map<const char*, poker_array*, char_key_op>& poker_map);
			static void archive_poker_list(poker_list& user_poker_list, archived_pokers& archived_pokers);
		public:
			static void find_single(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list);
			static void find_double(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list);
			static void find_three(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list);
			static void find_three_and_single(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list);
			static void find_three_and_double(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list);
			static void find_four(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list);
			static void find_four_and_two_single(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list);
			static void find_four_and_two_double(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list);
			static void find_single_chain(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list);
			static void find_double_chain(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list);
			static void find_three_chain(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list);
			static void find_three_chain_and_single(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list);
			static void find_three_chain_and_double(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list);
			static void find_double_joker(poker_list& user_poker_list, poker_array& finded_poker_list);
			static void find_bomb_or_double_joker(poker_list& user_poker_list, poker_array& finded_poker_list);
			static void find_big_pokers(poker_info& pi, poker_list& user_poker_list, poker_array& finded_poker_list);

		};
	}
}

#endif
