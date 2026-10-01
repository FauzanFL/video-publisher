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
    explicit VideoReader(const QString& filePath, QObject* parent = nullptr);
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
    GstElement* pipeline_ = nullptr;
    GstElement* appsink_ = nullptr;

    static GstFlowReturn on_new_sample(GstElement* sink, gpointer user_data);
    static gboolean on_bus_message(GstBus* bus, GstMessage* msg, gpointer user_data);
};

#endif // VIDEOREADER_H
