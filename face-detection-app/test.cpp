#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <map>
#include <ctime>

using namespace cv;
using namespace std;
using namespace chrono;

// ============================================================
// AI FACE DETECTION PRO v3.0
// Advanced Real-Time Face Detection + Tracking HUD
// ============================================================

struct Config {

    // --------------------------------------------------------
    // Camera
    // --------------------------------------------------------
    static constexpr int FRAME_WIDTH = 1280;
    static constexpr int FRAME_HEIGHT = 720;
    static constexpr int TARGET_FPS = 60;

    // --------------------------------------------------------
    // Haar Cascade
    // --------------------------------------------------------
    static constexpr double DETECTION_SCALE = 1.08;
    static constexpr int MIN_NEIGHBORS = 5;
    static constexpr int MIN_FACE_SIZE = 70;

    // --------------------------------------------------------
    // Image Processing
    // --------------------------------------------------------
    static constexpr int GAUSSIAN_KERNEL = 3;

    // --------------------------------------------------------
    // FPS smoothing
    // --------------------------------------------------------
    static constexpr double FPS_ALPHA = 0.10;

    // --------------------------------------------------------
    // Face tracking
    // --------------------------------------------------------
    static constexpr double MAX_TRACK_DISTANCE = 100.0;
    static constexpr int MAX_MISSED_FRAMES = 12;
    static constexpr double SMOOTHING_FACTOR = 0.35;

    // --------------------------------------------------------
    // HUD
    // --------------------------------------------------------
    static constexpr int CORNER_LINE_LENGTH = 24;
    static constexpr int CORNER_LINE_THICKNESS = 3;

    // --------------------------------------------------------
    // Scanner
    // --------------------------------------------------------
    static constexpr int SCANNER_SPEED = 5;

    // --------------------------------------------------------
    // Colors - OpenCV uses BGR
    // --------------------------------------------------------
    static Scalar NEON_GREEN() {
        return Scalar(0, 255, 170);
    }

    static Scalar CYAN() {
        return Scalar(255, 230, 0);
    }

    static Scalar BLUE() {
        return Scalar(255, 100, 0);
    }

    static Scalar RED() {
        return Scalar(0, 0, 255);
    }

    static Scalar WHITE() {
        return Scalar(255, 255, 255);
    }

    static Scalar DARK() {
        return Scalar(15, 15, 15);
    }

    static Scalar YELLOW() {
        return Scalar(0, 220, 255);
    }
};


// ============================================================
// Utility Functions
// ============================================================

string twoDigits(int value) {
    stringstream ss;
    ss << setw(2) << setfill('0') << value;
    return ss.str();
}


string currentTimeString() {

    auto now = system_clock::now();
    time_t current = system_clock::to_time_t(now);

    tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &current);
#else
    localtime_r(&current, &localTime);
#endif

    stringstream ss;

    ss << twoDigits(localTime.tm_hour)
       << ":"
       << twoDigits(localTime.tm_min)
       << ":"
       << twoDigits(localTime.tm_sec);

    return ss.str();
}


string timestampFile() {

    auto now = system_clock::now();
    time_t current = system_clock::to_time_t(now);

    tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &current);
#else
    localtime_r(&current, &localTime);
#endif

    stringstream ss;

    ss << localTime.tm_year + 1900
       << twoDigits(localTime.tm_mon + 1)
       << twoDigits(localTime.tm_mday)
       << "_"
       << twoDigits(localTime.tm_hour)
       << twoDigits(localTime.tm_min)
       << twoDigits(localTime.tm_sec);

    return ss.str();
}


double distanceBetween(Point a, Point b) {

    double dx = static_cast<double>(a.x - b.x);
    double dy = static_cast<double>(a.y - b.y);

    return sqrt(dx * dx + dy * dy);
}


Point rectCenter(const Rect& rect) {

    return Point(
        rect.x + rect.width / 2,
        rect.y + rect.height / 2
    );
}


Rect smoothRect(const Rect& oldRect,
                const Rect& newRect,
                double alpha) {

    int x = static_cast<int>(
        oldRect.x * (1.0 - alpha) +
        newRect.x * alpha
    );

    int y = static_cast<int>(
        oldRect.y * (1.0 - alpha) +
        newRect.y * alpha
    );

    int width = static_cast<int>(
        oldRect.width * (1.0 - alpha) +
        newRect.width * alpha
    );

    int height = static_cast<int>(
        oldRect.height * (1.0 - alpha) +
        newRect.height * alpha
    );

    return Rect(x, y, width, height);
}


// ============================================================
// Glow Effect
// ============================================================

void drawGlowEffect(
    Mat& image,
    const Rect& box,
    const Scalar& color,
    int radius = 3
) {

    for (int i = radius; i >= 1; --i) {

        double factor =
            0.15 + (0.10 * (radius - i));

        Scalar glowColor(
            color[0] * factor,
            color[1] * factor,
            color[2] * factor
        );

        rectangle(
            image,
            Rect(
                box.x - i,
                box.y - i,
                box.width + i * 2,
                box.height + i * 2
            ),
            glowColor,
            1
        );
    }
}


// ============================================================
// Corner Box
// ============================================================

void drawCornerBox(
    Mat& image,
    const Rect& box,
    const Scalar& color,
    int thickness = 3,
    int length = 25
) {

    int x = box.x;
    int y = box.y;
    int w = box.width;
    int h = box.height;

    // Top-left
    line(
        image,
        Point(x, y),
        Point(x + length, y),
        color,
        thickness
    );

    line(
        image,
        Point(x, y),
        Point(x, y + length),
        color,
        thickness
    );

    // Top-right
    line(
        image,
        Point(x + w, y),
        Point(x + w - length, y),
        color,
        thickness
    );

    line(
        image,
        Point(x + w, y),
        Point(x + w, y + length),
        color,
        thickness
    );

    // Bottom-left
    line(
        image,
        Point(x, y + h),
        Point(x + length, y + h),
        color,
        thickness
    );

    line(
        image,
        Point(x, y + h),
        Point(x, y + h - length),
        color,
        thickness
    );

    // Bottom-right
    line(
        image,
        Point(x + w, y + h),
        Point(x + w - length, y + h),
        color,
        thickness
    );

    line(
        image,
        Point(x + w, y + h),
        Point(x + w, y + h - length),
        color,
        thickness
    );
}


// ============================================================
// Horizontal Scanner
// ============================================================

class Scanner {

private:

    int y;
    int direction;

public:

    Scanner()
        : y(0),
          direction(1) {}

    void update(int height) {

        y += Config::SCANNER_SPEED * direction;

        if (y >= height - 1) {
            y = height - 1;
            direction = -1;
        }

        if (y <= 0) {
            y = 0;
            direction = 1;
        }
    }

    void draw(Mat& frame) {

        Scalar color = Config::CYAN();

        line(
            frame,
            Point(0, y),
            Point(frame.cols, y),
            color,
            1,
            LINE_AA
        );

        line(
            frame,
            Point(0, y + 1),
            Point(frame.cols, y + 1),
            Scalar(100, 100, 0),
            1,
            LINE_AA
        );
    }
};


// ============================================================
// FPS Counter
// ============================================================

class FPSCounter {

private:

    steady_clock::time_point lastTime;
    double fps;
    double alpha;

public:

    FPSCounter(double smoothing = Config::FPS_ALPHA)
        : fps(0.0),
          alpha(smoothing) {

        lastTime = steady_clock::now();
    }

    double update() {

        auto currentTime = steady_clock::now();

        duration<double> elapsed =
            currentTime - lastTime;

        if (elapsed.count() <= 0.0) {
            return fps;
        }

        double currentFPS =
            1.0 / elapsed.count();

        if (fps == 0.0) {
            fps = currentFPS;
        }
        else {

            fps =
                fps * (1.0 - alpha) +
                currentFPS * alpha;
        }

        lastTime = currentTime;

        return fps;
    }

    double getFPS() const {
        return fps;
    }
};


// ============================================================
// Face Detection Result
// ============================================================

struct Detection {

    Rect box;

    Point center;

    double score;

    Detection()
        : box(),
          center(),
          score(0.0) {}

    Detection(
        const Rect& r,
        double s
    )
        : box(r),
          center(rectCenter(r)),
          score(s) {}
};


// ============================================================
// Tracked Face
// ============================================================

struct TrackedFace {

    int id;

    Rect box;

    Point center;

    Point previousCenter;

    double score;

    int missedFrames;

    bool isNew;

    bool moving;

    double speed;

    TrackedFace()
        : id(-1),
          box(),
          center(),
          previousCenter(),
          score(0),
          missedFrames(0),
          isNew(false),
          moving(false),
          speed(0) {}
};


// ============================================================
// Face Detector
// ============================================================

class FaceDetector {

private:

    CascadeClassifier cascade;

    Mat gray;
    Mat processed;

    vector<Rect> faces;

    vector<int> rejectLevels;

    vector<double> levelWeights;

public:

    bool load(const string& cascadePath) {

        if (!cascade.load(cascadePath)) {

            cerr
                << "ERROR: Could not load Haar Cascade: "
                << cascadePath
                << endl;

            return false;
        }

        return true;
    }


    vector<Detection> detect(
        const Mat& frame
    ) {

        vector<Detection> results;

        if (frame.empty()) {
            return results;
        }

        // --------------------------------------------
        // Grayscale
        // --------------------------------------------

        cvtColor(
            frame,
            gray,
            COLOR_BGR2GRAY
        );

        // --------------------------------------------
        // Histogram normalization
        // --------------------------------------------

        equalizeHist(
            gray,
            gray
        );

        // --------------------------------------------
        // Light blur
        // --------------------------------------------

        GaussianBlur(
            gray,
            processed,
            Size(
                Config::GAUSSIAN_KERNEL,
                Config::GAUSSIAN_KERNEL
            ),
            0
        );

        faces.clear();
        rejectLevels.clear();
        levelWeights.clear();

        // --------------------------------------------
        // Advanced Haar detection
        // --------------------------------------------

        cascade.detectMultiScale(
            processed,
            faces,
            rejectLevels,
            levelWeights,
            Config::DETECTION_SCALE,
            Config::MIN_NEIGHBORS,
            0,
            Size(
                Config::MIN_FACE_SIZE,
                Config::MIN_FACE_SIZE
            ),
            Size(),
            true
        );

        // --------------------------------------------
        // Convert detections
        // --------------------------------------------

        for (size_t i = 0; i < faces.size(); ++i) {

            double score = 50.0;

            if (i < levelWeights.size()) {

                double weight =
                    levelWeights[i];

                // Haar level weights are NOT
                // calibrated probabilities.
                //
                // We transform them into a
                // readable 0-100 detection score.

                double normalized =
                    1.0 /
                    (
                        1.0 +
                        exp(-weight)
                    );

                score =
                    50.0 +
                    normalized * 49.0;
            }

            score =
                max(1.0, min(99.0, score));

            results.emplace_back(
                faces[i],
                score
            );
        }

        return results;
    }
};


// ============================================================
// Face Tracker
// ============================================================

class FaceTracker {

private:

    vector<TrackedFace> tracks;

    int nextID;

public:

    FaceTracker()
        : nextID(1) {}


    void update(
        const vector<Detection>& detections
    ) {

        // ----------------------------------------------------
        // Reset "new" status
        // ----------------------------------------------------

        for (auto& track : tracks) {
            track.isNew = false;
        }

        vector<bool> detectionUsed(
            detections.size(),
            false
        );

        vector<bool> trackMatched(
            tracks.size(),
            false
        );

        // ----------------------------------------------------
        // Match detections to existing faces
        // ----------------------------------------------------

        for (size_t d = 0;
             d < detections.size();
             ++d) {

            double bestDistance =
                Config::MAX_TRACK_DISTANCE;

            int bestTrack = -1;

            for (size_t t = 0;
                 t < tracks.size();
                 ++t) {

                if (trackMatched[t]) {
                    continue;
                }

                double distance =
                    distanceBetween(
                        detections[d].center,
                        tracks[t].center
                    );

                if (distance < bestDistance) {

                    bestDistance = distance;
                    bestTrack =
                        static_cast<int>(t);
                }
            }

            if (bestTrack >= 0) {

                TrackedFace& track =
                    tracks[bestTrack];

                track.previousCenter =
                    track.center;

                track.center =
                    detections[d].center;

                track.box =
                    smoothRect(
                        track.box,
                        detections[d].box,
                        Config::SMOOTHING_FACTOR
                    );

                track.score =
                    detections[d].score;

                track.missedFrames = 0;

                double movement =
                    distanceBetween(
                        track.previousCenter,
                        track.center
                    );

                track.speed =
                    movement;

                track.moving =
                    movement > 3.0;

                detectionUsed[d] = true;

                trackMatched[bestTrack] =
                    true;
            }
        }

        // ----------------------------------------------------
        // Create new tracks
        // ----------------------------------------------------

        for (size_t d = 0;
             d < detections.size();
             ++d) {

            if (detectionUsed[d]) {
                continue;
            }

            TrackedFace newTrack;

            newTrack.id = nextID++;

            newTrack.box =
                detections[d].box;

            newTrack.center =
                detections[d].center;

            newTrack.previousCenter =
                detections[d].center;

            newTrack.score =
                detections[d].score;

            newTrack.missedFrames = 0;

            newTrack.isNew = true;

            newTrack.moving = false;

            newTrack.speed = 0;

            tracks.push_back(
                newTrack
            );
        }

        // ----------------------------------------------------
        // Increase missed-frame counter
        // ----------------------------------------------------

        for (size_t t = 0;
             t < tracks.size();
             ++t) {

            if (!trackMatched[t]) {

                tracks[t].missedFrames++;

                tracks[t].moving = false;
            }
        }

        // ----------------------------------------------------
        // Remove lost faces
        // ----------------------------------------------------

        tracks.erase(
            remove_if(
                tracks.begin(),
                tracks.end(),
                [](const TrackedFace& face) {

                    return face.missedFrames >
                           Config::MAX_MISSED_FRAMES;
                }
            ),
            tracks.end()
        );
    }


    const vector<TrackedFace>& getTracks() const {

        return tracks;
    }


    int activeCount() const {

        int count = 0;

        for (const auto& track : tracks) {

            if (track.missedFrames == 0) {
                count++;
            }
        }

        return count;
    }
};


// ============================================================
// HUD Renderer
// ============================================================

class HUDRenderer {

private:

    Mat overlay;

public:

    // --------------------------------------------------------
    // Draw translucent rectangle
    // --------------------------------------------------------

    void panel(
        Mat& frame,
        Rect area,
        double opacity = 0.65
    ) {

        area &= Rect(
            0,
            0,
            frame.cols,
            frame.rows
        );

        if (area.width <= 0 ||
            area.height <= 0) {
            return;
        }

        overlay =
            frame(area).clone();

        rectangle(
            overlay,
            Rect(
                0,
                0,
                area.width,
                area.height
            ),
            Scalar(10, 15, 18),
            FILLED
        );

        addWeighted(
            overlay,
            opacity,
            frame(area),
            1.0 - opacity,
            0,
            frame(area)
        );
    }


    // --------------------------------------------------------
    // Crosshair
    // --------------------------------------------------------

    void renderCrosshair(
        Mat& frame
    ) {

        int cx =
            frame.cols / 2;

        int cy =
            frame.rows / 2;

        Scalar color =
            Config::WHITE();

        line(
            frame,
            Point(cx - 30, cy),
            Point(cx - 10, cy),
            color,
            1
        );

        line(
            frame,
            Point(cx + 10, cy),
            Point(cx + 30, cy),
            color,
            1
        );

        line(
            frame,
            Point(cx, cy - 30),
            Point(cx, cy - 10),
            color,
            1
        );

        line(
            frame,
            Point(cx, cy + 10),
            Point(cx, cy + 30),
            color,
            1
        );

        circle(
            frame,
            Point(cx, cy),
            32,
            color,
            1
        );

        // Circular ticks

        for (int angle = 0;
             angle < 360;
             angle += 30) {

            double rad =
                angle * CV_PI / 180.0;

            Point p1(
                cx +
                static_cast<int>(
                    27 * cos(rad)
                ),
                cy +
                static_cast<int>(
                    27 * sin(rad)
                )
            );

            Point p2(
                cx +
                static_cast<int>(
                    34 * cos(rad)
                ),
                cy +
                static_cast<int>(
                    34 * sin(rad)
                )
            );

            line(
                frame,
                p1,
                p2,
                color,
                1
            );
        }
    }


    // --------------------------------------------------------
    // Face rendering
    // --------------------------------------------------------

    void renderFaces(
        Mat& frame,
        const vector<TrackedFace>& tracks,
        bool blurFaces
    ) {

        for (const auto& face : tracks) {

            if (face.missedFrames > 0) {
                continue;
            }

            Rect box = face.box;

            box &= Rect(
                0,
                0,
                frame.cols,
                frame.rows
            );

            if (box.width <= 0 ||
                box.height <= 0) {
                continue;
            }

            Scalar neon =
                Config::NEON_GREEN();

            if (face.moving) {
                neon = Config::CYAN();
            }

            // ------------------------------------------------
            // Privacy blur
            // ------------------------------------------------

            if (blurFaces) {

                Mat faceRegion =
                    frame(box);

                GaussianBlur(
                    faceRegion,
                    faceRegion,
                    Size(31, 31),
                    0
                );
            }

            // ------------------------------------------------
            // Glow
            // ------------------------------------------------

            drawGlowEffect(
                frame,
                box,
                neon,
                3
            );

            // ------------------------------------------------
            // Corner box
            // ------------------------------------------------

            drawCornerBox(
                frame,
                box,
                neon,
                Config::CORNER_LINE_THICKNESS,
                Config::CORNER_LINE_LENGTH
            );

            // ------------------------------------------------
            // Center point
            // ------------------------------------------------

            circle(
                frame,
                face.center,
                4,
                Config::RED(),
                FILLED
            );

            circle(
                frame,
                face.center,
                9,
                Config::RED(),
                1
            );

            // ------------------------------------------------
            // Target label
            // ------------------------------------------------

            string target =
                "TARGET " +
                twoDigits(face.id);

            string status =
                face.moving
                    ? "TRACKING"
                    : "LOCKED";

            string label =
                target + "  " + status;

            int baseline = 0;

            Size textSize =
                getTextSize(
                    label,
                    FONT_HERSHEY_DUPLEX,
                    0.55,
                    1,
                    &baseline
                );

            int labelX =
                max(
                    5,
                    box.x
                );

            int labelY =
                max(
                    textSize.height + 10,
                    box.y
                );

            rectangle(
                frame,
                Point(
                    labelX,
                    labelY -
                    textSize.height -
                    10
                ),
                Point(
                    labelX +
                    textSize.width +
                    14,
                    labelY + 4
                ),
                Scalar(0, 0, 0),
                FILLED
            );

            putText(
                frame,
                label,
                Point(
                    labelX + 7,
                    labelY
                ),
                FONT_HERSHEY_DUPLEX,
                0.55,
                neon,
                1,
                LINE_AA
            );

            // ------------------------------------------------
            // Confidence / detection score
            // ------------------------------------------------

            int barWidth =
                box.width;

            int barHeight = 5;

            int barX =
                box.x;

            int barY =
                box.y +
                box.height +
                10;

            if (barY + barHeight <
                frame.rows) {

                rectangle(
                    frame,
                    Point(
                        barX,
                        barY
                    ),
                    Point(
                        barX +
                        barWidth,
                        barY +
                        barHeight
                    ),
                    Scalar(45, 45, 45),
                    FILLED
                );

                int filled =
                    static_cast<int>(
                        barWidth *
                        face.score /
                        100.0
                    );

                rectangle(
                    frame,
                    Point(
                        barX,
                        barY
                    ),
                    Point(
                        barX + filled,
                        barY +
                        barHeight
                    ),
                    neon,
                    FILLED
                );
            }

            // ------------------------------------------------
            // Score
            // ------------------------------------------------

            string score =
                "SCORE " +
                to_string(
                    static_cast<int>(
                        face.score
                    )
                ) +
                "%";

            putText(
                frame,
                score,
                Point(
                    box.x,
                    min(
                        frame.rows - 8,
                        box.y +
                        box.height +
                        30
                    )
                ),
                FONT_HERSHEY_SIMPLEX,
                0.48,
                Config::WHITE(),
                1,
                LINE_AA
            );

            // ------------------------------------------------
            // Coordinates
            // ------------------------------------------------

            string coordinates =
                "X:" +
                to_string(face.center.x) +
                " Y:" +
                to_string(face.center.y);

            putText(
                frame,
                coordinates,
                Point(
                    box.x,
                    min(
                        frame.rows - 8,
                        box.y +
                        box.height +
                        48
                    )
                ),
                FONT_HERSHEY_SIMPLEX,
                0.42,
                Scalar(180, 220, 220),
                1,
                LINE_AA
            );
        }
    }


    // --------------------------------------------------------
    // Top dashboard
    // --------------------------------------------------------

    void renderDashboard(
        Mat& frame,
        double fps,
        int faces,
        int totalDetected,
        int maxFaces,
        bool recording,
        bool blurFaces
    ) {

        panel(
            frame,
            Rect(
                15,
                15,
                390,
                190
            ),
            0.78
        );

        // Title

        putText(
            frame,
            "AI FACE DETECTION",
            Point(32, 45),
            FONT_HERSHEY_DUPLEX,
            0.85,
            Config::YELLOW(),
            2,
            LINE_AA
        );

        putText(
            frame,
            "PRO v3.0",
            Point(32, 70),
            FONT_HERSHEY_SIMPLEX,
            0.52,
            Config::CYAN(),
            1,
            LINE_AA
        );

        // Divider

        line(
            frame,
            Point(30, 82),
            Point(390, 82),
            Scalar(70, 100, 100),
            1
        );

        // Faces

        putText(
            frame,
            "ACTIVE TARGETS : " +
            to_string(faces),
            Point(32, 108),
            FONT_HERSHEY_SIMPLEX,
            0.55,
            Config::WHITE(),
            1,
            LINE_AA
        );

        // Total

        putText(
            frame,
            "TOTAL DETECTED : " +
            to_string(totalDetected),
            Point(32, 132),
            FONT_HERSHEY_SIMPLEX,
            0.55,
            Config::WHITE(),
            1,
            LINE_AA
        );

        // Maximum

        putText(
            frame,
            "MAX TARGETS : " +
            to_string(maxFaces),
            Point(32, 156),
            FONT_HERSHEY_SIMPLEX,
            0.55,
            Config::WHITE(),
            1,
            LINE_AA
        );

        // FPS

        string fpsText =
            "FPS : " +
            to_string(
                static_cast<int>(fps)
            );

        Scalar fpsColor =
            fps >= 30
                ? Config::NEON_GREEN()
                : Config::RED();

        putText(
            frame,
            fpsText,
            Point(230, 108),
            FONT_HERSHEY_SIMPLEX,
            0.55,
            fpsColor,
            2,
            LINE_AA
        );

        // Privacy

        putText(
            frame,
            blurFaces
                ? "PRIVACY : ON"
                : "PRIVACY : OFF",
            Point(230, 132),
            FONT_HERSHEY_SIMPLEX,
            0.48,
            blurFaces
                ? Config::YELLOW()
                : Scalar(160, 160, 160),
            1,
            LINE_AA
        );

        // Recording

        putText(
            frame,
            recording
                ? "REC : ACTIVE"
                : "REC : OFF",
            Point(230, 156),
            FONT_HERSHEY_SIMPLEX,
            0.48,
            recording
                ? Config::RED()
                : Scalar(160, 160, 160),
            recording ? 2 : 1,
            LINE_AA
        );

        // System status

        putText(
            frame,
            "● SYSTEM ONLINE",
            Point(32, 185),
            FONT_HERSHEY_SIMPLEX,
            0.45,
            Config::NEON_GREEN(),
            1,
            LINE_AA
        );
    }


    // --------------------------------------------------------
    // Right-side system information
    // --------------------------------------------------------

    void renderSystemPanel(
        Mat& frame,
        bool cameraOK
    ) {

        int width = 240;

        int x =
            frame.cols -
            width -
            15;

        panel(
            frame,
            Rect(
                x,
                15,
                width,
                150
            ),
            0.72
        );

        putText(
            frame,
            "SYSTEM MONITOR",
            Point(
                x + 15,
                42
            ),
            FONT_HERSHEY_DUPLEX,
            0.52,
            Config::CYAN(),
            1,
            LINE_AA
        );

        line(
            frame,
            Point(
                x + 12,
                52
            ),
            Point(
                x + width - 12,
                52
            ),
            Scalar(60, 80, 90),
            1
        );

        putText(
            frame,
            "CAMERA     " +
            string(
                cameraOK
                    ? "ONLINE"
                    : "ERROR"
            ),
            Point(
                x + 15,
                78
            ),
            FONT_HERSHEY_SIMPLEX,
            0.45,
            cameraOK
                ? Config::NEON_GREEN()
                : Config::RED(),
            1,
            LINE_AA
        );

        putText(
            frame,
            "RESOLUTION  " +
            to_string(frame.cols) +
            "x" +
            to_string(frame.rows),
            Point(
                x + 15,
                100
            ),
            FONT_HERSHEY_SIMPLEX,
            0.42,
            Config::WHITE(),
            1,
            LINE_AA
        );

        putText(
            frame,
            "TIME        " +
            currentTimeString(),
            Point(
                x + 15,
                122
            ),
            FONT_HERSHEY_SIMPLEX,
            0.42,
            Config::WHITE(),
            1,
            LINE_AA
        );

        putText(
            frame,
            "ENGINE      HAAR",
            Point(
                x + 15,
                144
            ),
            FONT_HERSHEY_SIMPLEX,
            0.42,
            Config::WHITE(),
            1,
            LINE_AA
        );
    }


    // --------------------------------------------------------
    // Bottom status bar
    // --------------------------------------------------------

    void renderStatusBar(
        Mat& frame,
        bool recording
    ) {

        int height = 40;

        rectangle(
            frame,
            Point(
                0,
                frame.rows - height
            ),
            Point(
                frame.cols,
                frame.rows
            ),
            Scalar(12, 16, 18),
            FILLED
        );

        line(
            frame,
            Point(
                0,
                frame.rows - height
            ),
            Point(
                frame.cols,
                frame.rows - height
            ),
            Scalar(50, 100, 100),
            1
        );

        putText(
            frame,
            "SYSTEM STATUS : ACTIVE",
            Point(
                20,
                frame.rows - 14
            ),
            FONT_HERSHEY_SIMPLEX,
            0.50,
            Config::NEON_GREEN(),
            1,
            LINE_AA
        );

        putText(
            frame,
            "Q/ESC EXIT",
            Point(
                280,
                frame.rows - 14
            ),
            FONT_HERSHEY_SIMPLEX,
            0.43,
            Scalar(170, 170, 170),
            1,
            LINE_AA
        );

        putText(
            frame,
            "S SCREENSHOT",
            Point(
                390,
                frame.rows - 14
            ),
            FONT_HERSHEY_SIMPLEX,
            0.43,
            Scalar(170, 170, 170),
            1,
            LINE_AA
        );

        putText(
            frame,
            "R RECORD",
            Point(
                540,
                frame.rows - 14
            ),
            FONT_HERSHEY_SIMPLEX,
            0.43,
            recording
                ? Config::RED()
                : Scalar(170, 170, 170),
            1,
            LINE_AA
        );

        putText(
            frame,
            "B PRIVACY",
            Point(
                650,
                frame.rows - 14
            ),
            FONT_HERSHEY_SIMPLEX,
            0.43,
            Scalar(170, 170, 170),
            1,
            LINE_AA
        );

        putText(
            frame,
            "F FULLSCREEN",
            Point(
                760,
                frame.rows - 14
            ),
            FONT_HERSHEY_SIMPLEX,
            0.43,
            Scalar(170, 170, 170),
            1,
            LINE_AA
        );

        putText(
            frame,
            currentTimeString(),
            Point(
                frame.cols - 100,
                frame.rows - 14
            ),
            FONT_HERSHEY_SIMPLEX,
            0.43,
            Config::WHITE(),
            1,
            LINE_AA
        );
    }


    // --------------------------------------------------------
    // New face alert
    // --------------------------------------------------------

    void renderNewFaceAlert(
        Mat& frame,
        const vector<TrackedFace>& tracks
    ) {

        for (const auto& face : tracks) {

            if (!face.isNew) {
                continue;
            }

            string alert =
                "NEW TARGET DETECTED";

            Size size =
                getTextSize(
                    alert,
                    FONT_HERSHEY_DUPLEX,
                    0.7,
                    2,
                    nullptr
                );

            int x =
                (frame.cols -
                 size.width) / 2;

            rectangle(
                frame,
                Point(
                    x - 15,
                    75
                ),
                Point(
                    x +
                    size.width +
                    15,
                    110
                ),
                Scalar(0, 0, 0),
                FILLED
            );

            putText(
                frame,
                alert,
                Point(
                    x,
                    100
                ),
                FONT_HERSHEY_DUPLEX,
                0.7,
                Config::YELLOW(),
                2,
                LINE_AA
            );
        }
    }
};


// ============================================================
// Main
// ============================================================

int main() {

    try {

        cout
            << "============================================"
            << endl;

        cout
            << "      AI FACE DETECTION PRO v3.0"
            << endl;

        cout
            << "============================================"
            << endl;

        cout
            << "Advanced Face Detection + Tracking System"
            << endl;

        cout
            << endl;

        cout
            << "CONTROLS:"
            << endl;

        cout
            << "  Q / ESC  - Exit"
            << endl;

        cout
            << "  S        - Screenshot"
            << endl;

        cout
            << "  R        - Start/Stop Recording"
            << endl;

        cout
            << "  B        - Privacy Blur"
            << endl;

        cout
            << "  F        - Fullscreen"
            << endl;

        cout
            << "  SPACE    - Reset Statistics"
            << endl;

        cout
            << "============================================"
            << endl;


        // ====================================================
        // CAMERA
        // ====================================================

        VideoCapture camera(0);

        if (!camera.isOpened()) {

            cerr
                << "ERROR: Cannot open camera."
                << endl;

            return -1;
        }

        camera.set(
            CAP_PROP_FRAME_WIDTH,
            Config::FRAME_WIDTH
        );

        camera.set(
            CAP_PROP_FRAME_HEIGHT,
            Config::FRAME_HEIGHT
        );

        camera.set(
            CAP_PROP_FPS,
            Config::TARGET_FPS
        );

        camera.set(
            CAP_PROP_AUTOFOCUS,
            1
        );


        // ====================================================
        // CASCADE
        // ====================================================

        FaceDetector detector;

        string cascadePath;

        try {

            cascadePath =
                samples::findFile(
                    "haarcascade_frontalface_default.xml"
                );

        }
        catch (...) {

            cerr
                << endl
                << "ERROR: Haar Cascade file was not found."
                << endl;

            cerr
                << "Make sure OpenCV haarcascades are installed."
                << endl;

            camera.release();

            return -1;
        }


        if (!detector.load(cascadePath)) {

            camera.release();

            return -1;
        }


        // ====================================================
        // SYSTEM COMPONENTS
        // ====================================================

        FPSCounter fpsCounter;

        FaceTracker tracker;

        HUDRenderer hud;

        Scanner scanner;


        // ====================================================
        // WINDOW
        // ====================================================

        const string WINDOW_NAME =
            "AI Face Detection Pro v3.0";

        namedWindow(
            WINDOW_NAME,
            WINDOW_NORMAL
        );

        resizeWindow(
            WINDOW_NAME,
            Config::FRAME_WIDTH,
            Config::FRAME_HEIGHT
        );


        // ====================================================
        // VARIABLES
        // ====================================================

        Mat frame;

        bool fullscreen = false;

        bool recording = false;

        bool privacyBlur = false;

        VideoWriter videoWriter;

        int totalDetected = 0;

        int maxFaces = 0;

        int screenshotCount = 0;

        int previousFaceCount = 0;


        // ====================================================
        // MAIN LOOP
        // ====================================================

        while (true) {

            // ------------------------------------------------
            // Capture
            // ------------------------------------------------

            if (!camera.read(frame)) {

                cerr
                    << "WARNING: Failed to capture frame."
                    << endl;

                break;
            }

            if (frame.empty()) {
                continue;
            }


            // ------------------------------------------------
            // Mirror
            // ------------------------------------------------

            flip(
                frame,
                frame,
                1
            );


            // ------------------------------------------------
            // FPS
            // ------------------------------------------------

            double fps =
                fpsCounter.update();


            // ------------------------------------------------
            // Face detection
            // ------------------------------------------------

            vector<Detection> detections =
                detector.detect(frame);


            // ------------------------------------------------
            // Update tracker
            // ------------------------------------------------

            tracker.update(
                detections
            );


            const vector<TrackedFace>& tracks =
                tracker.getTracks();


            // ------------------------------------------------
            // Statistics
            // ------------------------------------------------

            int activeFaces =
                tracker.activeCount();

            if (activeFaces > previousFaceCount) {

                totalDetected +=
                    activeFaces -
                    previousFaceCount;
            }

            previousFaceCount =
                activeFaces;

            maxFaces =
                max(
                    maxFaces,
                    activeFaces
                );


            // ------------------------------------------------
            // Scanner
            // ------------------------------------------------

            scanner.update(
                frame.rows
            );


            // ------------------------------------------------
            // HUD background
            // ------------------------------------------------

            hud.renderDashboard(
                frame,
                fps,
                activeFaces,
                totalDetected,
                maxFaces,
                recording,
                privacyBlur
            );


            hud.renderSystemPanel(
                frame,
                true
            );


            // ------------------------------------------------
            // Face targets
            // ------------------------------------------------

            hud.renderFaces(
                frame,
                tracks,
                privacyBlur
            );


            // ------------------------------------------------
            // Scanner
            // ------------------------------------------------

            scanner.draw(
                frame
            );


            // ------------------------------------------------
            // Crosshair
            // ------------------------------------------------

            hud.renderCrosshair(
                frame
            );


            // ------------------------------------------------
            // New face alert
            // ------------------------------------------------

            hud.renderNewFaceAlert(
                frame,
                tracks
            );


            // ------------------------------------------------
            // Recording indicator
            // ------------------------------------------------

            if (recording) {

                circle(
                    frame,
                    Point(
                        frame.cols - 35,
                        35
                    ),
                    7,
                    Config::RED(),
                    FILLED
                );

                putText(
                    frame,
                    "REC",
                    Point(
                        frame.cols - 80,
                        42
                    ),
                    FONT_HERSHEY_SIMPLEX,
                    0.45,
                    Config::RED(),
                    2,
                    LINE_AA
                );
            }


            // ------------------------------------------------
            // Bottom bar
            // ------------------------------------------------

            hud.renderStatusBar(
                frame,
                recording
            );


            // =================================================
            // RECORDING
            // =================================================

            if (recording) {

                if (!videoWriter.isOpened()) {

                    string filename =
                        "face_detection_" +
                        timestampFile() +
                        ".avi";

                    int codec =
                        VideoWriter::fourcc(
                            'M',
                            'J',
                            'P',
                            'G'
                        );

                    videoWriter.open(
                        filename,
                        codec,
                        30.0,
                        frame.size()
                    );

                    if (!videoWriter.isOpened()) {

                        cerr
                            << "ERROR: Could not start recording."
                            << endl;

                        recording = false;
                    }
                    else {

                        cout
                            << "Recording started: "
                            << filename
                            << endl;
                    }
                }

                if (videoWriter.isOpened()) {

                    videoWriter.write(
                        frame
                    );
                }
            }


            // =================================================
            // DISPLAY
            // =================================================

            imshow(
                WINDOW_NAME,
                frame
            );


            // =================================================
            // KEYBOARD
            // =================================================

            char key =
                static_cast<char>(
                    waitKey(1)
                );


            // ------------------------------------------------
            // EXIT
            // ------------------------------------------------

            if (
                key == 'q' ||
                key == 'Q' ||
                key == 27
            ) {

                cout
                    << "Exiting..."
                    << endl;

                break;
            }


            // ------------------------------------------------
            // SCREENSHOT
            // ------------------------------------------------

            if (
                key == 's' ||
                key == 'S'
            ) {

                string filename =
                    "screenshot_" +
                    timestampFile() +
                    ".jpg";

                if (
                    imwrite(
                        filename,
                        frame
                    )
                ) {

                    screenshotCount++;

                    cout
                        << "Screenshot saved: "
                        << filename
                        << endl;

                    cout
                        << "Total screenshots: "
                        << screenshotCount
                        << endl;
                }
            }


            // ------------------------------------------------
            // RECORDING
            // ------------------------------------------------

            else if (
                key == 'r' ||
                key == 'R'
            ) {

                recording =
                    !recording;

                if (!recording) {

                    if (videoWriter.isOpened()) {

                        videoWriter.release();

                        cout
                            << "Recording stopped."
                            << endl;
                    }
                }
                else {

                    cout
                        << "Recording enabled."
                        << endl;
                }
            }


            // ------------------------------------------------
            // PRIVACY BLUR
            // ------------------------------------------------

            else if (
                key == 'b' ||
                key == 'B'
            ) {

                privacyBlur =
                    !privacyBlur;

                cout
                    << "Privacy blur: "
                    << (
                        privacyBlur
                            ? "ON"
                            : "OFF"
                    )
                    << endl;
            }


            // ------------------------------------------------
            // FULLSCREEN
            // ------------------------------------------------

            else if (
                key == 'f' ||
                key == 'F'
            ) {

                fullscreen =
                    !fullscreen;

                if (fullscreen) {

                    setWindowProperty(
                        WINDOW_NAME,
                        WND_PROP_FULLSCREEN,
                        WINDOW_FULLSCREEN
                    );
                }
                else {

                    setWindowProperty(
                        WINDOW_NAME,
                        WND_PROP_FULLSCREEN,
                        WINDOW_NORMAL
                    );

                    resizeWindow(
                        WINDOW_NAME,
                        Config::FRAME_WIDTH,
                        Config::FRAME_HEIGHT
                    );
                }
            }


            // ------------------------------------------------
            // RESET STATISTICS
            // ------------------------------------------------

            else if (
                key == ' '
            ) {

                totalDetected = 0;

                maxFaces = 0;

                screenshotCount = 0;

                previousFaceCount =
                    activeFaces;

                cout
                    << "Statistics reset."
                    << endl;
            }
        }


        // ====================================================
        // CLEANUP
        // ====================================================

        if (videoWriter.isOpened()) {

            videoWriter.release();

            cout
                << "Recording file closed."
                << endl;
        }

        camera.release();

        destroyAllWindows();


        // ====================================================
        // FINAL REPORT
        // ====================================================

        cout
            << endl
            << "============================================"
            << endl;

        cout
            << "        SESSION SUMMARY"
            << endl;

        cout
            << "============================================"
            << endl;

        cout
            << "Maximum simultaneous faces : "
            << maxFaces
            << endl;

        cout
            << "Total detected events      : "
            << totalDetected
            << endl;

        cout
            << "Screenshots captured      : "
            << screenshotCount
            << endl;

        cout
            << "============================================"
            << endl;

        cout
            << "Program terminated successfully."
            << endl;


        return 0;
    }


    // ========================================================
    // EXCEPTION HANDLING
    // ========================================================

    catch (const exception& e) {

        cerr
            << endl
            << "FATAL ERROR: "
            << e.what()
            << endl;

        return -1;
    }
}
