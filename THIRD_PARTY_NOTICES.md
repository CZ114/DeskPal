# Third-party notices

The recovered Edge Impulse export includes its generated model, SDK and upstream dependencies. Their original file-level copyright notices and license texts remain intact. The SDK's top-level license identifies Apache-2.0 as its default, with component-specific exceptions such as BSD-3-Clause for Kiss FFT. The repository root license does not override these notices.

License files recovered with the export:

- [libraries/PoseDetection_inferencing/src/edge-impulse-sdk/LICENSE](libraries/PoseDetection_inferencing/src/edge-impulse-sdk/LICENSE)
- [libraries/PoseDetection_inferencing/src/edge-impulse-sdk/LICENSE-apache-2.0.txt](libraries/PoseDetection_inferencing/src/edge-impulse-sdk/LICENSE-apache-2.0.txt)
- [libraries/PoseDetection_inferencing/src/edge-impulse-sdk/classifier/postprocessing/tinyEKF/LICENSE.md](libraries/PoseDetection_inferencing/src/edge-impulse-sdk/classifier/postprocessing/tinyEKF/LICENSE.md)
- [libraries/PoseDetection_inferencing/src/edge-impulse-sdk/dsp/kissfft/LICENSE](libraries/PoseDetection_inferencing/src/edge-impulse-sdk/dsp/kissfft/LICENSE)
- [libraries/PoseDetection_inferencing/src/edge-impulse-sdk/porting/espressif/ESP-NN/LICENSE](libraries/PoseDetection_inferencing/src/edge-impulse-sdk/porting/espressif/ESP-NN/LICENSE)
- [libraries/PoseDetection_inferencing/src/edge-impulse-sdk/porting/ethos-core-driver/LICENSE.txt](libraries/PoseDetection_inferencing/src/edge-impulse-sdk/porting/ethos-core-driver/LICENSE.txt)
- [libraries/PoseDetection_inferencing/src/edge-impulse-sdk/tensorflow/LICENSE](libraries/PoseDetection_inferencing/src/edge-impulse-sdk/tensorflow/LICENSE)
- [libraries/PoseDetection_inferencing/src/edge-impulse-sdk/third_party/flatbuffers/LICENSE.txt](libraries/PoseDetection_inferencing/src/edge-impulse-sdk/third_party/flatbuffers/LICENSE.txt)
- [libraries/PoseDetection_inferencing/src/edge-impulse-sdk/third_party/gemmlowp/LICENSE](libraries/PoseDetection_inferencing/src/edge-impulse-sdk/third_party/gemmlowp/LICENSE)

The bundled SDK includes Edge Impulse, TensorFlow Lite, ARM CMSIS, FlatBuffers, gemmlowp and other upstream code. Some licenses are embedded in file headers rather than a standalone file. Consult the original files when redistributing individual components.

The firmware also depends on external Arduino/Seeed, Sensirion, ArduinoJson, PubSubClient and NTPClient libraries which are not vendored here. Install and use them under their own licenses.

`media/` contains documentary project photographs and an award certificate. These, and any institutional or vendor marks they contain, are excluded from the root software license. Historical slides, lecture materials and third-party illustrations are retained only in the separate local archive.
