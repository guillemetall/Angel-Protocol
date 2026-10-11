/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "low_priority.h"
#include "ble_manager.h"
#include "congestion.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define LP_QUEUE_SIZE  32
#define LP_MAX_PKT     64
#define LP_THRESHOLD   50

static struct {
    uint8_t  data[LP_MAX_PKT];
    uint16_t len;
} s_queue[LP_QUEUE_SIZE];

static uint8_t  s_head = 0;
static uint8_t  s_tail = 0;
static uint8_t  s_count = 0;
static bool     s_sos_activo = false;

void low_priority_init(void) {
    s_head = 0;
    s_tail = 0;
    s_count = 0;
    s_sos_activo = false;
}

void low_priority_set_sos(bool on) {
    s_sos_activo = on;
}

bool low_priority_sos_activo(void) {
    return s_sos_activo;
}

uint8_t low_priority_count(void) {
    return s_count;
}

bool low_priority_enqueue(uint8_t *data, uint16_t len) {
    if (len > LP_MAX_PKT) return false;
    if (s_count >= LP_QUEUE_SIZE) return false;
    memcpy(s_queue[s_tail].data, data, len);
    s_queue[s_tail].len = len;
    s_tail = (s_tail + 1) % LP_QUEUE_SIZE;
    s_count++;
    return true;
}

bool low_priority_pending(void) {
    return s_count > 0;
}

void low_priority_tick(void) {
    if (s_count == 0) return;
    if (s_sos_activo) return;
    if (congestion_nivel() > LP_THRESHOLD) return;
    ble_emitir(s_queue[s_head].data, s_queue[s_head].len, 3);
    s_head = (s_head + 1) % LP_QUEUE_SIZE;
    s_count--;
}
