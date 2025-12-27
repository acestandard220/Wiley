#include "log.h"

namespace Wiley {

	Log s_log;

	Log& Log::logger()
	{
		return s_log;
	}

}
