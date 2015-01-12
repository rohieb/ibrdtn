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
#include <ibrdtn/utils/Utils.h>
#include <ibrcommon/Logger.h>
#include <string>

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		DiscoveryService::DiscoveryService()
		 : /*FIXME*/_service_protocol(dtn::core::Node::CONN_UNDEFINED)
		{
		}

		DiscoveryService::DiscoveryService(const Discovery::tag_t service_tag, const std::string service_name)
		 : _service_tag(service_tag), _service_name(service_name)
		{
		}

		/*FIXME*/DiscoveryService::DiscoveryService(const dtn::core::Node::Protocol p, const std::string &parameters)
		 : _service_protocol(p), _service_name(asTag(p)), _service_parameters(parameters)
		{
		}

		/*FIXME*/DiscoveryService::DiscoveryService(const std::string &name, const std::string &parameters)
		 : _service_protocol(asProtocol(name)), _service_name(name), _service_parameters(parameters)
		{
		}

		DiscoveryService::~DiscoveryService()
		{
		}

		dtn::data::Length DiscoveryService::getLength(Discovery::Protocol version) const throw (Discovery::WrongVersionException)
		{
			dtn::data::Number length;
			switch (version)
			{
				case Discovery::DISCO_VERSION_02:
				{
					length = 0;
					std::vector<refcnt_ptr<Discovery::Type> >::const_iterator it;
					for (it = _parameters.begin(); it != _parameters.end(); it++)
					{
						length += (*it)->getLength(Discovery::DISCO_VERSION_02);
					}
					return 1               // sizeof(tag)
						+ length.getLength() // sizeof(length)
						+ length.get();      // sizeof(value)
				}
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					length = 0;
					std::vector<refcnt_ptr<Discovery::Type> >::const_iterator it;
					for (it = _parameters.begin(); it != _parameters.end(); it++)
					{
						length += (*it)->getLength(version);
						length += 1; // ";" separator
					}
					if (_parameters.size() > 0)
					{
						length -= 1; // no separator at the end
					}
					return _service_name.getLength() + length.getLength() + length.get();
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

		std::string DiscoveryService::serialize(const Discovery::Protocol version) const throw (Discovery::WrongVersionException, Discovery::IllegalServiceException)
		{
			std::ostringstream ss;
			// FIXME version
			switch (version)
			{
				case Discovery::DISCO_VERSION_02:
				{
					ss.put(static_cast<char>(_service_tag));
					std::vector<refcnt_ptr<Discovery::Type> >::const_iterator it;
					for(it = _parameters.begin(); it != _parameters.end(); it++)
					{
						ss << (*it)->serialize(version);
					}
					break;
				}
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					BundleString params;
					std::vector<refcnt_ptr<Discovery::Type> >::const_iterator it;
					for(it = _parameters.begin(); it != _parameters.end(); it++)
					{
						params.append((*it)->serialize(version));
						params.append(";");
					}
					if (_parameters.size() > 0)
					{
						// remove last ';' separator for compatibility
						params.erase(params.size());
					}

					ss << _service_name;
					ss << params;
				}
				default:
					throw Discovery::WrongVersionException();
			}

			return ss.str();
		}

		dtn::data::Length DiscoveryService::deserialize(const Discovery::Protocol version, std::istream& stream) throw (Discovery::WrongVersionException, Discovery::ParseException, Discovery::IllegalServiceException)
		{
			std::ostringstream err;
			size_t bytes_read = 0;

			switch (version)
			{
				case Discovery::DISCO_VERSION_00:
				case Discovery::DISCO_VERSION_01:
				{
					dtn::data::BundleString bs;
					stream >> bs;
					if (stream.bad() || stream.fail())
					{
						throw Discovery::ParseException("stream was bad after reading "
							"IPND service name", 0);
					}
					bytes_read = stream.gcount();

					_service_name = bs;

					stream >> bs;
					if (stream.bad() || stream.fail())
					{
						throw Discovery::ParseException("stream was bad after reading "
							"IPND service parameters", bytes_read);
					}
					bytes_read = stream.gcount();

					std::vector<string> params = dtn::utils::Utils::tokenize(";", bs);
					for (std::vector<string>::iterator it = params.begin();
					     it != params.end(); it++)
					{
						std::istringstream ss(*it);

						// FIXME: do our best to determine the parameter type
						Discovery::Type * p;
						size_t sep = it->find_first_of('=');
						std::string key, value;

						if (sep != std::string::npos)
						{
							// FIXME: parse k-v into parameter types
						}
						//p.deserialize(version, stream);
						_parameters.push_back(refcnt_ptr<Discovery::Type>(p));
					}

					break;
				} // case DISCO_VERSION_00/01

				case Discovery::DISCO_VERSION_02:
				{
					// FIXME: read constructed type tag & length
					char tag = stream.get();
					if (stream.bad() || stream.fail())
					{
						throw Discovery::ParseException("could not parse IPND 02 tag", 0);
					}
					bytes_read = 1;

					dtn::data::Number length;
					stream >> length;
					if (stream.bad() || stream.fail())
					{
						err << "could not parse length for IPND 02 tag " << std::hex << tag;
						throw Discovery::ParseException(err.str(), bytes_read);
					}
					bytes_read += stream.gcount();
					size_t last_good_pos = stream.tellg();

					while (!stream.bad() && !stream.fail())
					{
						Discovery::Type * p;

						switch (tag)
						{
							case Discovery::Boolean::TAG: p = new Discovery::Boolean; break;
							case Discovery::UInt64::TAG:  p = new Discovery::UInt64;  break;
							case Discovery::SInt64::TAG:  p = new Discovery::SInt64;  break;
							case Discovery::Fixed16::TAG: p = new Discovery::Fixed16; break;
							case Discovery::Fixed32::TAG: p = new Discovery::Fixed32; break;
							case Discovery::Fixed64::TAG: p = new Discovery::Fixed64; break;
							case Discovery::Float::TAG:   p = new Discovery::Float;   break;
							case Discovery::Double::TAG:  p = new Discovery::Double;  break;
							case Discovery::String::TAG:  p = new Discovery::String;  break;
							case Discovery::Bytes::TAG:   p = new Discovery::Bytes;   break;
							default:
							{
								tag = stream.get(); // get unknown tag field
								bytes_read = 1;
								dtn::data::Number length;
								stream >> length;
								bytes_read += stream.gcount();

								std::ostringstream err;
								if (stream.bad() || stream.fail())
								{
									err << "could not read length for unknown IPND 02 tag "
											<< std::hex << tag << ", giving up";
									throw Discovery::ParseException(err.str(), bytes_read);
								}

								stream.seekg(length.get(), std::ios_base::cur);
								IBRCOMMON_LOGGER_TAG("DiscoveryService", info)
									<< "don't know how to handle unknown IPND 02 tag " << std::hex
									<< tag << ", skipping" << IBRCOMMON_LOGGER_ENDL;
								continue;
							} // default
						} // switch

						try
						{
							bytes_read += p->deserialize(version, stream);
						}
						catch (Discovery::ParseException& e)
						{
							if (stream.bad() || stream.fail())
							{
								err << "could not parse value for IPND 02 tag " << std::hex
									<< tag << ": end of stream";
								throw Discovery::ParseException(err.str(), bytes_read);
							}
							stream.seekg(last_good_pos, std::ios_base::beg);
							stream.seekg(length.get(), std::ios_base::cur);
							bytes_read += length.get();
							IBRCOMMON_LOGGER_TAG("DiscoveryService", info)
								<< "could not parse value for IPND 02 tag " << std::hex
								<< tag << ", skipping. exception was: " << e.what()
								<< IBRCOMMON_LOGGER_ENDL;
							continue;
						}

						addParameter(p);
					} // while

					if (bytes_read != length.get()+length.getLength()+1)
					{
							IBRCOMMON_LOGGER_TAG("DiscoveryService", info) << "expected "
								<< length << " bytes, but read " << bytes_read
								<< " while parsing IPND 02 tag " << std::hex << tag
								<< IBRCOMMON_LOGGER_ENDL;
					}
					return bytes_read;
				} // case DISCO_VERSION_02

				default:
				{
					err << std::dec << version;
					throw Discovery::WrongVersionException(err.str());
				}
			} // switch version

			return bytes_read;
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
