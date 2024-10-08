// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: Apache-2.0

#ifndef USART_H
#define USART_H

#include <stddef.h>
#include <sentry/ktypes.h>
#include <sentry/managers/memory.h>

kstatus_t usart_probe(void);

kstatus_t usart_tx(const uint8_t *data, size_t data_len);


#endif/*!USART_H*/
