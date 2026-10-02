#include "videoreader.h"
#include "gst/app/gstappsink.h"
#include <iostream>
#include <QDebug>

VideoReader::VideoReader(const QString& filePath, StreamMode mode, QObject* parent)
    : QObject{parent},
    m_filePath(filePath),
    m_mode(mode),
    pipeline_(nullptr),
    appsink_(nullptr)
{
    GError* error = nullptr;
    std::string pipelineStr;

    // Membangun Pipeline GStreamer secara dinamis berdasarkan Mode
    if (m_mode == StreamMode::Compressed_H265) {
        // Mode 1: H.265 (Efisien, hanya parsing bitstream dari MP4)
        // Pipeline: Baca file MP4 -> Demux -> Parse H.265 byte-stream -> AppSink (sync=true)
        pipelineStr =
            "filesrc location=" + m_filePath.toStdString() + " ! qtdemux ! h265parse "
            "! video/x-h265,stream-format=byte-stream,alignment=au "
            "! appsink name=mysink emit-signals=true sync=true";
        qDebug() << "[VideoReader] Mode: Terkompresi H.265";
    } else {
        // Mode 2: Raw Frame (Mendecode MP4 menjadi frame mentah BGRA 640x480)
        // Menggunakan decodebin agar otomatis membaca segala format video lalu diconvert ke BGRA
        pipelineStr =
            "filesrc location=" + m_filePath.toStdString() + " ! decodebin ! videoconvert "
            "! video/x-raw,format=BGRA,width=640,height=480 "
            "! appsink name=mysink emit-signals=true sync=true";
        qDebug() << "[VideoReader] Mode: Raw Frame Mentah (BGRA 640x480)";
    }
    pipeline_ = gst_parse_launch(pipelineStr.c_str(), &error);

    if (error) {
        std::cerr << "[VideoReader] Pipeline GStreamer Error: " << error->message << std::endl;
        g_error_free(error);
        return;
    }

    appsink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "mysink");
    if (appsink_) {
        g_signal_connect_data(appsink_, "new-sample", G_CALLBACK(on_new_sample), this, nullptr, static_cast<GConnectFlags>(0));
    }

    // Pasang bus watch untuk menangkap event End of Stream (EOS) dan Error
    GstBus* bus = gst_element_get_bus(pipeline_);
    if (bus) {
        gst_bus_add_watch(bus, on_bus_message, this);
        gst_object_unref(bus);
    }
}

VideoReader::~VideoReader() {
    stop();
    if (appsink_) {
        gst_object_unref(appsink_);
        appsink_ = nullptr;
    }
    if (pipeline_) {
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
}

bool VideoReader::start() {
    if (!pipeline_) return false;
    return gst_element_set_state(pipeline_, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE;
}

void VideoReader::stop() {
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
    }
}

GstFlowReturn VideoReader::on_new_sample(GstElement* sink, gpointer user_data) {
    auto* self = static_cast<VideoReader*>(user_data);
    if (!self) return GST_FLOW_ERROR;

    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    if (!sample) return GST_FLOW_ERROR;

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    if (buffer) {
        GstMapInfo map;
        if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            std::vector<uint8_t> data(map.data, map.data + map.size);
            emit self->frameExtracted(data);
            gst_buffer_unmap(buffer, &map);
        }
    }

    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

gboolean VideoReader::on_bus_message(GstBus* bus, GstMessage* msg, gpointer user_data) {
    Q_UNUSED(bus);
    auto* self = static_cast<VideoReader*>(user_data);
    if (!self) return TRUE;

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        std::cout << "[VideoReader] End of Stream (Video Selesai)." << std::endl;
        emit self->eosReached();
        break;
    case GST_MESSAGE_ERROR: {
        GError* err = nullptr;
        gchar* debug_info = nullptr;
        gst_message_parse_error(msg, &err, &debug_info);
        QString errStr = QString::fromUtf8(err->message);
        std::cerr << "[VideoReader] Error: " << err->message << std::endl;
        g_error_free(err);
        g_free(debug_info);
        emit self->errorOccurred(errStr);
        break;
    }
    default:
        break;
    }
    return TRUE;
}