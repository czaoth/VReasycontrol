#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <iostream>
#include <WinSock2.h>  
#pragma comment(lib, "ws2_32.lib")  
#include<math.h>
using namespace cv;
using namespace std;
int thresh = 30;
int max_thresh = 255;
int big;

int bSums(Mat src)
{

	int counter = 0;
	//�������������ص�  
	Mat_<uchar>::iterator it = src.begin<uchar>();
	Mat_<uchar>::iterator itend = src.end<uchar>();
	
	int i = 0;
	int num1k=0;
	int num2k=0;
	for (; it != itend; ++it)
	{
		i++;
		
		if ((*it)>0) {
			counter += 1;
			if (counter == big/2) num1k = i;
			if (counter == 15+big/2) num2k = i;
		}  //ȡ��1500��1512����Ϊ��������
		
	}
	if (num2k - num1k < 55) { i = (num1k + num2k) / 2; }
	else { i = num2k; }
	big = counter;
	return i;
}

int main()
{
	//��ʼ��WSA  
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//�����׽���  
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}

	//��IP�Ͷ˿�  
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}

	//��ʼ����  
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		return 0;
	}
	//�ȴ�����
	SOCKET sClient;
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[255];
	while (true)
	{
		printf("�ȴ�����...\n");
		sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
		if (sClient == INVALID_SOCKET)
		{
			printf("accept error !");
			continue;
		}
		printf("���ܵ�һ�����ӣ�%s \r\n", inet_ntoa(remoteAddr.sin_addr));
		break;
	}

	//ͼ�������
	char c;
	VideoCapture inputVideo(0);    
	Mat src;
	Mat oneclo[3];
	int oldnum = 1;
	//vector<vector<Point>> contours;
	//vector<Vec4i> hierarchy;
	cvNamedWindow("���", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("����", CV_WINDOW_AUTOSIZE);
	for (;;)
	{
		
		inputVideo >> src;
		imshow("����", src);
		cvtColor(src, src, CV_BGR2GRAY);
		medianBlur(src,src,3);
		//cvFlip(src,src, 1);
		GaussianBlur(src, src, Size(7, 7), 4.5,3);
		threshold(src, src, 246, 255, THRESH_BINARY);
		//Canny(src, src, thresh, thresh * 3, 3);
		int num=bSums(src);


		//findContours(src, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
		int numx = num / 640;
		int numy = num % 640;
		int oldx = oldnum / 640;
		int oldy = oldnum % 640;
		int chax = sqrt(abs(oldx - numx) ^ 2 );
		int chay = sqrt(abs(oldy - numy) ^ 2);
		printf("%d-%d ��С%d\n", chax,chay,big);
		if (chax < 1) {
			if (chay < 1) { 
				numx = oldx;
				numy = oldy;
				num = oldnum;
			}//��������
		}
		
	    
		Point center = Point(640-numy, numx);
		Point center2 = Point(numy, numx);
		int r = 10;
		Mat picture(480, 640, CV_8UC3, Scalar(255, 255, 255));
		circle(src, center2, r, Scalar(0, 250, 0));
		circle(src, center2, r-5, Scalar(0, 0, 255));
		circle(picture, center, r, Scalar(0, 250, 0));
		circle(picture, center, r - 5, Scalar(0, 0, 255));
		imshow("���", src);
		imshow("����", picture);
		//int sennum = num;
		//sennum = htonl(sennum);
		//send(sClient,(const void*)&sennum, sizeof(sennum), 0);
		//char * sendData = "����";
		//sendData="���¸�ֵ����";
		//itoa(num, sendData, 10);

		char buf[32];
		sprintf(buf, "x%dy%dbig%d", numx,numy,big);
		send(sClient, buf, strlen(buf), 0);
		oldnum = num;

		c = waitKey(10);
		if (c == 27) break;
	}
	closesocket(sClient);
	closesocket(slisten);
	WSACleanup();
	return 0;
}

