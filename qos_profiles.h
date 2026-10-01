#pragma once
#include <dds/dds.hpp>

namespace QoSProfile {
inline dds::pub::qos::DataWriterQos VideoPub(dds::pub::Publisher& pub) {
    return pub.default_datawriter_qos()
           << dds::core::policy::Reliability::BestEffort()
           << dds::core::policy::Durability::Volatile()
           << dds::core::policy::History::KeepLast(3);
}
}