# Face Detection App

Real-time face detection from your webcam, built with C++ and OpenCV.

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![OpenCV](https://img.shields.io/badge/OpenCV-4.x-5C3EE8?style=flat-square&logo=opencv&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-blue?style=flat-square)

The app opens your webcam, detects faces in each frame using a Haar Cascade classifier, draws a box around every face, and shows a live face count.

## Features

- Real-time detection from a live webcam feed
- Bounding boxes around each detected face
- Live on-screen face count
- Lightweight, with no GPU or extra models required

## Quick Start

```bash
git clone https://github.com/<your-username>/face-detection-app.git
cd face-detection-app

g++ test.cpp -o face_detection `pkg-config --cflags --libs opencv4`
./face_detection
```

Run it from the project folder so the app can find `haarcascade_frontalface_default.xml`.

Press `Esc` or `q` to quit *(adjust this to match your code)*.

## Requirements

- OpenCV 4.x
- A C++ compiler (GCC, Clang, or MSVC)
- A working webcam

## Build on Windows (Visual Studio)

1. Add the OpenCV include and library paths to your project.
2. Link `opencv_world480.lib` (match the version you installed).
3. Build and run.

## Configuration

The app uses camera index `1` by default. If you have a single webcam, change it to `0` in `test.cpp`:

```cpp
VideoCapture video(0);
```

## Project Structure

```text
face-detection-app/
├── test.cpp                              # Main application
├── haarcascade_frontalface_default.xml   # Pre-trained face model
└── README.md
```

## How It Works

1. Open the webcam with `VideoCapture`.
2. Load the Haar Cascade model.
3. Convert each frame to grayscale.
4. Detect faces with `detectMultiScale()`.
5. Draw a rectangle around each face and display the count.

## Troubleshooting

| Problem | Fix |
| --- | --- |
| Black screen or "camera not found" | Change the camera index from `1` to `0` |
| Cascade file fails to load | Run the app from the folder containing the `.xml` file |
| `opencv4` not found by pkg-config | Install OpenCV dev packages, e.g. `sudo apt install libopencv-dev` |

## Roadmap

- [ ] Eye and smile detection
- [ ] Deep-learning face detection (e.g. DNN-based models)
- [ ] Save snapshots
- [ ] Video recording

## Contributing

Contributions are welcome. Fork the repo, create a feature branch, and open a pull request.

## License

Released under the [MIT License](LICENSE).
