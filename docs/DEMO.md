# Demonstration guide

This is a reproduction guide based on the source and original presentation. No historical video has been recovered and no hardware run was performed during packaging.

1. Connect the SHT40 and upload the firmware after completing [setup](SETUP.md).
2. Confirm successful Wi-Fi and MQTT connection on the serial output. The initial screen shows a sleeping face, clock and environmental readings.
3. Use an MQTT client such as MQTTX to subscribe to `weather/request`, `Wio-ENV`, `facial` and `reminders` on your own broker.
4. Press the **rightmost A button** to start the system.
5. Publish the content of [`examples/weather-response.json`](../examples/weather-response.json) on `weather/response`. Use `cloudy` first to avoid entering the sunbath path while checking motion interaction.
6. Observe class probabilities while the device remains still, is raised, or is waved. These interactions represent device motion; do not interpret them as validated human posture detection.
7. Allow the inactivity counter to reach its threshold, observe the reminder, then raise the device and observe the expression and affection response.
8. Restart the session and respond with `{"city":"Ningbo","weather":"sunny"}` to explore the light-dependent 30-second sunbath interaction.
9. The middle B button is an original affection-increment test shortcut. The leftmost C button acknowledges the original bedtime/sleep flow.

The bedtime condition has an original demo-time inconsistency and may show reminders during daytime. Review [KNOWN_LIMITATIONS.md](KNOWN_LIMITATIONS.md) before presenting the prototype.

## Historical recording recovery

The original deck references a file named `PPT_Presentation_IOT_mini project.mp4` via a private university SharePoint link. A 2026-09-21 HTTP check returned 404. This is an access result, not proof of permanent deletion.

The recovered Jianying project has a timeline duration of **1156.266666 seconds** (about **19 min 16 s**) and references three source clips:

| File | Recorded source duration |
| --- | --- |
| `2024-12-10 134612.mov` | 27.866666 s |
| `2024-12-10 134936.mov` | 406.1 s |
| `2024-12-10 135656.mov` | 867.4 s |

The timeline duration is editor metadata; it is not a recovered MP4's measured duration. The source paths no longer resolve. Audio, subtitles and the editor project are retained in the separate private archive; exact recovery links and paths are in that archive's index.

The recovered MP3 has a measured duration of **1156.310188 seconds** (about 19 min 16 s). Its SRT begins with a DeskPal presentation introduction, confirming that the audio/subtitles belong to this project. Both remain in the local archive because the opening includes a student identifier.
