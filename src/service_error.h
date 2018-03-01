#ifndef __DDZ_SERVICE_ERROR_H__
#define __DDZ_SERVICE_ERROR_H__
#include "service/service_error.h"

namespace dooqu_server
{
	namespace ddz
	{
		struct service_error : public dooqu_service::service::service_error
		{
		public:
			static const unsigned short GAME_IS_FULL = 4050;
		};
	}
}

#endif
