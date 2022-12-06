# smart_vision_project
smart_vision_project

##프로그램에서 데이터 처리를 하기 위해 사용할 자료 형식

1. 검출 벡터
![검출벡터_이미지](https://github.com/controlgit234/smart_vision_project/blob/main/%EA%B2%80%EC%B6%9C%EB%B2%A1%ED%84%B0_%EC%9D%B4%EB%AF%B8%EC%A7%80.JPG)

2. 보관 벡터
![보관벡터_이미지](https://github.com/controlgit234/smart_vision_project/blob/main/%EB%B3%B4%EA%B4%80%EB%B2%A1%ED%84%B0_%EC%9D%B4%EB%AF%B8%EC%A7%80.JPG)

### 작품의 전체코드
```
//스마트비전 프로젝트 - 주방보조 프로그램

#define _CRT_SECURE_NO_WARNINGS //time_t 를 쓰기 위한 추가
#include<iostream> 
#include"opencv2/opencv.hpp"
#include <sstream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace cv;
void on_mouse(int event, int x, int y, int flags, void* userdata);//마우스이벤트핸들러 
int camera_onoff = 0;//카메라의 on/off 나타내는 변수
string scan_off = "Waiting for food ingredient detection"; //검출기능이 동작하지 않을때 출력문장
string scan_on = "During the food ingredient detection operation";//검출기능이 동작중 출력문장
string consumption_input = "Input consumption ingredient information";//소비식재료 입력기능의 출력문장
string amount_of_storage = "Output storage ingredient information";//보관식재료 정보출력기능의
string program_end = "Shutting down a Program";//프로그램 종료기능의 출력문장
using namespace cv::dnn;//네임스페이스 생략
const float CONFIDENCE_THRESHOLD = 0.5;//인식률 경계값
const float NMS_THRESHOLD = 0.5;//
const int NUM_CLASSES = 3;
// colors for bounding boxes
const cv::Scalar colors[] = { {0, 255, 255},{255, 255, 0},{0, 255, 0},{255, 0, 0} };
const auto NUM_COLORS = sizeof(colors) / sizeof(colors[0]);
std::vector<std::string> class_names = { "tomato","onion","paprika" };

Mat dect_mat = Mat::zeros(NUM_CLASSES, 9, CV_32SC1);

void GUI_init(string& gui_window, Mat& gui_img); //GUI의 디자인 함수
void input_image_capture(); //카메라를 키고 사진을 저장하는 함수
void food_ingredients_dect(); // 저장한 사진의 식재료를 검출하는 함수
void pick_keeping_method(); //선택방법을 물어보는 함수
void add_ingredients(const Mat& mat); //식재료의 정보를 메모장에 기록하는 함수
Mat out_amount_of_storage(); //현재 저장된 식재료 정보를 출력하는 함수
void program_exit(); //프로그램을 종료하는 함수
void erase_ingredients(); //선택한 식재료를 삭제하는 함수

int main() {//메인함수 시작

	string gui_window = "Control GUI"; //GUI의 윈도우이름을 생성
	namedWindow(gui_window); // GUI윈도우 생성

	Mat gui_img(500, 500, CV_8UC3, Scalar(255, 255, 255)); // GUI Mat 객체 생성
	
	GUI_init(gui_window, gui_img); // GUI 디자인을 만들어줌

	setMouseCallback("Control GUI", on_mouse, &gui_img); //GUI에 이벤트 핸들러 추가
	imshow("Control GUI", gui_img); //GUI 출력

	while (1) { if (waitKey() == 27) break; } //esc를 누르면 종료
	return 0; // 메인함수 반환문

} //메인함수 종료

void on_mouse(int event, int x, int y, int flags, void* userdata) {//마우스 이벤트 핸들러

	Mat gui_img = *(Mat*)userdata;//GUI 객체
	if ((event == EVENT_LBUTTONDOWN)) {//오른쪽으로 클릭시
		if ((y < (gui_img.rows / 2)) && (x < (gui_img.cols / 2))) {//검출 버튼
			if (camera_onoff == 0) {//버튼이 첫번째로 눌렸을때
				for (int r = 0; r < (gui_img.rows / 2); r++)//검출버튼의  
					for (int c = 0; c < (gui_img.cols / 2); c++)//영역의 색깔을
						gui_img.at<Vec3b>(r, c) = Vec3b(255, 204, 0);//카메라 off->on 하늘색으로 변화
				camera_onoff = 1;//카메라가 켜졌다는 것을 표시
				putText(gui_img, scan_on, Point((gui_img.cols / 30), gui_img.rows / 4), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));
				input_image_capture();//카메라 켜지는 함수 호출
			}
			else if (camera_onoff == 1) {//버튼이 두번째로 눌렀을때
				for (int r = 0; r < (gui_img.rows / 2); r++)//검출버튼의 
					for (int c = 0; c < (gui_img.cols / 2); c++)//영역을
						gui_img.at<Vec3b>(r, c) = Vec3b(0, 102, 255);//카메라 on->off 주황색으로 변화
				camera_onoff = 0;//카메라 off를 표시
				putText(gui_img, scan_off, Point((gui_img.cols / 12), gui_img.rows / 4), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//카메라 off 상태표시문
				food_ingredients_dect();//식재료 검출 함수호출
				pick_keeping_method();//보관법 선택함수 호출
				add_ingredients(dect_mat);//식재료 추가함수 호출
			}

		}
	}

	if (flags & EVENT_FLAG_LBUTTON) {//버튼을 누르고 있을때
		if ((y < (gui_img.rows / 2)) && (x > (gui_img.cols / 2))) {//보관량버튼의 경우
			for (int r = 0; r < (gui_img.rows / 2); r++)//보관량버튼의 영역
				for (int c = ((gui_img.cols / 2) + 1); c < (gui_img.cols); c++)//보관량버튼의 영역
					gui_img.at<Vec3b>(r, c) = Vec3b(255, 204, 0);//누르면 하늘색으로 변화
		}
		else if ((y > (gui_img.rows / 2)) && (x < (gui_img.cols / 2))) {//소비버튼의 경우
			for (int r = ((gui_img.rows / 2) + 1); r < (gui_img.rows); r++)//소비버튼의 경우
				for (int c = 0; c < (gui_img.cols / 2); c++)//소비버튼의 경우
					gui_img.at<Vec3b>(r, c) = Vec3b(255, 204, 0);//누르면 하늘색으로 변화
		}
		else if ((y > (gui_img.rows / 2)) && (x > (gui_img.cols / 2))) {//종료버튼의 경우
			for (int r = ((gui_img.rows / 2) + 1); r < (gui_img.rows); r++)//종료버튼의 경우
				for (int c = ((gui_img.cols / 2) + 1); c < (gui_img.cols); c++)//종료버튼의 경우
					gui_img.at<Vec3b>(r, c) = Vec3b(255, 204, 0);//누르면 하늘색으로 변화
		}
		putText(gui_img, consumption_input, Point(((gui_img.cols / 2) + (gui_img.cols / 15)), (gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//소비버튼 표시문 출력
		putText(gui_img, amount_of_storage, Point((gui_img.cols / 12), ((gui_img.rows / 2) + (gui_img.rows / 4))), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//보관량버튼 표시문 출력
		putText(gui_img, program_end, Point(((gui_img.cols / 2) + (gui_img.cols / 8)), ((gui_img.rows / 2) + gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//종료버튼 표시문 출력


	}

	if (event == EVENT_LBUTTONUP) {//오른쪽 마우스를 올린 경우
		if ((y < (gui_img.rows / 2)) && (x > (gui_img.cols / 2))) {//보관량버튼의 경우
			for (int r = 0; r < (gui_img.rows / 2); r++)//보관량버튼의 영역
				for (int c = ((gui_img.cols / 2) + 1); c < (gui_img.cols); c++)//보관량버튼의 영역
					gui_img.at<Vec3b>(r, c) = Vec3b(51, 255, 102);//떼면 녹색으로 변화
			out_amount_of_storage();//보관량 출력함수 호출
		}
		else if ((y > (gui_img.rows / 2)) && (x < (gui_img.cols / 2))) {//소비량 버튼의 경우
			for (int r = ((gui_img.rows / 2) + 1); r < (gui_img.rows); r++)//소비량 버튼의 역역
				for (int c = 0; c < (gui_img.cols / 2); c++)//소비량 버튼의 영역
					gui_img.at<Vec3b>(r, c) = Vec3b(153, 51, 0);//떼면 남색으로 변화
			erase_ingredients();//보관량 수정함수 호출
		}
		else if ((y > (gui_img.rows / 2)) && (x > (gui_img.cols / 2))) {//종료버튼의 경우
			for (int r = ((gui_img.rows / 2) + 1); r < (gui_img.rows); r++)//종료버튼의 영역
				for (int c = ((gui_img.cols / 2) + 1); c < (gui_img.cols); c++)//종료버튼의 영역
					gui_img.at<Vec3b>(r, c) = Vec3b(204, 204, 204);//떼면 회색으로 변화
			program_exit();//프로그램 종료함수 호출
		}

		putText(gui_img, consumption_input, Point(((gui_img.cols / 2) + (gui_img.cols / 15)), (gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//보관량버튼 표시문출력
		putText(gui_img, amount_of_storage, Point((gui_img.cols / 12), ((gui_img.rows / 2) + (gui_img.rows / 4))), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//소비량버튼 표시문출력
		putText(gui_img, program_end, Point(((gui_img.cols / 2) + (gui_img.cols / 8)), ((gui_img.rows / 2) + gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//종료버튼 표시문출력
	}

	imshow("Control GUI", gui_img);//결과출력
}

void GUI_init(string& gui_window,Mat& gui_img) {//GUI 디자인 초기화 함수
	
	for (int r = 0; r < gui_img.rows; r++)//GUI 행만큼 반복
		for (int c = 0; c < gui_img.cols; c++)//GUI 열만큼 반복
		{
			if ((r < (gui_img.rows / 2)) && (c < (gui_img.cols / 2))) {//검출버튼의 경우
				gui_img.at<Vec3b>(r, c) = Vec3b(0, 102, 255);//카메라온-주황색
			}
			else if ((r < (gui_img.rows / 2)) && (c > (gui_img.cols / 2))) {//보관량버튼의 경우
				gui_img.at<Vec3b>(r, c) = Vec3b(51, 255, 102);//소비 식재료- 녹색
			}
			else if ((r > (gui_img.rows / 2)) && (c < (gui_img.cols / 2))) {//소비량버튼의 경우
				gui_img.at<Vec3b>(r, c) = Vec3b(153, 51, 0);//보관량 - 남색
			}
			else if ((r > (gui_img.rows / 2)) && (c > (gui_img.cols / 2))) {//종료버튼의 경우
				gui_img.at<Vec3b>(r, c) = Vec3b(204, 204, 204);//프로그램종료-회색
			}
			else//버튼경계선
			{
				gui_img.at<Vec3b>(r, c) = Vec3b(0, 0, 0);//버튼경계선-검은색
			}
		}

	putText(gui_img, scan_off, Point((gui_img.cols / 12), gui_img.rows / 4), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//검출버튼 표시문
	putText(gui_img, consumption_input, Point(((gui_img.cols / 2) + (gui_img.cols / 15)), (gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//보관량버튼 표시문
	putText(gui_img, amount_of_storage, Point((gui_img.cols / 12), ((gui_img.rows / 2) + (gui_img.rows / 4))), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//소비량버튼 표시문
	putText(gui_img, program_end, Point(((gui_img.cols / 2) + (gui_img.cols / 8)), ((gui_img.rows / 2) + gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//종료버튼 표시문

	imshow("Control GUI", gui_img);//결과출력
}



void input_image_capture() {//카메라 호출함수
	VideoCapture cap(1);//카메라 on
	Mat frame;//영상객체 생성
	while (1) {//동영상 반복문
		cap >> frame;//카메라에서 영상1개를 가져옴
		imshow("frame", frame);//영상출력
		if (waitKey(100) == 27) { imwrite("input.jpg", frame); destroyWindow("frame"); break; }//esc를 누르면 영상저장후 카메라 off
	}//반복문 종료
}

void food_ingredients_dect() {//식재료 검출 함수
	//std::vector<std::string> class_names = { "tomato","onion","paprika" };
	auto net = cv::dnn::readNetFromDarknet("yolov4-foodingredients3.cfg", "yolov4-foodingredients3_final.weights");//네트워크,가중치파일 
	//net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
	//net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
	net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);//백엔드 지정
	net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);//정보처리장치 지정
	auto output_names = net.getUnconnectedOutLayersNames();
	cv::Mat frame = imread("input.jpg");//입력영상 지정
	cv::Mat blob;
	std::vector<cv::Mat> detections;//검출벡터

	dect_mat.setTo(0);//검출기능을 여러번 사용시 검출행렬을 초기화 해야함

	if (frame.empty())
	{
		cerr << "frame empty" << endl;
	}

	clock_t start, end;//검출시간 측정
	cv::dnn::blobFromImage(frame, blob, 1 / 255.f, cv::Size(416, 416), cv::Scalar(),
		true, false, CV_32F);//영상을 객체에 입력한다
	net.setInput(blob);//영상을 객체에 입력
	start = clock();//검출시간 측정시작
	net.forward(detections, output_names);//검출
	end = clock();//검출시간 측정종료

	std::vector<int> indices[NUM_CLASSES];//검출객체 저장벡터
	std::vector<cv::Rect> boxes[NUM_CLASSES];//바운딩박스 저장벡터
	std::vector<float> scores[NUM_CLASSES];//검출신뢰도 저장벡터


	for (auto& output : detections)
	{
		const auto num_boxes = output.rows;
		for (int i = 0; i < num_boxes; i++)
		{
			auto x = output.at<float>(i, 0) * frame.cols;
			auto y = output.at<float>(i, 1) * frame.rows;
			auto width = output.at<float>(i, 2) * frame.cols;
			auto height = output.at<float>(i, 3) * frame.rows;
			cv::Rect rect(x - width / 2, y - height / 2, width, height);
			for (int c = 0; c < NUM_CLASSES; c++)
			{
				auto confidence = *output.ptr<float>(i, 5 + c);
				if (confidence >= CONFIDENCE_THRESHOLD)
				{
					boxes[c].push_back(rect);
					scores[c].push_back(confidence);

				}
			}
		}
	}

	for (int c = 0; c < NUM_CLASSES; c++)
		cv::dnn::NMSBoxes(boxes[c], scores[c], 0.0, NMS_THRESHOLD,
			indices[c]);
	for (int c = 0; c < NUM_CLASSES; c++)
	{
		for (int i = 0; i < indices[c].size(); ++i)
		{
			const auto color = colors[c % NUM_COLORS];
			auto idx = indices[c][i];
			const auto& rect = boxes[c][idx];
			cv::rectangle(frame, cv::Point(rect.x, rect.y), cv::Point(rect.x + rect.width,
				rect.y + rect.height), color, 3);
			std::string label_str = class_names[c] +
				": " + cv::format("%.02lf", scores[c][idx]);
			int baseline;
			auto label_bg_sz = cv::getTextSize(label_str,
				cv::FONT_HERSHEY_COMPLEX_SMALL, 1, 1, &baseline);
			cv::rectangle(frame, cv::Point(rect.x, rect.y - label_bg_sz.height -
				baseline - 10), cv::Point(rect.x + label_bg_sz.width, rect.y),
				color, cv::FILLED);
			cv::putText(frame, label_str, cv::Point(rect.x, rect.y - baseline - 5),
				cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(0, 0, 0));
		}
	}

	time_t timer;//시간을 표시하는 객체
	struct tm* t;//시간을 표시하는 객체
	timer = time(NULL); // 1970년 1월 1일 0시 0분 0초부터 시작하여 현재까지의 초
	t = localtime(&timer);//시간 객체
	for (int r = 0; r < NUM_CLASSES; r++)//클래스 수만큼
	{
		if (indices[r].size() != 0) dect_mat.at<int>(r, 0) = 1;//검출객체 있을 경우
		dect_mat.at<int>(r, 1) = indices[r].size();//검출 수량 저장
	}

	for (int r = 0; r < NUM_CLASSES; r++)//클래스 수 만큼
		for (int c = 2; c < 7; c++)//검출시각 저장 반복문
		{
			if (c == 2) {//검출년도
				dect_mat.at<int>(r, c) = (t->tm_year + 1900);//검출년도는 1900년에서 지난 년이므로 +1900
			}
			else if (c == 3) {//검출 월
				dect_mat.at<int>(r, c) = (t->tm_mon + 1);//검출월은 0~11이므로 +1후 검출벡터에 저장
			}
			else if (c == 4) {//검출 일
				dect_mat.at<int>(r, c) = t->tm_mday;//검출일 검출벡터에 저장
			}
			else if (c == 5) {//검출 시간
				dect_mat.at<int>(r, c) = t->tm_hour;//검출일 검출벡터에 저장
			}
			else if (c == 6) {
				dect_mat.at<int>(r, c) = t->tm_min;//검출 분 검출벡터에 저장
			}
		}

	//출력한 객체의 이름, 수량 출력

	std::cout << "\n\n";
	for (int i = 0; i < NUM_CLASSES; i++) {//클래스수 만큼 반본
		std::cout << class_names[i] << " : " << indices[i].size() << endl;//검출결과 클래스 수와 수량을 출력
	}
	std::cout << "영상1개의 검출시간 " << end - start << " ms " << endl;//처리시간 표시

	namedWindow("dect", WINDOW_NORMAL);//결과 결과 윈도우 생성
	cv::imshow("dect", frame);//검출 결과 영상 출력
	while (waitKey() != 27);//키 입력대기
	destroyWindow("dect");//검출결과 윈도우 삭제
}


void pick_keeping_method() {//보관방법 선택기능
	int choice_num = 0;//선택번호 저장변수
	for (int r = 0; r < NUM_CLASSES; r++) {//클래스 수 만큼 저장
		if (dect_mat.at<int>(r, 0) == 0) continue;//검출이 안된 행을 처리 안함

		string file_name = class_names[r] + "_keeping_method_kr_ANSI.txt";//보관법 리스트 파일이름
		string str;//파일의 한줄

		ifstream file(file_name);//보관법 파일 오픈

		std::cout << "\n\n " << class_names[r] + " 의 보관법 리스트 " << endl; 

		if (file.is_open())
		{
			while (!file.eof()) {
				getline(file, str);
				std::cout << str << endl;
			}
		}
		file.close();

		std::cout << "\n\n 리스트 에서 " << class_names[r] << "의 원하는 보관방법의 번호를 입력 해주세요 : ";
		cin >> choice_num;
		cin.ignore();

		if (choice_num == -1) { cerr << "잘못인식한 사진입니다 \n"; break; }

		string search_num = "#" + to_string(choice_num);

		ifstream file2(file_name);
		string str2;
		stringstream ss;
		int expiration_date = 1000;

		if (file2.is_open())
		{
			while (!file2.eof()) {
				getline(file2, str2);
				if (str2 == search_num) break;

			}

			while (!file2.eof()) {
				getline(file2, str2);
				if (str2.find("보관기간: ") != string::npos) {
					str2.erase(0, 9);
					ss.str(str2);
					break;
				}
			}
		}

		file2.close();

		ss >> expiration_date;
		dect_mat.at<int>(r, 7) = choice_num;
		dect_mat.at<int>(r, 8) = expiration_date;

		std::cout << "선택하신 " << class_names[r] << " 의 보관방법의 보관기간은 " << expiration_date << "일 입니다." << endl;
	}
	if (choice_num == -1);
	//else std::cout << dect_mat << endl; //결과행렬 확인용 코드
};

void add_ingredients(const Mat& mat) {
	for (int r = 0; r < NUM_CLASSES; r++) {
		if ((mat.at<int>(r, 0) == 0) || (mat.at<int>(r, 1) == 0) || (mat.at<int>(r, 7) == 0) || (mat.at<int>(r, 8) == 0)) continue;
		string add = "";
		for (int c = 0; c < mat.cols; c++) {
			add += (' ' + to_string(mat.at<int>(r, c)));
		}

		string add_file_name = class_names[r] + "_amount_of_storage.txt";
		ofstream ofs(add_file_name, ios::out | ios::app);

		ofs.write(add.c_str(), add.size());
		ofs << "\n";
		ofs.close();
		std::cout << class_names[r] + " 식재료 정보를 파일에 기록하였습니다. \n";
	}
}

Mat out_amount_of_storage() {
	Mat storage_list;
	for (int r = 0; r < NUM_CLASSES; r++) {
		string read_file_name = class_names[r] + "_amount_of_storage.txt";
		ifstream ifs(read_file_name);

		string storage_str;

		if (ifs.is_open())
		{
			while (!ifs.eof()) {
				Mat storage = Mat::zeros(1, 9, CV_32SC1);
				getline(ifs, storage_str);
				if (storage_str == "")continue;
				stringstream ss;
				ss.str(storage_str);
				int dump = 0;

				for (int c = 0; c < 9; c++) {
					if (c == 0) { storage.at<int>(0, c) = r; ss >> dump; }
					else {
						ss >> storage.at<int>(0, c);
					}
				}

				if (storage.empty()) continue;
				storage_list.push_back(storage);
			}
		}
		ifs.close();
	}

	//cout << storage_list << endl; //결과행렬 확인용 코드

	if (storage_list.empty()) { std::cout << "현재 보관된 식재료가 없습니다" << endl; }
	else {
		std::cout << "\n\n\n현재 보관된 식재료 정보 \n\n";

		for (int r = 0; r < storage_list.rows; r++) {
			std::cout << "보관번호: " << r << "   ";

			for (int name = 0; name < class_names.size(); name++) {
				if (storage_list.at<int>(r, 0) == name) std::cout << class_names[name] << " - ";
			}

			time_t dect_day, end;
			struct tm dect_time;
			double diff;

			int remaining_expiration_date, elapsed_day;
			int de_year = storage_list.at<int>(r, 2);
			int de_mon = storage_list.at<int>(r, 3);
			int de_day = storage_list.at<int>(r, 4);

			dect_time.tm_year = (de_year - 1900);
			dect_time.tm_mon = (de_mon - 1);
			dect_time.tm_mday = de_day;
			dect_time.tm_hour = 0;
			dect_time.tm_min = 0;
			dect_time.tm_sec = 0;
			dect_time.tm_isdst = 0;

			dect_day = mktime(&dect_time);
			time(&end);

			diff = difftime(end, dect_day);
			elapsed_day = (diff / (60 * 60 * 24));
			remaining_expiration_date = (storage_list.at<int>(r, 8) - elapsed_day);

			std::cout << " 수량: " << storage_list.at<int>(r, 1) << " , 구매일: " << de_year << "년 " << de_mon << "월 " << de_day << "일 , 남은 보관기한: " << remaining_expiration_date << endl;
		}
	}
	return storage_list;

}

void program_exit() {
	abort();
}

void erase_ingredients() {
	int num = 0; int amount = 0;
	Mat storage_list = out_amount_of_storage();
	std::cout << "\n\n소모한 식재료의 보관번호를 입력해주세요: ";
	cin >> num;
	cin.ignore();

	std::cout << "소모한 식재료의 수량을 적어주세요: ";
	cin >> amount;
	cin.ignore();

	storage_list.at<int>(num, 1) -= amount;
	int change_file_class = storage_list.at<int>(num, 0);

	Mat change_list;

	for (int r = 0; r < storage_list.rows; r++) {
		if ((storage_list.at<int>(r, 0) == change_file_class) && (storage_list.at<int>(r, 1) > 0)) {
			Mat food(1, 9, CV_32SC1);
			for (int c = 0; c < 9; c++) {
				if (c == 0) food.at<int>(0, c) = 1;
				else food.at<int>(0, c) = storage_list.at<int>(r, c);
			}
			change_list.push_back(food);
		}
	}

	string file = class_names[change_file_class] + "_amount_of_storage.txt";
	ofstream ofs(file);
	ofs << "";
	ofs.close();

	ofstream ofs2(file, ios::out | ios::app);
	for (int r = 0; r < change_list.rows; r++) {
		string str = "";
		for (int c = 0; c < change_list.cols; c++)
			str += (' ' + to_string(change_list.at<int>(r, c)));
		ofs2.write(str.c_str(), str.size());
		ofs2 << "\n";
	}
	ofs2.close();

	std::cout << "\n보괸된 식재료정보를 변경 하였습니다\n";
}

```
