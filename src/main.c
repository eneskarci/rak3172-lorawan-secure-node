/*
 * app_3172_2 - Stage 2 + Stage 3 + Stage 4 (Zephyr 4.3.0 / RAK3172)
 *
 * 2.x: LoRaWAN config + OTAA join
 * 3.x: Uplink send loop (confirmed/unconfirmed)
 * 4.1-4.4: Secure payload (HMAC-SHA256) + payload string print
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/lorawan/lorawan.h>
#include <zephyr/sys/printk.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* 2.x / 3.x configuration */
#include "lorawan_config.h"

/* 4.1: HMAC key */
#include "hmac_key.h"

/* 4.2: mbedTLS HMAC-SHA256 */
#include "mbedtls/md.h"

LOG_MODULE_REGISTER(app_3172_2, LOG_LEVEL_INF);

/* ============================================================
 * 2. LORAWAN CONFIGURATION
 * ============================================================ */

static int lorawan_join_otaa(void)
{
    /* 2.2 OTAA Join Parameters (Zephyr 4.3.0 API) */
    struct lorawan_join_config join_cfg = {0};

    /* Activation mode (explicit) */
    join_cfg.mode = LORAWAN_ACT_OTAA;

    /* LoRaWAN 1.0.3 OTAA identifiers */
    join_cfg.dev_eui       = (uint8_t *)LORAWAN_DEVICE_EUI;
    join_cfg.otaa.join_eui = (uint8_t *)LORAWAN_JOIN_EUI;
    join_cfg.otaa.app_key  = (uint8_t *)LORAWAN_APP_KEY;

    /* DevNonce: 0 -> stack generates for demo */
    join_cfg.otaa.dev_nonce = 0;

    /* 2.6 ADR */
    lorawan_enable_adr(LORAWAN_ADR_ENABLED);

    /* 2.7 Join retry logic */
    for (int i = 1; i <= JOIN_MAX_RETRIES; i++) {
        LOG_INF("LoRaWAN join attempt %d/%d", i, JOIN_MAX_RETRIES);

        int ret = lorawan_join(&join_cfg);
        if (ret == 0) {
            LOG_INF("LoRaWAN join successful");
            return 0;
        }

        LOG_WRN("Join failed (%d), retry in %d ms...", ret, JOIN_RETRY_INTERVAL_MS);
        k_msleep(JOIN_RETRY_INTERVAL_MS);
    }

    return -ETIMEDOUT;
}

/* ============================================================
 * 4.1-4.4 SECURE PAYLOAD (HMAC-SHA256)
 * Payload format:  "T:%.1f,H:%.1f#<64hex>"
 * ============================================================ */

#define DATA_PART_MAX_SIZE  32
#define HMAC_BIN_SIZE       32
#define HMAC_HEX_SIZE       64
#define PAYLOAD_MAX_SIZE    128

typedef struct {
    float temperature;
    float humidity;
} SensorData_t;

/* 4.2: HMAC-SHA256 compute */
static int hmac_sha256_compute(const uint8_t *key, size_t key_len,
                               const uint8_t *data, size_t data_len,
                               uint8_t *out32)
{
    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md == NULL) {
        return -1;
    }

    return mbedtls_md_hmac(md, key, key_len, data, data_len, out32);
}

/* 4.3: bytes -> hex string */
static void bytes_to_hex_string(const uint8_t *bytes, size_t len,
                                char *hex, size_t hex_size)
{
    static const char hc[] = "0123456789abcdef";

    if (hex_size < (2U * len + 1U)) {
        if (hex_size > 0U) {
            hex[0] = '\0';
        }
        return;
    }

    for (size_t i = 0; i < len; i++) {
        hex[2U * i]     = hc[(bytes[i] >> 4) & 0x0F];
        hex[2U * i + 1] = hc[bytes[i] & 0x0F];
    }
    hex[2U * len] = '\0';
}

/* 4.4: build secure payload string */
static bool create_secure_payload(const SensorData_t *s,
                                  uint8_t *payload,
                                  uint16_t *payload_len)
{
    char data_part[DATA_PART_MAX_SIZE];
    uint8_t hmac_bin[HMAC_BIN_SIZE];
    char hmac_hex[HMAC_HEX_SIZE + 1];

    /* 4.4-1: Data part */
    int data_len = snprintk(data_part, sizeof(data_part),
                            "T:%.1f,H:%.1f",
                            (double)s->temperature,
                            (double)s->humidity);
    if (data_len <= 0 || data_len >= (int)sizeof(data_part)) {
        return false;
    }

    /* 4.4-2: HMAC over data part */
    int ret = hmac_sha256_compute((const uint8_t *)HMAC_SECRET_KEY, HMAC_KEY_SIZE,
                                  (const uint8_t *)data_part, (size_t)data_len,
                                  hmac_bin);
    if (ret != 0) {
        return false;
    }

    /* 4.4-3: HMAC -> hex */
    bytes_to_hex_string(hmac_bin, HMAC_BIN_SIZE, hmac_hex, sizeof(hmac_hex));

    /* 4.4-4: final payload: data#hmac */
    int out_len = snprintk((char *)payload, PAYLOAD_MAX_SIZE, "%s#%s", data_part, hmac_hex);
    if (out_len <= 0 || out_len >= PAYLOAD_MAX_SIZE) {
        return false;
    }

    *payload_len = (uint16_t)out_len;
    return true;
}

/* ============================================================
 * Random temp/hum generation (demo) - 
 * ============================================================ */

static uint32_t prng_state;

static uint32_t xorshift32(void)
{
    uint32_t x = prng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    prng_state = x;
    return x;
}

/* 1 decimal precision */
static float rand_range_1dp(float minv, float maxv)
{
    uint32_t r = xorshift32();
    float u = (float)r / (float)UINT32_MAX;
    float v = minv + u * (maxv - minv);

    int scaled = (int)(v * 10.0f + 0.5f);
    return (float)scaled / 10.0f;
}

/* ============================================================
 * 3.x UPLINK SEND LOOP
 * ============================================================ */

static int send_uplink_ascii(const uint8_t *payload, uint16_t len)
{
    enum lorawan_message_type type =
        LORAWAN_USE_CONFIRMED_UPLINK ? LORAWAN_MSG_CONFIRMED : LORAWAN_MSG_UNCONFIRMED;

    /* lorawan_send expects uint8_t length */
    if (len > 255U) {
        return -EINVAL;
    }

    return lorawan_send(LORAWAN_APP_PORT, (uint8_t *)payload, (uint8_t)len, type);
}

int main(void)
{
    LOG_INF("=== App_3172_2 Start ===");

    /* PRNG seed */
    prng_state = (uint32_t)k_cycle_get_32();
    if (prng_state == 0U) {
        prng_state = 0xA5A5A5A5u;
    }

    /* 2.1 Region: EU868 via Kconfig (CONFIG_LORAMAC_REGION_EU868) */

    /* 2.x start LoRaWAN stack */
    int ret = lorawan_start();
    if (ret < 0) {
        LOG_ERR("lorawan_start failed: %d", ret);
        return ret;
    }

    /* 2.x OTAA join */
    ret = lorawan_join_otaa();
    if (ret < 0) {
        LOG_ERR("join failed permanently: %d", ret);
        return ret;
    }

    LOG_INF("Joined OK. send loop starting...");

    while (1) {
        /* 3.x demo sensor values (random) */
        SensorData_t s;
        s.temperature = rand_range_1dp(-10.0f, 40.0f);
        s.humidity    = rand_range_1dp(10.0f, 95.0f);

        LOG_INF("Temp=%.1f C, Hum=%.1f %%RH",
                (double)s.temperature,
                (double)s.humidity);

        /* 4.4 build secure payload string */
        uint8_t payload[PAYLOAD_MAX_SIZE];
        uint16_t payload_len = 0;

        if (!create_secure_payload(&s, payload, &payload_len)) {
            LOG_ERR("payload creation failed");
            k_sleep(K_SECONDS(TX_INTERVAL_SECONDS));
            continue;
        }

        LOG_INF("PayloadStr (%u bytes): %s", payload_len, (char *)payload);

        /* 3.x send uplink */
        ret = send_uplink_ascii(payload, payload_len);
        if (ret < 0) {
            LOG_ERR("lorawan_send failed: %d", ret);
        } else {
            LOG_INF("Uplink sent (%s), port=%d",
                    LORAWAN_USE_CONFIRMED_UPLINK ? "CONFIRMED" : "UNCONFIRMED",
                    LORAWAN_APP_PORT);
        }

        k_sleep(K_SECONDS(TX_INTERVAL_SECONDS));
    }

    return 0;
}
