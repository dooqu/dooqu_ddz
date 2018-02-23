#ifndef __POKER_PARSER_H__
#define __POKER_PARSER_H__

#include "poker_info.h"

namespace dooqu_server
{
	namespace ddz
	{
		class poker_parser
		{
		protected:
			static bool is_single_chain(char cards[], int size, poker_info& pi);
			static bool is_double_chain(char cards[], int size, poker_info& pi);
			static bool is_three_chain(char cards[], int size, poker_info& pi);
			static bool is_three_or_four_and_single_or_double(char cards[], int size, poker_info& pi); 
		public:
			static poker_info& parse(char cards[], int size, poker_info& pi);
			static void sortCards(char* cards, int size);
			static bool is_large(poker_info& pi1, poker_info& pi2);
			static bool is_poker(const char* poker_value);
		};
	}
}

#endif