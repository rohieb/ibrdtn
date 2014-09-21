/**
 * Discovery.h
 *
 * Copyright (C) 2014 IBR, TU Braunschweig
 *
 * Written-by: Roland Hieber <r.hieber@tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef DISCOVERY_H_
#define DISCOVERY_H_

#include <ibrcommon/Exceptions.h>
#include <string>
#include <sstream>

namespace dtn {
	namespace net {
		class WrongVersionException : public ibrcommon::Exception
		{
			public:
				WrongVersionException(std::string what = "(n/a)") throw()
				 : ibrcommon::Exception(std::string("Unspecified or unsupported "
							"discovery protocol version: ").append(what)) {}
				virtual ~WrongVersionException() throw() {}
		};

		class IllegalServiceException : public ibrcommon::Exception
		{
			public:
				IllegalServiceException(std::string what = "(n/a)") throw()
				 : ibrcommon::Exception(std::string("No such service, or illegal "
							"parameters for service: ").append(what)) {}
				virtual ~IllegalServiceException() throw() {}
		};

		class ParseException : public ibrcommon::Exception
		{
			size_t bytes_read_;
			public:
				ParseException(std::string what = "(n/a)", size_t bytes_read = 0) throw()
				 : ibrcommon::Exception(), bytes_read_(bytes_read)
				{
					std::ostringstream ss;
					ss << "Parse error at input byte " << bytes_read << ": " << what;
					_what = ss.str();
				}
				virtual ~ParseException() throw() {}
				inline const size_t getBytesRead() const { return bytes_read_; }
		};

		/** Definitions common to the Discovery* classes */
		namespace Discovery {
			enum Protocol
			{
				DTND_IPDISCOVERY = 0x00, /*< Legacy DTND discovery */
				DISCO_VERSION_00 = 0x01, /*< IPND draft 00 */
				DISCO_VERSION_01 = 0x02, /*< IPND draft 01 */
				DISCO_UNSPEC     = 0xFF, /*< Unspecified discovery protocol */
			};
		}
	} // namespace net
} // namespace dtn
#endif // DISCOVERY_H_
