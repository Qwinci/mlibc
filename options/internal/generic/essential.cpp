#include <string.h>
#include <stdint.h>

namespace {
	// Needed since we cannot declare a templated enum.
	template<typename T>
	struct word_helper {
		using underlying [[gnu::aligned(1)]] = T;
		enum class [[gnu::may_alias]] word_enum : underlying { };
	};

	template<typename T>
	using word = typename word_helper<T>::word_enum;

	template<typename T>
	[[gnu::always_inline]]
	inline word<T> alias_load(const unsigned char *&p) {
		word<T> value = *reinterpret_cast<const word<T> *>(p);
		p += sizeof(T);
		return value;
	}

	template<typename T>
	[[gnu::always_inline]]
	inline void alias_store(unsigned char *&p, word<T> value) {
		*reinterpret_cast<word<T> *>(p) = value;
		p += sizeof(T);
	}

#ifdef __LP64__
	void *forward_copy(void *__restrict dest, const void *__restrict src, size_t n) {
		auto curDest = reinterpret_cast<unsigned char *>(dest);
		auto curSrc = reinterpret_cast<const unsigned char *>(src);

		while(n >= 8 * 8) {
			auto w1 = alias_load<uint64_t>(curSrc);
			auto w2 = alias_load<uint64_t>(curSrc);
			auto w3 = alias_load<uint64_t>(curSrc);
			auto w4 = alias_load<uint64_t>(curSrc);
			auto w5 = alias_load<uint64_t>(curSrc);
			auto w6 = alias_load<uint64_t>(curSrc);
			auto w7 = alias_load<uint64_t>(curSrc);
			auto w8 = alias_load<uint64_t>(curSrc);
			alias_store<uint64_t>(curDest, w1);
			alias_store<uint64_t>(curDest, w2);
			alias_store<uint64_t>(curDest, w3);
			alias_store<uint64_t>(curDest, w4);
			alias_store<uint64_t>(curDest, w5);
			alias_store<uint64_t>(curDest, w6);
			alias_store<uint64_t>(curDest, w7);
			alias_store<uint64_t>(curDest, w8);
			n -= 8 * 8;
		}
		if(n >= 4 * 8) {
			auto w1 = alias_load<uint64_t>(curSrc);
			auto w2 = alias_load<uint64_t>(curSrc);
			auto w3 = alias_load<uint64_t>(curSrc);
			auto w4 = alias_load<uint64_t>(curSrc);
			alias_store<uint64_t>(curDest, w1);
			alias_store<uint64_t>(curDest, w2);
			alias_store<uint64_t>(curDest, w3);
			alias_store<uint64_t>(curDest, w4);
			n -= 4 * 8;
		}
		if(n >= 2 * 8) {
			auto w1 = alias_load<uint64_t>(curSrc);
			auto w2 = alias_load<uint64_t>(curSrc);
			alias_store<uint64_t>(curDest, w1);
			alias_store<uint64_t>(curDest, w2);
			n -= 2 * 8;
		}
		if(n >= 8) {
			auto w = alias_load<uint64_t>(curSrc);
			alias_store<uint64_t>(curDest, w);
			n -= 8;
		}
		if(n >= 4) {
			auto w = alias_load<uint32_t>(curSrc);
			alias_store<uint32_t>(curDest, w);
			n -= 4;
		}
		if(n >= 2) {
			auto w = alias_load<uint16_t>(curSrc);
			alias_store<uint16_t>(curDest, w);
			n -= 2;
		}
		if(n)
			*curDest = *curSrc;
		return dest;
	}
#else // !__LP64__
	void *forward_copy(void *dest, const void *src, size_t n) {
		for(size_t i = 0; i < n; i++)
			((char *)dest)[i] = ((const char *)src)[i];
		return dest;
	}
#endif // __LP64__ / !__LP64__
}

// --------------------------------------------------------------------------------------
// memcpy() implementation.
// --------------------------------------------------------------------------------------


void *memcpy(void *__restrict dest, const void *__restrict src, size_t n) {
	return forward_copy(dest, src, n);
}

extern "C" void *__memcpy_chk(void *__restrict dest, const void *__restrict src, size_t len, size_t destlen) {
	return memcpy(dest, src, len);
}


// --------------------------------------------------------------------------------------
// memset() implementation.
// --------------------------------------------------------------------------------------

#ifdef __LP64__

void *memset(void *dest, int val, size_t n) {
	auto curDest = reinterpret_cast<unsigned char *>(dest);
	unsigned char byte = val;

	// Get rid of misalignment.
	while(n && (reinterpret_cast<uintptr_t>(curDest) & 7)) {
		*curDest++ = byte;
		--n;
	}

	auto pattern64 = static_cast<word<uint64_t>>(
			static_cast<uint64_t>(byte)
			| (static_cast<uint64_t>(byte) << 8)
			| (static_cast<uint64_t>(byte) << 16)
			| (static_cast<uint64_t>(byte) << 24)
			| (static_cast<uint64_t>(byte) << 32)
			| (static_cast<uint64_t>(byte) << 40)
			| (static_cast<uint64_t>(byte) << 48)
			| (static_cast<uint64_t>(byte) << 56));

	auto pattern32 = static_cast<word<uint32_t>>(
			static_cast<uint32_t>(byte)
			| (static_cast<uint32_t>(byte) << 8)
			| (static_cast<uint32_t>(byte) << 16)
			| (static_cast<uint32_t>(byte) << 24));

	auto pattern16 = static_cast<word<uint16_t>>(
			static_cast<uint16_t>(byte)
			| (static_cast<uint16_t>(byte) << 8));

	while(n >= 8 * 8) {
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		n -= 8 * 8;
	}
	if(n >= 4 * 8) {
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		n -= 4 * 8;
	}
	if(n >= 2 * 8) {
		alias_store<uint64_t>(curDest, pattern64);
		alias_store<uint64_t>(curDest, pattern64);
		n -= 2 * 8;
	}
	if(n >= 8) {
		alias_store<uint64_t>(curDest, pattern64);
		n -= 8;
	}
	if(n >= 4) {
		alias_store<uint32_t>(curDest, pattern32);
		n -= 4;
	}
	if(n >= 2) {
		alias_store<uint16_t>(curDest, pattern16);
		n -= 2;
	}
	if(n)
		*curDest = byte;
	return dest;
}

#else // !__LP64__

void *memset(void *dest, int byte, size_t count) {
	for(size_t i = 0; i < count; i++)
		((char *)dest)[i] = (char)byte;
	return dest;
}

#endif // __LP64__ / !__LP64__

// --------------------------------------------------------------------------------------
// "Non-optimized" functions.
// --------------------------------------------------------------------------------------

void *memmove(void *dest, const void *src, size_t size) {
	char *dest_bytes = (char *)dest;
	char *src_bytes = (char *)src;
	if(dest_bytes < src_bytes) {
		return forward_copy(dest, src, size);
	}else if(dest_bytes > src_bytes) {
		for(size_t i = 0; i < size; i++)
			dest_bytes[size - i - 1] = src_bytes[size - i - 1];
	}
	return dest;
}

extern "C" void *__memmove_chk(void *dest, const void *src, size_t len, size_t destlen) {
	return memmove(dest, src, len);
}

size_t strlen(const char *s) {
	size_t len = 0;
	for(size_t i = 0; s[i]; i++)
		len++;
	return len;
}
