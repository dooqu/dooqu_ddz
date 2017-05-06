#ifndef __POKER_INFO_H__
#define __POKER_INFO_H__

namespace dooqu_server
{
	namespace ddz
	{
		struct poker_info
		{
			typedef enum
			{
				ERR = -1,
				ZERO = 0,
				SINGLE = 1,
				DOUBLE = 2,
				THREE = 3,
				THREE_ONE = 4,
				THREE_DOUBLE = 5,
				FOUR_TWO_SINGLE = 6,
				FOUR_TWO_DOUBLE = 7,
				SINGLE_CHAIN = 8,
				DOUBLE_CHAIN = 9,
				THREE_CHAIN = 10,
				TRHEE_CHAIN_SINGLE = 11,
				THREE_CHAIN_DOUBLE = 12,
				BOMB = 13,
				DOUBLE_JOKER = 14
			}  poker_type;

			int length;
			char value;
			poker_type type;

			poker_info()
			{
				this->zero();
			}

			poker_info(poker_type type, char value, int length)
			{
				this->type = type;
				this->value = value;
				this->length = length;
			}

			void set(poker_type type, char value, int length)
			{
				this->type = type;
				this->value = value;
				this->length = length;
			}

			void zero()
			{
				this->type = ZERO;
				this->value = '0';
				this->length = 0;
			}
		};
	}
}

#endif