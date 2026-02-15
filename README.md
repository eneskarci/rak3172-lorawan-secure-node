# RAK3172 LoRaWAN Secure Sensor Node
Zephyr RTOS 4.3.0 | STM32WL55 | EU868

Bu proje, RAK3172 (STM32WL55) tabanlý bir LoRaWAN sensör düðümünün
Zephyr RTOS kullanýlarak geliþtirilmiþ güvenli veri iletim uygulamasýdýr.

Uygulama, EU868 bölgesinde OTAA (Over-The-Air Activation) ile
LoRaWAN aðýna baðlanýr ve belirli aralýklarla sýcaklýk ve nem verisi gönderir.
Sensör verileri demo amaçlý olarak rastgele üretilmektedir.

Gönderilen her veri paketi HMAC-SHA256 algoritmasý ile imzalanýr.
Bu sayede backend tarafýnda veri bütünlüðü doðrulanabilir ve
paket üzerinde deðiþiklik olup olmadýðý kontrol edilebilir.

---

## Veri Formatý

Payload ASCII formatýnda gönderilir:

T:23.5,H:48.1#<64_hex_hmac>

---

## Özellikler

- LoRaWAN 1.0.3 OTAA
- EU868 Bölge
- ADR Aktif
- Confirmed / Unconfirmed Uplink
- mbedTLS HMAC-SHA256
- Zephyr LoRaWAN Stack
- Join Retry Mekanizmasý

---

## Derleme

west build -b rak3172 app_3172_2

west flash

---

## Güvenlik Notu

Bu repoda bulunan anahtarlar test amaçlýdýr.
Gerçek ürünlerde kriptografik anahtarlar kaynak kod içinde tutulmamalýdýr.
Secure element veya güvenli provisioning yöntemleri kullanýlmalýdýr.
