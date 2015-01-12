// FIXME: file header
#include "net/Discovery.h"

namespace dtn {
	namespace net {
		namespace Discovery {
			std::map<tag_t, Type::Factory *> Type::Factory::type_registry_;

			/** convert certain types to string. Used for IPND 00/01. */
			template<typename T> std::string toString(const T& value)
			{
				std::ostringstream ss;
				ss << std::dec << value;
				return ss.str();
			}
			template<> std::string toString<bool>(const bool& value)
			{
				return (value) ? "true" : "false";
			}
			template<> std::string toString<dtn::data::SDNV<uint64_t> >(const dtn::data::SDNV<uint64_t>& value)
			{
				return toString<uint64_t>(value.get());
			}
			template<> std::string toString<dtn::data::SDNV<int64_t> >(const dtn::data::SDNV<int64_t>& value)
			{
				return toString<int64_t>(value.get());
			}
			template<> std::string toString<dtn::data::BundleString>(const dtn::data::BundleString& value)
			{
				return value;
			}

			/** convert strings to certain types. Used for IPND 00/01. */
			template<typename T> T& fromString(const std::string& s, T& val)
			{
				std::istringstream ss(s);
				ss >> val;
				return val;
			}
			template<> bool& fromString(const std::string& s, bool& val)
			{
				if (s == "true" || s == "1")
				{
					val = true;
				}
				else
				{
					val = false;
				}
			}
			template<> dtn::data::SDNV<uint64_t>& fromString(const std::string& s, dtn::data::SDNV<uint64_t>& val)
			{
				uint64_t tmp;
				std::istringstream ss(s);
				ss >> tmp;
				val = tmp;
				return val;
			}
			template<> dtn::data::SDNV<int64_t>& fromString(const std::string& s, dtn::data::SDNV<int64_t>& val)
			{
				int64_t tmp;
				std::istringstream ss(s);
				ss >> tmp;
				val = tmp;
				return val;
			}
			template<> dtn::data::BundleString& fromString(const std::string& s, dtn::data::BundleString& val)
			{
				val = s;
				return val;
			}
		}
	}
}
