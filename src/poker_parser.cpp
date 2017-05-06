#include "poker_parser.h"
#include <cstring>

namespace dooqu_server
{
	namespace ddz
	{

		bool poker_parser::is_poker(const char* poker_value)
		{
			if(std::strlen(poker_value) != 2)
				return false;

			if(poker_value[0] < 'A' || poker_value[0] > 'O')
				return false;

			if(poker_value[1] < '1' || poker_value[1] > '4')
				return false;

			if((poker_value[0] == 'N' || poker_value[0] == 'O') && (poker_value[1] != '1' ))
				return false;

			return true;
		}

		//是否是单顺
		bool poker_parser::is_single_chain(char cards[], int size, poker_info& pi)
		{
			if (size < 5)
				return false;

			for (int i = 0; i < size; i++)
			{
				if (cards[i] >= 'M')
					return false;

				if (i > 0 && (cards[i] - cards[i - 1]) != 1)
				{
					return false;
				}
			}

			pi.set(poker_info::SINGLE_CHAIN, cards[0], size);
			return true;	
		}

		bool poker_parser::is_double_chain(char cards[], int size, poker_info& pi)
		{
			if ((size % 2) != 0 || size < 6)
				return false;

			for (int i = 1; i < size; i++)
			{
				if (cards[i] >= 'M')
					return false;

				if ((i % 2) != 0)
				{
					if (cards[i] != cards[i - 1])
						return false;
				}
				else
				{
					if ((cards[i] - cards[i - 1]) != 1)
						return false;
				}
			}

			pi.set(poker_info::DOUBLE_CHAIN, cards[0], size);
			return true;
		}


		bool poker_parser::is_three_chain(char cards[], int size, poker_info& pi)
		{
			char v3[] = {'0', '0', '0', '0', '0', '0'};
			char vx[] ={'0', '0', '0', '0', '0', '0'};

			int v3Pos = 0;
			int vxPos = 0;

			int sameNum = 1;
			int currNum = 1;

			int v3len = 6;
			int vxlen = 6;

			int countLen = size;

			for (int i = 1; i < countLen; i++)
			{
				//Console.WriteLine("i={0},{1}", i, cards[i]);

				if (v3Pos >= v3len || vxPos >= vxlen)
				{
					//Console.WriteLine("over");
					return false;
					//isError = true;
					//break;
				}

				if (cards[i] == cards[i - 1])
				{
					++sameNum;

					if (i < (countLen - 1))
					{
						//如果不是最后一个元素，就重新循环;
						continue;
					}
				}

				//Console.WriteLine("和上一个不相同");

				switch (sameNum)
				{
				case 4:
					{
						return false;
						//Console.WriteLine("找到一个4个的" + cards[i - 1]);
						//设定好3个的，赶紧处理一个的
						if (currNum != 1 && vxPos != 0)
						{
							//Console.WriteLine("小张数目不对");
							return false;
							//isError = true;
							//break;
						}
						vx[vxPos++] = cards[i - 1];
						goto CASE3;
					}
					//break;
CASE3:
				case 3:
					{
						v3[v3Pos++] = cards[i - 1];
						sameNum = 1;
					}
					break;
				default:
					{
						if (sameNum > 4)
						{
							// v3Pos = 0;
							//isError = true;
							//break;
							return false;
						}

						if (sameNum != currNum)
						{
							if (vxPos == 0)
							{
								currNum = sameNum;
								vx[vxPos++] = cards[i - 1];
								//Console.WriteLine("重新设定single值为" + currNum.ToString());
								//Console.WriteLine(cards[i - 1] + "入VX");
							}
							else
							{
								//Console.WriteLine("error1");
								return false;
								//isError = true;
								//break;
							}
						}
						else
						{
							vx[vxPos++] = cards[i - 1];
							//Console.WriteLine(cards[i - 1] + "入VX");
						}

						sameNum = 1;
					}
					break;
				}

				if (i == (countLen - 1) && cards[i] != cards[i - 1])
				{
					//Console.WriteLine("最后一个不同");
					if (currNum != 1)
					{
						//Console.WriteLine("最后一个数目不对" + currNum);
						return false;
					}
					vx[vxPos++] = cards[i];
				}
			}

			//for (int i = 0; i < v3Pos; i++)
			//    Console.WriteLine("v3[{0}]={1}", i, v3[i]);


			//for (int i = 0; i < vxPos; i++)
			//    Console.WriteLine("vx[{0}]={1}", i, vx[i]);

			if (v3Pos > 1)
			{
				for (int i = 1; i < v3Pos; i++)
				{
					if ((v3[i] - v3[i - 1]) != 1)
						return false;

					if (v3[i] >= 'M')
						return false;
				}

				if (v3Pos == vxPos)
				{
					if (currNum == 2)
						pi.type = poker_info::THREE_CHAIN_DOUBLE;
					else if (currNum == 1)
						pi.type = poker_info::TRHEE_CHAIN_SINGLE;
				}
				else if (vxPos == 0)
				{
					pi.type = poker_info::THREE_CHAIN;
				}

				pi.length = size;
			}

			if (pi.type != poker_info::ERR)
				pi.value = v3[0];

			return pi.type != poker_info::ERR;
		}

	bool poker_parser::is_three_or_four_and_single_or_double(char cards[], int size, poker_info& pi)
		{
			//Array.Sort(cards);
			//Console.WriteLine(cards);

			//主牌值
			char mainVal = '\0';
			//主牌的数(3or4)
			int mainVals = 0;

			//零牌的总数
			int incidentalValNum = 0;

			//当前零牌的单双数(1 or 2);
			int incidentalVals = 1;

			int sameNum = 1;

			int cardsLen = size;

			for (int i = 1; i < cardsLen; i++)
			{
				//Console.WriteLine("i={0},{1}", i, cards[i]);

				if (cards[i] == cards[i - 1])
				{
					++sameNum;

					if (i < (cardsLen - 1))
					{
						//如果不是最后一个元素，就重新循环;
						continue;
					}
				}

				// Console.WriteLine("计算相同数:" + sameNum.ToString());

				switch (sameNum)
				{
				case 4:
					{
						if (mainVal != '\0')
							return false;

						mainVal = cards[i - 1];

						mainVals = 4;

						sameNum = 1;
					}
					break;

				case 3:
					{
						if (mainVal != '\0')
							return false;

						mainVal = cards[i - 1];

						mainVals = 3;

						sameNum = 1;
					}
					break;

				default:
					{
						if (sameNum > 4)
						{
							return false;
						}

						//剩下就是 1 和 2的情况
						//如果sameNum 和 当前的平均副牌数不同
						if (sameNum != incidentalVals)
						{
							//incidentalVals == 0 也就是第一次找到一个副牌
							if (incidentalValNum == 0)
							{
								//设定副牌的平均数
								incidentalVals = sameNum;
								//增加副牌总数
								incidentalValNum++;
								//Console.WriteLine("重新设定1");
							}
							else
							{
								//否则的话，说明出现了新的数目的副牌，错误。
								return false;
							}
						}
						else
						{
							//增加副牌总数
							incidentalValNum++;
						}

						//重新设定sameNum;
						sameNum = 1;
					}
					break;
				} //end switch.


				if (i == (cardsLen - 1) && cards[i] != cards[i - 1])
				{
					//Console.WriteLine("最后一个不同");
					if (incidentalVals != 1)
					{
						// Console.WriteLine("最后一个数目不对");
						return false;
					}
					incidentalValNum++;
				}
			}

			if (mainVals == 3)
			{
				if (incidentalValNum == 0)
				{
					pi.type = poker_info::THREE;
				}
				else if (incidentalValNum == 1)
				{
					if (incidentalVals == 1)
					{
						pi.type = poker_info::THREE_ONE;
					}
					if (incidentalVals == 2)
					{
						pi.type = poker_info::THREE_DOUBLE;
					}
				}
			}
			else if (mainVals == 4)
			{
				if (incidentalValNum == 0)
				{
					pi.type = poker_info::BOMB;
				}
				else if (incidentalValNum == 1)
				{
					if (incidentalVals == 2)
					{
						pi.type = poker_info::FOUR_TWO_SINGLE;
					}
				}
				else if (incidentalValNum == 2)
				{
					if (incidentalVals == 1)
					{
						pi.type = poker_info::FOUR_TWO_SINGLE;
					}
					else if (incidentalVals == 2)
					{
						pi.type = poker_info::FOUR_TWO_DOUBLE;
					}
				}
			}

			if (pi.type != poker_info::ERR)
			{
				pi.value = mainVal;

				pi.length = cardsLen;
			}

			return pi.type != poker_info::ERR;
		}

		void poker_parser::sortCards(char* cards, int size)
		{
			for(int i = 0; i < size - 1; i++)     
			{         
				for(int j = 0; j < size - i - 1; j++)         
				{				
					if(cards[j] > cards[j + 1])  
					{              
						char temp = cards[j];
						cards[j] = cards[j + 1];
						cards[j + 1] = temp;        
					}        
				}     
			}  
		}

		bool poker_parser::is_large(poker_info& pi1, poker_info& pi2)
		{
			if (pi1.type == pi2.type)
			{
				return (pi1.value > pi2.value) && (pi1.length == pi2.length);
			}
			else
			{
				if (pi2.type == poker_info::ZERO && pi1.type != poker_info::ERR)
				{
					return true;
				}

				if (pi1.type == poker_info::DOUBLE_JOKER)
				{
					return true;
				}
				else if (pi1.type == poker_info::BOMB && pi2.type != poker_info::DOUBLE_JOKER)
				{
					return true;
				}
			}
			return false;
		}

		poker_info& poker_parser::parse(char* cards, int size, poker_info& pi)
		{
			pi.set(poker_info::ERR, '0', 0);

			if(size < 0 || size >= 20)
				return pi;

			poker_parser::sortCards(cards, size);

			switch(size)
			{
			case 1:
				pi.set(poker_info::SINGLE, cards[0], 1);
				break;

			case 2:
				{
					if(cards[0] == cards[1])
					{
						pi.set(poker_info::DOUBLE, cards[0], 2);
					}
					else if(cards[0] == 'N' && cards[1] == 'O')
					{
						pi.set(poker_info::DOUBLE_JOKER, cards[0], 2);
					}
				}
				break;
			case 3:
				{
					if(cards[0] == cards[1] && cards[1] == cards[2] && cards[0] < 'N')
					{
						pi.set(poker_info::THREE, cards[0], 3);
					}
				}
				break;

			case 4:
				{
					if(poker_parser::is_three_or_four_and_single_or_double(cards, size, pi))
						return pi;
				}
				break;

			case 5:
				{
					if(poker_parser::is_single_chain(cards, size, pi))
					{
						return pi;
					}
					else if(poker_parser::is_three_or_four_and_single_or_double(cards, size, pi))
					{
						return pi;
					}
				}
				break;

			case 6:
				{
					if(poker_parser::is_single_chain(cards, size, pi))
					{
						return pi;
					}
					else if(poker_parser::is_double_chain(cards, size, pi))
					{
						return pi;
					}
					else if(poker_parser::is_three_chain(cards, size, pi))
					{
						return pi;
					}
					else if(poker_parser::is_three_or_four_and_single_or_double(cards, size, pi))
					{
						return pi;
					}
				}
				break;

			case 7:
				{
					if(poker_parser::is_single_chain(cards, size, pi))
					{
						return pi;
					}
				}
				break;

			case 8:
				{
					if(poker_parser::is_single_chain(cards, size, pi))
					{
						return pi;
					}
					else if(poker_parser::is_double_chain(cards, size, pi))
					{
						return pi;
					}
					else if(poker_parser::is_three_or_four_and_single_or_double(cards, size, pi))
					{
						return pi;
					}
					else if(poker_parser::is_three_chain(cards, size, pi))
					{
						return pi;
					}
				}
				break;

			default:
				{
					if(poker_parser::is_three_chain(cards, size, pi))
					{
						return pi;
					}
					else if(poker_parser::is_single_chain(cards, size, pi))
					{
						return pi;
					}
					else if( (size % 2) == 0 && poker_parser::is_double_chain(cards, size, pi))
					{
						return pi;
					}
				}
				break;
			}

			return pi;
		}
	}
}