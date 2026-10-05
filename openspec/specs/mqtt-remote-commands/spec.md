# mqtt-remote-commands Specification

## Purpose

Define cómo una estación tinyGS recibe, valida y ejecuta comandos de gestión entregados por MQTT en el topic de su propia estación, incluyendo el comando remoto `set_password` y sus acuses de error.

## Requirements

### Requirement: Despacho de comandos por el topic de la estación

La estación SHALL recibir los comandos de gestión por la suscripción comodín del árbol `cmnd` y SHALL resolver el comando por el último segmento del topic. Los comandos de gestión de estación como `set_password` SHALL NOT ejecutarse cuando llegan por el topic global.

#### Scenario: Comando recibido en el topic de estación

- **WHEN** llega un mensaje a `tinygs/<user>/<station>/cmnd/set_password`
- **THEN** la estación lo despacha al manejador de `set_password`

#### Scenario: Comando de estación recibido por el topic global

- **WHEN** llega un mensaje `set_password` a `tinygs/global/set_password`
- **THEN** la estación no cambia la contraseña de la consola web

### Requirement: set_password acepta un payload JSON con la contraseña

El comando `set_password` SHALL interpretar un payload JSON en forma de objeto con un campo de texto `pass` como la nueva contraseña de la consola web (usuario `admin`).

#### Scenario: Payload válido

- **WHEN** se recibe `set_password` con `{"pass":"una-clave-valida"}`
- **THEN** la estación toma `una-clave-valida` como contraseña candidata

#### Scenario: Payload ausente o mal formado

- **WHEN** se recibe `set_password` con un payload que no es un objeto JSON o no contiene un campo de texto `pass`
- **THEN** la estación informa un error de payload y no cambia la contraseña

### Requirement: set_password valida la longitud de la contraseña

La estación SHALL aceptar únicamente contraseñas de entre 8 y 32 caracteres inclusive y SHALL rechazar una contraseña vacía.

#### Scenario: Contraseña demasiado corta

- **WHEN** se recibe `set_password` con un valor `pass` de menos de 8 caracteres, incluida la cadena vacía
- **THEN** la estación informa un error de longitud y no cambia la contraseña

#### Scenario: Contraseña demasiado larga

- **WHEN** se recibe `set_password` con un valor `pass` de más de 32 caracteres
- **THEN** la estación informa un error de longitud y no cambia la contraseña

### Requirement: Cambio de contraseña correcto persiste y reinicia

Ante un `set_password` válido, la estación SHALL guardar la nueva contraseña de la consola web y SHALL reiniciarse para que el panel web, el endpoint de subida de firmware y el punto de acceso usen la nueva credencial.

#### Scenario: Contraseña aplicada

- **WHEN** se recibe `set_password` con una contraseña válida
- **THEN** la estación guarda la nueva contraseña y se reinicia, sin publicar un acuse de éxito del comando

#### Scenario: Confirmación tras el reinicio

- **WHEN** la estación vuelve a conectar a MQTT después del reinicio por cambio de contraseña
- **THEN** publica su telemetría de `welcome` como de costumbre

### Requirement: El fallo de cambio de contraseña se acusa sin reiniciar

Cuando un comando `set_password` no puede aplicarse, la estación SHALL publicar un resultado numérico distinto de cero en `tinygs/<user>/<station>/stat/set_password` y SHALL NOT reiniciarse.

#### Scenario: Acuse de error

- **WHEN** `set_password` falla por payload mal formado o por longitud inválida
- **THEN** la estación publica un código de resultado distinto de cero en el topic `stat` del comando y continúa funcionando con la contraseña anterior
