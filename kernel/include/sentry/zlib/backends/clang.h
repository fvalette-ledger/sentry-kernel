// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: Apache-2.0

#ifndef ZLIB_CLANG_H
#define ZLIB_CLANG_H

#ifndef ZLIB_COMPILER_H
#error "error: do not include this file directly!"
#endif

#define __PACKED __attribute__((packed, aligned(1)))

#if __STDC_VERSION__ == 201112L

#define __MAYBE_UNUSED __attribute__((unused))
#define __UNUSED __attribute__((unused))
#define __ALIGNAS(x) _Alignas(x)

#endif/*! std C11 */


/* C23 compilation use case */
#if __STDC_VERSION__ == 202311L

#define __MAYBE_UNUSED __attribute__((maybe_unused))
#define __UNUSED __attribute__((unused))
#define __ALIGNAS(x) alignas(x)

#endif/*! std C23 */

#endif/*!ZLIB_GCC_H */
