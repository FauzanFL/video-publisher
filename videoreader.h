#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <QObject>
#include <QString>
#include <gst/gst.h>
#include <vector>

class VideoReader : public QObject
{
    Q_OBJECT
public:
    // Pilihan mode output GStreamer
    enum class StreamMode {
        Compressed_H264, // Ekstrak byte-stream H.264 (Bandwidth kecil)
        Raw_BGRA         // Decode menjadi frame mentah BGRA (Bandwidth besar)
    };

    explicit VideoReader(const QString& filePath, StreamMode mode, QObject* parent = nullptr);
    ~VideoReader() override;

    bool start();
    void stop();

signals:
    // Sinyal dipancarkan saat frame H.265 baru berhasil diekstrak
    void frameExtracted(const std::vector<uint8_t>& data);
    void eosReached();
    void errorOccurred(const QString& message);

private:
    QString m_filePath;
    StreamMode m_mode;
    GstElement* pipeline_ = nullptr;
    GstElement* appsink_ = nullptr;

    static GstFlowReturn on_new_sample(GstElement* sink, gpointer user_data);
    static gboolean on_bus_message(GstBus* bus, GstMessage* msg, gpointer user_data);
};

#endif // VIDEOREADER_H
