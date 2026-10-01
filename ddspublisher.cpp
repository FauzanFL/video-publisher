#include "ddspublisher.h"
#include "qos_profiles.h"
#include <iostream>

DdsPublisher::DdsPublisher(dds::domain::DomainParticipant participant, QObject* parent)
    : QObject(parent),
    publisher_(participant),
    frame_topic_(participant, "VideoTopic"),
    frame_writer_(publisher_, frame_topic_, QoSProfile::VideoPub(publisher_))
{}

void DdsPublisher::publishFrame(const std::vector<uint8_t> &frameData)
{
    if (frameData.empty()) {
        return;
    }

    VideoMsg::Frame frame;
    frame.id(frame_counter_++);
    frame.data(frameData);

    frame_writer_.write(frame);

    emit framePublished(frame.id(), frameData.size());
}