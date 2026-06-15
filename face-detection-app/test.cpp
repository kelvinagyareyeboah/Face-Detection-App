
        prevTick = currentTick;

        // Draw Face Boxes
        for (size_t i = 0; i < faces.size(); i++)
        {
            Reci];

            rectangle(
                frame,
                face,
                Scalar(
                3
            );

            // Face Center
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

            // Face ID
            string label =
                "Face #" + to_string(i + 1);

            putText(
                frame,
                label,
                Point(face.x, face.y - 10),
                FONT_HERSHEY_SIMPLEX,
                0.7,
                Scalar(0, 255, 0),
                2
            );
        }

        // Top Information Panel
        rectangle(
            frame,
            Point(0, 0),
            Point(450, 110),
            Scalar(30, 30, 30),
            FILLED
        );

        putText(
            frame,
            "Real-Time Face Detection System",
            Point(10, 25),
            FONT_HERSHEY_DUPLEX,
            0.7,
            Scalar(0, 255, 255),
            2
        );

        putText(
            frame,
            "Faces Detected: " +
            to_string(faces.size()),
            Point(10, 55),
            FONT_HERSHEY_SIMPLEX,
            0.7,
            Scalar(255, 255, 255),
            2
        );

        putText(
            frame,
            "FPS: " +
            to_string((int)fps),
            Point(10, 85),
            FONT_HERSHEY_SIMPLEX,
            0.7,
            Scalar(0, 255, 0),
            2
        );

        putText(
            frame,
            "Press Q to Exit",
            Point(250, 85),
            FONT_HERSHEY_SIMPLEX,
            0.7,
            Scalar(0, 0, 255),
            2
        );

        // Display Window
        imshow("AI Face Detection", frame);

        char key = (char)waitKey(1);

        if (key == 'q' || key == 'Q')
            break;
    }

    video.release();
    destroyAllWindows();

    return 0;
}
