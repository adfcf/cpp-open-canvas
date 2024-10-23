#pragma once
#include <cstdint>
#include <vector>

struct GLFWwindow;

namespace oc {

    using color = std::uint32_t;
    using depth = float;

    constexpr color Alpha{ 0xff'00'00'00 };
    constexpr color Blue{ 0xff'ff'00'00 };
    constexpr color Green{ 0xff'00'ff'00 };
    constexpr color Red{ 0xff'00'00'ff };

    struct KeyEventArgs {
        int key;
        int action;
    };

    struct CursorEventArgs {
        double x;
        double y;
    };

    using key_event_callback = void (*)(KeyEventArgs);
    using cursor_event_callback = void (*)(CursorEventArgs);

    color get_red(color c);
    color get_green(color c);
    color get_blue(color c);
    color get_alpha(color c);
    color color_from(color red, color green, color blue, color alpha);

}

namespace oc {

    class Window;

    class Framebuffer {

        const int m_width;
        const int m_height;

        std::vector<color> m_color_buffer;
        std::vector<depth> m_depth_buffer;

    public:

        Framebuffer(int width, int height);

        color& color_at(int x, int y) {
            return m_color_buffer[x + (y * m_width)];
        }

        depth& depth_at(int x, int y) {
            return m_depth_buffer[x + (y * m_width)];
        }

        int get_width() const {
            return m_width;
        }

        int get_height() const {
            return m_height;
        }

    private:

        friend class Window;
        const color* raw() const {
            return m_color_buffer.data();
        }

    };

    class Window {
        GLFWwindow* m_handle{ nullptr };
        Framebuffer m_framebuffer;
    public:

        key_event_callback key_callback{ nullptr };
        cursor_event_callback cursor_callback{ nullptr };

        Window(const char* title, int width, int height, float scale);

        ~Window();

        Framebuffer& get_framebuffer() {
            return m_framebuffer;
        }

        double get_time() const;

        void close() const;

        void disable_cursor() const;

        void swap() const;

        bool is_alive() const;

    };

}

