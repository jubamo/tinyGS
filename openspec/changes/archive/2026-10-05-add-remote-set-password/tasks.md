# Tasks

## 1. Setter de contraseña en ConfigManager

- [x] 1.1 Añadir `bool setWebPassword(const char* pass)` público en `tinyGS/src/ConfigManager/ConfigManager.h` y su implementación en `ConfigManager.cpp`: rechazar si `strlen(pass) < 8` o `> 32`, copiar con `strncpy` al buffer de `getApPasswordParameter()` garantizando terminación, llamar a `saveConfig()` y devolver el resultado; verificar con `pio run -e ESP32`.
- [x] 1.2 Registrar en el log el cambio de contraseña sin incluir el valor en claro; verificar por inspección que ningún `Log::*` del nuevo código imprime `pass`.

## 2. Comando MQTT set_password

- [x] 2.1 Añadir la constante PROGMEM `commandSetPassword = "set_password"` y la declaración `manageSetPassword(char* payload, size_t payload_len)` en `tinyGS/src/Mqtt/MQTT_Client.h`; verificar con `pio run -e ESP32`.
- [x] 2.2 Implementar `manageSetPassword` en `MQTT_Client.cpp`: parsear el objeto JSON con `doc["pass"]` y devolver `1` si no es un objeto o falta `pass`, `2` si la longitud es inválida, `0` si `setWebPassword` tuvo éxito; verificar por inspección que los tres caminos devuelven el código correspondiente.
- [x] 2.3 Enganchar la rama `set_password` en `manageMQTTData` después de `if (global) return;`: en éxito registrar el mensaje y llamar a `ESP.restart()` sin continuar al ack; en error asignar el código a `result` y dejar que se publique el ack existente; verificar con `pio run -e ESP32`.
- [x] 2.4 Documentar el contrato del comando (topic `tinygs/<user>/<station>/cmnd/set_password`, payload `{"pass":"..."}`, validación 8..32, reinicio en éxito, códigos de error `1`/`2` en `stat/set_password`) en `doc/mqtt-commands.md`; verificar que el topic y el payload coinciden con las constantes y el parseo del código.

## 3. Verificación de integración

- [x] 3.1 Compilar los entornos principales (`pio run -e ESP32 -e ESP32-S3 -e ESP32-C3`) y verificar que todos construyen sin errores.
- [x] 3.2 Con la placa conectada a MQTT, publicar `{"pass":"clave12345"}` en `tinygs/<user>/<station>/cmnd/set_password` y verificar que la placa se reinicia y reenvía `welcome`; comprobar que la nueva contraseña abre el panel web.
- [x] 3.3 Publicar `{"pass":"corta"}` y un payload mal formado, y verificar que se recibe un ack no cero en `stat/set_password` y que la placa no se reinicia y conserva la contraseña anterior.
- [x] 3.4 Publicar `set_password` en `tinygs/global/set_password` y verificar que ninguna estación cambia la contraseña.
