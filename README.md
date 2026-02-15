# RAK3172 LoRaWAN Secure Sensor Node  
**Zephyr RTOS 4.3.0 | STM32WL55 | EU868 | OTAA**

![Zephyr](https://img.shields.io/badge/Zephyr-4.3.0-blue)
![LoRaWAN](https://img.shields.io/badge/LoRaWAN-1.0.3-green)
![Region](https://img.shields.io/badge/Region-EU868-orange)
![MCU](https://img.shields.io/badge/MCU-STM32WL55-red)
![Security](https://img.shields.io/badge/Security-HMAC--SHA256-purple)

---

### Proje Tanımı

Bu proje, **RAK3172 (STM32WL55)** tabanlı bir LoRaWAN sensör düğümünün Zephyr RTOS kullanılarak geliştirilmiş güvenli veri iletim uygulamasıdır.

Cihaz EU868 bölgesinde çalışır, OTAA (Over-The-Air Activation) ile ağa katılır ve periyodik olarak sıcaklık/nem verisi gönderir. Gönderilen her paket **HMAC-SHA256** ile imzalanır. Bu sayede backend tarafında veri bütünlüğü doğrulanabilir.

---

### Teknik Özellikler

- LoRaWAN 1.0.3
- OTAA Activation
- EU868 Region
- ADR (Adaptive Data Rate) aktif
- Confirmed / Unconfirmed uplink desteği
- Zephyr Native LoRaWAN Stack
- mbedTLS HMAC-SHA256

---

### Güvenlik Mekanizması

Gönderilen her payload aşağıdaki formatta imzalanır:

```
T:23.5,H:48.1#<64_hex_hmac>
```

- `T` → Sıcaklık  
- `H` → Nem  
- `#` sonrası → HMAC-SHA256 (64 hex karakter)

Backend tarafında aynı secret key ile yeniden hesaplama yapılarak veri doğrulaması yapılabilir.

---

### Derleme ve Yükleme

```bash
west build -b rak3172 app_3172_2
west flash
```

---

## English

### Project Description

This project is a secure LoRaWAN sensor node implementation developed for **RAK3172 (STM32WL55)** using Zephyr RTOS.

The device operates in the EU868 region, joins via OTAA, and periodically transmits temperature/humidity data. Each payload is signed with **HMAC-SHA256**, enabling backend-side data integrity verification.

---

### Technical Features

- LoRaWAN 1.0.3
- OTAA Activation
- EU868 Region
- ADR Enabled
- Confirmed / Unconfirmed uplink
- Zephyr Native LoRaWAN Stack
- mbedTLS HMAC-SHA256

---

### Security Mechanism

Each transmitted payload is formatted as:

```
T:23.5,H:48.1#<64_hex_hmac>
```

- `T` → Temperature  
- `H` → Humidity  
- After `#` → 64-character hex HMAC-SHA256 signature  

The backend can recompute and validate integrity using the shared secret key.

---

### Build & Flash

```bash
west build -b rak3172 app_3172_2
west flash
```
