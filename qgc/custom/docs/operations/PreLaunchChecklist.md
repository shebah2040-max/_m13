# Pre-Launch Checklist — M130 GCS

- **Document ID**: OPS-PREFLIGHT-001
- **Revision**: A

> كل بند يُسجَّل في AuditLog مع معرف المستخدم + الوقت + نتيجة (GO / NO-GO / HOLD).
> التوقيع الرقمي يتم عبر `M130_COMMAND_CHECKLIST_SIGN` إلى الطائرة لضمان التزامن.

## T-60:00 — System Bring-Up
- [ ] GCS booted, all consoles healthy
- [ ] Protocol version handshake successful
- [ ] All FactGroups receiving data
- [ ] Watchdogs green
- [ ] FDR recording confirmed
- [ ] Audit log connected
- [ ] Users authenticated: Operator, Flight Director, Safety Officer, RSO

## T-45:00 — Environment
- [ ] Weather within limits
- [ ] Winds aloft ≤ spec
- [ ] No active NOTAMs in corridor
- [ ] Airspace clear (range safety poll)

## T-30:00 — Vehicle Health
- [ ] Sensors online (IMU/GPS/Baro/Mag)
- [ ] GPS fix 3D, ≥ 10 satellites, no jamming
- [ ] Servos 4/4 online
- [ ] Battery voltage within nominal
- [ ] Actuator self-test passed
- [ ] MPC solver test OK
- [ ] MHE initialization OK

## T-15:00 — Mission Load
- [ ] Mission file loaded and signed
- [ ] Safety Envelope loaded and signed by Safety Officer
- [ ] Waypoints confirmed
- [ ] IIP prediction within range safety corridor

## T-10:00 — Safety Arming
- [ ] FTS system test (dry-run) OK
- [ ] RSO authenticated at console
- [ ] Safety Officer authenticated at console
- [ ] Communication links redundant & healthy
- [ ] Recording to FDR confirmed
- [ ] Ground tracking assets online

## T-05:00 — Final Go/No-Go Poll
- [ ] Flight Director: GO
- [ ] Operator: GO
- [ ] Safety Officer: GO
- [ ] RSO: GO
- [ ] Weather: GO

## T-02:00 — Arm
- [ ] ARM command sent
- [ ] Vehicle acknowledges ARMED
- [ ] GCS state machine in ARMED

## T-00:00 — Launch
- [ ] Launch command sent
- [ ] State transition ARMED → BOOST observed
- [ ] Sensor data alive, envelope clean
- [ ] Commanded fins responding

## Abort Criteria (any at any time)
- Any Warning-level alert uncleared > 30 s
- Any Emergency-level alert
- Any safety envelope Emergency violation
- Link loss > 10 s in BOOST/CRUISE
- Any RSO call for HOLD/ABORT
