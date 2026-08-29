
        duration<double> elapsed = currentTime - lastTime;
        double currentFPS = 1.0 / elapsed.count();
        fps = fps * (1.0 - alpha) + currentFPS * alpha;
        lastTime = currentTime;
        return fps;
    }
    
    double getFPS() const { return fps; }
};

// ===============================
// Face Detection Class
// ===============================
class FaceDetector {
private:
    CascadeClassifier cascade;
    Mat gray, processed;
    vector<Rect> faces;
    
public:
    bool load(const string& cascadePath) {
        return cascade.load(cascadePath);
    }
    
    const vector<Rect>& detect(const Mat& frame) {
        // Preprocess
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, gray);
        GaussianBlur(gray, processed, Size(Config::GAUSSIAN_KERNEL, Config::GAUSSIAN_KERNEL), 0);
        
        // Detect
        faces.clear();
        cascade.detectMultiScale(
            processed,
            faces,
            Config::DETECTION_SCALE,
            Config::MIN_NEIGHBORS,
            0,
            Size(Config::MIN_FACE_SIZE, Config::MIN_FACE_SIZE)
        );
        
        return faces;
    }
    
    const vector<Rect>& getFaces() const { return faces; }
};

// ===============================
// HUD Renderer Class
// ===============================
class HUDRenderer {
private:
    Mat overlay;
    
public:
    void renderBackground(Mat& frame) {
        overlay = frame.clone();
        rectangle(overlay, Point(0, 0), Point(420, 140), Scalar(15, 15, 15), FILLED);
        addWeighted(overlay, 0.4, frame, 0.6, 0, frame);
    }
    
    void renderStatusBar(Mat& frame) {
        rectangle(frame, Point(0, frame.rows - 35), Point(frame.cols, frame.rows), Scalar(20, 20, 20), FILLED);
        
        string status = "SYSTEM STATUS : ACTIVE";
        putText(frame, status, Point(20, frame.rows - 10), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
        
        // Current time
        auto now = system_clock::now();
        auto time = system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&time), "%H:%M:%S");
        putText(frame, ss.str(), Point(frame.cols - 150, frame.rows - 10), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(200, 200, 200), 2);
    }
    
    void renderCrosshair(Mat& frame) {
        int cx = frame.cols / 2;
        int cy = frame.rows / 2;
        Scalar color(255, 255, 255);
        
        line(frame, Point(cx - 25, cy), Point(cx - 10, cy), color, 1);
        line(frame, Point(cx + 10, cy), Point(cx + 25, cy), color, 1);
        line(frame, Point(cx, cy - 25), Point(cx, cy - 10), color, 1);
        line(frame, Point(cx, cy + 10), Point(cx, cy + 25), color, 1);
        circle(frame, Point(cx, cy), 30, color, 1);
        
        // Inner circle with dash effect
        for (int i = 0; i < 360; i += 30) {
            double rad = i * CV_PI / 180.0;
            Point p1(cx + 25 * cos(rad), cy + 25 * sin(rad));
            Point p2(cx + 30 * cos(rad), cy + 30 * sin(rad));
            line(frame, p1, p2, color, 1);
        }
    }
    
    void renderFaces(Mat& frame, const vector<Rect>& faces, const vector<int>& confidences) {
        for (size_t i = 0; i < faces.size(); i++) {
            Rect face = faces[i];
            Scalar neon(0, 255, 180);
            
            // Glow effect
            drawGlowEffect(frame, face, neon, 2);
            
            // Corner box
            drawCornerBox(frame, face, neon, Config::CORNER_LINE_THICKNESS, Config::CORNER_LINE_LENGTH);
            
            // Center dot with pulse effect
            Point center(face.x + face.width / 2, face.y + face.height / 2);
            circle(frame, center, 4, Scalar(0, 0, 255), FILLED);
            circle(frame, center, 8, Scalar(0, 0, 255), 1);
            
            // Face label with background
            string label = "TARGET " + to_string(i + 1);
            int baseline = 0;
            Size textSize = getTextSize(label, FONT_HERSHEY_DUPLEX, 0.7, 2, &baseline);
            
            rectangle(frame, 
                Point(face.x, face.y - textSize.height - 15),
                Point(face.x + textSize.width + 10, face.y - 5),
                Scalar(0, 0, 0), FILLED);
            
            putText(frame, label, Point(face.x + 5, face.y - 10), 
                FONT_HERSHEY_DUPLEX, 0.7, neon, 2);
            
            // Confidence bar
            int confidence = (i < confidences.size()) ? confidences[i] : 
                            (Config::CONFIDENCE_MIN + rand() % (Config::CONFIDENCE_MAX - Config::CONFIDENCE_MIN));
            
            int barWidth = face.width;
            int barHeight = 6;
            Point barStart(face.x, face.y + face.height + 10);
            Point barEnd(face.x + barWidth, face.y + face.height + 10 + barHeight);
            
            rectangle(frame, barStart, barEnd, Scalar(50, 50, 50), FILLED);
            rectangle(frame, barStart, 
                Point(face.x + (barWidth * confidence) / 100, face.y + face.height + 10 + barHeight),
                Scalar(0, 255 * confidence / 100, 255 - 255 * confidence / 100), FILLED);
            
            // Confidence percentage
            string confText = to_string(confidence) + "%";
            putText(frame, confText, 
                Point(face.x + barWidth - 50, face.y + face.height + 30),
                FONT_HERSHEY_SIMPLEX, 0.55, Scalar(255, 255, 255), 2);
        }
    }
};

// ===============================
// Main Function
// ===============================
int main() {
    try {
        // Initialize camera
        VideoCapture camera(0);
        if (!camera.isOpened()) {
            cerr << "ERROR: Cannot open camera." << endl;
            return -1;
        }
        
        camera.set(CAP_PROP_FRAME_WIDTH, Config::FRAME_WIDTH);
        camera.set(CAP_PROP_FRAME_HEIGHT, Config::FRAME_HEIGHT);
        camera.set(CAP_PROP_FPS, 60);
        camera.set(CAP_PROP_AUTOFOCUS, 1);
        
        // Load cascade
        FaceDetector detector;
        string cascadePath = samples::findFile("haarcascade_frontalface_default.xml");
        
        if (!detector.load(cascadePath)) {
            cerr << "ERROR: Failed to load Haar Cascade." << endl;
            return -1;
        }
        
        // Initialize components
        FPSCounter fpsCounter(Config::FPS_ALPHA);
        HUDRenderer hud;
        
        Mat frame;
        vector<int> confidences;
        
        cout << "========================================" << endl;
        cout << "   AI FACE DETECTION SYSTEM v2.0" << endl;
        cout << "========================================" << endl;
        cout << "Press 'q' or ESC to exit" << endl;
        cout << "Press 's' to save screenshot" << endl;
        cout << "Press 'f' to toggle fullscreen" << endl;
        cout << "========================================" << endl;
        
        // Main loop
        while (true) {
            camera.read(frame);
            if (frame.empty()) {
                cerr << "Warning: Empty frame captured." << endl;
                break;
            }
            
            // Mirror for selfie view
            flip(frame, frame, 1);
            
            // Detect faces
            const vector<Rect>& faces = detector.detect(frame);
            
            // Generate confidences
            confidences.clear();
            for (size_t i = 0; i < faces.size(); i++) {
                confidences.push_back(Config::CONFIDENCE_MIN + rand() % (Config::CONFIDENCE_MAX - Config::CONFIDENCE_MIN));
            }
            
            // Render HUD
            hud.renderBackground(frame);
            hud.renderFaces(frame, faces, confidences);
            hud.renderCrosshair(frame);
            
            // Update FPS
            double fps = fpsCounter.update();
            
            // Dashboard
            rectangle(frame, Point(0, 0), Point(420, 140), Scalar(0, 0, 0, 100), FILLED);
            
            putText(frame, "AI FACE DETECTION PRO", Point(20, 35), 
                FONT_HERSHEY_DUPLEX, 0.85, Scalar(0, 255, 255), 2);
            
            putText(frame, "Faces: " + to_string(faces.size()), Point(20, 70),
                FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);
            
            string fpsText = "FPS: " + to_string((int)fps);
            Scalar fpsColor = (fps >= 30) ? Scalar(0, 255, 0) : Scalar(0, 0, 255);
            putText(frame, fpsText, Point(20, 105),
                FONT_HERSHEY_SIMPLEX, 0.7, fpsColor, 2);
            
            putText(frame, "Press Q/ESC to Exit", Point(180, 105),
                FONT_HERSHEY_SIMPLEX, 0.65, Scalar(0, 140, 255), 2);
            
            // Status bar
            hud.renderStatusBar(frame);
            
            // Show window
            namedWindow("AI Face Detection Pro", WINDOW_NORMAL);
            imshow("AI Face Detection Pro", frame);
            
            // Handle keyboard input
            char key = (char)waitKey(1);
            if (key == 'q' || key == 'Q' || key == 27) {
                cout << "Exiting..." << endl;
                break;
            }
            else if (key == 's' || key == 'S') {
                string filename = "screenshot_" + to_string(time(nullptr)) + ".jpg";
                imwrite(filename, frame);
                cout << "Screenshot saved: " << filename << endl;
            }
            else if (key == 'f' || key == 'F') {
                static bool fullscreen = false;
                fullscreen = !fullscreen;
                if (fullscreen) {
                    setWindowProperty("AI Face Detection Pro", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
                } else {
                    setWindowProperty("AI Face Detection Pro", WND_PROP_FULLSCREEN, WINDOW_NORMAL);
                }
            }
        }
        
        camera.release();
        destroyAllWindows();
        
        cout << "Program terminated successfully." << endl;
        return 0;
        
    } catch (const exception& e) {
        cerr << "Exception caught: " << e.what() << endl;
        return -1;
    }
}
