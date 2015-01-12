/**
 * IPNDTest.cpp
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

#include "IPNDTest.h"
#include "net/DiscoveryBeacon.h"
#include "net/DiscoveryService.h"
#include "core/Node.h"
#include <sstream>
#include <arpa/inet.h>

// debugging
//#include <ibrcommon/data/Base64Stream.h>
//#include <ibrcommon/Logger.h>

CPPUNIT_TEST_SUITE_REGISTRATION(IPNDTest);
IPNDTest::IPNDTest()
	: param1(0), param2(0), param3(0), param4(0)
{
	// debugging
	//ibrcommon::Logger::enableBuffer(200);
	//ibrcommon::Logger::enableAsync(); // enable asynchronous logging feature (thread-safe)
	//ibrcommon::Logger::setVerbosity(40);
	//ibrcommon::Logger::addStream(std::cout, ibrcommon::Logger::LOGGER_DEBUG,
		//ibrcommon::Logger::LOG_DATETIME | ibrcommon::Logger::LOG_LEVEL |
		//ibrcommon::Logger::LOG_TAG);
}

/** Test IPND service descriptions */
void IPNDTest::testSerialization(dtn::net::Discovery::Protocol version) {
	using namespace dtn::net;

	DiscoveryService service1(TEST_SERVICE_TAG_1, "test1srv");
	service1.addParameter(new Discovery::Boolean(true, "bool"));
	service1.addParameter(new Discovery::UInt64(0x1234567890abcd, "uint64"));
	service1.addParameter(new Discovery::SInt64(-0x1234567890abcd, "sint64"));
	service1.addParameter(new Discovery::Fixed16(0xcdef, "fixed16"));
	service1.addParameter(new Discovery::Fixed32(0xfedcba98, "fixed32"));
	DiscoveryService service2(TEST_SERVICE_TAG_2, "test2srv");
	service2.addParameter(new Discovery::Fixed64(0x1234567890abcdef, "fixed64"));
	service2.addParameter(new Discovery::Float(3.141592653, "float"));
	service2.addParameter(new Discovery::Double(3.141592653, "double"));
	service2.addParameter(new Discovery::String("hello world!", "string"));
	service2.addParameter(new Discovery::Bytes("null\0bytes", "bytes"));

	DiscoveryBeacon beacon(version, dtn::data::EID("dtn://test.dtn"));
	beacon.setSequencenumber(0xfffe);
	beacon.addService(service);
	beacon.setPeriod(0x1245);

	// wrap bacon into a packet
	std::stringstream packet;
	packet << beacon;

	// for debugging: print byte representation
	//std::cout << std::endl << std::endl;
	//ibrcommon::Base64Stream b64(std::cout, false, 72);
	//b64 << packet.str();
	//b64.flush();
	//std::cout << std::endl << std::endl;

	// unwrap packet
	DiscoveryBeacon new_beacon;
	packet >> new_beacon;

	// check bacons before wrapping and after unwrapping
	DiscoveryBeacon::service_list& services = beacon.getServices();
	DiscoveryBeacon::service_list& read_services = new_beacon.getServices();

	CPPUNIT_ASSERT_EQUAL(beacon.getEID().getString(),
		new_beacon.getEID().getString());

	if (version >= dtn::net::Discovery::DISCO_VERSION_02)
	{
		CPPUNIT_ASSERT_EQUAL(beacon.getPeriod(), new_beacon.getPeriod());
	}
	// FIXME: bloom filter

	CPPUNIT_ASSERT_EQUAL(services.size(), read_services.size());

	DiscoveryBeacon::service_list::iterator it, it_new;
	for (it = services.begin(), it_new = read_services.begin();
	   !(it == services.end() || it_new == read_services.end());
	     it++, it_new++)
	{
		CPPUNIT_ASSERT(*it == *it_new);
	}
}

void IPNDTest::testDraft00Serialization()
{
	testSerialization(dtn::net::Discovery::DISCO_VERSION_00);
}

void IPNDTest::testDraft01Serialization()
{
	testSerialization(dtn::net::Discovery::DISCO_VERSION_01);
}

void IPNDTest::testDraft02Serialization()
{
	testSerialization(dtn::net::Discovery::DISCO_VERSION_02);
}
