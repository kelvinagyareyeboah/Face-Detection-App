
            Scalar boxColor(0, 255, 0);

            rectangle(
                frame,
                face,
                boxColor,
                3
            );

            Point center(
                face.x + face.width / 2,
                face.y + face.height / 2
            );

            circle(
                frame,
                center,
                4,
                Scalar(255, 0, 0),
                FILLED
            );

            string label =
                "Face " + to_string(i + 1);

            int baseline = 0;

            Size textSize =
                getTextSize(
                    label,
                    FONT_HERSHEY_SIMPLEX,
                    0.6,
                    2,
                    &baseline
                );

            rectangle(
                frame,
                Point(face.x, face.y - 35),
                Point(face.x + textSize.width + 10, face.y),
                Scalar(0, 255, 0),
                FILLED
            );

            putText(
                frame,
                label,
                Point(face.x + 5, face.y - 10),
                FONT_HERSHEY_SIMPLEX,
                0.6,
                Scalar(0, 0, 0),
                2
            );
        }

        // ==========================
        // FPS Calculation
        // ==========================
        double currentTime = (double)getTickCount();

        fps = 0.9 * fps +
              0.1 * (
                  getTickFrequency() /
                  (currentTime - prevTime)
              );

        prevTime = currentTime;

        // ==========================
        // Dashboard Panel
        // ==========================
        rectangle(
            frame,
            Point(0, 0),
            Point(450, 120),
            Scalar(20, 20, 20),
            FILLED
        );

        putText(
            frame,
            "AI FACE DETECTION SYSTEM",
            Point(15, 30),
            FONT_HERSHEY_DUPLEX,
            0.8,
            Scalar(0, 255, 255),
            2
        );

        putText(
            frame,
            "Faces: " + to_string(faces.size()),
            Point(15, 65),
            FONT_HERSHEY_SIMPLEX,
            0.7,
            Scalar(255, 255, 255),
            2
        );

        putText(
            frame,
            "FPS: " + to_string((int)fps),
            Point(15, 95),
            FONT_HERSHEY_SIMPLEX,
            0.7,
            Scalar(0, 255, 0),
            2
        );

        putText(
            frame,
            "Q = Exit",
            Point(280, 95),
            FONT_HERSHEY_SIMPLEX,
            0.7,
            Scalar(0, 0, 255),
            2
        );

        // ==========================
        // Crosshair
        // ==========================
        int centerX = frame.cols / 2;
        int centerY = frame.rows / 2;

        line(
            frame,
            Point(centerX - 20, centerY),
            Point(centerX + 20, centerY),
            Scalar(255, 255, 255),
            1
        );

        line(
            frame,
            Point(centerX, centerY - 20),
            Point(centerX, centerY + 20),
            Scalar(255, 255, 255),
            1
        );

        // ==========================
        // Show Frame
        // ==========================
        imshow("AI Face Detection Pro", frame);

        char key = (char)waitKey(1);

        if (key == 'q' || key == 'Q')
            break;
    }

    camera.release();
    destroyAllWindows();

    return 0;
}
