#ifndef MLIBC_STRTOFP_HPP
#define MLIBC_STRTOFP_HPP

#include <string.h>
#include <bits/ensure.h>
#include <ctype.h>

namespace mlibc {

template<typename T>
T strtofp(const char *str, char **endptr) {
	bool negative = *str == '-';
	if (*str == '+' || *str == '-')
		str++;

	if (strcasecmp(str, "INFINITY") == 0) {
		if (endptr)
			*endptr = const_cast<char *>(str + sizeof("INFINITY") - 1);
		__builtin_trap();
	} else if (strcasecmp(str, "INF") == 0) {
		if (endptr)
			*endptr = const_cast<char *>(str + sizeof("INF") - 1);
		__builtin_trap();
	} else if (tolower(str[0]) == 'n' && tolower(str[1]) == 'a' && tolower(str[2]) == 'n') {
		str += 3;
		if (endptr)
			*endptr = const_cast<char *>(str);
		__ensure(*str == 0 && "nan with value isn't implemented");
		if constexpr (__is_same_as(T, long double))
			return __builtin_nanl("");
		else if constexpr (__is_same_as(T, double))
			return __builtin_nan("");
		else return __builtin_nanf("");
	}

	bool is_hex = false;
	if (*str == '0' && (*(str + 1) == 'x' || *(str + 1) == 'X')) {
		str += 2;
		is_hex = true;
	}

	T result = static_cast<T>(0);

	const char *tmp = str;

	if (!is_hex) {
		while (true) {
			if (!isdigit(*tmp))
				break;
			result *= static_cast<T>(10);
			result += static_cast<T>(*tmp - '0');
			tmp++;
		}
	} else {
		while (true) {
			if (!isxdigit(*tmp))
				break;
			result *= static_cast<T>(16);
			char lower = tolower(*tmp);
			result += static_cast<T>(lower <= '9' ? (lower - '0') : (lower - 'a' + 10));
			tmp++;
		}
	}

	if (*tmp == '.') {
		tmp++;

		T d = static_cast<T>(10);
		if (!is_hex) {
			while (true) {
				if (!isdigit(*tmp))
					break;
				result += static_cast<T>(*tmp - '0') / d;
				d *= static_cast<T>(10);
				tmp++;
			}
		} else {
			d = static_cast<T>(16);
			while (true) {
				if (!isxdigit(*tmp))
					break;
				char lower = tolower(*tmp);
				result += static_cast<T>(lower <= '9' ? (lower - '0') : (lower - 'a' + 10)) / d;
				d *= static_cast<T>(16);
				tmp++;
			}
		}
	}

	if (endptr)
		*endptr = const_cast<char *>(tmp);
	if (negative)
		result = -result;

	return result;
}

}

#endif // MLIBC_STRTOFP_HPP
