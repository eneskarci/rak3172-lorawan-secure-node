#pragma once
#include <stdbool.h>
#include <stdint.h>

/* ============================================================
 * 2. LORAWAN CONFIGURATION (Zephyr 4.3.0 / RAK3172)
 * LoRaWAN 1.0.3 OTAA: DevEUI + JoinEUI + AppKey
 * ============================================================ */

/* 2.2 OTAA Identifiers
 * NOT: Bu 3 değer Network Server (TTN/ChirpStack) tarafı ile birebir aynı olmalı.
 */
static const uint8_t LORAWAN_DEVICE_EUI[8] = { 0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18 };

static const uint8_t LORAWAN_JOIN_EUI[8] = { 0x08, 0x9C, 0x69, 0x93, 0x27, 0x6D, 0xD9, 0xA2 };
/* AppKey (16 bytes) */
static const uint8_t LORAWAN_APP_KEY[16] = {
    0x2B,0x7E,0x15,0x16,
    0x28,0xAE,0xD2,0xA6,
    0xAB,0xF7,0x15,0x88,
    0x09,0xCF,0x4F,0x3C
};

/* 2.5 FPort */
#define LORAWAN_APP_PORT 1

/* 2.6 ADR */
#define LORAWAN_ADR_ENABLED true

/* 2.7 Timing */
#define TX_INTERVAL_SECONDS    60
#define JOIN_RETRY_INTERVAL_MS 60000
#define JOIN_MAX_RETRIES       10

/* 3.x Confirmed uplink selection (used by main.c) */
#define LORAWAN_USE_CONFIRMED_UPLINK true
