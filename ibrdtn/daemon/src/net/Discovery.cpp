// FIXME: file header
#include "net/Discovery.h"

namespace dtn {
	namespace net {
		namespace Discovery {
			std::map<tag_t, Type::Factory *> Type::Factory::type_registry_;
		}
	}
}
