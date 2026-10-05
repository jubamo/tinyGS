# Proposal

## Why

Hoy el operador solo puede cambiar la contraseña de la consola web (usuario `admin`) de su estación accediendo físicamente al panel de configuración en modo AP. No existe una vía remota: si la contraseña se filtra o se olvida, hay que ir a la placa. Añadir el cambio de contraseña al canal de comandos MQTT existente permite rotarla en remoto por un enlace ya autenticado y cifrado con TLS.

## What Changes

- Nuevo comando MQTT `set_password`, de ámbito de estación, recibido en `tinygs/<user>/<station>/cmnd/set_password`.
- Payload JSON `{"pass":"..."}`; no se usa la MAC (el topic ya aísla la estación).
- Validación de longitud: entre 8 y 32 caracteres; no se permite vaciar la contraseña.
- En caso de éxito: se persiste la contraseña de la consola web y la placa se reinicia; no se emite ack (el `welcome` al reconectar confirma).
- En caso de error (payload mal formado o longitud inválida): se emite un ack numérico en `stat/set_password` y no se reinicia.
- No se añaden topics ni suscripciones MQTT nuevas: el comando viaja por la suscripción `cmnd/#` ya existente.
- El comando nunca se acepta por el árbol global.

## Capabilities

### New Capabilities

- `mqtt-remote-commands`: contrato de comandos MQTT entrantes de la estación (resolución por topic de estación, payload, validación y acks), incluido el nuevo comando remoto `set_password`.

### Modified Capabilities

<!-- Ninguna: el proyecto aún no tiene specs. -->

## Impact

- `tinyGS/src/Mqtt/MQTT_Client.h` y `MQTT_Client.cpp`: constante del comando y manejador.
- `tinyGS/src/ConfigManager/ConfigManager.h` y `ConfigManager.cpp`: setter público que valida, persiste y dispara el reinicio.
- No hay cambios en backend ni web dentro de este repositorio; el contrato se documenta para el futuro publicador.
