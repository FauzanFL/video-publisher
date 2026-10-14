#include <QCoreApplication>
#include <gst/gst.h>
#include <dds/dds.hpp>
#include <iostream>
#include <memory>

#include "ddspublisher.h"
#include "videoreader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Inisialisasi GStreamer
    gst_init(&argc, &argv);

    // Inisialisasi DDS DomainParticipant (Domain 99)
    dds::domain::DomainParticipant participant(99);

    // Membuat objek DdsPublisher dan VideoReader
    DdsPublisher publisher(participant);
    VideoReader reader("rtsp://localhost:8554/stream", VideoReader::StreamMode::Compressed_H264);

    // Hubungkan Sinyal Qt: VideoReader -> Slot DdsPublisher
    QObject::connect(&reader, &VideoReader::frameExtracted,
                     &publisher, &DdsPublisher::publishFrame);

    // Logging aktivitas pengiriman setiap 30 frame
    QObject::connect(&publisher, &DdsPublisher::framePublished,
                     [](uint32_t frameId, size_t size) {
                         if (frameId % 1 == 0) {
                             std::cout << "[Publisher] Mengirim frame #" << frameId << " | Ukuran: " << size << " bytes" << std::endl;
                         }
                     });

    // Otomatis keluar aplikasi saat video selesai (EOS) atau error
    QObject::connect(&reader, &VideoReader::eosReached, &a, &QCoreApplication::quit);
    QObject::connect(&reader, &VideoReader::errorOccurred, [&a](const QString& err) {
        std::cerr << "[Main] Error: " << err.toStdString() << std::endl;
        QCoreApplication::exit(-1);
    });

    std::cout << "[Publisher] Memulai pengiriman video simulasi..." << std::endl;

    if (!reader.start()) {
        std::cerr << "[Main] Gagal menjalankan VideoReader!" << std::endl;
        return -1;
    }

    return QCoreApplication::exec();
}
