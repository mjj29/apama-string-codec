/**
 * Title:        StringCodec.cpp
 * Description:  Universal Messaging transport
 * $Copyright (c) 2016-2017 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors.$
 * Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG
 * @Version:     $Id: StringCodec.cpp 321386 2017-12-14 10:26:04Z cat $
 */

#include <sag_connectivity_plugins.hpp>
#include <connectivity-plugins/include/ConfigParse.h>
#include <unicode/ucnv.h>

namespace com {
namespace softwareag {
namespace connectivity {
namespace plugins {

class StringCodec: public com::softwareag::connectivity::AbstractSimpleCodec
{
public:
	explicit StringCodec(const CodecConstructorParameters &params)
		: com::softwareag::connectivity::AbstractSimpleCodec(params),
		nullTerminated(false),
		checkEncoding(false),
		metadataCharsetKey{"charset"}
	{
		std::tie(nullTerminated, checkEncoding) = getConfigOptions<bool, bool>("StringCodec", config,
				std::make_tuple("nullTerminated", "useMessageEncoding"),
				std::make_tuple(false, false),
				std::make_tuple(false, false));
	}

	virtual bool transformMessageTowardsTransport(com::softwareag::connectivity::Message &m)
	{
		data_t payload = std::move(m.getPayload());
		const char *str = get<const char*>(payload);
		auto &meta = m.getMetadataMap();

		std::string charset = "utf-8";
		map_t::iterator it;
		if (checkEncoding && (it = meta.find(metadataCharsetKey)) != meta.end()) {
			charset = get<const char*>(it->second);
			size_t len=2*(strlen(str)+2);
			buffer_t buffer(len);
			UErrorCode errCode=U_ZERO_ERROR;
			fprintf(stderr, "converted from: %s\n", str);
			ucnv_convert(charset.c_str(), "UTF-8", (char*) buffer.begin(), len, str, -1, &errCode);
			fprintf(stderr, "converted to: %s\n", (const char *) buffer.begin());
			if(U_FAILURE (errCode)) {
				throw std::runtime_error(("Could not convert string into encoding "+charset+" because of ")+u_errorName(errCode));
			}
			m.setPayload(std::move(buffer));
		} else {
			size_t len = strlen(str)+(nullTerminated?1:0);
			buffer_t buffer(len);
			std::copy(str, str+len, buffer.begin());
			m.setPayload(std::move(buffer));
		}

		meta[metadataCharsetKey] = data_t{charset};
		return true;
	}

	virtual bool transformMessageTowardsHost(com::softwareag::connectivity::Message &m)
	{
		data_t payload = std::move(m.getPayload());
		buffer_t &data = get<buffer_t>(payload);
		auto &meta = m.getMetadataMap();
		if (0 == data.size()) {
			m.setPayload(data_t(""));
		} else {
			map_t::iterator it;
			if (checkEncoding && (it = meta.find(metadataCharsetKey)) != meta.end()) {
				size_t len=(data.size()+2)*4;
				char* target=(char *)malloc(len);
				UErrorCode errCode=U_ZERO_ERROR;
				ucnv_convert("UTF-8", get<const char*>(it->second), target, len, (char*) data.begin(), data.size(), &errCode);
				if(U_FAILURE (errCode)) {
					throw std::runtime_error(("Could not convert string into encoding "+std::string(get<const char*>(it->second))+" because of ")+u_errorName(errCode));
				}
				m.setPayload(data_t(target));
			} else {
				m.setPayload(data_t((const char*)data.begin(), data.size()));
			}
		}
		return true;
	}

private:
	bool nullTerminated;
	bool checkEncoding;
	data_t metadataCharsetKey;
};

SAG_DECLARE_CONNECTIVITY_CODEC_CLASS(StringCodec)

}}}} // com.softwareag.connectivity.plugins



