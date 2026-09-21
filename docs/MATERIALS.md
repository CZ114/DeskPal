# Recovered materials and provenance

Recovery date: **2026-09-21**. The original files were copied without alteration into a sibling local archive. Source paths and full SHA-256 hashes are retained in the private audit manifest. The public repository contains only selected project source, model export, photographs and new documentation.

| Material | Recovery result | Repository treatment |
| --- | --- | --- |
| Submitted `IOT_miniProject_code.ino` | Found; byte-identical to firmware inside submitted ZIP | Network settings externalized as `firmware/DeskPal/DeskPal.ino` |
| Earlier `final_1.0.12.ino` | Found | Preserved in private archive |
| `IOT_mini project_code.zip` | Found | Original kept private because it contains hardcoded network settings |
| `PoseDetection_inferencing` | Found, version 1.0.13 | Included under `libraries/` with original license notices |
| `Final_iot_demo.pptx` | Found, 15 slides | Original retained in private archive |
| `Final_iot_slides.pdf` | Found, 15 pages | Original retained in private archive |
| `PPT.docx` | Found, business-case notes (not a complete presentation transcript) | Original retained in private archive |
| Award certificate | Found as original image inside `award.docx` and web rendition | Web rendition in `media/`; original retained locally |
| Prototype and reminder photos | Found in prior portfolio assets | Included in `media/` |
| Presentation MP3 and SRT | Found | Original retained in private archive |
| Jianying video project | Found with source-clip references | Original retained in private archive |
| Final presentation MP4 | Link found; HTTP 404 on recovery date | Not recovered |
| Three source MOV clips | Names and obsolete paths found | Not recovered |
| Raw ML training/evaluation data | No matching project export found | Not recovered |
| Lecture PDF and external illustration assets | Found alongside project | Local contextual archive only; not licensed as DeskPal work |

## Why original coursework files are separate

The deck contains a student identifier, a private university sharing link and third-party illustrations. The source ZIP contains original Wi-Fi settings. These files remain available locally but are not staged for public Git distribution. A reviewed publication edition of the historical deck would require explicit redaction and a material-rights review; this recovery does not silently change the original historical record.

## Changes in the public application

The submitted sketch is renamed to `DeskPal.ino` inside the matching Arduino sketch directory. Four hardcoded network/client strings are replaced by macros from a gitignored `config.h`; a safe `config.example.h` is provided. An attribution header is added. No application control-flow changes are included.

## Evidence boundaries

The certificate establishes the award and date. The code and model headers establish implemented logic and exported model configuration. Photos document the original prototype appearance. The slides describe both demonstrated behavior and future proposals. None of these substitutes for a fresh firmware build, live hardware run or independent model evaluation.
