// FIXME header

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
				//DISCO_VERSION_02 = 0x04, /*< IPND draft 02 */
				DISCO_UNSPEC     = 0xFF, /*< Unspecified discovery protocol */
			};

			typedef uint8_t tag_t;

			class Type {
			public:
				virtual std::string serialize(const Protocol version) const = 0;
				virtual dtn::data::Length deserialize(const Protocol version, std::istream& stream) = 0;

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


			/**
			 * @tparam T Value type
			 * @tparam Tag IPND TLV Service Tag
			 * @tparam Length Length of the value, in bytes.  If @c 0, the value type
			 *  takes care of prefixing its value with a length field when shifted
			 *  onto a stream, like dtn::data::Number or dtn::data::BundleString do.
			 *  If greater than @c 0, the length is implicitly encoded in the @c Tag
			 *  and not written to the stream.
			 */
			template<typename T, const tag_t Tag, const dtn::data::Length Length>
			class Field : public Type
			{
			public:
				static const tag_t TAG = Tag;

				class Factory : public Type::Factory
				{
					Factory() : Type::Factory(Tag) {}
					virtual Type * create() { return new Field<T, Tag, Length>(); };
				};

				/**
				 * @param hint Parameter name for IPND draft 00/01
				 */
				Field()
					: value_(0), hint_("") {}
				Field(const T& value, const std::string& hint = "")
					: value_(value), hint_(hint) {}

				/** @throws std::runtime_error if serialization fails */
				virtual std::string serialize(const Protocol version) const
				{
					// FIXME: version
					std::ostringstream ss;

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
							<< " for IPND tag " << Tag << " with length " << Length;
						throw std::runtime_error(err.str());
					}

					return ss.str();
				}

				/**
				 * @returns number of bytes read from string
				 * @throws ParseException if field could not be decoded 
				 */
				virtual dtn::data::Length deserialize(const Protocol version, std::istream& stream)
				{
					// FIXME: version
					uint8_t tag;
					size_t length = Length;
					size_t bytes_read = 0, bytes_temp = 0;
					std::ostringstream err;

					// tag
					stream.read((char *) (&tag), 1);
					if (!stream.good())
					{
						err << "could not parse IPND tag " << Tag;
						throw ParseException(err.str(), bytes_read);
					}
					if (tag != (uint8_t) Tag)
					{
						err << "expected IPND tag " << Tag << ", but found " << (int) tag;
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

					if (!stream.good())
					{
						err << "could not parse value for IPND tag " << Tag
							<< " with length " << Length;
						throw ParseException(err.str(), bytes_read);
					}
					bytes_read += stream.gcount();

					return bytes_read;
				}
			private:
				T value_;
				std::string hint_;
			};

			typedef Field<uint8_t,                 0, 1> BooleanField;
			typedef Field<dtn::data::Number,       1, 0> UInt64Field;
			typedef Field<dtn::data::Number,       2, 0> SInt64Field;
			typedef Field<uint16_t,                3, 2> Fixed16Field;
			typedef Field<uint32_t,                4, 4> Fixed32Field;
			typedef Field<uint64_t,                5, 8> Fixed64Field;
			typedef Field<float,                   6, 2> FloatField;
			typedef Field<double,                  7, 4> DoubleField;
			typedef Field<dtn::data::BundleString, 8, 0> StringField;
			typedef Field<dtn::data::BundleString, 9, 0> BytesField;
		} // namespace Discovery
	} // namespace net
} // namespace dtn



