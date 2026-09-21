# Known limitations of the recovered 2024 prototype

This release preserves the submitted application behavior. Packaging changes are limited to file naming, attribution and externalized configuration.

- **Build not reproduced:** original external dependency versions are missing; no Wio board core was installed in the checked Arduino CLI environment, and no hardware compilation or upload is claimed.
- **No new evaluation:** raw training/testing recordings and evaluation splits were not found. Slide-reported training settings are historical descriptions, not independently reproduced results.
- **Blocking network:** Wi-Fi and MQTT retry loops can block indefinitely. No TLS or broker authentication is implemented.
- **Weather gate:** motion interaction waits for a manually published weather JSON response; there is no autonomous weather API or response timeout.
- **Bedtime demo condition:** `currentHour >= 23 || currentHour <= 20` triggers at 00:00–20:59 and 23:00–23:59; this differs from the comment describing an 11 pm reminder. One MQTT message also says “16 PM”. These are preserved original demo inconsistencies.
- **Sampling deadline arithmetic:** `delayMicroseconds(next_tick - micros())` may underflow if a sampling deadline is missed. Timing requires hardware review before making real-time claims.
- **Counter semantics:** the inactivity counter measures loop outcomes, not elapsed sedentary time; the wave branch can increment it in addition to the non-rise branch.
- **Wave timing:** the 5-second consecutive-wave window interacts with blocking delays and approximately 2 seconds of acquisition; recognition timing has not been revalidated.
- **Affection latch:** the maximum-affection flag remains set after subsequent decreases, preventing later increases.
- **Initialization:** sensor and NTP setup order, bus readiness and board-specific Wi-Fi includes need verification against the selected dependency versions.
- **Presentation claims:** commercial projections, proposed health benefits, BLE fallback, TLS, two-factor authentication, regulatory approvals, cloud updates and support policies in the course slides are proposals, not verified capabilities of this firmware.
- **Historical media:** final MP4 and raw MOV clips remain missing. The presentation's title date and award date differ; the certificate independently specifies December 2024.
