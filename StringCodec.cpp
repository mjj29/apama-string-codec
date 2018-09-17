/**
 * Title:        StringCodec.cpp
 * Description:  Universal Messaging transport
 * $Copyright (c) 2016-2017 Software AG, Darmstadt, Germany and/or Software AG U                                                                             SA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or t                                                                             heir licensors.$
 * Use, reproduction, transfer, publication or disclosure is prohibited except a                                                                             s specifically provided for in your License Agreement with Software AG
 * @Version:     $Id: StringCodec.cpp 321386 2017-12-14 10:26:04Z cat $
 */

#include <sag_connectivity_plugins.hpp>
#include <connectivity-plugins/include/MessageData.h>
#include <connectivity-plugins/include/ConfigParse.h>

namespace com {
namespace softwareag {
namespace connectivity {
namespace plugins {

class StringCodec: public com::softwareag::connectivity::AbstractSimpleCodec
{
public:
        explicit StringCodec(const CodecConstructorParameters &params)
                : com::softwareag::connectivity::AbstractSimpleCodec(params),
                  nullTerminated(false)
        {
                std::tie(nullTerminated) = getConfigOptions<bool>("StringCodec",                                                                              config,
                                std::make_tuple("nullTerminated"),
                                std::make_tuple(false),
                                std::make_tuple(false));
        }

        virtual bool transformMessageTowardsTransport(com::softwareag::connectiv                                                                             ity::Message &m)
        {
                data_t payload = std::move(m.getPayload());
                const char *str = get<const char*>(payload);
                size_t len = strlen(str)+(nullTerminated?1:0);
                buffer_t buffer(len);
                std::copy(str, str+len, buffer.begin());
                m.setPayload(std::move(buffer));
                return true;
        }

        virtual bool transformMessageTowardsHost(com::softwareag::connectivity::                                                                             Message &m)
        {
                data_t payload = std::move(m.getPayload());
                buffer_t &data = get<buffer_t>(payload);
                if (0 == data.size()) {
                        m.setPayload(data_t(""));
                } else {
                        m.setPayload(data_t((const char*)data.begin(), data.size                                                                             ()));
                }
                return true;
        }

private:
        bool nullTerminated;
};

SAG_DECLARE_CONNECTIVITY_CODEC_CLASS(StringCodec)

}}}} // com.softwareag.connectivity.plugins



