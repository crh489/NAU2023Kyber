#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "crypt.c";
#include <filesystem>
#include <fileapi.h>
#include <cuda.h>
#include <chrono>
#include "./KyberCUDA/kybercuda.cu"
#pragma warning(disable : 4996)
using namespace cv;
using namespace std;
using namespace chrono;
namespace fs = std::filesystem;
// constant declarations
const string CASCADE_CLASSIFIER = "Resources/haarcascade_frontalface_default.xml";
// function declarations
string getInputFile(int);
int getKyber();
int getMode();
int getInputMode();
int getNumberThreads();
string generateOutputFolder(string, int, int, int, int);
int getFaces(Mat, int, int, int, int, int, string, FILE*);
int kybercuda(int kyber_implementation, char* input_file_path, char* encryption_file_path, char* decryption_file_path, int mode, int numThreads);
int crypt(int kyber_implementation, char* input_file_path, char* encryption_file_path, char* decryption_file_path, int mode, int numThreads, long double* difference_enc, long double* difference_dec);
inline bool exists_test(const string& name);
const char* get_filename_ext(const char* filename);

// Main
int main(int argc, char* argv[])
{
	// var declarations
	int kyberImplementation = 0;
	int mode = 0;
	int inputMode = 0;
	int numThreads = 1;
	int numFaces = 0;
	FILE* output_CSV;
	string path;
	string outputFolder, output_CSV_filename;
	Mat frame, img;

	// Step 1: get input file from user
	if (argc == 6)								//input from command line arguments:
	{
		kyberImplementation = atoi(argv[1]);
		mode = atoi(argv[2]);
		inputMode = atoi(argv[3]);
		numThreads = atoi(argv[4]);
		path = argv[5];
		path = "Input/" + path;
	}
	else
	{											// interactive input
		printf("\nNo valid command line arguments.\n");
		kyberImplementation = getKyber();
		mode = getMode();
		inputMode = getInputMode();
		numThreads = 1;
		numFaces = 0;
		path = getInputFile(inputMode);
		if (mode == 2) {
			numThreads = getNumberThreads();
		}
	}



	if (inputMode == 1) {

		img = imread(path);
		outputFolder = generateOutputFolder(path, kyberImplementation, mode, numThreads, inputMode);
		output_CSV_filename = "./Output/" + outputFolder + "/Results.csv";
		output_CSV = fopen(output_CSV_filename.c_str(), "a+");
		fprintf(output_CSV, "Face #,Encryption Time (ms),Decryption Time (ms)\n");
		numFaces = getFaces(img, kyberImplementation, mode, numThreads, inputMode, numFaces, outputFolder, output_CSV);
	}
	else if (inputMode == 2) {
		int numFrames = 0;
		int frameCount = 0;
		outputFolder = generateOutputFolder(path, kyberImplementation, mode, numThreads, inputMode);
		output_CSV_filename = "./Output/" + outputFolder + "/Results.csv";
		output_CSV = fopen(output_CSV_filename.c_str(), "a+");
		fprintf(output_CSV, "Face #,Encryption Time (ms),Decryption Time (ms)\n");
		VideoCapture capture;
		capture.open(path);
		numFrames = capture.get(CAP_PROP_FRAME_COUNT);
		if (capture.isOpened() == 0)
		{
			cout << "The video file cannot be opened." << endl;
			return -1;
		}
		for (frameCount = 0; frameCount < numFrames - 1; frameCount++) {
			if (!capture.read(frame)) {
				cout << "\n Cannot read the video file. \n";
				break;
			}
			if (frame.rows > 0) {
				numFaces += getFaces(frame, kyberImplementation, mode, numThreads, inputMode, numFaces, outputFolder, output_CSV);
			}
		}
		capture.release();
		fclose(output_CSV);
	}
	else if(inputMode == 3){
		path = "";
		outputFolder = generateOutputFolder(path, kyberImplementation, mode, numThreads, inputMode);
		output_CSV_filename = "./Output/" + outputFolder + "/Results.csv";
		output_CSV = fopen(output_CSV_filename.c_str(), "a+");
		fprintf(output_CSV, "Face #,Encryption Time (ms),Decryption Time (ms)\n");
		VideoCapture capture(0);
		if (!capture.isOpened()) {
			cout << "\nNo video stream detected\n" << endl;
			system("pause");
			return -1;
		}
		namedWindow("Video Player");
		while (true) { //Taking an everlasting loop to show the video//
			capture >> img;
			if (img.empty()) { //Breaking the loop if no video frame is detected//
				break;
			}
			imshow("Video Player", img);//Showing the video//
			numFaces += getFaces(img, kyberImplementation, mode, numThreads, inputMode, numFaces, outputFolder, output_CSV);
			char c = (char)waitKey(25);//Allowing 25 milliseconds frame processing time and initiating break condition//
			if (c == 27) { //If 'Esc' is entered break the loop//
				break;
			}
		}
		capture.release();//Releasing the buffer memory//
		fclose(output_CSV);
	}
	return 0;
}

string generateOutputFolder(string fileName, int kyberImplementation, int mode, int numThreads, int inputMode) {
	int condition = 0;
	string kyber;
	string runtimeMode;
	string outputFolder;
	string inputType;
	string path1, path2, path3, path4;
	system_clock::time_point now = std::chrono::system_clock::now();
	time_t tt = system_clock::to_time_t(now);
	tm utc_tm = *gmtime(&tt);
	tm local_tm = *localtime(&tt);

	switch (kyberImplementation) {
	case 1: {
		kyber = "512";
		break;
	}
	case 2: {
		kyber = "768";
		break;
	}
	case 3: {
		kyber = "1024";
		break;
	}
	}

	switch (mode) {
	case 1: {
		runtimeMode = "Single-Threaded";
		break;
	}
	case 2: {
		runtimeMode = "Multi-Threaded (CPU)";
		break;
	}
	case 3: {
		runtimeMode = "Multi-Threaded (GPU)";
		break;
	}
	}

	switch (inputMode) {
	case 1: {
		inputType = "Image";
		break;
	}
	case 2: {
		inputType = "Video";
		break;
	}
	case 3: {
		inputType = "Webcam";
		break;
	}
	}

	if (inputMode == 1 || inputMode == 2) {
		outputFolder = fileName.substr(6, fileName.length()) + "_" + inputType + "_Kyber" + kyber + "_" + runtimeMode + " (" + to_string(numThreads) + " threads)";
	}
	else {
		outputFolder = "Webcam" + inputType + "_Kyber" + kyber + "_" + runtimeMode + " (" + to_string(numThreads) + " threads)";
	}
	path1 = "./Output/" + outputFolder;
	path2 = "./Output/" + outputFolder + "/Faces";
	path3 = "./Output/" + outputFolder + "/Encryption";
	path4 = "./Output/" + outputFolder + "/Decryption";
	fs::create_directories(path1);
	fs::create_directories(path2);
	fs::create_directories(path3);
	fs::create_directories(path4);
	return outputFolder;
}
int getNumberThreads() {
	int condition = 0;
	int numThreads;
	cout << "\nEnter Number of Threads to use (Even numbers recommended)\n\tEnter:\t";
	while (!condition) {
		cin >> numThreads;
		if (numThreads >= 1 && numThreads <= 128) {
			condition = 1;
		}
		else {
			printf("\nInvalid Input (1-128)\n\tEnter:\t");
		}
	}
	return numThreads;
}
int getInputMode() {
	int condition = 0;
	int InputMode;
	cout << "\nEnter Input Mode: 1. Image\t2. Video\t3. Webcam\n\tEnter:\t";
	while (!condition) {
		cin >> InputMode;
		if (InputMode >= 1 && InputMode <= 3) {
			condition = 1;
		}
		else {
			printf("\nInvalid Input (1-3)\n\tEnter:\t");
		}
	}
	return InputMode;
}

// Step 1: get input file from user
// TODO:  add input validation
string getInputFile(int inputMode)
{
	int numFrames = 0;
	int condition = 0;
	string path, ext;
	Mat img;
	cout << "\nEnter input file, including extension (from Input folder):\n\tEnter:\t";
	while (!(condition)) {
		cin >> path;
		path = "Input/" + path;
		if (exists_test(path)) {
			ext = get_filename_ext(path.c_str());
			if (inputMode == 1) {
				if (ext == "jpg" || ext == "png" || ext == "bmp" || ext == "tiff") {
					condition = 1;
				}
				else {
					printf("\nFile Path is not an Image\n\tEnter:\t");
				}
			}
			if (inputMode == 2) {
				if (ext == "mp4" || ext == "mov" || ext == "avi") {
					condition = 1;
				}
				else {
					printf("\nFile Path is not a Video\n\tEnter:\t");
				}
			}
			if (inputMode == 3) {
				condition = 1;
			}
		}
		else {
			printf("\nInvalid File Path\n\tEnter:\t");
		}

	}
	return path;
}

int getKyber()
{
	int condition = 0;
	int kyberImplementation;
	cout << "\nEnter Kyber Implementation: 1. Kyber512\t2. Kyber768\t3. Kyber1024\n\tEnter:\t";
	while (!condition) {
		cin >> kyberImplementation;
		if (kyberImplementation >= 1 && kyberImplementation <= 3) {
			condition = 1;
		}
		else {
			printf("\nInvalid Input (1-3)\n\tEnter:\t");
		}
	}
	return kyberImplementation;
}

int getMode()
{
	int condition = 0;
	int mode;
	cout << "\nEnter Processing Mode: 1. Single Threaded\t2. Multi-Threaded (CPU)\t3. Multi-Threaded (GPU)\n\tEnter:\t";
	while (!condition) {
		cin >> mode;
		if (mode >= 1 && mode <= 3) {
			condition = 1;
		}
		else {
			printf("\nInvalid Input (1-3)\n\tEnter:\t");
		}
	}

	return mode;
}

// Step 2: detect faces, output face files and face outlines file
int getFaces(Mat img, int kyberImplementation, int mode, int numThreads, int inputMode, int faceNum, string outputFolder, FILE* output_CSV)
{
	int i = 0;
	// Load facial recognition XML
	CascadeClassifier faceCascade;
	faceCascade.load(CASCADE_CLASSIFIER);

	// test for loaded xml file
	if (faceCascade.empty())
	{
		cout << "facial recognition XML file not loaded" << endl;
	}

	vector<Rect> faces;
	faceCascade.detectMultiScale(img, faces, 1.1, 8);	// detect faces

	// for each detected face: save as separate image, draw rectangle 
	for (i = 0; i < faces.size(); i++)
	{
		// Get dimensions for face
		int xLower = faces[i].tl().x;
		int xUpper = faces[i].br().x;
		int yLower = faces[i].br().y;
		int yUpper = faces[i].tl().y;

		Mat croppedImage = img(Range(yUpper, yLower), Range(xLower, xUpper));

		// build filepath, output each croppedImage to a separate file
		string outputPath = ("Output/" + outputFolder + "/Faces/face" + to_string(i + faceNum) + ".bmp");
		string encryptedPath = ("Output/" + outputFolder + "/Encryption/face" + to_string(i + faceNum) + ".bmp");
		string decryptedPath = ("Output/" + outputFolder + "/Decryption/face" + to_string(i + faceNum) + ".bmp");
		imwrite(outputPath, croppedImage);

		// draw rectangle around face in original image
		if (inputMode == 1) rectangle(img, faces[i].tl(), faces[i].br(), Scalar(0, 255, 0), 2);


		// pass face file to crypt function
		// input arguments for crypt: char* kyber_implementation, char* input_file_path, char* encryption_file_path, char* decryption_file_path
		char output_path[1024];
		char encrypted_path[1024];
		char decrypted_path[1024];
		long double difference_enc = 0, difference_dec = 0;
		string csv_entry;
		strncpy(output_path, outputPath.c_str(), sizeof(output_path));
		strncpy(encrypted_path, encryptedPath.c_str(), sizeof(encrypted_path));
		strncpy(decrypted_path, decryptedPath.c_str(), sizeof(decrypted_path));

		if (mode == 3)
			//printf("CUDA NOT AVAILABLE");
			kybercuda(kyberImplementation, output_path, encrypted_path, decrypted_path, mode, numThreads);	// call cuda (GPU) version of crypt function
		else
			crypt(kyberImplementation, output_path, encrypted_path, decrypted_path, mode, numThreads, &difference_enc, &difference_dec);	// call CPU version of crypt function
		csv_entry = to_string(i + faceNum) + "," + to_string(difference_enc) + "," + to_string(difference_dec) + "\n";
		fprintf(output_CSV, csv_entry.c_str());
		if (inputMode == 1) {
			if (exists_test(encrypted_path) && exists_test(decrypted_path)) {
				Mat encrypted = imread(encrypted_path);
				Mat decrypted = imread(decrypted_path);
				string encrypted_window = "Encrypted Image " + to_string(i);
				string decrypted_window = "Decrypted Image " + to_string(i);
				namedWindow(encrypted_window, WINDOW_NORMAL);
				namedWindow(decrypted_window, WINDOW_NORMAL);
				imshow(encrypted_window, encrypted);
				imshow(decrypted_window, decrypted);
			}
		}
	}

	if (inputMode == 1) {
		// show detected faces in a display window
		imshow("Display Window", img);
		fclose(output_CSV);
		waitKey(0);
	}

	return i;
}

inline bool exists_test(const string& name) {
	if (FILE* file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

const char* get_filename_ext(const char* filename) {
	const char* dot = strrchr(filename, '.');
	if (!dot || dot == filename) return "";
	return dot + 1;
}