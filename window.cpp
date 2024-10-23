#include "window.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <exception>
#include <iostream>

static oc::Window* global_window{ nullptr };

static bool global_glfw_initialized{ false };

static void create_gl_objects(GLsizei width, GLsizei height) {

	{
		float vertices[]{
			-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0
		};

		GLuint vao{};
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint vbo{};
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, nullptr);
		glEnableVertexAttribArray(0);
	}

	{

		const char* vs = R"(
		#version 330 core
    
        layout (location = 0) in vec2 position;
    
        out vec2 tex_mapping;
    
        void main() {
            gl_Position = vec4(position.xy, 0.0, 1.0);
            vec2 mapping = vec2(min(position.x + 1.0, 1.0), min(position.y + 1.0, 1.0));
			tex_mapping = vec2(mapping.x, 1.0 - mapping.y);
        }
		)";
		const char* fs = R"(
            #version 330 core
    
            in vec2 tex_mapping;
            out vec4 fragment_color;
    
            uniform sampler2D tex2d;
    
            void main() {
                fragment_color = texture(tex2d, tex_mapping);
            }
		)";

		auto vs_h = glCreateShader(GL_VERTEX_SHADER);
		auto fs_h = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vs_h, 1, &vs, nullptr);
		glCompileShader(vs_h);

		int success{ 0 };
		glGetShaderiv(vs_h, GL_COMPILE_STATUS, &success);
		if (success == 0) {
			char buf[512];
			glGetShaderInfoLog(vs_h, 512, nullptr, buf);
			std::cout << buf << '\n';
		}

		glShaderSource(fs_h, 1, &fs, nullptr);
		glCompileShader(fs_h);
		success = 0;
		glGetShaderiv(vs_h, GL_COMPILE_STATUS, &success);
		if (success == 0) {
			char buf[512];
			glGetShaderInfoLog(vs_h, 512, nullptr, buf);
			std::cout << buf << '\n';
		}


		GLuint p = glCreateProgram();
		glAttachShader(p, vs_h);
		glAttachShader(p, fs_h);
		glLinkProgram(p);
		glUseProgram(p);
	}

	{
		GLuint texture_handle{ 0 };
		glGenTextures(1, &texture_handle);
		glBindTexture(GL_TEXTURE_2D, texture_handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

}

static void global_key_event_func(GLFWwindow* handle, int key, int scan, int action, int mods) {
	if (global_window) {
		if (global_window->key_callback) {
			global_window->key_callback(oc::KeyEventArgs{ .key{ key }, .action{ action } });
		} else if (key == GLFW_KEY_ESCAPE) {
			global_window->close();
		}
	}
}

static void global_cursor_event_func(GLFWwindow* handle, double x, double y) {
	if (global_window && global_window->cursor_callback) {
		global_window->cursor_callback(oc::CursorEventArgs{ .x{ x }, .y{ y } });
	}
}

oc::Window::Window(const char* title, int width, int height, float scale) :
	m_framebuffer { static_cast<int>(width * scale), static_cast<int>(height * scale) }
{

	if (global_glfw_initialized) {
		std::cerr << "Cannot reinitialize GLFW.";
		std::exit(EXIT_SUCCESS);
	}

	if (!glfwInit()) {
		std::cerr << "Could initialize GLFW.";
		std::exit(EXIT_SUCCESS);
	}

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	m_handle = glfwCreateWindow(
		width, 
		height,
		title, 
		nullptr, 
		nullptr
	);

	if (!m_handle) {
		std::cerr << "Could not create GLFW window.";
		std::exit(EXIT_SUCCESS);
	}

	const auto* videoMode{ glfwGetVideoMode(glfwGetPrimaryMonitor()) };
	glfwSetWindowPos(m_handle, (videoMode->width/2) - (width/2), (videoMode->height / 2) - (height/2));

	glfwSetKeyCallback(m_handle, global_key_event_func);
	glfwSetCursorPosCallback(m_handle, global_cursor_event_func);

	glfwShowWindow(m_handle);
	glfwMakeContextCurrent(m_handle);

	if (glewInit() != GLEW_OK) {
		std::cerr << "Could not initialize GLEW!";
		std::exit(EXIT_SUCCESS);
	}
	create_gl_objects(m_framebuffer.get_width(), m_framebuffer.get_height());

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glfwSwapInterval(1);

	global_glfw_initialized = true;
	global_window = this;

}

oc::Window::~Window() {
	std::cout << "Terminating..." << std::endl;
	glfwDestroyWindow(m_handle);
	glfwTerminate();
}

double oc::Window::get_time() const {
	return glfwGetTime();
}

void oc::Window::close() const {
	glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
}

void oc::Window::disable_cursor() const {
	glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void oc::Window::swap() const {
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_framebuffer.get_width(), m_framebuffer.get_height(), GL_RGBA, GL_UNSIGNED_BYTE, m_framebuffer.raw());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glfwSwapBuffers(m_handle);
}

bool oc::Window::is_alive() const {
	glfwPollEvents();
	return !glfwWindowShouldClose(m_handle);
}

oc::Framebuffer::Framebuffer(int width, int height) :
	m_width{width}, m_height{height}, m_color_buffer(width*height), m_depth_buffer(width*height)
{

	for (int i = 0; i < m_color_buffer.size(); ++i) {
		m_color_buffer[i] = oc::color_from(255, 255, 255, 255);
	}

}

oc::color oc::get_red(color c) {
	return c & oc::Red;
}

oc::color oc::get_green(color c) {
	return c & oc::Green;
}

oc::color oc::get_blue(color c) {
	return c & oc::Blue;
}

oc::color oc::get_alpha(color c) {
	return c & oc::Alpha;
}

oc::color oc::color_from(color red, color green, color blue, color alpha) {
	return (alpha << 24) | (blue << 16) | (green << 8) | (red << 0);
}




