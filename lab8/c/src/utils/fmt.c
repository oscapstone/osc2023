#include "oscos/utils/fmt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "oscos/libc/ctype.h"
#include "oscos/libc/string.h"

#define OCT_MAX_N_DIGITS 22
#define DEC_MAX_N_DIGITS 20
#define HEX_MAX_N_DIGITS 16

typedef enum {
  LM_NONE,
  LM_HH,
  LM_H,
  LM_L,
  LM_LL,
  LM_J,
  LM_Z,
  LM_T,
  LM_UPPER_L
} length_modifier_t;

static const char LOWER_HEX_DIGITS[16] = "0123456789abcdef";
static const char UPPER_HEX_DIGITS[16] = "0123456789ABCDEF";

static size_t
_render_unsigned_dec(char digits[const static DEC_MAX_N_DIGITS + 1],
                     const uintmax_t x) {
  digits[DEC_MAX_N_DIGITS] = '\0';

  size_t n_digits = 0;
  for (uintmax_t xc = x; xc > 0; xc /= 10) {
    digits[DEC_MAX_N_DIGITS - ++n_digits] = '0' + xc % 10;
  }

  return n_digits;
}

static size_t
_render_unsigned_base_p2(char *const restrict digits, const size_t digits_len,
                         const uintmax_t x, const size_t log2_base,
                         const char *const restrict digit_template) {
  const uintmax_t mask = (1 << log2_base) - 1;

  digits[digits_len] = '\0';

  size_t n_digits = 0;
  for (uintmax_t xc = x; xc > 0; xc >>= log2_base) {
    digits[digits_len - ++n_digits] = digit_template[xc & mask];
  }

  return n_digits;
}

static size_t _puts_generic(void (*const putc)(unsigned char, void *),
                            void *const putc_arg, const char *const s) {
  size_t n_chars_printed = 0;
  for (const char *c = s; *c; c++) {
    putc(*c, putc_arg);
    n_chars_printed++;
  }
  return n_chars_printed;
}

static size_t _puts_limited_generic(void (*const putc)(unsigned char, void *),
                                    void *const putc_arg, const char *const s,
                                    const size_t limit) {
  size_t n_chars_printed = 0, i = 0;
  for (const char *c = s; i < limit && *c; i++, c++) {
    putc(*c, putc_arg);
    n_chars_printed++;
  }
  return n_chars_printed;
}

static size_t _pad_generic(void (*const putc)(unsigned char, void *),
                           void *const putc_arg, const unsigned char pad,
                           const size_t width) {
  for (size_t i = 0; i < width; i++) {
    putc(pad, putc_arg);
  }
  return width;
}

static size_t _vprintf_generic_putc(void (*const putc)(unsigned char, void *),
                                    void *const putc_arg, const unsigned char c,
                                    const bool flag_minus,
                                    const size_t min_field_width) {
  size_t n_chars_printed = 0;

  if (flag_minus) {
    putc(c, putc_arg);
    n_chars_printed++;

    if (min_field_width > 1) {
      n_chars_printed += _pad_generic(putc, putc_arg, ' ', min_field_width - 1);
    }
  } else {
    if (min_field_width > 1) {
      n_chars_printed += _pad_generic(putc, putc_arg, ' ', min_field_width - 1);
    }

    putc(c, putc_arg);
    n_chars_printed++;
  }

  return n_chars_printed;
}

static size_t _vprintf_generic_puts(void (*const putc)(unsigned char, void *),
                                    void *const putc_arg, const char *const s,
                                    const bool flag_minus,
                                    const size_t min_field_width,
                                    const bool precision_valid,
                                    const size_t precision) {
  const size_t len = strlen(s);

  // Calculate width.

  const size_t width = precision_valid && precision < len ? precision : len;
  const size_t pad_width =
      min_field_width > width ? min_field_width - width : 0;

  // Print the characters.

  size_t n_chars_printed = 0;

  // Padding if right justified.
  if (!flag_minus) {
    n_chars_printed += _pad_generic(putc, putc_arg, ' ', pad_width);
  }
  // The string.
  if (precision_valid) {
    n_chars_printed += _puts_limited_generic(putc, putc_arg, s, precision);
  } else {
    n_chars_printed += _puts_generic(putc, putc_arg, s);
  }
  // Padding if left justified.
  if (flag_minus) {
    n_chars_printed += _pad_generic(putc, putc_arg, ' ', pad_width);
  }

  return n_chars_printed;
}

static size_t _vprintf_generic_print_signed_dec(
    void (*const putc)(unsigned char, void *), void *const putc_arg,
    const intmax_t x, const bool flag_minus, const bool flag_plus,
    const bool flag_space, const bool flag_zero, const size_t min_field_width,
    const size_t precision) {
  // Render the digits.

  char digits[DEC_MAX_N_DIGITS + 1];
  const size_t n_digits = _render_unsigned_dec(digits, x < 0 ? -x : x);

  // Calculate width.

  size_t width = n_digits;
  if (precision > n_digits) {
    width = precision;
  }
  width += x < 0 || flag_plus || flag_space; // Sign character.

  const size_t pad_width =
      min_field_width > width ? min_field_width - width : 0;

  // Print the characters.

  size_t n_chars_printed = 0;

  // Padding if right justified.
  if (!flag_minus) {
    n_chars_printed +=
        _pad_generic(putc, putc_arg, flag_zero ? '0' : ' ', pad_width);
  }
  // Sign.
  if (x < 0) {
    putc('-', putc_arg);
    n_chars_printed++;
  } else if (flag_plus) {
    putc('+', putc_arg);
    n_chars_printed++;
  } else if (flag_space) {
    putc(' ', putc_arg);
    n_chars_printed++;
  }
  // Leading zeros.
  if (precision > n_digits) {
    n_chars_printed += _pad_generic(putc, putc_arg, '0', precision - n_digits);
  }
  // The digits.
  n_chars_printed +=
      _puts_generic(putc, putc_arg, digits + (DEC_MAX_N_DIGITS - n_digits));
  // Padding if left justified.
  if (flag_minus) {
    n_chars_printed += _pad_generic(putc, putc_arg, ' ', pad_width);
  }

  return 0;
}

static size_t _vprintf_generic_print_unsigned_oct(
    void (*const putc)(unsigned char, void *), void *const putc_arg,
    const uintmax_t x, const bool flag_minus, const bool flag_hash,
    const bool flag_zero, const size_t min_field_width,
    const size_t precision) {
  // Render the digits.

  char digits[OCT_MAX_N_DIGITS + 1];
  const size_t n_digits = _render_unsigned_base_p2(digits, OCT_MAX_N_DIGITS, x,
                                                   3, LOWER_HEX_DIGITS);

  // Calculate width.

  const size_t effective_precision =
      flag_hash && n_digits + 1 > precision ? n_digits + 1 : precision;

  size_t width = n_digits;
  if (effective_precision > n_digits) {
    width = effective_precision;
  }

  const size_t pad_width =
      min_field_width > width ? min_field_width - width : 0;

  // Print the characters.

  size_t n_chars_printed = 0;

  // Padding if right justified.
  if (!flag_minus) {
    n_chars_printed +=
        _pad_generic(putc, putc_arg, flag_zero ? '0' : ' ', pad_width);
  }
  // Leading zeros.
  if (effective_precision > n_digits) {
    n_chars_printed +=
        _pad_generic(putc, putc_arg, '0', effective_precision - n_digits);
  }
  // The digits.
  n_chars_printed +=
      _puts_generic(putc, putc_arg, digits + (OCT_MAX_N_DIGITS - n_digits));
  // Padding if left justified.
  if (flag_minus) {
    n_chars_printed += _pad_generic(putc, putc_arg, ' ', pad_width);
  }

  return 0;
}

static size_t _vprintf_generic_print_unsigned_hex(
    void (*const putc)(unsigned char, void *), void *const putc_arg,
    const uintmax_t x, const bool flag_minus, const bool flag_hash,
    const bool flag_zero, const size_t min_field_width, const size_t precision,
    const bool is_upper) {
  const char *const digit_template =
      is_upper ? UPPER_HEX_DIGITS : LOWER_HEX_DIGITS;
  const char *const prefix = is_upper ? "0X" : "0x";

  // Render the digits.

  char digits[HEX_MAX_N_DIGITS + 1];
  const size_t n_digits =
      _render_unsigned_base_p2(digits, HEX_MAX_N_DIGITS, x, 4, digit_template);

  // Calculate width.

  size_t width = n_digits;
  if (precision > n_digits) {
    width = precision;
  }
  // 0x prefix.
  if (flag_hash && x != 0) {
    width += 2;
  }

  const size_t pad_width =
      min_field_width > width ? min_field_width - width : 0;

  // Print the characters.

  size_t n_chars_printed = 0;

  // Padding if right justified.
  if (!flag_minus) {
    n_chars_printed +=
        _pad_generic(putc, putc_arg, flag_zero ? '0' : ' ', pad_width);
  }
  // 0x prefix.
  if (flag_hash && x != 0) {
    n_chars_printed += _puts_generic(putc, putc_arg, prefix);
  }
  // Leading zeros.
  if (precision > n_digits) {
    n_chars_printed += _pad_generic(putc, putc_arg, '0', precision - n_digits);
  }
  // The digits.
  n_chars_printed +=
      _puts_generic(putc, putc_arg, digits + (HEX_MAX_N_DIGITS - n_digits));
  // Padding if left justified.
  if (flag_minus) {
    n_chars_printed += _pad_generic(putc, putc_arg, ' ', pad_width);
  }

  return 0;
}

static size_t _vprintf_generic_print_unsigned_dec(
    void (*const putc)(unsigned char, void *), void *const putc_arg,
    const uintmax_t x, const bool flag_minus, const bool flag_zero,
    const size_t min_field_width, const size_t precision) {
  // Render the digits.

  char digits[DEC_MAX_N_DIGITS + 1];
  const size_t n_digits = _render_unsigned_dec(digits, x);

  // Calculate width.

  size_t width = n_digits;
  if (precision > n_digits) {
    width = precision;
  }

  const size_t pad_width =
      min_field_width > width ? min_field_width - width : 0;

  // Print the characters.

  size_t n_chars_printed = 0;

  // Padding if right justified.
  if (!flag_minus) {
    n_chars_printed +=
        _pad_generic(putc, putc_arg, flag_zero ? '0' : ' ', pad_width);
  }
  // Leading zeros.
  if (precision > n_digits) {
    n_chars_printed += _pad_generic(putc, putc_arg, '0', precision - n_digits);
  }
  // The digits.
  n_chars_printed +=
      _puts_generic(putc, putc_arg, digits + (DEC_MAX_N_DIGITS - n_digits));
  // Padding if left justified.
  if (flag_minus) {
    n_chars_printed += _pad_generic(putc, putc_arg, ' ', pad_width);
  }

  return 0;
}

#define READ_ARG_S(LM, AP)                                                     \
  ((LM) == LM_NONE ? va_arg(AP, int)                                           \
   : (LM) == LM_HH ? (signed char)va_arg(AP, int)                              \
   : (LM) == LM_H  ? (short)va_arg(AP, int)                                    \
   : (LM) == LM_L  ? va_arg(AP, long)                                          \
   : (LM) == LM_LL ? va_arg(AP, long long)                                     \
   : (LM) == LM_J  ? va_arg(AP, intmax_t)                                      \
   : (LM) == LM_Z  ? va_arg(AP, /* signed size_t = */ long)                    \
   : (LM) == LM_T  ? va_arg(AP, ptrdiff_t)                                     \
                   : (__builtin_unreachable(), 0))

#define READ_ARG_U(LM, AP)                                                     \
  ((LM) == LM_NONE ? va_arg(AP, unsigned)                                      \
   : (LM) == LM_HH ? (unsigned char)va_arg(AP, int)                            \
   : (LM) == LM_H  ? (unsigned short)va_arg(AP, int)                           \
   : (LM) == LM_L  ? va_arg(AP, unsigned long)                                 \
   : (LM) == LM_LL ? va_arg(AP, unsigned long long)                            \
   : (LM) == LM_J  ? va_arg(AP, uintmax_t)                                     \
   : (LM) == LM_Z  ? va_arg(AP, size_t)                                        \
   : (LM) == LM_T  ? va_arg(AP, /* unsigned ptrdiff_t = */ unsigned long)      \
                   : (__builtin_unreachable(), 0))

int vprintf_generic(const printf_vtable_t *const vtable, void *const vtable_arg,
                    const char *const restrict format, va_list ap) {
  size_t n_chars_printed = 0;
  for (const char *restrict c = format; *c;) {
    if (*c == '%') {
      c++;

      // Flags.

      bool flag_minus = false, flag_plus = false, flag_space = false,
           flag_hash = false, flag_zero = false;

      for (;;) {
        if (*c == '-') {
          c++;
          flag_minus = true;
        } else if (*c == '+') {
          c++;
          flag_plus = true;
        } else if (*c == ' ') {
          c++;
          flag_space = true;
        } else if (*c == '#') {
          c++;
          flag_hash = true;
        } else if (*c == '0') {
          c++;
          flag_zero = true;
        } else {
          break;
        }
      }

      // Minimum field width.

      bool min_field_width_specified = false;
      size_t min_field_width = 0;
      if (*c == '*') {
        c++;
        min_field_width_specified = true;
        const int arg = va_arg(ap, int);
        if (arg < 0) {
          flag_minus = true;
          min_field_width = -arg;
        } else {
          min_field_width = arg;
        }
      } else if (isdigit(*c)) {
        min_field_width_specified = true;
        for (; isdigit(*c); c++) {
          min_field_width = min_field_width * 10 + (*c - '0');
        }
      }

      // Precision.

      bool precision_specified = false, precision_valid = false;
      size_t precision = 0;
      if (*c == '.') {
        c++;
        precision_specified = true;

        if (*c == '*') {
          c++;
          const int arg = va_arg(ap, int);
          if (arg >= 0) {
            precision_valid = true;
            precision = arg;
          }
        } else {
          precision_valid = true;
          for (; isdigit(*c); c++) {
            precision = precision * 10 + (*c - '0');
          }
        }
      }

      // Length modifier.

      length_modifier_t length_modifier = LM_NONE;

      switch (*c) {
      case 'h':
        c++;
        if (*c == 'h') {
          c++;
          length_modifier = LM_HH;
        } else {
          length_modifier = LM_H;
        }
        break;

      case 'l':
        c++;
        if (*c == 'l') {
          c++;
          length_modifier = LM_LL;
        } else {
          length_modifier = LM_L;
        }
        break;

      case 'j':
        c++;
        length_modifier = LM_J;
        break;

      case 'z':
        c++;
        length_modifier = LM_Z;
        break;

      case 't':
        c++;
        length_modifier = LM_T;
        break;

      case 'L':
        c++;
        length_modifier = LM_UPPER_L;
        break;
      }

      // Format specifier.

      switch (*c) {
      case '%':
        c++;
        if (flag_minus || flag_plus || flag_space || flag_hash || flag_zero ||
            min_field_width_specified || precision_specified ||
            length_modifier != LM_NONE)
          __builtin_unreachable();
        vtable->putc('%', vtable_arg);
        n_chars_printed++;
        break;

      case 'c':
        c++;
        switch (length_modifier) {
        case LM_NONE:
          if (flag_hash || flag_zero)
            __builtin_unreachable();
          n_chars_printed +=
              _vprintf_generic_putc(vtable->putc, vtable_arg, va_arg(ap, int),
                                    flag_minus, min_field_width);
          break;

        default:
          __builtin_unreachable();
        }
        break;

      case 's':
        c++;
        switch (length_modifier) {
        case LM_NONE:
          if (flag_hash || flag_zero)
            __builtin_unreachable();
          n_chars_printed += _vprintf_generic_puts(
              vtable->putc, vtable_arg, va_arg(ap, const char *), flag_minus,
              min_field_width, precision_valid, precision);
          break;

        default:
          __builtin_unreachable();
        }
        break;

      case 'd':
      case 'i':
        c++;
        if (flag_hash)
          __builtin_unreachable();
        n_chars_printed += _vprintf_generic_print_signed_dec(
            vtable->putc, vtable_arg, READ_ARG_S(length_modifier, ap),
            flag_minus, flag_plus, flag_space, flag_zero, min_field_width,
            precision_valid ? precision : 1);
        break;

      case 'o':
        c++;
        n_chars_printed += _vprintf_generic_print_unsigned_oct(
            vtable->putc, vtable_arg, READ_ARG_U(length_modifier, ap),
            flag_minus, flag_hash, flag_zero, min_field_width,
            precision_specified ? precision : 1);
        break;

      case 'x':
      case 'X': {
        const bool is_upper = *c == 'X';
        c++;
        n_chars_printed += _vprintf_generic_print_unsigned_hex(
            vtable->putc, vtable_arg, READ_ARG_U(length_modifier, ap),
            flag_minus, flag_hash, flag_zero, min_field_width,
            precision_specified ? precision : 1, is_upper);
        break;
      }

      case 'u':
        c++;
        if (flag_hash)
          __builtin_unreachable();
        n_chars_printed += _vprintf_generic_print_unsigned_dec(
            vtable->putc, vtable_arg, READ_ARG_S(length_modifier, ap),
            flag_minus, flag_zero, min_field_width,
            precision_valid ? precision : 1);
        break;

      case 'n':
        c++;
        if (flag_minus || flag_plus || flag_space || flag_hash || flag_zero ||
            min_field_width_specified || precision_specified)
          __builtin_unreachable();
        switch (length_modifier) {
        case LM_NONE:
          *va_arg(ap, int *) = n_chars_printed;
          break;

        case LM_HH:
          *va_arg(ap, signed char *) = n_chars_printed;
          break;

        case LM_H:
          *va_arg(ap, short *) = n_chars_printed;
          break;

        case LM_L:
          *va_arg(ap, long *) = n_chars_printed;
          break;

        case LM_LL:
          *va_arg(ap, long long *) = n_chars_printed;
          break;

        case LM_J:
          *va_arg(ap, intmax_t *) = n_chars_printed;
          break;

        case LM_Z:
          *va_arg(ap, /* signed size_t = */ long *) = n_chars_printed;
          break;

        case LM_T:
          *va_arg(ap, ptrdiff_t *) = n_chars_printed;
          break;

        default:
          __builtin_unreachable();
        }
        break;

      case 'p':
        c++;
        switch (length_modifier) {
        case LM_NONE:
          n_chars_printed += _vprintf_generic_print_unsigned_hex(
              vtable->putc, vtable_arg, (uintmax_t)va_arg(ap, void *),
              flag_minus, flag_hash, flag_zero, min_field_width,
              precision_specified ? precision : 1, false);
          break;

        default:
          __builtin_unreachable();
        }
        break;

      default:
        __builtin_unreachable();
      }
    } else {
      vtable->putc(*c++, vtable_arg);
      n_chars_printed++;
    }
  }

  vtable->finalize(vtable_arg);
  return n_chars_printed;
}
