/*
 * DiscoveryService.cpp
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

#include "net/DiscoveryService.h"
#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/BundleString.h"
#include <string>

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		DiscoveryService::DiscoveryService()
		 : /**/_service_protocol(dtn::core::Node::CONN_UNDEFINED)
		{
		}

		DiscoveryService::DiscoveryService(const Discovery::tag_t service_tag, const std::string service_name)
		 : service_tag_(service_tag), service_name_(service_name)
		{
		}

		/**/DiscoveryService::DiscoveryService(const dtn::core::Node::Protocol p, const std::string &parameters)
		 : _service_protocol(p), _service_name(asTag(p)), _service_parameters(parameters)
		{
		}

		/**/DiscoveryService::DiscoveryService(const std::string &name, const std::string &parameters)
		 : _service_protocol(asProtocol(name)), _service_name(name), _service_parameters(parameters)
		{
		}

		DiscoveryService::~DiscoveryService()
		{
			for (std::list<DiscoveryTypePtr>::iterator it = parameters_.begin();
			     it != parameters_.end();
			     it++)
			{
				delete (*it).p;
			}
		}

		dtn::data::Length DiscoveryService::getLength(Discovery::Protocol version) const throw (Discovery::WrongVersionException)
		{
			dtn::data::Number length;
			switch (version)
			{
				case Discovery::DISCO_VERSION_02:
				{
					if (_param)
					{
						length = _param->getLength(Discovery::DISCO_VERSION_02);
						return 1               // tag
							+ length.getLength() // length
							+ length.get();      // value
					}
					else
					{
						return 0;
					}
				}
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					if (_param)
					{
						BundleString name(asTag(_service_protocol));
						return name.getLength() + _param->getLength(version);
					}
					else
					{
						return 0;
					}
				}
				default:
				{
					std::ostringstream err;
					err << std::hex << version;
					throw Discovery::WrongVersionException(err.str());
				}
			}
		}

		dtn::core::Node::Protocol DiscoveryService::getProtocol() const
		{
			return _service_protocol;
		}

		const std::string& DiscoveryService::getName() const
		{
			return _service_name;
		}

		const std::string& DiscoveryService::getParameters() const
		{
			return _service_parameters;
		}

		void DiscoveryService::update(const std::string &parameters)
		{
			_service_parameters = parameters;
		}

		std::string serialize(const Discovery::Protocol version) const throw (Discovery::WrongVersionException, Discovery::IllegalServiceException)
		{
			std::ostringstream ss;
			// FIXME version
			switch (version)
			{
				case Discovery::DISCO_VERSION_02:
				{
					ss.write((char *) service_tag_, 1);
					for(std::list<DiscoveryTypePtr>::const_iterator it;
					    it != parameters_.end(); it++)
					{
						ss << (*it).serialize(version);
					}
					break;
				}
				default:
					throw Discovery::WrongVersionException();
			}

			return ss.str();
		}

		dtn::data::Length deserialize(const Discovery::Protocol version, std::istream& stream) throw (Discovery::WrongVersionException, Discovery::IllegalServiceException)
		{
			// FIXME version
			switch (version)
			{
				case Discovery::DISCO_VERSION_02:
				{
					ss.put((char) service_tag_);
					for(std::list<DiscoveryTypePtr>::const_iterator it;
					    it != parameters_.end(); it++)
					{
						ss << (*it).serialize(version);
					}
					break;
				}
				default:
					throw Discovery::WrongVersionException();
			}

			return ss.str();
		}
		}

		dtn::core::Node::Protocol DiscoveryService::asProtocol(const std::string &tag)
		{
			if (tag == "undefined") {
				return dtn::core::Node::CONN_UNDEFINED;
			}
			else if (tag == "udpcl") {
				return dtn::core::Node::CONN_UDPIP;
			}
			else if (tag == "tcpcl") {
				return dtn::core::Node::CONN_TCPIP;
			}
			else if (tag == "lowpancl") {
				return dtn::core::Node::CONN_LOWPAN;
			}
			else if (tag == "bt") {
				return dtn::core::Node::CONN_BLUETOOTH;
			}
			else if (tag == "http") {
				return dtn::core::Node::CONN_HTTP;
			}
			else if (tag == "file") {
				return dtn::core::Node::CONN_FILE;
			}
			else if (tag == "dgram:udp") {
				return dtn::core::Node::CONN_DGRAM_UDP;
			}
			else if (tag == "dgram:eth") {
				return dtn::core::Node::CONN_DGRAM_ETHERNET;
			}
			else if (tag == "dgram:lowpan") {
				return dtn::core::Node::CONN_DGRAM_LOWPAN;
			}
			else if (tag == "p2p:wifi") {
				return dtn::core::Node::CONN_P2P_WIFI;
			}
			else if (tag == "p2p:bt") {
				return dtn::core::Node::CONN_P2P_BT;
			}
			else if (tag == "email") {
				return dtn::core::Node::CONN_EMAIL;
			}

			return dtn::core::Node::CONN_UNSUPPORTED;
		}

		std::string DiscoveryService::asTag(const dtn::core::Node::Protocol proto)
		{
			switch (proto)
			{
			case dtn::core::Node::CONN_UNSUPPORTED:
				return "unsupported";

			case dtn::core::Node::CONN_UNDEFINED:
				return "undefined";

			case dtn::core::Node::CONN_UDPIP:
				return "udpcl";

			case dtn::core::Node::CONN_TCPIP:
				return "tcpcl";

			case dtn::core::Node::CONN_LOWPAN:
				return "lowpancl";

			case dtn::core::Node::CONN_BLUETOOTH:
				return "bt";

			case dtn::core::Node::CONN_HTTP:
				return "http";

			case dtn::core::Node::CONN_FILE:
				return "file";

			case dtn::core::Node::CONN_DGRAM_UDP:
				return "dgram:udp";

			case dtn::core::Node::CONN_DGRAM_ETHERNET:
				return "dgram:eth";

			case dtn::core::Node::CONN_DGRAM_LOWPAN:
				return "dgram:lowpan";

			case dtn::core::Node::CONN_P2P_WIFI:
				return "p2p:wifi";

			case dtn::core::Node::CONN_P2P_BT:
				return "p2p:bt";

			case dtn::core::Node::CONN_EMAIL:
				return "email";
			}

			return "unknown";
		}

		std::ostream &operator<<(std::ostream &stream, const DiscoveryService &service)
		{
			BundleString name(service._service_name);
			BundleString parameters(service._service_parameters);

			stream << name << parameters;

			return stream;
		}

		std::istream &operator>>(std::istream &stream, DiscoveryService &service)
		{
			BundleString name;
			BundleString parameters;

			stream >> name >> parameters;

			service._service_protocol = DiscoveryService::asProtocol((const std::string&)name);
			service._service_name = (const std::string&)name;
			service._service_parameters = (const std::string&)parameters;

			return stream;
		}
	}
}
