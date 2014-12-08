/*
 * DiscoveryService.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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

#ifndef DISCOVERYSERVICE_H_
#define DISCOVERYSERVICE_H_

#include "core/Node.h"
#include "net/Discovery.h"
#include <ibrdtn/data/Number.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>

namespace dtn
{
	namespace net
	{
		class DiscoveryService
		{
		public:
			DiscoveryService();
			DiscoveryService(Discovery::tag_t service_tag);
			/**/DiscoveryService(const dtn::core::Node::Protocol p, const std::string &parameters);
			/**/DiscoveryService(const std::string &name, const std::string &parameters);
			virtual ~DiscoveryService();

			dtn::data::Length getLength(Discovery::Protocol version) const throw (Discovery::WrongVersionException);

			dtn::core::Node::Protocol getProtocol() const;
			const std::string& getName() const;
			const std::string& getParameters() const;

			/**
			 * Update the parameters of this service
			 */
			void update(const std::string &parameters);

			/**
			 * convert a protocol identifier to a tag
			 */
			static std::string asTag(const dtn::core::Node::Protocol proto);
			static dtn::core::Node::Protocol asProtocol(const std::string &tag);

			/**
			 * @param param the DiscoveryService takes ownership of this pointer
			 */
			void addParameter(Discovery::Type * param) {
				parameters_.push_back(DiscoveryTypePtr(param));
			}

			std::string serialize(const Discovery::Protocol version) const throw (Discovery::WrongVersionException, Discovery::IllegalServiceException);
			dtn::data::Length deserialize(const Discovery::Protocol version, std::istream& stream) throw (Discovery::WrongVersionException, Discovery::IllegalServiceException);



		protected:
			/** workaround for putting abstract base class ptrs into a container */
			struct DiscoveryTypePtr
			{
				DiscoveryTypePtr(Discovery::Type * ptr) : p(ptr) {}
				Discovery::Type * p;
			};
			Discovery::tag_t service_tag_;
			std::list<DiscoveryTypePtr> parameters_;
			/**/dtn::core::Node::Protocol _service_protocol;
			/**/std::string _service_name;
			/**/std::string _service_parameters;

		private:
			friend std::ostream &operator<<(std::ostream &stream, const DiscoveryService &service);
			friend std::istream &operator>>(std::istream &stream, DiscoveryService &service);
		};
	}
}

#endif /* DISCOVERYSERVICE_H_ */
