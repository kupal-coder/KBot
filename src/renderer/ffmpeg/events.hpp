#pragma once

#include "render_settings.hpp"

#include <Geode/Result.hpp>

#include <filesystem>
#include <vector>

/*
 * FFmpeg API (eclipse.ffmpeg-api) event bindings.
 *
 * The old bindings in this file were written against Geode 4's legacy event
 * system (`class MyEvent : public geode::Event` + `post()`), which no longer
 * exists in Geode 5 - events are now the templated `geode::Event<Marker, ...>`.
 *
 * FFmpeg API v2.0.0 is the Geode 5 build of that mod, but its client header is
 * not published anywhere anymore (the EclipseMenu/ffmpeg-api repository is
 * gone), and the event protocol cannot be reconstructed reliably from the
 * compiled mod. Rather than deleting the renderer's API path outright, the
 * interface is kept intact and stubbed out so the rest of the renderer keeps
 * compiling and can report a clean error instead of crashing.
 *
 * To restore FFmpeg API support: drop in the official v2 header for
 * `ffmpeg::events` and set XDBOT_FFMPEG_API_SUPPORTED to 1.
 */
#define XDBOT_FFMPEG_API_SUPPORTED 0

namespace ffmpeg::events {

namespace impl {
    inline geode::Result<> unsupported() {
        return geode::Err("FFmpeg API support is not available in this build");
    }
}

class Recorder {
public:
    Recorder() = default;
    ~Recorder() = default;

    bool isValid() const { return false; }

    geode::Result<> init(RenderSettings const&) { return impl::unsupported(); }

    void stop() {}

    geode::Result<> writeFrame(const std::vector<uint8_t>&) { return impl::unsupported(); }

    static std::vector<std::string> getAvailableCodecs() { return {}; }
};

class AudioMixer {
public:
    AudioMixer() = delete;

    static geode::Result<> mixVideoAudio(
        std::filesystem::path const&, std::filesystem::path const&, std::filesystem::path const&
    ) {
        return impl::unsupported();
    }

    static geode::Result<> mixVideoRaw(
        std::filesystem::path const&, const std::vector<float>&, std::filesystem::path const&
    ) {
        return impl::unsupported();
    }
};

}
