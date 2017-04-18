//copyright Eric Budai 2017
#pragma once

#define NOMINMAX

#include "memory_mapped_file.h"
#include <atomic>
#include <Windows.h>

namespace spry
{
	struct log
	{
		using clock = std::chrono::high_resolution_clock;
		
		enum class level : int { none = 0, fatal, info, warn, debug, trace };

		log(const char* filename = "spry.binlog") : filename(filename) { }
		~log() = default;

		template <typename... Args> constexpr inline void fatal(Args&&... args)
		{
			if (level == level::none) return;
			write({ clock::now(), write_strings(std::forward<Args>(args))..., newline{} });
		}
		template <typename... Args> constexpr inline void info(Args&&... args)
		{
			if (level < level::info) return;
			write({ clock::now(), write_strings(std::forward<Args>(args))..., newline{} });
		}
		template <typename... Args> constexpr inline void warn(Args&&... args)
		{
			if (level < level::warn) return;
			write({ clock::now(), write_strings(std::forward<Args>(args))..., newline{} });
		}
		template <typename... Args> constexpr inline void debug(Args&&... args)
		{
			if (level < level::debug) return;
			write({ clock::now(), write_strings(std::forward<Args>(args))..., newline{} });
		}
		template <typename... Args> constexpr inline void trace(Args&&... args)
		{
			if (level < level::trace) return;
			write({ clock::now(), write_strings(std::forward<Args>(args))..., newline{} });
		}

		void disable() { set_level(level::none); }
		void set_to_fatal() { set_level(level::fatal); }
		void set_to_info() { set_level(level::info); }
		void set_to_warn() { set_level(level::warn); }
		void set_to_debug() { set_level(level::debug); }
		void set_to_trace() { set_level(level::trace); }

	private:

		void set_level(level new_level) { level = new_level; }

		inline void write(std::initializer_list<arg>&& args)
		{
			static std::atomic<uint64_t> page_counter{ 0 };
			static thread_local page messages{ filename.data(), page_counter++ };

			const auto length = args.size() * sizeof(arg);
			if (messages.free_space() < length) { messages.flip_to_page(page_counter++); }
			messages.write(std::move(args));
		}

		template <typename T> 
		inline std::enable_if_t<!std::is_pointer_v<T>, T&&> write_strings(T&& arg)
		{
			return std::forward<T>(arg);
		}

		inline const char* write_strings(std::string& string)
		{
			auto data = string.c_str();
			return write_strings(data);
		}
		
		template <typename T, size_t N> 
		constexpr inline std::enable_if_t<!std::is_pointer_v<T&&>, ct_string> write_strings(T(&string)[N])
		{
			return string;
		}

		template <typename T>
		inline std::enable_if_t<std::is_pointer_v<T>, const char*> write_strings(T& string)
		{
			static std::atomic<uint64_t> page_counter{ 0 };
			static thread_local page strings{ (filename + "strings").data(), page_counter++ };

			const auto length = std::strlen(string);
			if (strings.free_space() < length) strings.flip_to_page(page_counter++);
			const auto pointer = strings.write(reinterpret_cast<const uint8_t*>(string), length);
			return reinterpret_cast<const char*>(pointer);
		}

		std::string filename;
		std::atomic<level> level;
	};
}