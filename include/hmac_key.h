#pragma once
#include <stddef.h>

/*
 * 4.1 HMAC Secret Key
 * ?? SECURITY NOTE (doc):
 * - Production: plaintext source code i?inde tutulmamal?.
 * - Secure Element / Encrypted Flash / Provisioning ile saklanmal?.
 */

// Test/Development key (32 characters)
static const char HMAC_SECRET_KEY[] = "SensecapStm32WL55SecretKey2024";
#define HMAC_KEY_SIZE 32
