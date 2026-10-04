# MQTT commands

This document describes the MQTT commands a tinyGS station accepts. All
station commands arrive on the station's own command topic and are routed by
the last topic segment; the station never accepts station-scoped commands from
the global topic.

## set_password

Changes the web console password for the station (HTTP user `admin`, the same
credential used by the web dashboard, the firmware upload endpoint, and the
access point).

- **Topic:** `tinygs/<user>/<station>/cmnd/set_password`
- **Payload:** JSON object with a `pass` string, for example `{"pass":"clave12345"}`
- **Scope:** station only. Publishing to `tinygs/global/set_password` has no effect.

### Validation

The password must be 8 to 32 characters long. An empty password is rejected so
that a remote command cannot disable authentication.

### Behaviour

- **Valid password:** the station stores the new password and reboots. It does
  not publish a success acknowledgment; the `welcome` telemetry sent when it
  reconnects confirms the change. The web dashboard, the `/update` firmware
  upload endpoint and the access point all use the new password after the
  reboot.
- **Invalid command:** the station keeps running with the previous password and
  publishes a numeric result on `tinygs/<user>/<station>/stat/set_password`:

  | Result | Meaning                          |
  | ------ | -------------------------------- |
  | `0`    | success (not published)          |
  | `1`    | malformed payload or missing `pass` |
  | `2`    | password length outside 8..32    |

The password is never written to the device logs in clear text.
