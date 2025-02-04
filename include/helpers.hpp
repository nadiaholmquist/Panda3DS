#pragma once
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#include "termcolor.hpp"

// We have to detect and special-case AppleClang at the moment since its C++20 support is finicky and doesn't quite support std::bit_cast
#if defined(__clang__) && defined(__apple_build_version__)
#define HELPERS_APPLE_CLANG
#else
#include <bit>
#endif

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;
using uint = unsigned int;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

namespace Helpers {
	[[noreturn]] static void panic(const char* fmt, ...) {
		std::va_list args;
		va_start(args, fmt);
		std::cout << termcolor::on_red << "[FATAL] ";
		std::vprintf(fmt, args);
		std::cout << termcolor::reset << "\n";
		va_end(args);

		exit(1);
	}

	static void warn(const char* fmt, ...) {
		std::va_list args;
		va_start(args, fmt);
		std::cout << termcolor::on_red << "[Warning] ";
		std::vprintf(fmt, args);
		std::cout << termcolor::reset << "\n";
		va_end(args);
	}

	static std::vector<u8> loadROM(std::string directory) {
		std::ifstream file(directory, std::ios::binary);
		if (file.fail()) panic("Couldn't read %s", directory.c_str());

		std::vector<u8> ROM;

		file.unsetf(std::ios::skipws);
		ROM.insert(ROM.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

		file.close();

		printf("%s loaded successfully\n", directory.c_str());
		return ROM;
	}

	static constexpr bool buildingInDebugMode() {
#ifdef NDEBUG
		return false;
#endif
		return true;
	}

	static void debug_printf(const char* fmt, ...) {
		if constexpr (buildingInDebugMode()) {
			std::va_list args;
			va_start(args, fmt);
			std::vprintf(fmt, args);
			va_end(args);
		}
	}

	/// Sign extend an arbitrary-size value to 32 bits
	static constexpr u32 inline signExtend32(u32 value, u32 startingSize) {
		auto temp = (s32)value;
		auto bitsToShift = 32 - startingSize;
		return (u32)(temp << bitsToShift >> bitsToShift);
	}

	/// Sign extend an arbitrary-size value to 16 bits
	static constexpr u16 signExtend16(u16 value, u32 startingSize) {
		auto temp = (s16)value;
		auto bitsToShift = 16 - startingSize;
		return (u16)(temp << bitsToShift >> bitsToShift);
	}

	/// Create a mask with `count` number of one bits.
	template <typename T, usize count>
	static constexpr T ones() {
		constexpr usize bitsize = CHAR_BIT * sizeof(T);
		static_assert(count <= bitsize, "count larger than bitsize of T");

		if (count == T(0)) {
			return T(0);
		}
		return static_cast<T>(~static_cast<T>(0)) >> (bitsize - count);
	}

	/// Extract bits from an integer-type
	template <usize offset, typename T>
	static constexpr T getBit(T value) {
		return (value >> offset) & T(1);
	}

	/// Extract bits from an integer-type
	template <usize offset, usize bits, typename T>
	static constexpr T getBits(T value) {
		return (value >> offset) & ones<T, bits>();
	}

	/// Check if a bit "bit" of value is set
	static constexpr bool isBitSet(u32 value, int bit) { return (value >> bit) & 1; }

	/// rotate number right
	template <typename T>
	static constexpr T rotr(T value, int bits) {
		constexpr auto bitWidth = sizeof(T) * 8;
		bits &= bitWidth - 1;
		return (value >> bits) | (value << (bitWidth - bits));
	}

	// rotate number left
	template <typename T>
	static constexpr T rotl(T value, int bits) {
		constexpr auto bitWidth = sizeof(T) * 8;
		bits &= bitWidth - 1;
		return (value << bits) | (value >> (bitWidth - bits));
	}

	/// Used to make the compiler evaluate beeg loops at compile time for the tablegen
	template <typename T, T Begin, class Func, T... Is>
	static constexpr void static_for_impl(Func&& f, std::integer_sequence<T, Is...>) {
		(f(std::integral_constant<T, Begin + Is>{}), ...);
	}

	template <typename T, T Begin, T End, class Func>
	static constexpr void static_for(Func&& f) {
		static_for_impl<T, Begin>(std::forward<Func>(f), std::make_integer_sequence<T, End - Begin>{});
	}

	// For values < 0x99
	static constexpr inline u8 incBCDByte(u8 value) { return ((value & 0xf) == 0x9) ? value + 7 : value + 1; }

#ifdef HELPERS_APPLE_CLANG
	template <class To, class From>
	constexpr To bit_cast(const From& from) noexcept {
		return *reinterpret_cast<const To*>(&from);
	}
#else
	template <class To, class From>
	constexpr To bit_cast(const From& from) noexcept {
		return std::bit_cast<To, From>(from);
	}
#endif
};  // namespace Helpers

// UDLs for memory size values
constexpr size_t operator""_KB(unsigned long long int x) { return 1024ULL * x; }
constexpr size_t operator""_MB(unsigned long long int x) { return 1024_KB * x; }
constexpr size_t operator""_GB(unsigned long long int x) { return 1024_MB * x; }

// useful macros
// likely/unlikely
#ifdef __GNUC__
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif
