#include<o

	CascadeClassifier 
		video.read(img);
		
		for (int i = 0; i < faces.size(); i++) {
			rectangle(img, faces[i].tl(), faces[i].br(), Scalar(50, 50, 255), 3);
			rectangle(img, Point(0,0), Point(250,70), Scalar(50, 50, 255), FILLED);
			putText(img, to_string(faces.size())+" Face Found", Point(10, 40), FONT_HERSHEY_DUPLEX, 1,

		
		imshow("Frame", img);
		waitKey(1);
		

	}
}
