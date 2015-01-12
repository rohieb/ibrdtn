// FIXME header
#ifndef _DISCOVERY_H_
#define _DISCOVERY_H_

#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/BundleString.h"
#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <stdexcept>

namespace dtn {
	namespace net {
		namespace Discovery {

			class WrongVersionException : public ibrcommon::Exception
			{
				public:
					WrongVersionException(std::string what = "(n/a)")
					 : ibrcommon::Exception(std::string("Unspecified or unsupported "
								"discovery protocol version: ").append(what)) {}
					virtual ~WrongVersionException() throw() {}
			};

			class IllegalServiceException : public ibrcommon::Exception
			{
				public:
					IllegalServiceException(std::string what = "(n/a)")
					 : ibrcommon::Exception(std::string("No such service, or illegal "
								"parameters for service: ").append(what)) {}
					virtual ~IllegalServiceException() throw() {}
			};

			class ParseException : public ibrcommon::Exception
			{
				size_t bytes_read_;
				public:
					ParseException(std::string what = "(n/a)", size_t bytes_read = 0)
					 : ibrcommon::Exception(), bytes_read_(bytes_read)
					{
						std::ostringstream ss;
						ss << "Parse error at input byte " << bytes_read << ": " << what;
						_what = ss.str();
					}
					virtual ~ParseException() throw() {}
					inline const size_t getBytesRead() const { return bytes_read_; }
			};

			enum Protocol {
				DTND_IPDISCOVERY = 0x00, /*< Legacy DTND discovery */
				DISCO_VERSION_00 = 0x01, /*< IPND draft 00 */
				DISCO_VERSION_01 = 0x02, /*< IPND draft 01 */
				DISCO_VERSION_02 = 0x04, /*< IPND draft 02 */
				DISCO_UNSPEC     = 0xFF, /*< Unspecified discovery protocol */
			};

			typedef uint8_t tag_t;

			class Type {
			public:
				virtual std::string serialize(const Protocol version) const = 0;
				virtual dtn::data::Length deserialize(const Protocol version, std::istream& stream) = 0;
				virtual dtn::data::Length getLength(const Protocol version) const = 0;

				class Factory {
				private:
					static std::map<tag_t, Type::Factory *> type_registry_;
					tag_t tag_;
				public:
					Factory(tag_t tag) : tag_(tag) {
						type_registry_[tag] = this;
					}
					~Factory() {
						type_registry_.erase(tag_);
					}
					virtual Type * create() = 0;
				};
			};

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

			/**
			 * @tparam T Value type
			 * @tparam Length Length of the value, in bytes.  If @c 0, the value type
			 *  takes care of prefixing its value with a length field when shifted
			 *  onto a stream, like dtn::data::Number or dtn::data::BundleString do.
			 *  If greater than @c 0, the length is implicitly encoded in the @c Tag
			 *  and not written to the stream.
			 */
			template<int Length, typename T> struct LengthHelper
			{
				static dtn::data::Length getLength(const T& value) { return Length; }
			};
			template<typename T> struct LengthHelper<0, T>
			{
				static dtn::data::Length getLength(const T& value) { return value.getLength(); }
			};

			/**
			 * Implementation of a primitive (non-constructed) IPND type
			 * @tparam T Value type
			 * @tparam Tag IPND TLV Service Tag
			 * @tparam Length Length of the value, in bytes.  If @c 0, the value type
			 *  takes care of prefixing its value with a length field when shifted
			 *  onto a stream, like dtn::data::Number or dtn::data::BundleString do.
			 *  If greater than @c 0, the length is implicitly encoded in the @c Tag
			 *  and not written to the stream.
			 */
			template<typename T, const tag_t Tag, const dtn::data::Length Length>
			class PrimitiveType : public Type
			{
			public:
				static const tag_t TAG = Tag;

				// FIXME: needed?
				//class Factory : public Type::Factory
				//{
					//Factory() : Type::Factory(Tag) {}
					//virtual Type * create() { return new PrimitiveType<T, Tag, Length>(); };
				//};

				/**
				 * @param hint Parameter name for IPND draft 00/01
				 */
				PrimitiveType()
					: value_(0), hint_("") {}
				PrimitiveType(const T& value, const std::string& hint = "")
					: value_(value), hint_(hint) {}

				/**
				 * @throws std::runtime_error if serialization fails
				 * @throws WrongVersionException if the given Discovery version is not
				 *  implemented
				 */
				virtual std::string serialize(const Protocol version) const
				{
					// FIXME: version
					std::ostringstream ss;

					switch (version)
					{
						case DISCO_VERSION_02:
						{
							// tag
							ss.put((char) Tag);
							if (!ss.good())
							{
								std::ostringstream err;
								err << "stream was bad after writing IPND tag " << Tag;
								throw std::runtime_error(err.str());
							}

							// length(?) and value
							if (Length == 0)
							{
								// value type takes care of length
								ss << value_;
							}
							else
							{
								// FIXME: is this network byte order?
								ss.write((char *) &value_, Length);
							}

							if (!ss.good())
							{
								std::ostringstream err;
								err << "stream was bad after writing value " << value_
									<< " for IPND 02 tag " << Tag << " with length " << Length;
								throw std::runtime_error(err.str());
							}

							break;
						}

						case DISCO_VERSION_01:
						case DISCO_VERSION_00:
						{
							ss << hint_ << "=" << value_;
							if (!ss.good())
							{
								std::ostringstream err;
								err << "stream was bad after writing value " << value_
									<< " for IPND 00/01 hint " << hint_;
								throw std::runtime_error(err.str());
							}
							break;
						}

						default:
						{
							std::ostringstream err;
							err << std::hex << version;
							throw WrongVersionException(err.str());
						}
					}

					return ss.str();
				}

				/**
				 * @returns number of bytes read from string
				 * @throws ParseException if field could not be decoded
				 * @throws WrongVersionException if the given Discovery version is not
				 *  implemented
				 */
				virtual dtn::data::Length deserialize(const Protocol version, std::istream& stream)
				{
					std::ostringstream err;
					// FIXME: version

					size_t bytes_read = 0;
					switch (version)
					{
						case DISCO_VERSION_02:
						{
							uint8_t tag;
							size_t length = Length;
							size_t bytes_temp = 0;

							// tag
							stream.read((char *) (&tag), 1);
							if (stream.bad() || stream.fail())
							{
								err << "could not parse IPND 02 tag " << Tag;
								throw ParseException(err.str(), bytes_read);
							}
							if (tag != (uint8_t) Tag)
							{
								err << "expected IPND 02 tag " << Tag << ", but found " << (int) tag;
								throw ParseException(err.str(), bytes_read);
							}
							bytes_read += stream.gcount();

							// length(?) and value
							if (Length == 0)
							{
								stream >> value_;
							}
							else
							{
								// FIXME: is this network byte order?
								stream.read((char *) &value_, Length);
							}

							if (stream.bad() || stream.fail())
							{
								err << "could not parse value for IPND tag " << Tag
									<< " with length " << Length;
								throw ParseException(err.str(), bytes_read);
							}
							bytes_read += stream.gcount();

							return bytes_read;
						}

						case DISCO_VERSION_01:
						case DISCO_VERSION_00:
						{
							std::string s;
							stream >> s;
							if (stream.bad() || stream.fail())
							{
								throw ParseException("could not parse IPND 00/01 field", 0);
							}

							size_t tok = s.find('=');
							if (tok == std::string::npos)
							{
								err << "could not parse IPND 00/01 field: " << s;
								throw ParseException(err.str(), 0);
							}

							hint_ = s.substr(0, tok-1);
							fromString<T>(s.substr(tok+1), value_);

							return s.size();
						}

						default:
						{
							std::ostringstream err;
							err << std::hex << version;
							throw WrongVersionException(err.str());
						}
					}
				}

				/**
				 * @returns number of bytes neccessary for serializing
				 * @throws WrongVersionException if the given Discovery version is not
				 *  implemented
				 */
				virtual dtn::data::Length getLength(const Protocol version) const
				{
					switch (version)
					{
						case DISCO_VERSION_00:
						case DISCO_VERSION_01:
						{
							std::string val = toString<T>(value_);
							return hint_.size()
								+ 1                      // sizeof('=')
								+ val.size();
						}

						case DISCO_VERSION_02:
						{
							return LengthHelper<Length, T>::getLength(value_);
						}

						default:
						{
							std::ostringstream err;
							err << std::hex << version;
							throw WrongVersionException(err.str());
						}
					}
				}

			private:
				T value_;
				std::string hint_;
			};

			typedef PrimitiveType<uint8_t,                   0, 1> Boolean;
			typedef PrimitiveType<dtn::data::SDNV<uint64_t>, 1, 0> UInt64;
			typedef PrimitiveType<dtn::data::SDNV<int64_t>,  2, 0> SInt64;
			typedef PrimitiveType<uint16_t,                  3, 2> Fixed16;
			typedef PrimitiveType<uint32_t,                  4, 4> Fixed32;
			typedef PrimitiveType<uint64_t,                  5, 8> Fixed64;
			typedef PrimitiveType<float,                     6, 2> Float;
			typedef PrimitiveType<double,                    7, 4> Double;
			typedef PrimitiveType<dtn::data::BundleString,   8, 0> String;
			typedef PrimitiveType<dtn::data::BundleString,   9, 0> Bytes;
		} // namespace Discovery
	} // namespace net
} // namespace dtn

#endif // _DISCOVERY_H_
