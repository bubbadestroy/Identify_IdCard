#ifndef HEADERS
#define HEADERS

#include <cmath>
#include <string>
#include <fstream>
#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/ml/ml.hpp>

using namespace std;
using namespace cv;
using namespace ml;

class Sex
{
public:
	string male = "男";
	string female = "女";
};

Mat getResizeGrayImage(const Mat &in);                             // 获取灰度图,调整图大小
void posDetect(const Mat &inputImage, vector<RotatedRect> &rects); // 身份证号码定位，并加黑线框
bool isEligible(const RotatedRect &candidate);                     // 判断矩形是否符合要求
Mat Cut_Area(const Mat &inputImage, RotatedRect &rects_optimal);   // 裁剪身份证号码
void splitCharacter(Mat &inputImage, vector<Mat> &dst_mat);        // 切割数字字符

void getAnnXML();                                   // 获取训练矩阵和标签矩阵，并保存进xml文件
void calcGradientFeat(const Mat &imgSrc, Mat &out); // 计算样本的特征
float sumMatValue(const Mat &image);                // 计算灰度值和
Mat projectHistogram(const Mat &img, int t);        // 计算直方图

void trainAnn(Ptr<ANN_MLP> &ann, int nlayers, int numCharacters);                       // 训练神经网络
string classifyCharacter(Ptr<ANN_MLP> &ann, Ptr<ANN_MLP> &annX, vector<Mat> &char_Mat); // 分类数字字符
void recognitionRate(const string &inputString, string &char_result, int numb);         // 统计并打印识别率
string identifyIdCard(const Mat &inputImage);                                           // 身份证识别

#endif