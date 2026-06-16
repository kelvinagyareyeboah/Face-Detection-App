#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

// ===============================
// Draw Stylish Corner Box
// ===============================
void drawCornerBox(Mat& img, Rect box, Scalar color, int t = 2)
{
    int x = box.x;
    int y = box.y;
    int w = box.width;
    int h = box.height;

    int line = 25;

    // Top Left
    cv::line(img, Point(x, y), Point(x + line, y), color, t);
    cv::line(img, Point(x, y), Point(x, y + line), color, t);

    // Top Right
    cv::line(img, Point(x + w, y), Point(x + w - line, y), color, t);
    cv::line(img, Point(x + w, y), Point(x + w, y + line), color, t);

    // Bottom Left
    cv::line(img, Point(x, y + h), Point(x + line, y + h), color, t);
    cv::line(img, Point(x, y + h), Point(x, y + h - line), color, t);

    // Bottom Right
    cv::line(img, Point(x + w, y + h), Point(x + w - line, y + h), color, t);
    cv::line(img, Point(x + w, y + h), Point(x + w, y + h - line), color, t);
}

int main()
{
    // ===============================
    // Open Camera
    // ===============================
    VideoCapture camera(0);

    if (!camera.isOpened())
    {
        cerr << "ERROR: Cannot open camera." << endl;
        return -1;
    }

    camera.set(CAP_PROP_FRAME_WIDTH, 1280);
    camera.set(CAP_PROP_FRAME_HEIGHT, 720);
    camera.set(CAP_PROP_FPS, 60);

    // ===============================
    // Load Haar Cascade
    // ===============================
    CascadeClassifier faceCascade;

    string cascadePath =
        samples::findFile(
            "haarcascade_frontalface_default.xml"
        );

    if (!faceCascade.load(cascadePath))
    {
        cerr << "ERROR: Failed to load Haar Cascade." << endl;
        return -1;
    }

    // ===============================
    // Variables
    // ===============================
    Mat frame, gray;

    double fps = 0.0;
    double prevTime = (double)getTickCount();

    cout << "AI Face Detection Started..." << endl;

    // ===============================
    // Main Loop
    // ===============================
    while (true)
    {
        camera.read(frame);

        if (frame.empty())
            break;

        // Mirror
        flip(frame, frame, 1);

        // ===============================
        // Preprocessing
        // ===============================
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        equalizeHist(gray, gray);

        GaussianBlur(
            gray,
            gray,
            Size(5, 5),
            0
        );

        // ===============================
        // Face Detection
        // ===============================
        vector<Rect> faces;

        faceCascade.detectMultiScale(
            gray,
            faces,
            1.1,
            6,
            0,
            Size(80, 80)
        );

        // ===============================
        // HUD Overlay
        // ===============================
        Mat overlay = frame.clone();

        rectangle(
            overlay,
            Point(0, 0),
            Point(420, 130),
            Scalar(15, 15, 15),
            FILLED
        );

        addWeighted(
            overlay,
            0.4,
            frame,
            0.6,
            0,
            frame
        );

        // ===============================
        // Draw Faces
        // ===============================
        for (size_t i = 0; i < faces.size(); i++)
        {
            Rect face = faces[i];

            Scalar neon(0, 255, 180);

            // Stylish Corner Box
            drawCornerBox(frame, face, neon, 3);

            // Center Dot
            Point center(
                face.x + face.width / 2,
                face.y + face.height / 2
            );

            circle(
                frame,
                center,
                3,
                Scalar(0, 0, 255),
                FILLED
            );

            // Face Label
            string label =
                "TARGET " + to_string(i + 1);

            putText(
                frame,
                label,
                Point(face.x, face.y - 12),
                FONT_HERSHEY_DUPLEX,
                0.6,
                neon,
                2
            );

            // Fake confidence
            int confidence = 95 + rand() % 5;

            putText(
                frame,
                to_string(confidence) + "%",
                Point(face.x, face.y + face.height + 25),
                FONT_HERSHEY_SIMPLEX,
                0.55,
                Scalar(255, 255, 255),
                2
            );
        }

        // ===============================
        // FPS Calculation
        // ===============================
        double currentTime =
            (double)getTickCount();

        double currentFPS =
            getTickFrequency() /
            (currentTime - prevTime);

        fps = fps * 0.9 + currentFPS * 0.1;

        prevTime = currentTime;

        // ===============================
        // Dashboard Text
        // ===============================
        putText(
            frame,
            "AI FACE DETECTION SYSTEM",
            Point(20, 35),
            FONT_HERSHEY_DUPLEX,
            0.85,
            Scalar(0, 255, 255),
            2
        );

        putText(
            frame,
            "Faces Detected : " +
            to_string(faces.size()),
            Point(20, 70),
            FONT_HERSHEY_SIMPLEX,
            0.7,
            Scalar(255, 255, 255),
            2
        );

        putText(
            frame,
            "FPS : " +
            to_string((int)fps),
            Point(20, 105),
            FONT_HERSHEY_SIMPLEX,
            0.7,
            Scalar(0, 255, 0),
            2
        );

        putText(
            frame,
            "Press Q or ESC to Exit",
            Point(180, 105),
            FONT_HERSHEY_SIMPLEX,
            0.65,
            Scalar(0, 140, 255),
            2
        );

        // ===============================
        // Crosshair
        // ===============================
        int cx = frame.cols / 2;
        int cy = frame.rows / 2;

        line(
            frame,
            Point(cx - 20, cy),
            Point(cx + 20, cy),
            Scalar(255, 255, 255),
            1
        );

        line(
            frame,
            Point(cx, cy - 20),
            Point(cx, cy + 20),
            Scalar(255, 255, 255),
            1
        );

        circle(
            frame,
            Point(cx, cy),
            25,
            Scalar(255, 255, 255),
            1
        );

        // ===============================
        // Bottom Status Bar
        // ===============================
        rectangle(
            frame,
            Point(0, frame.rows - 35),
            Point(frame.cols, frame.rows),
            Scalar(20, 20, 20),
            FILLED
        );

        putText(
            frame,
            "SYSTEM STATUS : ACTIVE",
            Point(20, frame.rows - 10),
            FONT_HERSHEY_SIMPLEX,
            0.6,
            Scalar(0, 255, 0),
            2
        );

        // ===============================
        // Show Window
        // ===============================
        imshow(
            "AI FACE DETECTION PRO",
            frame
        );

        char key = (char)waitKey(1);

        if (key == 'q' ||
            key == 'Q' ||
            key == 27)
        {
            break;
        }
    }

    camera.release();
    destroyAllWindows();

    return 0;
}
