

//����Ʈ���� ������Ʈ - �ֹ溸�� ���α׷�

#define _CRT_SECURE_NO_WARNINGS //time_t �� ���� ���� �߰�
#include<iostream> 
#include"opencv2/opencv.hpp"
#include <sstream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace cv;
void on_mouse(int event, int x, int y, int flags, void* userdata);//���콺�̺�Ʈ�ڵ鷯 
int camera_onoff = 0;//ī�޶��� on/off ��Ÿ���� ����
string scan_off = "Waiting for food ingredient detection"; //�������� �������� ������ ��¹���
string scan_on = "During the food ingredient detection operation";//�������� ������ ��¹���
string consumption_input = "Input consumption ingredient information";//�Һ����� �Է±���� ��¹���
string amount_of_storage = "Output storage ingredient information";//��������� ������±����
string program_end = "Shutting down a Program";//���α׷� �������� ��¹���
using namespace cv::dnn;//���ӽ����̽� ����
const float CONFIDENCE_THRESHOLD = 0.5;//�νķ� ��谪
const float NMS_THRESHOLD = 0.5;//
const int NUM_CLASSES = 3;
// colors for bounding boxes
const cv::Scalar colors[] = { {0, 255, 255},{255, 255, 0},{0, 255, 0},{255, 0, 0} };
const auto NUM_COLORS = sizeof(colors) / sizeof(colors[0]);
std::vector<std::string> class_names = { "tomato","onion","paprika" };

Mat dect_mat = Mat::zeros(NUM_CLASSES, 9, CV_32SC1);

void GUI_init(string& gui_window, Mat& gui_img); //GUI�� ������ �Լ�
void input_image_capture(); //ī�޶� Ű�� ������ �����ϴ� �Լ�
void food_ingredients_dect(); // ������ ������ ����Ḧ �����ϴ� �Լ�
void pick_keeping_method(); //���ù���� ����� �Լ�
void add_ingredients(const Mat& mat); //������� ������ �޸��忡 ����ϴ� �Լ�
Mat out_amount_of_storage(); //���� ����� ����� ������ ����ϴ� �Լ�
void program_exit(); //���α׷��� �����ϴ� �Լ�
void erase_ingredients(); //������ ����Ḧ �����ϴ� �Լ�

int main() {//�����Լ� ����

	string gui_window = "Control GUI"; //GUI�� �������̸��� ����
	namedWindow(gui_window); // GUI������ ����

	Mat gui_img(500, 500, CV_8UC3, Scalar(255, 255, 255)); // GUI Mat ��ü ����

	GUI_init(gui_window, gui_img); // GUI �������� �������

	setMouseCallback("Control GUI", on_mouse, &gui_img); //GUI�� �̺�Ʈ �ڵ鷯 �߰�
	imshow("Control GUI", gui_img); //GUI ���

	while (1) { if (waitKey() == 27) break; } //esc�� ������ ����
	return 0; // �����Լ� ��ȯ��

} //�����Լ� ����

void on_mouse(int event, int x, int y, int flags, void* userdata) {//���콺 �̺�Ʈ �ڵ鷯

	Mat gui_img = *(Mat*)userdata;//GUI ��ü
	if ((event == EVENT_LBUTTONDOWN)) {//���������� Ŭ����
		if ((y < (gui_img.rows / 2)) && (x < (gui_img.cols / 2))) {//���� ��ư
			if (camera_onoff == 0) {//��ư�� ù��°�� ��������
				for (int r = 0; r < (gui_img.rows / 2); r++)//�����ư��  
					for (int c = 0; c < (gui_img.cols / 2); c++)//������ ������
						gui_img.at<Vec3b>(r, c) = Vec3b(255, 204, 0);//ī�޶� off->on �ϴû����� ��ȭ
				camera_onoff = 1;//ī�޶� �����ٴ� ���� ǥ��
				putText(gui_img, scan_on, Point((gui_img.cols / 30), gui_img.rows / 4), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));
				input_image_capture();//ī�޶� ������ �Լ� ȣ��
			}
			else if (camera_onoff == 1) {//��ư�� �ι�°�� ��������
				for (int r = 0; r < (gui_img.rows / 2); r++)//�����ư�� 
					for (int c = 0; c < (gui_img.cols / 2); c++)//������
						gui_img.at<Vec3b>(r, c) = Vec3b(0, 102, 255);//ī�޶� on->off ��Ȳ������ ��ȭ
				camera_onoff = 0;//ī�޶� off�� ǥ��
				putText(gui_img, scan_off, Point((gui_img.cols / 12), gui_img.rows / 4), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//ī�޶� off ����ǥ�ù�
				food_ingredients_dect();//����� ���� �Լ�ȣ��
				pick_keeping_method();//������ �����Լ� ȣ��
				add_ingredients(dect_mat);//����� �߰��Լ� ȣ��
			}

		}
	}

	if (flags & EVENT_FLAG_LBUTTON) {//��ư�� ������ ������
		if ((y < (gui_img.rows / 2)) && (x > (gui_img.cols / 2))) {//��������ư�� ���
			for (int r = 0; r < (gui_img.rows / 2); r++)//��������ư�� ����
				for (int c = ((gui_img.cols / 2) + 1); c < (gui_img.cols); c++)//��������ư�� ����
					gui_img.at<Vec3b>(r, c) = Vec3b(255, 204, 0);//������ �ϴû����� ��ȭ
		}
		else if ((y > (gui_img.rows / 2)) && (x < (gui_img.cols / 2))) {//�Һ��ư�� ���
			for (int r = ((gui_img.rows / 2) + 1); r < (gui_img.rows); r++)//�Һ��ư�� ���
				for (int c = 0; c < (gui_img.cols / 2); c++)//�Һ��ư�� ���
					gui_img.at<Vec3b>(r, c) = Vec3b(255, 204, 0);//������ �ϴû����� ��ȭ
		}
		else if ((y > (gui_img.rows / 2)) && (x > (gui_img.cols / 2))) {//�����ư�� ���
			for (int r = ((gui_img.rows / 2) + 1); r < (gui_img.rows); r++)//�����ư�� ���
				for (int c = ((gui_img.cols / 2) + 1); c < (gui_img.cols); c++)//�����ư�� ���
					gui_img.at<Vec3b>(r, c) = Vec3b(255, 204, 0);//������ �ϴû����� ��ȭ
		}
		putText(gui_img, consumption_input, Point(((gui_img.cols / 2) + (gui_img.cols / 15)), (gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//�Һ��ư ǥ�ù� ���
		putText(gui_img, amount_of_storage, Point((gui_img.cols / 12), ((gui_img.rows / 2) + (gui_img.rows / 4))), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//��������ư ǥ�ù� ���
		putText(gui_img, program_end, Point(((gui_img.cols / 2) + (gui_img.cols / 8)), ((gui_img.rows / 2) + gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//�����ư ǥ�ù� ���


	}

	if (event == EVENT_LBUTTONUP) {//������ ���콺�� �ø� ���
		if ((y < (gui_img.rows / 2)) && (x > (gui_img.cols / 2))) {//��������ư�� ���
			for (int r = 0; r < (gui_img.rows / 2); r++)//��������ư�� ����
				for (int c = ((gui_img.cols / 2) + 1); c < (gui_img.cols); c++)//��������ư�� ����
					gui_img.at<Vec3b>(r, c) = Vec3b(51, 255, 102);//���� ������� ��ȭ
			out_amount_of_storage();//������ ����Լ� ȣ��
		}
		else if ((y > (gui_img.rows / 2)) && (x < (gui_img.cols / 2))) {//�Һ� ��ư�� ���
			for (int r = ((gui_img.rows / 2) + 1); r < (gui_img.rows); r++)//�Һ� ��ư�� ����
				for (int c = 0; c < (gui_img.cols / 2); c++)//�Һ� ��ư�� ����
					gui_img.at<Vec3b>(r, c) = Vec3b(153, 51, 0);//���� �������� ��ȭ
			erase_ingredients();//������ �����Լ� ȣ��
		}
		else if ((y > (gui_img.rows / 2)) && (x > (gui_img.cols / 2))) {//�����ư�� ���
			for (int r = ((gui_img.rows / 2) + 1); r < (gui_img.rows); r++)//�����ư�� ����
				for (int c = ((gui_img.cols / 2) + 1); c < (gui_img.cols); c++)//�����ư�� ����
					gui_img.at<Vec3b>(r, c) = Vec3b(204, 204, 204);//���� ȸ������ ��ȭ
			program_exit();//���α׷� �����Լ� ȣ��
		}

		putText(gui_img, consumption_input, Point(((gui_img.cols / 2) + (gui_img.cols / 15)), (gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//��������ư ǥ�ù����
		putText(gui_img, amount_of_storage, Point((gui_img.cols / 12), ((gui_img.rows / 2) + (gui_img.rows / 4))), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//�Һ񷮹�ư ǥ�ù����
		putText(gui_img, program_end, Point(((gui_img.cols / 2) + (gui_img.cols / 8)), ((gui_img.rows / 2) + gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//�����ư ǥ�ù����
	}

	imshow("Control GUI", gui_img);//������
}

void GUI_init(string& gui_window, Mat& gui_img) {//GUI ������ �ʱ�ȭ �Լ�

	for (int r = 0; r < gui_img.rows; r++)//GUI �ุŭ �ݺ�
		for (int c = 0; c < gui_img.cols; c++)//GUI ����ŭ �ݺ�
		{
			if ((r < (gui_img.rows / 2)) && (c < (gui_img.cols / 2))) {//�����ư�� ���
				gui_img.at<Vec3b>(r, c) = Vec3b(0, 102, 255);//ī�޶��-��Ȳ��
			}
			else if ((r < (gui_img.rows / 2)) && (c > (gui_img.cols / 2))) {//��������ư�� ���
				gui_img.at<Vec3b>(r, c) = Vec3b(51, 255, 102);//�Һ� �����- ���
			}
			else if ((r > (gui_img.rows / 2)) && (c < (gui_img.cols / 2))) {//�Һ񷮹�ư�� ���
				gui_img.at<Vec3b>(r, c) = Vec3b(153, 51, 0);//������ - ����
			}
			else if ((r > (gui_img.rows / 2)) && (c > (gui_img.cols / 2))) {//�����ư�� ���
				gui_img.at<Vec3b>(r, c) = Vec3b(204, 204, 204);//���α׷�����-ȸ��
			}
			else//��ư��輱
			{
				gui_img.at<Vec3b>(r, c) = Vec3b(0, 0, 0);//��ư��輱-������
			}
		}

	putText(gui_img, scan_off, Point((gui_img.cols / 12), gui_img.rows / 4), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//�����ư ǥ�ù�
	putText(gui_img, consumption_input, Point(((gui_img.cols / 2) + (gui_img.cols / 15)), (gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//��������ư ǥ�ù�
	putText(gui_img, amount_of_storage, Point((gui_img.cols / 12), ((gui_img.rows / 2) + (gui_img.rows / 4))), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//�Һ񷮹�ư ǥ�ù�
	putText(gui_img, program_end, Point(((gui_img.cols / 2) + (gui_img.cols / 8)), ((gui_img.rows / 2) + gui_img.rows / 4)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0));//�����ư ǥ�ù�

	imshow("Control GUI", gui_img);//������
}



void input_image_capture() {//ī�޶� ȣ���Լ�
	VideoCapture cap(1);//ī�޶� on
	Mat frame;//����ü ����
	while (1) {//������ �ݺ���
		cap >> frame;//ī�޶󿡼� ����1���� ������
		imshow("frame", frame);//�������
		if (waitKey(100) == 27) { imwrite("input.jpg", frame); destroyWindow("frame"); break; }//esc�� ������ ���������� ī�޶� off
	}//�ݺ��� ����
}

void food_ingredients_dect() {//����� ���� �Լ�
	//std::vector<std::string> class_names = { "tomato","onion","paprika" };
	auto net = cv::dnn::readNetFromDarknet("yolov4-foodingredients3.cfg", "yolov4-foodingredients3_final.weights");//��Ʈ��ũ,����ġ���� 
	//net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
	//net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
	net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);//�鿣�� ����
	net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);//����ó����ġ ����
	auto output_names = net.getUnconnectedOutLayersNames();
	cv::Mat frame = imread("input.jpg");//�Է¿��� ����
	cv::Mat blob;
	std::vector<cv::Mat> detections;//���⺤��

	dect_mat.setTo(0);//�������� ������ ���� ��������� �ʱ�ȭ �ؾ���

	if (frame.empty())
	{
		cerr << "frame empty" << endl;
	}

	clock_t start, end;//����ð� ����
	cv::dnn::blobFromImage(frame, blob, 1 / 255.f, cv::Size(416, 416), cv::Scalar(),
		true, false, CV_32F);//������ ��ü�� �Է��Ѵ�
	net.setInput(blob);//������ ��ü�� �Է�
	start = clock();//����ð� ��������
	net.forward(detections, output_names);//����
	end = clock();//����ð� ��������

	std::vector<int> indices[NUM_CLASSES];//���ⰴü ���庤��
	std::vector<cv::Rect> boxes[NUM_CLASSES];//�ٿ���ڽ� ���庤��
	std::vector<float> scores[NUM_CLASSES];//����ŷڵ� ���庤��


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

	time_t timer;//�ð��� ǥ���ϴ� ��ü
	struct tm* t;//�ð��� ǥ���ϴ� ��ü
	timer = time(NULL); // 1970�� 1�� 1�� 0�� 0�� 0�ʺ��� �����Ͽ� ��������� ��
	t = localtime(&timer);//�ð� ��ü
	for (int r = 0; r < NUM_CLASSES; r++)//Ŭ���� ����ŭ
	{
		if (indices[r].size() != 0) dect_mat.at<int>(r, 0) = 1;//���ⰴü ���� ���
		dect_mat.at<int>(r, 1) = indices[r].size();//���� ���� ����
	}

	for (int r = 0; r < NUM_CLASSES; r++)//Ŭ���� �� ��ŭ
		for (int c = 2; c < 7; c++)//����ð� ���� �ݺ���
		{
			if (c == 2) {//����⵵
				dect_mat.at<int>(r, c) = (t->tm_year + 1900);//����⵵�� 1900�⿡�� ���� ���̹Ƿ� +1900
			}
			else if (c == 3) {//���� ��
				dect_mat.at<int>(r, c) = (t->tm_mon + 1);//������� 0~11�̹Ƿ� +1�� ���⺤�Ϳ� ����
			}
			else if (c == 4) {//���� ��
				dect_mat.at<int>(r, c) = t->tm_mday;//������ ���⺤�Ϳ� ����
			}
			else if (c == 5) {//���� �ð�
				dect_mat.at<int>(r, c) = t->tm_hour;//������ ���⺤�Ϳ� ����
			}
			else if (c == 6) {
				dect_mat.at<int>(r, c) = t->tm_min;//���� �� ���⺤�Ϳ� ����
			}
		}

	//����� ��ü�� �̸�, ���� ���

	std::cout << "\n\n";
	for (int i = 0; i < NUM_CLASSES; i++) {//Ŭ������ ��ŭ �ݺ�
		std::cout << class_names[i] << " : " << indices[i].size() << endl;//������ Ŭ���� ���� ������ ���
	}
	std::cout << "����1���� ����ð� " << end - start << " ms " << endl;//ó���ð� ǥ��

	namedWindow("dect", WINDOW_NORMAL);//��� ��� ������ ����
	cv::imshow("dect", frame);//���� ��� ���� ���
	while (waitKey() != 27);//Ű �Է´��
	destroyWindow("dect");//������ ������ ����
}


void pick_keeping_method() {//������� ���ñ��
	int choice_num = 0;//���ù�ȣ ���庯��
	for (int r = 0; r < NUM_CLASSES; r++) {//Ŭ���� �� ��ŭ ����
		if (dect_mat.at<int>(r, 0) == 0) continue;//������ �ȵ� ���� ó�� ����

		string file_name = class_names[r] + "_keeping_method_kr_ANSI.txt";//������ ����Ʈ �����̸�
		string str;//������ ����

		ifstream file(file_name);//������ ���� ����

		std::cout << "\n\n " << class_names[r] + " �� ������ ����Ʈ " << endl;

		if (file.is_open())
		{
			while (!file.eof()) {
				getline(file, str);
				std::cout << str << endl;
			}
		}
		file.close();

		std::cout << "\n\n ����Ʈ ���� " << class_names[r] << "�� ���ϴ� ��������� ��ȣ�� �Է� ���ּ��� : ";
		cin >> choice_num;
		cin.ignore();

		if (choice_num == -1) { cerr << "�߸��ν��� �����Դϴ� \n"; break; }//���� �߸��ν��� �����̸� ������ȣ -1�� �Է�

		string search_num = "#" + to_string(choice_num);//������ ������ ��ȣ���� �̵��ϱ� ���� ���ڿ�

		ifstream file2(file_name);//������ ���Ͽ���
		string str2;//���پ��б� ���� ���ڿ�
		stringstream ss;//���ڿ����� ���� ������ ���� ��ü
		int expiration_date = 1000;//�����Ⱓ �ʱ�ȭ

		if (file2.is_open())//���� ���� Ȯ�ι�
		{
			while (!file2.eof()) {//���� ������ �б�
				getline(file2, str2);//������ ���پ� ���ڿ��� ����
				if (str2 == search_num) break;//���ϴ� ������ ��ȣ�� �̵� �ϸ� ����

			}

			while (!file2.eof()) {
				getline(file2, str2);//���پ� �б�
				if (str2.find("�����Ⱓ: ") != string::npos) {//�����Ⱓ�� ���ԵǾ� ������ 
					str2.erase(0, 9);//���� �ѱۺκ��� �������
					ss.str(str2);//���ڿ��� ���ںκи� ��ü�� ����
					break;//�ݺ��� ����
				}
			}
		}

		file2.close();//������� ���� ����

		ss >> expiration_date;//���ڿ��� �����Ⱓ�� int������ ����
		dect_mat.at<int>(r, 7) = choice_num;//���⺤�Ϳ� ���ù�ȣ ����
		dect_mat.at<int>(r, 8) = expiration_date;//���⺤�Ϳ� �����Ⱓ ����

		std::cout << "�����Ͻ� " << class_names[r] << " �� ��������� �����Ⱓ�� " << expiration_date << "�� �Դϴ�." << endl;//��� ���
	}
	//if (choice_num == -1);//��� ��� Ȯ�ο�
	//else std::cout << dect_mat << endl; //������ Ȯ�ο� �ڵ�
};

void add_ingredients(const Mat& mat) {//�������Ḧ �����ϴ� �Լ�
	for (int r = 0; r < NUM_CLASSES; r++) {//Ŭ������ ��ŭ �ݺ�
		if ((mat.at<int>(r, 0) == 0) || (mat.at<int>(r, 1) == 0) || (mat.at<int>(r, 7) == 0) || (mat.at<int>(r, 8) == 0)) continue;//���⺤�Ϳ��� ����Ȱ͸� ó��
		string add = "";//���� ����� ������ ���ڿ�
		for (int c = 0; c < mat.cols; c++) {//���⺤���� ����ŭ �ݺ�
			add += (' ' + to_string(mat.at<int>(r, c)));//���⺤�͸� ���ڿ��� ����
		}

		string add_file_name = class_names[r] + "_amount_of_storage.txt";//������ ������ �����̸�
		ofstream ofs(add_file_name, ios::out | ios::app);//������ ���� open

		ofs.write(add.c_str(), add.size());//������ ���Ͽ� ���⹮�ڿ� ���
		ofs << "\n";//�ٹٲ�
		ofs.close();//������ ���� ����
		std::cout << class_names[r] + " ����� ������ ���Ͽ� ����Ͽ����ϴ�. \n";//����� ���
	}
}//�������� �����ϴ� �Լ� ����

Mat out_amount_of_storage() {//������ ����ϴ� �Լ�
	Mat storage_list;//��ü �������� ������ ���
	for (int r = 0; r < NUM_CLASSES; r++) {//Ŭ���� �� ��ŭ �ݺ�
		string read_file_name = class_names[r] + "_amount_of_storage.txt";//������ �����̸�
		ifstream ifs(read_file_name);//���� ����

		string storage_str;//������ ������ ������ ��ü

		if (ifs.is_open())//������ ������
		{
			while (!ifs.eof()) {//���� �������� ���� �ݺ�
				Mat storage = Mat::zeros(1, 9, CV_32SC1);//�������� �ʱ�ȭ
				getline(ifs, storage_str);//������ ���پ� �б�
				if (storage_str == "")continue;//���� ������ ������ ó�� �����ݺ�������
				stringstream ss;//���ڿ����� ���ڸ� �����ϴ� ��ü
				ss.str(storage_str);//���پ� ���ڿ��� ����
				int dump = 0;//���� ������ ù��° ��Ҹ� ���� ��������

				for (int c = 0; c < 9; c++) {//�� �ٿ��� ���ڸ� �����ؼ� �������Ϳ� �Է�
					if (c == 0) { storage.at<int>(0, c) = r; ss >> dump; }//1��Ҵ� ������ Ŭ���� ��ȣ�� �Է�
					else {
						ss >> storage.at<int>(0, c);//���ڿ� ���ڸ� �������Ϳ� �Է�
					}
				}

				if (storage.empty()) continue;//�������Ͱ� ��������� �����ݺ���
				storage_list.push_back(storage);//��������Ʈ�� ������ ��������� ����
			}
		}
		ifs.close();//������ ������ ����
	}

	//cout << storage_list << endl; //������ Ȯ�ο� �ڵ�

	if (storage_list.empty()) { std::cout << "���� ������ ����ᰡ �����ϴ�" << endl; }//��������Ʈ�� ������
	else {//���� ����Ʈ�� ������
		std::cout << "\n\n\n���� ������ ����� ���� \n\n";//��� ���

		for (int r = 0; r < storage_list.rows; r++) {//����Ʈ �ุŭ �ݺ�
			std::cout << "������ȣ: " << r << "   ";//������ȣ ���

			for (int name = 0; name < class_names.size(); name++) {//Ŭ���� �̸� ��� �ݺ���
				if (storage_list.at<int>(r, 0) == name) std::cout << class_names[name] << " - ";//Ŭ���� �̸� ���
			}

			time_t dect_day, end;//���� �ð��� ����ð��� ������ ��ü
			struct tm dect_time;//dect_day ��ü�� ����� ���� ����ü
			double diff;//��¥���̰� ����

			int remaining_expiration_date, elapsed_day;//���� �����Ⱓ ����
			int de_year = storage_list.at<int>(r, 2);//����⵵ ����
			int de_mon = storage_list.at<int>(r, 3);//����� ����
			int de_day = storage_list.at<int>(r, 4);//������ ����

			dect_time.tm_year = (de_year - 1900);//����ü�� ����⵵ ����
			dect_time.tm_mon = (de_mon - 1);//����ü�� ����� ����
			dect_time.tm_mday = de_day;//����ü�� ������ ����
			dect_time.tm_hour = 0;//��� x
			dect_time.tm_min = 0;//��� x
			dect_time.tm_sec = 0;//��� x
			dect_time.tm_isdst = 0;//��� x

			dect_day = mktime(&dect_time);//����ü�� time_t��ü�� ��ȯ
			time(&end);//����ð� time_t ��ü����

			diff = difftime(end, dect_day);
			elapsed_day = (diff / (60 * 60 * 24));
			remaining_expiration_date = (storage_list.at<int>(r, 8) - elapsed_day);

			std::cout << " ����: " << storage_list.at<int>(r, 1) << " , ������: " << de_year << "�� " << de_mon << "�� " << de_day << "�� , ���� ��������: " << remaining_expiration_date << endl;
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
	std::cout << "\n\n�Ҹ��� ������� ������ȣ�� �Է����ּ���: ";
	cin >> num;
	cin.ignore();

	std::cout << "�Ҹ��� ������� ������ �����ּ���: ";
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

	std::cout << "\n������ ����������� ���� �Ͽ����ϴ�\n";
}