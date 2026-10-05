# Design

## Context

Referencia de motivación: `proposal.md` (Why). Estado actual relevante:

- `MQTT_Client::subscribeToAll()` abre solo dos suscripciones comodín: `tinygs/global/#` y `tinygs/<user>/<station>/cmnd/#`. Los comandos se resuelven en `manageMQTTData()` por el último segmento del topic, con una cadena de `strcmp`; no hay un topic por comando.
- La contraseña de la consola web es `getApPasswordParameter()->valueBuffer`, que en IotWebConf2 es `_apPassword`. La usan dos superficies: la autenticación HTTP de los handlers de `ConfigManager` (efecto inmediato al escribir el buffer) y el endpoint `/update` vía `httpUpdater.updateCredentials(...)`, que solo se refresca al entrar en `STATE_ONLINE` (IotWebConf2.cpp:746-750).
- `ConfigManager::saveConfig()` marca `remoteSave = true`; `configSavedCallback()` salta `forceApMode`/`scheduleRestart` en guardados remotos, pero reinicia si cambió el nombre de estación.
- El hermano más cercano es `set_name` (`[mac, valor]`), hoy el único comando de identidad.
- No existe todavía ningún publicador: el contrato lo definimos nosotros.

## Goals / Non-Goals

**Goals:**

- Permitir rotar la contraseña de la consola web (`admin`) en remoto por el canal MQTT ya cifrado con TLS.
- No añadir topics ni suscripciones MQTT.
- Dejar una única credencial coherente en panel web, endpoint `/update` y AP tras la operación.

**Non-Goals:**

- Cambiar la contraseña de MQTT o de WiFi.
- Añadir un comando gemelo en la consola web local (`!pass`).
- Implementar el publicador (backend/web); aquí solo se fija el contrato.
- Cifrado/hash en reposo: se mantiene el almacenamiento actual en NVS.
- Consultar o recuperar remotamente la contraseña actual.

## Decisions

1. **Reutilizar la suscripción comodín y añadir la palabra `set_password`.** Alternativa descartada: topic dedicado con su propia suscripción, que añadiría estado en el broker, un `SUBSCRIBE` extra y lógica de re-suscripción, sin aportar aislamiento (el topic de estación ya lo da).
2. **Payload `{"pass":"..."}` sin MAC.** Alternativa descartada: `[mac, pass]` como `set_name`; la MAC es defensiva y redundante porque `cmnd/<station>` ya entrega a una sola placa. Descartado texto plano por ser menos extensible.
3. **Ámbito de estación, nunca global.** La rama se coloca después de la barrera `if (global) return;` de `manageMQTTData()`.
4. **Validación de longitud 8..32 y prohibición de vacío.** IotWebConf exige vacío o >=8; se excluye el vacío para que un comando remoto no pueda desactivar la autenticación del panel. El buffer es de 33 bytes (`IOTWEBCONF_PASSWORD_LEN`).
5. **Guardar y reiniciar.** Alternativa descartada: guardar y llamar explícitamente a `httpUpdater.updateCredentials(...)` para refrescar `/update` sin reiniciar. Se prefiere el reinicio porque, además de sincronizar `/update`, garantiza que el AP y todo el arranque usen la nueva credencial, y sigue el patrón ya existente de `reset` (`ESP.restart()` dentro del callback).
6. **Sin ack de éxito; ack solo en error.** Un `ESP.restart()` inmediato impide publicar el ack al final de `manageMQTTData()`; la confirmación de éxito es el `welcome` que la placa envía al reconectar. Los errores no reinician, así que sí publican código: `1` payload inválido, `2` longitud inválida.
7. **El setter vive en `ConfigManager`; el reinicio, en la capa MQTT.** El setter público encapsula validación + persistencia de la credencial (así `httpUpdater` privado no se expone). El `ESP.restart()` lo ejecuta el manejador MQTT al confirmar el éxito, igual que `reset`, para que `ConfigManager` no decida reiniciar la placa.

## Risks / Trade-offs

- [Bloqueo si se fija una contraseña desconocida] → La pulsación larga del botón llama a `forceDefaultPassword(true)`, que lleva a `STATE_NOT_CONFIGURED` y al AP de rescate con la contraseña por defecto (vacía); no hay bloqueo permanente.
- [El reinicio interrumpe brevemente la recepción de radio y el seguimiento de TLE] → Aceptable y coherente con `reset` y `update`.
- [La contraseña viaja en el payload MQTT] → El canal está cifrado (`SECURE_MQTT`); el manejador no debe registrar la contraseña en claro en los logs.
- [Quien pueda publicar en el topic de comandos puede cambiarla] → Mismo nivel de confianza que `reset`/`tx`; el comando no añade un privilegio nuevo.
- [Escritura en NVS en cada cambio] → Operación infrecuente; sin impacto relevante.
