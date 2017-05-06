#include "poker_finder.h"
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <set>
#include <iterator>
#include <memory>
#include <assert.h>


namespace dooqu_server
{
	namespace ddz
	{
		void poker_finder::archive_poker_list(poker_list& user_poker_list, archived_pokers& archived_pokers)
		{
			//先升序排列所有牌面
			//std::sort(user_poker_list.begin(), user_poker_list.end(), char_key_op());
			//创建一个数组， 分门别类的存储数据
			//CCArray* hash = CCArray::create();
			//从0-3， 0对应single，1对应double，2对应three，3对应bomb
			//for(unsigned int i = 0; i < 4; i++)
			//{
			//	archived_pokers.push_back(new std::vector<poker_array*>());
			//}

			//if(user_poker_list.size() <= 0)
			//{
			//	return;
			//}

			const char* poker_value = *user_poker_list.begin();
			int curr_type_num = 0;
			int size = user_poker_list.size();
			poker_array type_pokers;
			type_pokers.push_back(poker_value);

			if (user_poker_list.size() == 1)
			{
				archived_pokers.at(0)->push_back(type_pokers);
				return;
			}

			poker_list::iterator curr_poker = user_poker_list.begin();
			++curr_poker;
			poker_list::iterator last_poker = user_poker_list.end();
			--last_poker;

			for (; curr_poker != user_poker_list.end(); ++curr_poker)
			{
				const char* curr_value = *curr_poker;

				if (curr_value[0] == poker_value[0])
				{
					++curr_type_num;
					type_pokers.push_back(*curr_poker);
				}
				else
				{
					archived_pokers.at(curr_type_num)->push_back(type_pokers);
					poker_value = curr_value;

					curr_type_num = 0;
					///type_pokers = new poker_array();
					type_pokers.clear();
					type_pokers.push_back(*curr_poker);
				}

				if (curr_poker == last_poker)
				{
					archived_pokers.at(curr_type_num)->push_back(type_pokers);
				}
			}
		}


		poker_array* poker_finder::find_type_value(archived_pokers& hash_pokers, int type_num, char value)
		{
			type_num -= 1;

			//assert(type_num < 0);

			if (type_num < 0)
				type_num = 0;

			if (type_num > 3)
				type_num = 3;

			std::vector<poker_array >* type_array = hash_pokers.at(type_num);

			int type_array_size = type_array->size();

			for (int i = 0; i < type_array_size; ++i)
			{
				poker_array& type_pokers = type_array->at(i);

				if (type_pokers.at(0)[0] > value)
				{
					return &type_pokers;
				}
			}

			return NULL;
		}

		void poker_finder::find_single(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < 1)
				return;

			archived_pokers pokers_archived;
			//boost::shared_ptr<void> p((void*)NULL, boost::bind(&poker_finder::release_archived_array, boost::ref(pokers_archived)));
			poker_finder::archive_poker_list(user_poker_list, pokers_archived);

			poker_array* type_pokers = poker_finder::find_type_value(pokers_archived, 1, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				return;
			}

			type_pokers = poker_finder::find_type_value(pokers_archived, 2, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				return;
			}

			type_pokers = poker_finder::find_type_value(pokers_archived, 3, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				return;
			}

			type_pokers = poker_finder::find_type_value(pokers_archived, 4, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				return;
			}
			//finded_poker_list.push_back(*user_poker_list.begin());
		}


		void poker_finder::find_double(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < 2)
				return;

			archived_pokers pokers_archived;

			//boost::shared_ptr<void> p((void*)NULL, boost::bind(&poker_finder::release_archived_array, boost::ref(pokers_archived)));
			poker_finder::archive_poker_list(user_poker_list, pokers_archived);

			poker_array* type_pokers = poker_finder::find_type_value(pokers_archived, 2, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				finded_poker_list.push_back(type_pokers->at(1));
				return;
			}

			type_pokers = poker_finder::find_type_value(pokers_archived, 3, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				finded_poker_list.push_back(type_pokers->at(1));
				return;
			}

			type_pokers = poker_finder::find_type_value(pokers_archived, 4, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				finded_poker_list.push_back(type_pokers->at(1));
				return;
			}
		}

		void poker_finder::find_three(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < 3)
				return;

			archived_pokers pokers_archived;
			//boost::shared_ptr<void> p((void*)NULL, boost::bind(&poker_finder::release_archived_array, boost::ref(pokers_archived)));
			poker_finder::archive_poker_list(user_poker_list, pokers_archived);

			poker_array* type_pokers = poker_finder::find_type_value(pokers_archived, 3, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				finded_poker_list.push_back(type_pokers->at(1));
				finded_poker_list.push_back(type_pokers->at(2));
				return;
			}

			type_pokers = poker_finder::find_type_value(pokers_archived, 4, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				finded_poker_list.push_back(type_pokers->at(1));
				finded_poker_list.push_back(type_pokers->at(2));
				return;
			}
		}

		void poker_finder::find_three_and_single(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < 4)
				return;

			poker_list user_poker_list_clone(user_poker_list.begin(), user_poker_list.end());

			poker_finder::find_three(user_poker_list, poker_value, finded_poker_list);

			if (finded_poker_list.size() < 3)
			{
				finded_poker_list.clear();
				return;
			}

			for (int i = 0; i < finded_poker_list.size(); i++)
			{
				user_poker_list_clone.erase(finded_poker_list.at(i));
			}

			poker_array single_finded;
			while (true)
			{
				single_finded.clear();
				poker_finder::find_single(user_poker_list_clone, '0', single_finded);

				if (single_finded.size() < 1)
				{
					finded_poker_list.clear();
					return;
				}

				//如果找到的单牌和三张牌牌值一样，那该单牌不符合要求，继续寻找
				if (finded_poker_list.at(0)[0] == single_finded.at(0)[0])
				{
					user_poker_list_clone.erase(single_finded.at(0));
					continue;
				}
				else
				{
					finded_poker_list.push_back(single_finded.at(0));
					return;
				}
			}
		}


		void poker_finder::find_three_and_double(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < 5)
				return;

			poker_list user_poker_list_clone(user_poker_list.begin(), user_poker_list.end());

			poker_finder::find_three(user_poker_list, poker_value, finded_poker_list);

			if (finded_poker_list.size() < 3)
			{
				finded_poker_list.clear();
				return;
			}

			for (int i = 0; i < finded_poker_list.size(); i++)
			{
				user_poker_list_clone.erase(finded_poker_list.at(i));
			}

			poker_array double_finded;
			poker_finder::find_double(user_poker_list_clone, '0', double_finded);

			if (double_finded.size() < 2)
			{
				finded_poker_list.clear();
				return;
			}
			else
			{
				finded_poker_list.push_back(double_finded.at(0));
				finded_poker_list.push_back(double_finded.at(1));
			}
		}


		void poker_finder::find_four(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < 4)
				return;

			archived_pokers pokers_archived;
			//boost::shared_ptr<void> p((void*)NULL, boost::bind(&poker_finder::release_archived_array, boost::ref(pokers_archived)));
			poker_finder::archive_poker_list(user_poker_list, pokers_archived);

			poker_array* type_pokers = poker_finder::find_type_value(pokers_archived, 4, poker_value);
			if (type_pokers != NULL)
			{
				finded_poker_list.push_back(type_pokers->at(0));
				finded_poker_list.push_back(type_pokers->at(1));
				finded_poker_list.push_back(type_pokers->at(2));
				finded_poker_list.push_back(type_pokers->at(3));
				return;
			}
		}


		void poker_finder::find_four_and_two_single(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < 5)
				return;

			poker_finder::find_four(user_poker_list, poker_value, finded_poker_list);

			if (finded_poker_list.size() < 4)
			{
				finded_poker_list.clear();
				return;
			}

			poker_list user_poker_clone(user_poker_list.begin(), user_poker_list.end());

			for (int i = 0; i < finded_poker_list.size(); i++)
			{
				user_poker_clone.erase(finded_poker_list.at(i));
			}

			poker_array single_finded;
			poker_array single_pokers;

			while (true)
			{
				single_finded.clear();
				poker_finder::find_single(user_poker_clone, '0', single_finded);

				if (single_finded.size() < 1)
				{
					finded_poker_list.clear();
					return;
				}
				else
				{
					user_poker_clone.erase(single_finded.at(0));
					if (single_pokers.size() < 1)
					{
						single_pokers.push_back(single_finded.at(0));
					}
					else
					{
						if (single_pokers.at(0)[0] == single_finded.at(0)[0])
						{
							continue;
						}
						else
						{
							finded_poker_list.push_back(single_pokers.at(0));
							finded_poker_list.push_back(single_finded.at(0));
							return;
						}
					}
				}
			}
		}

		void poker_finder::find_four_and_two_double(poker_list& user_poker_list, char poker_value, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < 8)
				return;

			poker_finder::find_four(user_poker_list, poker_value, finded_poker_list);

			if (finded_poker_list.size() < 4)
			{
				finded_poker_list.clear();
				return;
			}

			poker_list user_poker_clone(user_poker_list.begin(), user_poker_list.end());

			for (int i = 0; i < finded_poker_list.size(); i++)
			{
				user_poker_clone.erase(finded_poker_list.at(i));
			}

			poker_array double_finded;
			poker_array double_pokers;

			while (true)
			{
				double_finded.clear();
				poker_finder::find_double(user_poker_clone, '0', double_finded);

				if (double_finded.size() < 2)
				{
					finded_poker_list.clear();
					return;
				}
				else
				{
					user_poker_clone.erase(double_finded.at(0));
					user_poker_clone.erase(double_finded.at(1));

					if (double_pokers.size() == 0)
					{
						double_pokers.push_back(double_finded.at(0));
						double_pokers.push_back(double_finded.at(1));
					}
					else
					{
						if (double_pokers.at(0)[0] == double_finded.at(0)[0])
						{
							continue;
						}
						else
						{
							finded_poker_list.push_back(double_pokers.at(0));
							finded_poker_list.push_back(double_pokers.at(1));
							finded_poker_list.push_back(double_finded.at(0));
							finded_poker_list.push_back(double_finded.at(1));
							return;
						}
					}
				}
			}
		}


		void poker_finder::find_single_chain_inner(poker_array& user_poker_list, char poker_value, int length, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < length)
			{
				return;
			}

			std::sort(user_poker_list.begin(), user_poker_list.end(), char_key_op());
			//先正序列排列所有牌面

			//如果排序之后，最后一个元素都没有value大，那就不用比了
			if (user_poker_list.at(user_poker_list.size() - 1)[0] <= poker_value)
			{
				return;
			}

			//从第0个开始，依次尝试查找
			for (int i = 0; (i < user_poker_list.size()) && ((user_poker_list.size() - i) >= length); i++)
			{
				if (user_poker_list.at(i)[0] > poker_value)
				{
					finded_poker_list.clear();

					for (int j = i; j < user_poker_list.size() && finded_poker_list.size() < length; j++)
					{
						if (user_poker_list.at(j)[0] >= 'M')
						{
							break;
						}

						if (finded_poker_list.size() == 0)
						{
							//如果返回值数组中是空的，那么就算这个是合规的第一张牌，先放进去
							finded_poker_list.push_back(user_poker_list.at(j));
						}
						else
						{
							int r = user_poker_list.at(j)[0] - finded_poker_list.at(finded_poker_list.size() - 1)[0];
							//如果返回的数组不为空， 那么当前这样牌应该比返回值牌中最后一张大 "1"，才对。
							if (r == 0)
							{
								continue;
							}
							else if (r == 1)
							{
								finded_poker_list.push_back(user_poker_list.at(j));
							}
							else
							{
								break;
							}
						}
					}

					if (finded_poker_list.size() == length)
					{
						return;
					}
				}
			}

			finded_poker_list.clear();
		}


		void poker_finder::find_single_chain(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < length)
			{
				return;
			}

			//将所有的牌面按类型分堆，分堆的目的是优先挑小牌型的牌组成一套能管上家的牌，尽量最后取3个和4个的牌面
			archived_pokers pokers_archived;
			//函数离开前要释放
			//boost::shared_ptr<void> p((void*)NULL, boost::bind(&poker_finder::release_archived_array, boost::ref(pokers_archived)));
			poker_finder::archive_poker_list(user_poker_list, pokers_archived);

			poker_array pokers_to_find;

			//先从小牌型开始凑拍，先把所有的单牌凑起来，看能不能组成一个合规的顺子， 再依次把对子牌、三顺牌和炸弹牌凑进来。
			//目的就是尽量的先不动大牌，尽量用小牌凑成顺子
			for (int i = 0; i < pokers_archived.size(); i++)
			{
				std::vector<poker_array>* type_pokers = pokers_archived.at(i);
				//得到当前牌型下的所有牌面：0代表单牌， 1代表对子， 2代表三个的，3代表四个的
				for (int j = 0; j < type_pokers->size(); j++)
				{
					//如果是包括对子以上的牌，那我只把第一张加进来，因为是单顺，其他牌加进来只会碍事。
					pokers_to_find.push_back(type_pokers->at(j).at(0));
				}

				if (pokers_to_find.size() < length)
				{
					continue;
				}
				else
				{
					//如果当前凑起来的牌面的长度和目标长度一致了，那么尝试去看是否已经成为了合规的顺子。
					poker_finder::find_single_chain_inner(pokers_to_find, poker_value, length, finded_poker_list);

					if (finded_poker_list.size() == length)
					{
						return;
					}
				}
			}
		}



		void poker_finder::find_double_chain_inner(poker_array& user_poker_list, char poker_value, int length, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < length)
			{
				return;
			}

			std::sort(user_poker_list.begin(), user_poker_list.end(), char_key_op());
			//先正序列排列所有牌面

			//如果排序之后，最后一个元素都没有value大，那就不用比了
			if (user_poker_list.at(user_poker_list.size() - 1)[0] <= poker_value)
			{
				return;
			}

			//创建一个用户承载返回值的数组。
			for (int i = 0; (i < user_poker_list.size()) && ((user_poker_list.size() - i) >= length); i++)
			{
				if (user_poker_list.at(i)[0] > poker_value)
				{
					finded_poker_list.clear();

					for (int j = i; j < user_poker_list.size() && finded_poker_list.size() < length; j++)
					{
						if (user_poker_list.at(j)[0] >= 'M')
						{
							break;
						}

						if (finded_poker_list.size() == 0)
						{
							//如果返回值数组中是空的，那么就算这个是合规的第一张牌，先放进去
							finded_poker_list.push_back(user_poker_list.at(j));
						}
						else
						{
							int r1 = user_poker_list.at(j)[0];
							int r2 = finded_poker_list.at(finded_poker_list.size() - 1)[0];
							//如果返回的数组不为空， 那么当前这样牌应该比返回值牌中最后一张大 "1"，才对。

							int r = r1 - r2;
							if (finded_poker_list.size() % 2 == 0)
							{
								if (r == 0)
								{
									continue;
								}
								else if (r == 1)
								{
									finded_poker_list.push_back(user_poker_list.at(j));
								}
								else
								{
									break;
								}
							}
							else
							{
								if (r == 0)
								{
									finded_poker_list.push_back(user_poker_list.at(j));
								}
								else
								{
									break;
								}
							}
						}
					}

					if (finded_poker_list.size() == length)
					{
						return;
					}
				}
			}

			finded_poker_list.clear();
		}

		void poker_finder::find_double_chain(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < length)
			{
				return;
			}

			//将所有的牌面按类型分堆，分堆的目的是优先挑小牌型的牌组成一套能管上家的牌，尽量最后取3个和4个的牌面
			archived_pokers pokers_archived;
			//boost::shared_ptr<void> p((void*)NULL, boost::bind(&poker_finder::release_archived_array, boost::ref(pokers_archived)));
			poker_finder::archive_poker_list(user_poker_list, pokers_archived);

			poker_array pokers_to_find;

			//先从小牌型开始凑拍，先把所有的单牌凑起来，看能不能组成一个合规的顺子， 再依次把对子牌、三顺牌和炸弹牌凑进来。
			//目的就是尽量的先不动大牌，尽量用小牌凑成顺子
			for (int i = 1; i < pokers_archived.size(); i++)
			{
				std::vector<poker_array>* type_pokers = pokers_archived.at(i);
				//得到当前牌型下的所有牌面：0代表单牌， 1代表对子， 2代表三个的，3代表四个的
				for (int j = 0; j < type_pokers->size(); j++)
				{
					//如果是包括对子以上的牌，那我只把第一张加进来，因为是单顺，其他牌加进来只会碍事。
					pokers_to_find.push_back(type_pokers->at(j).at(0));
					pokers_to_find.push_back(type_pokers->at(j).at(1));
				}

				if (pokers_to_find.size() < length)
				{
					continue;
				}
				else
				{
					//如果当前凑起来的牌面的长度和目标长度一致了，那么尝试去看是否已经成为了合规的顺子。
					poker_finder::find_double_chain_inner(pokers_to_find, poker_value, length, finded_poker_list);

					if (finded_poker_list.size() == length)
					{
						return;
					}
				}
			}
		}

		void poker_finder::find_three_chain(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < length)
			{
				return;
			}

			length /= 3;

			//poker_list user_poker_map(user_poker_list.begin(), user_poker_list.end());
			poker_list user_poker_list_curr(user_poker_list.begin(), user_poker_list.end());
			std::map<const char*, poker_array*, char_key_op> three_type_pokers;
			//函数离开前要对three_type_pokers进行释放
			
			boost::shared_ptr<void> p((void*)NULL, boost::bind(&poker_finder::release_three_types_map, boost::ref(three_type_pokers)));
			
			poker_array pokers_single_chain;

			while (true)
			{
				poker_array pokers_finded;
				poker_finder::find_three(user_poker_list_curr, poker_value, pokers_finded);
				if (pokers_finded.size() == 0)
				{
					return;
				}

				for (poker_array::iterator curr_poker = pokers_finded.begin();
					curr_poker != pokers_finded.end(); curr_poker++)
				{
					user_poker_list_curr.erase(*curr_poker);
				}

				if (pokers_finded.at(0)[0] < poker_value)
				{
					finded_poker_list.clear();
					continue;
				}

				if (pokers_finded.at(0)[0] > 'M')
				{
					finded_poker_list.clear();
					return;
				}

				poker_array* three_pokers = new poker_array(pokers_finded.begin(), pokers_finded.end());
				three_type_pokers.insert(make_pair(three_pokers->at(0), three_pokers));
				pokers_single_chain.push_back(three_pokers->at(0));

				if (pokers_single_chain.size() < length)
				{
					continue;
				}


				//开始对单顺进行验证
				poker_array finded_single_chain_container;
				poker_finder::find_single_chain_inner(pokers_single_chain, poker_value, length, finded_single_chain_container);

				if (finded_single_chain_container.size() == length)
				{
					for (poker_array::iterator curr_poker = finded_single_chain_container.begin(); curr_poker != finded_single_chain_container.end(); ++curr_poker)
					{
						std::map<const char*, poker_array*, char_key_op>::iterator poker_array_finded = three_type_pokers.find(*curr_poker);
						if (poker_array_finded != three_type_pokers.end())
						{
							poker_array* three_pokers = (*poker_array_finded).second;
							for (int i = 0; i < 3; i++)
							{
								finded_poker_list.push_back(three_pokers->at(i));
							}
						}
						else
						{
							finded_poker_list.clear();
							return;
						}
					}
					return;
				}
				else
				{
					finded_single_chain_container.clear();
					continue;
				}
			}
		}


		void poker_finder::find_three_chain_and_single(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < length)
				return;

			//建立一个临时数据，存放更更新的用户拥有的扑克牌
			poker_list user_poker_list_clone(user_poker_list.begin(), user_poker_list.end());

			//先用finded_poker_list来承载三顺牌型
			poker_finder::find_three_chain(user_poker_list, poker_value, (length / 4) * 3, finded_poker_list);

			//如果没有找到，那么返回
			if (finded_poker_list.size() == 0)
				return;

			//将三顺牌型从用户牌面中去掉，供之后找单顺
			for (int i = 0; i < finded_poker_list.size(); i++)
			{
				user_poker_list_clone.erase(finded_poker_list.at(i));
			}

			length /= 4;

			poker_array single_pokers;
			poker_array single_poker_finded;

			while (true)
			{
				single_poker_finded.clear();
				poker_finder::find_single(user_poker_list_clone, '0', single_poker_finded);
				if (single_poker_finded.size() < 1)
				{
					finded_poker_list.clear();
					return;
				}
				user_poker_list_clone.erase(single_poker_finded.at(0));

				bool finded = false;

				for (int i = 0; i < single_pokers.size(); i++)
				{
					if (single_pokers.at(i)[0] == single_poker_finded.at(0)[0])
					{
						finded = true;
						break;
					}
				}

				if (finded == true)
					continue;

				for (int i = 0; i < finded_poker_list.size(); i++)
				{
					if (finded_poker_list.at(i)[0] == single_poker_finded.at(0)[0])
					{
						finded = true;
						break;
					}
				}

				if (finded == true)
					continue;

				single_pokers.push_back(single_poker_finded.at(0));

				if (single_pokers.size() >= length)
				{
					for (int i = 0; i < single_pokers.size(); i++)
					{
						finded_poker_list.push_back(single_pokers.at(i));
					}

					return;
				}
			}
		}

		void poker_finder::find_three_chain_and_double(poker_list& user_poker_list, char poker_value, int length, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < length)
				return;


			//建立一个临时数据，存放更更新的用户拥有的扑克牌
			poker_list user_poker_list_clone(user_poker_list.begin(), user_poker_list.end());

			//先用finded_poker_list来承载三顺牌型
			poker_finder::find_three_chain(user_poker_list, poker_value, (length / 5) * 3, finded_poker_list);

			//如果没有找到，那么返回
			if (finded_poker_list.size() == 0)
				return;

			//将三顺牌型从用户牌面中去掉，供之后找double
			for (int i = 0; i < finded_poker_list.size(); i++)
			{
				user_poker_list_clone.erase(finded_poker_list.at(i));
			}

			length /= 5;

			poker_array double_pokers;
			poker_array double_poker_finded;

			while (true)
			{
				double_poker_finded.clear();
				poker_finder::find_double(user_poker_list_clone, '0', double_poker_finded);
				if (double_poker_finded.size() < 2)
				{
					finded_poker_list.clear();
					return;
				}

				user_poker_list_clone.erase(double_poker_finded.at(0));
				user_poker_list_clone.erase(double_poker_finded.at(1));

				bool finded = false;

				for (int i = 0; i < double_pokers.size(); i++)
				{
					if (double_pokers.at(i)[0] == double_poker_finded.at(0)[0])
					{
						finded = true;
						break;
					}
				}

				if (finded == true)
					continue;

				//for(int i = 0; i < finded_poker_list.size(); i++)
				//{
				//	if(finded_poker_list.at(i)[0] == double_poker_finded.at(0)[0])
				//	{
				//		finded = true;
				//		break;
				//	}
				//}

				//if(finded == true)
				//	continue;

				double_pokers.push_back(double_poker_finded.at(0));
				double_pokers.push_back(double_poker_finded.at(1));

				if (double_pokers.size() >= length * 2)
				{
					for (int i = 0; i < double_pokers.size(); i++)
					{
						finded_poker_list.push_back(double_pokers.at(i));
					}
					return;
				}
			}
		}

		void poker_finder::find_double_joker(poker_list& user_poker_list, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() < 2)
				return;

			poker_list::iterator curr_poker = user_poker_list.end();

			if ((*--curr_poker)[0] == 'O' && (*--curr_poker)[0] == 'N')
			{
				finded_poker_list.push_back(*curr_poker);
				finded_poker_list.push_back(*(++curr_poker));
				return;
			}
		}

		void poker_finder::find_bomb_or_double_joker(poker_list& user_poker_list, poker_array& finded_poker_list)
		{
			if (user_poker_list.size() >= 4)
			{
				poker_finder::find_four(user_poker_list, '0', finded_poker_list);
				return;
			}

			if (user_poker_list.size() >= 2)
			{
				poker_finder::find_double_joker(user_poker_list, finded_poker_list);
			}
		}

		void poker_finder::find_big_pokers(poker_info& pi, poker_list& user_poker_list, poker_array& finded_poker_list)
		{
			switch (pi.type)
			{
			case poker_info::ZERO:
				poker_finder::find_single(user_poker_list, '0', finded_poker_list);
				assert(finded_poker_list.size() == 1 || finded_poker_list.size() == 0);
				break;

			case poker_info::SINGLE:
				poker_finder::find_single(user_poker_list, pi.value, finded_poker_list);
				assert(finded_poker_list.size() == 1 || finded_poker_list.size() == 0);

				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::DOUBLE:
				poker_finder::find_double(user_poker_list, pi.value, finded_poker_list);
				assert(finded_poker_list.size() == 2 || finded_poker_list.size() == 0);

				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::THREE:
				poker_finder::find_three(user_poker_list, pi.value, finded_poker_list);
				assert(finded_poker_list.size() == 3 || finded_poker_list.size() == 0);

				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::THREE_ONE:
				poker_finder::find_three_and_single(user_poker_list, pi.value, finded_poker_list);
				assert(finded_poker_list.size() == 4 || finded_poker_list.size() == 0);
				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::THREE_DOUBLE:
				poker_finder::find_three_and_double(user_poker_list, pi.value, finded_poker_list);
				assert(finded_poker_list.size() == 5 || finded_poker_list.size() == 0);
				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::BOMB:
				poker_finder::find_four(user_poker_list, pi.value, finded_poker_list);

				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::FOUR_TWO_SINGLE:
				poker_finder::find_four_and_two_single(user_poker_list, pi.value, finded_poker_list);

				assert(finded_poker_list.size() == 6 || finded_poker_list.size() == 0);
				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::FOUR_TWO_DOUBLE:
				poker_finder::find_four_and_two_double(user_poker_list, pi.value, finded_poker_list);
				assert(finded_poker_list.size() == 8 || finded_poker_list.size() == 0);
				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::SINGLE_CHAIN:
				poker_finder::find_single_chain(user_poker_list, pi.value, pi.length, finded_poker_list);

				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::DOUBLE_CHAIN:
				poker_finder::find_double_chain(user_poker_list, pi.value, pi.length, finded_poker_list);

				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::THREE_CHAIN:
				poker_finder::find_three_chain(user_poker_list, pi.value, pi.length, finded_poker_list);

				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::TRHEE_CHAIN_SINGLE:
				poker_finder::find_three_chain_and_single(user_poker_list, pi.value, pi.length, finded_poker_list);

				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;


			case poker_info::THREE_CHAIN_DOUBLE:
				poker_finder::find_three_chain_and_double(user_poker_list, pi.value, pi.length, finded_poker_list);

				if (finded_poker_list.size() == 0)
				{
					poker_finder::find_bomb_or_double_joker(user_poker_list, finded_poker_list);
				}
				break;

			case poker_info::DOUBLE_JOKER:
				break;
			}
		}


		//释放archived_pokers
		//void poker_finder::release_archived_array(archived_pokers& pokers_archived)
		//{
		//	for(archived_pokers::iterator curr_pokers = pokers_archived.begin(); curr_pokers != pokers_archived.end(); ++curr_pokers)
		//	{
		//		std::vector<poker_array*>* type_array = (*curr_pokers);

		//		for(std::vector<poker_array*>::iterator curr_type_array = type_array->begin(); curr_type_array != type_array->end(); ++curr_type_array)
		//		{
		//			poker_array* poker_array = (*curr_type_array);
		//			poker_array->clear();

		//			delete poker_array;
		//		}

		//		type_array->clear();

		//		delete type_array;
		//	}

		//	pokers_archived.clear();
		//}

		void poker_finder::release_three_types_map(std::map<const char*, poker_array*, char_key_op>& poker_map)
		{
			for (std::map<const char*, poker_array*, char_key_op>::iterator curr_array = poker_map.begin(); curr_array != poker_map.end(); ++curr_array)
			{
				delete (*curr_array).second;
			}
			poker_map.clear();
		}
	}
}

