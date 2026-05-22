
1. Add OpenCV include and lib paths to your project properties.
2. Link against the OpenCV libraries (e.g. `opencv_world480.lib`).
3. Build and run.

> **Note:** The camera index is set to `1` by default. Change `VideoCapture video(1)` to `VideoCapture video(0)
## H
1. Opens the webcam stream via `V
2. Loads the Haar Cascade fron
3. On each frame, runs `detectMultiScale` to find faces.
4. Draws a rectangle around each detected face and overlays the face count.

## Dependencies

| Library | Purpose |
|---|---|
| `opencv2/imgcodecs` | Image reading/writing |
| `opencv2/highgui` | Window display |
| `opencv2/imgproc` | Image processing |
| `opencv2/objdetect` | Haar Cascade detection |

## License

MIT

