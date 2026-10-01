#ifndef DDSPUBLISHER_H
#define DDSPUBLISHER_H

#include <QObject>
#include <vector>
#include <dds/dds.hpp>
#include "VideoMessage.hpp"

class DdsPublisher : public QObject {
    Q_OBJECT
public:
    explicit DdsPublisher(dds::domain::DomainParticipant participant, QObject* parent = nullptr);
    ~DdsPublisher() override = default;

public slots:
    // Slot Qt yang dipanggil untuk mempublikasikan byte frame ke DDS
    void publishFrame(const std::vector<uint8_t>& frameData);

signals:
    // Sinyal Qt untuk pemantauan / logging pengiriman
    void framePublished(uint32_t frameId, size_t size);

private:
    dds::pub::Publisher publisher_;
    dds::topic::Topic<VideoMsg::Frame> frame_topic_;
    dds::pub::DataWriter<VideoMsg::Frame> frame_writer_;
    uint32_t frame_counter_ = 0;
};

#endif // DDSPUBLISHER_H
