/**
 * DiscoveryServiceParam.cpp
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

#include "net/DiscoveryServiceParam.h"
#include <ibrdtn/data/BundleString.h>
#include <ibrdtn/utils/Utils.h>
#include <stdexcept>
#include <cstring>

namespace dtn
{
	namespace net
	{
		/**
		 * Dispatcher to the leaf classes' operator==()
		 * @throws std::invalid_argument if lhs and rhs are different types
		 */
		DiscoveryServiceParam& DiscoveryServiceParam::operator=(const DiscoveryServiceParam& o)
		{
			try {
				return dynamic_cast<IPServiceParam&>(*this)
					.operator=(dynamic_cast<const IPServiceParam&>(o));
			} catch (const std::bad_cast& e) { }

			try {
				return dynamic_cast<DatagramServiceParam&>(*this)
					.operator=(dynamic_cast<const DatagramServiceParam&>(o));
			} catch (const std::bad_cast& e) { }

			try {
				return dynamic_cast<DTNTPServiceParam&>(*this)
					.operator=(dynamic_cast<const DTNTPServiceParam&>(o));
			} catch (const std::bad_cast& e) { }

			try {
				return dynamic_cast<LOWPANServiceParam&>(*this)
					.operator=(dynamic_cast<const LOWPANServiceParam&>(o));
			} catch (const std::bad_cast& e) { }

			try {
				return dynamic_cast<EMailServiceParam&>(*this)
					.operator=(dynamic_cast<const EMailServiceParam&>(o));
			} catch (const std::bad_cast& e) { }

			throw std::invalid_argument("lhs and rhs are different types");
		}

		/** Dispatcher to the leaf classes' operator==() */
		bool DiscoveryServiceParam::operator==(const DiscoveryServiceParam& o) const
		{
			try {
				return dynamic_cast<const IPServiceParam&>(o) ==
					dynamic_cast<const IPServiceParam&>(*this);
			} catch (const std::bad_cast& e) { }

			try {
				return dynamic_cast<const DatagramServiceParam&>(o) ==
					dynamic_cast<const DatagramServiceParam&>(*this);
			} catch (const std::bad_cast& e) { }

			try {
				return dynamic_cast<const DHTServiceParam&>(o) ==
					dynamic_cast<const DHTServiceParam&>(*this);
			} catch (const std::bad_cast& e) { }

			try {
				return dynamic_cast<const DTNTPServiceParam&>(o) ==
					dynamic_cast<const DTNTPServiceParam&>(*this);
			} catch (const std::bad_cast& e) { }

			try {
				return dynamic_cast<const LOWPANServiceParam&>(o) ==
					dynamic_cast<const LOWPANServiceParam&>(*this);
			} catch (const std::bad_cast& e) { }

			try {
				return dynamic_cast<const EMailServiceParam&>(o) ==
					dynamic_cast<const EMailServiceParam&>(*this);
			} catch (const std::bad_cast& e) { }

			return false;
		}

		/**
		 * Factory Method to generate a concrete instance from IPND draft 00/01
		 * parameter strings. The parameter string is interpreted dependent on the
		 * present keys, and in the following order:
		 *   @li @c email is present: returns a new EMailServiceParam
		 *   @li @c version and @c quality and @c timestamp are set: returns a new
		 *    DTNTPServiceParam
		 *   @li @c port and @c address are present and @address is smaller than
		 *    5 characters:	returns a new LOWPANServiceParam
		 *   @li @c port is present: returns a new IPServiceParam
		 * @param params a key=value string with IPND draft 00/01 parameters
		 * @return a pointer to a DiscoveryServiceParam subclass representing the
		 *  given parameter string.
		 * @throw an IllegalServiceException is thrown if the parameter string
		 *  could not be parsed, or if it can be represented by more than one
		 *  class.
		 */
		DiscoveryServiceParam * DiscoveryServiceParam::fromKeyValueString(const std::string& parameters) throw (IllegalServiceException)
		{
			std::string address = "";
			bool address_set = false;

			uint16_t port = 0;
			bool port_set = false;

			unsigned int version = 0;
			bool version_set = false;

			double quality = 0;
			bool quality_set = false;

			dtn::data::Size timestamp = 0;
			bool timestamp_set = false;

			try
			{
				std::vector<string> params = dtn::utils::Utils::tokenize(";", parameters);
				for (std::vector<string>::const_iterator param = params.begin();
				     param != params.end();
				     param++)
				{
					std::vector<string> part = dtn::utils::Utils::tokenize("=", *param);
					if (part.size() > 1)
					{
						if (part[0] == "port")
						{
							std::istringstream ss(part[1]);
							ss >> std::dec >> port;
							port_set = true;
						}
						else if (part[0] == "ip")
						{
							address = part[1];
							if ((address.find(".") != std::string::npos &&
							     address.size() > INET_ADDRSTRLEN) ||
							    (address.find(":") != std::string::npos &&
							     address.size() > INET6_ADDRSTRLEN) ||
									(address.find(".") == std::string::npos &&
									 address.find(":") == std::string::npos &&
							     address.size() > 5)) // maxlen(uint16_t), for LoWPAN
							{
								std::ostringstream err;
								err << "Could not parse address: " << address;
								throw IllegalServiceException(err.str());
							}
							address_set = true;
						}
						else if (part[0] == "version")
						{
							std::istringstream ss(part[1]);
							ss >> std::dec >> version;
							version_set = true;
						}
						else if (part[0] == "quality")
						{
							std::istringstream ss(part[1]);
							ss >> std::dec >> quality;
							quality_set = true;
						}
						else if (part[0] == "timestamp")
						{
							// note: string contains timestamp as string, not as SDNV
							std::istringstream ss(part[1]);
							ss >> std::dec >> timestamp;
							timestamp_set = true;
						}
						else if (part[0] == "email")
						{
							return new EMailServiceParam(part[1]);
						}
					}
				}

				if (version_set && quality_set && timestamp_set)
				{
					return new DTNTPServiceParam(version, quality, timestamp);
				}
				else if (port_set && port != 0 && address_set && address.size() <= 5)
				{
					uint16_t id;
					std::istringstream ss(address);
					ss >> id;
					return new LOWPANServiceParam(id, port);
				}
				else if (port_set && port != 0) // address could be omitted
				{
					return new IPServiceParam(address, port);
				}
			}
			catch (const std::exception& e)
			{
				std::ostringstream err;
				err << "Don't know how to interpret parameter string '" << parameters;
				err << "': " << e.what();
				throw IllegalServiceException(err.str());
			}
		}

/******************************************************************************/

		IPServiceParam::IPServiceParam()
			: _address(""), _port(0)
		{
		}

		/**
		 * @param ip IP address, in IPv4 dotted decimal or IPv6 notation
		 * @param port Port number, in host byte order
		 */
		IPServiceParam::IPServiceParam(const std::string& address, const uint16_t port)
			: _address(address), _port(port)
		{
		}

		DiscoveryServiceParam * IPServiceParam::clone() const
		{
			return new IPServiceParam(*this);
		}

		DiscoveryServiceParam& IPServiceParam::operator=(const IPServiceParam& o)
		{
			_address = o._address;
			_port = o._port;
			return *this;
		}

		bool IPServiceParam::operator==(const IPServiceParam& o) const
		{
			return _address == o._address && _port == o._port;
		}

		dtn::data::Length IPServiceParam::getLength(Discovery::Protocol version) const
		{
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					return dtn::data::BundleString(buildParamString()).getLength();
				}
			}
			return 0;
		}

		/** Build a @c key=value string for IPND drafts 00 and 01 */
		std::string IPServiceParam::buildParamString() const
		{
			std::ostringstream kv;
			kv << "port=" << _port;

			if (_address.size() != 0)
			{
				kv << ";ip=" << _address;
			}

			return kv.str();
		}

		std::string IPServiceParam::pack(Discovery::Protocol version) const throw (IllegalServiceException, WrongVersionException)
		{
			std::ostringstream ss;
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					ss << dtn::data::BundleString(buildParamString());
					return ss.str();
				}
			}
			std::ostringstream err;
			err << std::hex << (int) version;
			throw WrongVersionException(err.str());
		}

		/**
		 * @see DiscoveryServiceParam::unpack()
		 * @throw @c WrongVersionException if @c version is any of
		 *  Discovery::DISCO_VERSION_00 or Discovery::DISCO_VERSION_01 (use
		 *  DiscoveryServiceParam::fromKeyValueString instead), or
		 *  Discovery::DTND_IPDISCOVERY (no service information specified in
		 *  protocol)
		 */
		DiscoveryServiceParam * IPServiceParam::unpack(std::istream& stream, Discovery::Protocol version) throw (ParseException, IllegalServiceException, WrongVersionException)
		{
			std::ostringstream err;
			err << std::hex << (int) version;
			throw WrongVersionException(err.str());
		}

/******************************************************************************/

		LOWPANServiceParam::LOWPANServiceParam()
		 : _address(0), _port(0)
		{
		}

		LOWPANServiceParam::LOWPANServiceParam(const uint16_t addr, const uint16_t port)
		 : _address(addr), _port(port)
		{
		}

		/** @see DiscoveryServiceParam::clone() */
		DiscoveryServiceParam * LOWPANServiceParam::clone() const
		{
			return new LOWPANServiceParam(*this);
		}

		DiscoveryServiceParam& LOWPANServiceParam::operator=(const LOWPANServiceParam& o)
		{
			_address = o._address;
			_port = o._port;
			return *this;
		}

		bool LOWPANServiceParam::operator==(const LOWPANServiceParam& o) const
		{
			return _address == o._address && _port == o._port;
		}

		dtn::data::Length LOWPANServiceParam::getLength(Discovery::Protocol version) const
		{
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					return dtn::data::BundleString(buildParamString()).getLength();
				}
			}
			return 0;
		}

		/** Build a @c key=value string for IPND drafts 00 and 01 */
		std::string LOWPANServiceParam::buildParamString() const
		{
			//FIXME better naming for address and panid
			std::ostringstream kv;
			kv << "port=" << _port;
			kv << ";ip=" << _address;
			return kv.str();
		}

		std::string LOWPANServiceParam::pack(Discovery::Protocol version) const throw (WrongVersionException)
		{
			std::ostringstream ss;
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					ss << dtn::data::BundleString(buildParamString());
					return ss.str();
				}
			}
			std::ostringstream err;
			err << std::hex << version;
			throw WrongVersionException(err.str());
		};

		/**
		 * @see DiscoveryServiceParam::unpack()
		 * @throw @c WrongVersionException if @version is any of
		 *  @c Discovery::DISCO_VERSION_00 or @c Discovery::DISCO_VERSION_01 (use
		 *  DiscoveryServiceParam::fromKeyValueString instead), or
		 *  @c Discovery::DTND_IPDISCOVERY (no service information specified in
		 *  protocol)
		 */
		DiscoveryServiceParam * LOWPANServiceParam::unpack(std::istream& stream, Discovery::Protocol version) throw (ParseException, IllegalServiceException, WrongVersionException)
		{
			std::ostringstream err;
			err << std::hex << (int) version;
			throw WrongVersionException(err.str());
		}

/******************************************************************************/

		DatagramServiceParam::DatagramServiceParam()
		 : _address("")
		{
		}

		DatagramServiceParam::DatagramServiceParam(const std::string& addr)
		 : _address(addr)
		{
		}

		/** @see DiscoveryServiceParam::clone() */
		DiscoveryServiceParam * DatagramServiceParam::clone() const
		{
			return new DatagramServiceParam(*this);
		}

		DiscoveryServiceParam& DatagramServiceParam::operator=(const DatagramServiceParam& o)
		{
			_address = o._address;
			return *this;
		}

		bool DatagramServiceParam::operator==(const DatagramServiceParam& o) const
		{
			return _address == o._address;
		}

		dtn::data::Length DatagramServiceParam::getLength(Discovery::Protocol version) const
		{
			return dtn::data::BundleString(_address).getLength();
		}

		std::string DatagramServiceParam::pack(Discovery::Protocol version) const throw (WrongVersionException)
		{
			std::ostringstream ss;
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					ss << dtn::data::BundleString(_address);
					return ss.str();
				}
			}
			std::ostringstream err;
			err << std::hex << version;
			throw WrongVersionException(err.str());
		};

		/** @see DiscoveryServiceParam::unpack() */
		DiscoveryServiceParam * DatagramServiceParam::unpack(std::istream& stream, Discovery::Protocol version) throw (ParseException, IllegalServiceException, WrongVersionException)
		{
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					dtn::data::BundleString a;
					stream >> a;
					if (stream.fail())
					{
						throw ParseException("cannot parse DatagramServiceParam", 0);
					}
					return new DatagramServiceParam(a);
				}
			}
			std::ostringstream err;
			err << std::hex << version;
			throw WrongVersionException(err.str());
		}

/******************************************************************************/

		EMailServiceParam::EMailServiceParam()
		 : _address("")
		{
		}

		EMailServiceParam::EMailServiceParam(const std::string& addr)
		 : _address(addr)
		{
		}

		/** @see DiscoveryServiceParam::clone() */
		DiscoveryServiceParam * EMailServiceParam::clone() const
		{
			return new EMailServiceParam(*this);
		}

		DiscoveryServiceParam& EMailServiceParam::operator=(const EMailServiceParam& o)
		{
			_address = o._address;
			return *this;
		}

		bool EMailServiceParam::operator==(const EMailServiceParam& o) const
		{
			return _address == o._address;
		}

		/** Build a @c key=value string for IPND drafts 00 and 01 */
		std::string EMailServiceParam::buildParamString() const
		{
			std::ostringstream kv;
			kv << "email=" << _address;
			return kv.str();
		}

		dtn::data::Length EMailServiceParam::getLength(Discovery::Protocol version) const
		{
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					return dtn::data::BundleString(buildParamString()).getLength();
				}
			}
			return 0;
		}

		std::string EMailServiceParam::pack(Discovery::Protocol version) const throw (WrongVersionException)
		{
			std::ostringstream ss;
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					ss << dtn::data::BundleString(buildParamString());
					return ss.str();
				}
			}
			std::ostringstream err;
			err << std::hex << version;
			throw WrongVersionException(err.str());
		};

		/**
		 * @see DiscoveryServiceParam::unpack()
		 * @throw @c WrongVersionException if @version is any of
		 *  @c Discovery::DISCO_VERSION_00 or @c Discovery::DISCO_VERSION_01 (use
		 *  DiscoveryServiceParam::fromKeyValueString instead), or
		 *  @c Discovery::DTND_IPDISCOVERY (no service information specified in
		 *  protocol)
		 */
		DiscoveryServiceParam * EMailServiceParam::unpack(std::istream& stream, Discovery::Protocol version) throw (ParseException, IllegalServiceException, WrongVersionException)
		{
			std::ostringstream err;
			err << std::hex << version;
			throw WrongVersionException(err.str());
		}

/******************************************************************************/

		DHTServiceParam::DHTServiceParam()
		 : _proxy(false), _port(0)
		{
		}

		DHTServiceParam::DHTServiceParam(const uint16_t port, bool proxy)
		 : _proxy(proxy), _port(port)
		{
		}

		/** @see DiscoveryServiceParam::clone() */
		DiscoveryServiceParam * DHTServiceParam::clone() const
		{
			return new DHTServiceParam(*this);
		}

		DiscoveryServiceParam& DHTServiceParam::operator=(const DHTServiceParam& o)
		{
			_port = o._port;
			_proxy = o._proxy;
			return *this;
		}

		bool DHTServiceParam::operator==(const DHTServiceParam& o) const
		{
			return _port == o._port && _proxy == o._proxy;
		}

		std::string DHTServiceParam::buildParamString() const
		{
			std::ostringstream kv;
			if (_port != 0)
			{
				kv << "port=" << _port << ";";
			}
			if (!_proxy)
			{
				kv << "proxy=false";
			}
			return kv.str();
		}

		dtn::data::Length DHTServiceParam::getLength(Discovery::Protocol version) const
		{
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					return dtn::data::BundleString(buildParamString()).getLength();
				}
			}
			return 0;
		}

		std::string DHTServiceParam::pack(Discovery::Protocol version) const throw (WrongVersionException)
		{
			std::ostringstream ss;
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					ss << dtn::data::BundleString(buildParamString());
					return ss.str();
				}
			}
			std::ostringstream err;
			err << std::hex << version;
			throw WrongVersionException(err.str());
		};

		/** @see DiscoveryServiceParam::unpack() */
		DiscoveryServiceParam * DHTServiceParam::unpack(std::istream& stream, Discovery::Protocol version) throw (ParseException, IllegalServiceException, WrongVersionException)
		{
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					uint16_t port = 0;
					bool port_set = false;
					bool proxy = true;
					bool proxy_set = false;

					dtn::data::BundleString bs;
					stream >> bs;
					if (stream.fail())
					{
						throw ParseException("cannot parse DHTServiceParam", 0);
					}

					std::vector<string> params = dtn::utils::Utils::tokenize(";", bs);
					for (std::vector<string>::const_iterator param = params.begin();
					     param != params.end();
					     param++)
					{
						std::vector<string> part = dtn::utils::Utils::tokenize("=", *param);
						if (part.size() > 1)
						{
							if (part[0] == "port")
							{
								std::istringstream ss(part[1]);
								ss >> std::dec >> port;
								port_set = true;
							}
							else if (part[0] == "proxy")
							{
								proxy_set = true;
								if (part[1] == "false")
								{
									proxy = false;
								}
							}
						}
					}

					if (!(port_set || proxy_set) || port == 0)
					{
						std::ostringstream err;
						err << "cannot parse DHTServiceParam string " << (std::string)bs;
						throw ParseException(err.str(), 0);
					}

					return new DHTServiceParam(port, proxy);
				}
			}

			std::ostringstream err;
			err << std::hex << version;
			throw WrongVersionException(err.str());
		}

/******************************************************************************/

		DTNTPServiceParam::DTNTPServiceParam()
		 : _version(0), _quality(0), _timestamp(0)
		{
		}

		DTNTPServiceParam::DTNTPServiceParam(const unsigned int& version, const double& quality, const dtn::data::Timestamp& timestamp)
		 : _version(version), _quality(quality), _timestamp(timestamp)
		{
		}

		/** @see DiscoveryServiceParam::clone() */
		DiscoveryServiceParam * DTNTPServiceParam::clone() const
		{
			return new DTNTPServiceParam(*this);
		}

		DiscoveryServiceParam& DTNTPServiceParam::operator=(const DTNTPServiceParam& o)
		{
			_version = o._version;
			_quality = o._quality;
			_timestamp = o._timestamp;
			return *this;
		}

		bool DTNTPServiceParam::operator==(const DTNTPServiceParam& o) const
		{
			return _version == o._version && _quality == o._quality
				&& _timestamp == o._timestamp;
		}

		std::string DTNTPServiceParam::buildParamString() const
		{
			std::ostringstream kv;
			kv << "version=" << _version;
			kv << ";quality=" << _quality;
			kv << ";timestamp=" << _timestamp.toString() << ";";
			return kv.str();
		}

		dtn::data::Length DTNTPServiceParam::getLength(Discovery::Protocol version) const
		{
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					return dtn::data::BundleString(buildParamString()).getLength();
				}
			}
			return 0;
		}

		std::string DTNTPServiceParam::pack(Discovery::Protocol version) const throw (WrongVersionException)
		{
			std::ostringstream ss;
			switch (version)
			{
				case Discovery::DISCO_VERSION_01:
				case Discovery::DISCO_VERSION_00:
				{
					ss << dtn::data::BundleString(buildParamString());
					return ss.str();
				}
			}
			std::ostringstream err;
			err << std::hex << version;
			throw WrongVersionException(err.str());
		};

		/**
		 * @see DiscoveryServiceParam::unpack()
		 * @throw @c WrongVersionException if @version is any of
		 *  @c Discovery::DISCO_VERSION_00 or @c Discovery::DISCO_VERSION_01 (use
		 *  DiscoveryServiceParam::fromKeyValueString instead), or
		 *  @c Discovery::DTND_IPDISCOVERY (no service information specified in
		 *  protocol)
		 */
		DiscoveryServiceParam * DTNTPServiceParam::unpack(std::istream& stream, Discovery::Protocol version) throw (ParseException, IllegalServiceException, WrongVersionException)
		{
			std::ostringstream err;
			err << std::hex << version;
			throw WrongVersionException(err.str());
		}
	} // namespace net
} // namespace dtn
