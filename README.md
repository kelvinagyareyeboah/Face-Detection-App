# 👁️ Face Detection App

<p align="center">

<img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white"/>
<img src="https://img.shields.io/badge/OpenCV-5C3EE8?style=for-the-badge&logo=opencv&logoColor=white"/>
<img src="https://img.shields.io/badge/Computer%20Vision-AI-success?style=for-the-badge"/>
<img src="https://img.shields.io/badge/License-MIT-blue?style=for-the-badge"/>
<img src="https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge"/>

</p>

---

## 🧠 Overview

A real-time **Face Detection Application** built with **C++** and **OpenCV** using the powerful **Haar Cascade Classifier**.

The application captures live webcam video, detects human faces in real time, draws bounding boxes around detected faces, and displays the total number of faces currently visible on screen.

---

# ✨ Features

✅ Real-time webcam face detection
✅ Haar Cascade frontal face recognition
✅ Live face count display
✅ Fast and lightweight performance
✅ Bounding box visualization
✅ Cross-platform support
✅ Beginner-friendly OpenCV project

---

# 📸 Demo

The application:

* Opens your webcam stream
* Detects faces frame-by-frame
* Draws rectangles around detected faces
* Displays the total face count in real time

---

# 🛠️ Tech Stack

| Technology                                                                                             | Purpose                   |
| ------------------------------------------------------------------------------------------------------ | ------------------------- |
| ![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=flat-square\&logo=cplusplus\&logoColor=white) | Core programming language |
| ![OpenCV](https://img.shields.io/badge/OpenCV-5C3EE8?style=flat-square\&logo=opencv\&log)  | Computer vision library   |
| Haar Cascade                                                                                           | Face detection algorithm  |
| Webcam                                                                                                 | Live video input          |

---

# 📂 Project Structure

```bash id="ozth3e"
face-detection-app/
│
├── test.cpp
├── haarcascade_frontalface_default.xml
└── README.md
```

---

# ⚙️ Requirements

Before running the project, ensure you have:

* OpenCV 4.x installed
* A C++ compiler:

  * GCC
  * Clang
  * MSVC (Visual Studio)
* A working webcam

---

# 🚀 Build & Run

## 🔹 Using g++

```bash id="qoqrlx"
g++ test.cpp -o face_detection `pkg-config --cflags --libs opencv4`
./face_detection
```

---

## 🔹 Using Visual Studio (MSVC)

### Steps:

1. Configure OpenCV include paths
2. Configure OpenCV library paths
3. Link OpenCV libraries:

```bash id="5fjnij"
opencv_world480.lib
```

4. Build and run the project

---

# ⚠️ Camera Configuration

The default webcam index is:

```cpp id="b39os5"
VideoCapture video(1);
```

If you only have one webcam, change it to:

```cpp id="w1cfxk"
VideoCapture video(0);
```

---

# 🔍 How It Works

1️⃣ Opens webcam stream using `VideoCapture`
2️⃣ Loads the Haar Cascade XML model
3️⃣ Converts frames for processing
4️⃣ Detects faces using `detectMultiScale()`
5️⃣ Draws rectangles around detected faces
6️⃣ Displays face count in real time

---

# 📦 OpenCV Modules Used

| Module              | Purpose                 |
| ------------------- | ----------------------- |
| `opencv2/highgui`   | Display windows         |
| `opencv2/imgproc`   | Image processing        |
| `opencv2/objdetect` | Face detection          |
| `opencv2/imgcodecs` | Image encoding/decoding |

---

# 🎯 Future Improvements

* 😎 Eye & smile detection
* 🧠 Deep learning-based face recognition
* 📸 Face snapshot capture
* 🎥 Video recording support
* 📊 Detection analytics dashboard
* ⚡ GPU acceleration

---

# 🤝 Contributing

Contributions are welcome!

```bash id="1olcvf"
1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to your branch
5. Open a Pull Request
```

---

# 📄 License

This project is licensed under the **MIT License**.

---

# 👨‍💻 Developer

Built with ❤️ using **C++ & OpenCV**

---

<p align="center">

⭐ Star the repository if you found this project useful!

</p>

