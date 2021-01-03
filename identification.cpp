#include "headers.h"

// 获取灰度图,调整图大小
Mat getResizeGrayImage(const Mat &in) {
  Mat resize_Gray, tempImage;
  cvtColor(in, tempImage, COLOR_BGR2GRAY);           // RGB转灰度
  bilateralFilter(tempImage, resize_Gray, -1, 5, 5); // 双边滤波，保留图像边缘(文字)

  // 调整图片大小
  if (resize_Gray.rows > 700 || resize_Gray.cols > 600) {
    Mat resizeR(450, 600, CV_8UC1);
    cv::resize(resize_Gray, resizeR, resizeR.size());
    return resizeR;
  } else
    return resize_Gray;
}

// 身份证号码定位，并加黑线框
void posDetect(const Mat &intputImage, vector<RotatedRect> &rects) {
  Mat ImageBinary, tempImage;
  threshold(intputImage, ImageBinary, 0, 255, THRESH_OTSU); // 二值化
  bitwise_not(ImageBinary, tempImage);                      // 反转
  ImageBinary = tempImage;

  Mat element = getStructuringElement(MORPH_RECT, Size(15, 3)); // 矩形结构元素，15*3接近长宽比（身份证号区域）
  morphologyEx(ImageBinary, tempImage, MORPH_CLOSE, element);   // 闭运算
  ImageBinary = tempImage;

  vector<vector<Point>> contours;                                        // 轮廓向量
  findContours(ImageBinary, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE); // 只检测外轮廓，保存物体边界上所有连续的轮廓点到contours向量内

  // 寻找身份证号所在的轮廓
  vector<vector<Point>>::iterator itc = contours.begin();
  while (itc != contours.end()) // 遍历每一个轮廓
  {
    RotatedRect mr = minAreaRect(Mat(*itc)); // 最小有界矩形区域
    if (isEligible(mr))                      // 若符合要求
    {
      rects.push_back(mr);
      ++itc;
    } else {
      itc = contours.erase(itc);
    }
  }

  Mat outputImage = intputImage.clone();
  Point2f vertices[4]; // 矩形的4个顶点，变量类型为代表坐标的两个float值
  if(rects.size() != 0)
    rects[0].points(vertices);
  for (int i = 0; i < 4; i++)
    line(outputImage, vertices[i], vertices[(i + 1) % 4], Scalar(0, 0, 0)); // 画黑色线条，第二、三参数分别代表线段的起点、终点
}

// 判断矩形是否符合要求
bool isEligible(const RotatedRect &candidate) {
  const double error = 0.3;         // 误差率
  const double aspect = 4.5 / 0.25; // 长宽比（身份证号区域）

  double min = 10 * aspect * 10;         // 最小区域面积
  double max = 50 * aspect * 50;         // 最大区域面积
  double rmin = aspect - aspect * error; // 考虑误差后的最小长宽比
  double rmax = aspect + aspect * error; // 考虑误差后的最大长宽比

  double area = candidate.size.width * candidate.size.height;              // 当前区域面积
  double r = (double)candidate.size.width / (double)candidate.size.height; // 当前区域长宽比
  if (r < 1)
    r = 1 / r; // 保证矩形较长的边为长，较短的边为宽

  if ((area >= min && area <= max) && (r >= rmin && r <= rmax)) // 若在规定的面积范围和长宽比范围内
    return true;
  else
    return false;
}

// 裁剪身份证号码
Mat Cut_Area(const Mat &inputImage, RotatedRect &rects_optimal) {
  Mat output;
  float angle, r;

  angle = rects_optimal.angle;                                            // 角度
  r = (float)rects_optimal.size.width / (float)rects_optimal.size.height; // 宽高比
  if (r < 1)
    angle += 90; // 旋转90度,保证较长的一边为水平状态，较短的一边为垂直状态

  Mat rotmat = getRotationMatrix2D(rects_optimal.center, angle, 1); // 获取绕着中心点旋转angle角的变换矩阵
  Mat img_rotated;
  // 旋转图像，参数分别为待旋转图像、旋转后图像、变换矩阵、旋转后图像大小、双立方插值算法
  warpAffine(inputImage, img_rotated, rotmat, inputImage.size(), INTER_CUBIC);

  Size rect_size = rects_optimal.size; // 与身份证号矩形框大小相同的矩形框
  if (r < 1)
    std::swap(rect_size.width, rect_size.height); // 保证矩形较长的边为长，较短的边为宽
  Mat img_crop;
  // 裁剪图像，参数分别为待裁剪图像、被裁剪矩阵大小、被裁剪矩阵中心、裁剪后图像
  getRectSubPix(img_rotated, rect_size, rects_optimal.center, img_crop);

  // 用光照直方图调整裁剪后图像。令其宽、高统一，提高神经网络训练和识别的效率
  Mat resultResized;
  resultResized.create(20, 300, CV_8UC1);
  resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);

  output = resultResized.clone();
  return output;
}

// 切割数字字符
void splitCharacter(Mat &inputImage, vector<Mat> &dst_mat) {
  Mat img_threshold;
  threshold(inputImage, img_threshold, 0, 255, THRESH_OTSU); // 二值化

  vector<bool> flag; // 记录每一列像素中有无属于数字的像素
  int i, j;

  for (j = 0; j < img_threshold.cols; ++j) {
    flag.push_back(false);
    for (int i = 0; i < img_threshold.rows; ++i) {
      if (img_threshold.at<uchar>(i, j) == 0) // 白底黑字，故为0
      {
        flag[j] = true;
        break;
      }
    }
  }

  int x[2][18] = {0};
  int count = 0;

  // 身份证号前17个数字
  for (i = 0, j = 0; j < 17 && i + 2 < flag.size(); ++i) {
    if (flag[i] == true) {
      count++;

      // 检测后两列，若flag都为false说明到达了数字的边缘
      if (flag[i + 1] == false && flag[i + 2] == false) {
        x[0][j] = i - (count - 1); // 数字开始的列数
        x[1][j] = count;           // 数字占有的列数

        j++;
        count = 0;
      }
    }
  }

  // 因裁剪出的图像的最后几列，flag都为true，故最后一个数字特殊处理
  j = 17;
  for (; i < img_threshold.cols; ++i) {
    if (flag[i] == true)
      count++;

    if (i == img_threshold.cols - 1) {
      x[0][j] = i - (count - 1); // 数字开始的列数
      x[1][j] = count;           // 数字占有的列数
    }
  }

  // 结果写入
  Mat outputImage = 255 - inputImage; // 灰度图反转，与训练样本保持一致（黑底白字）

  for (i = 0; i <= 17; i++) {
    dst_mat.push_back(Mat(outputImage, Rect(x[0][i], 0, x[1][i], outputImage.rows)));
  }
}



// 分类数字字符
string classifyCharacter(Ptr<ANN_MLP> &ann, Ptr<ANN_MLP> &annX, vector<Mat> &char_Mat) {
  string idResult = "000000000000000000";
  Mat output(1, 11, CV_32FC1); // 结果矩阵，0~9 X 十一种结果
  Mat char_feature;
  Point maxLoc;
  double maxVal;

  // 前17个数字
  for (int i = 0; i < char_Mat.size() - 1; ++i) {
    if(char_Mat[i].data){
    calcGradientFeat(char_Mat[i], char_feature); // 计算字符特征
    ann->predict(char_feature, output);          // 预测

    // 寻找最大最小值及其位置，参数分别为寻找范围、最小值、最大值、最小值位置、最大值位置
    minMaxLoc(output, 0, &maxVal, 0, &maxLoc); // 结果矩阵中的数值代表正确结果的概率，寻找最大概率
    idResult[i] = (char)(maxLoc.x + '0');
    }
  }

  // 最后一个数字
  calcGradientFeat(char_Mat[17], char_feature); // 计算字符特征
  annX->predict(char_feature, output);          // 预测

  // 寻找最大最小值及其位置，参数分别为寻找范围、最小值、最大值、最小值位置、最大值位置
  minMaxLoc(output, 0, &maxVal, 0, &maxLoc); // 结果矩阵中的数值代表正确结果的概率，寻找最大概率
  if (maxLoc.x == 10)
	  idResult[17] = 'X';
  else idResult[17] = (char)(maxLoc.x + '0');

  return idResult;
}

// 统计并打印识别率
void recognitionRate(const string &inputString, string &char_result, int numb) {
  int count = 0;
  string correct_result = inputString;

  // 统计
  for (int i = 0; i < 18; i++)
    if (correct_result[i] == char_result[i])
      count++;

  cout << "正确结果：";
  for (int i = 0; i < 18; i++)
    cout << correct_result[i] << " ";
  cout << endl;

  cout << "识别结果：";
  for (int i = 0; i < 18; i++) 
      cout << char_result[i] << " ";

  cout << endl;

  cout << endl;
  cout << "编号：" << numb << endl;
  cout << "总字符：18" << endl;
  cout << "正确字符：" << count << endl;
  cout << "识别率：" << count * 1.0 / 18 * 100 << "%" << endl;
}


// 身份证识别
string identifyIdCard(const Mat &inputImage) {
	Mat ImageGray = getResizeGrayImage(inputImage); // 获取灰度图,调整图大小

	vector<RotatedRect> rects;                  // 平面上的旋转矩形
	posDetect(ImageGray, rects);                // 身份证号码定位，并加黑线框
	Mat result;
	if (rects.size() != 0)
	  result = Cut_Area(ImageGray, rects[0]); // 裁剪身份证号码

	vector<Mat> dst_mat;
	splitCharacter(result, dst_mat); // 切割数字字符

	// 身份证识别
	Ptr<ANN_MLP> ann = ANN_MLP::load("ann/ann_param");    // 加载神经网络-多层感知器
	Ptr<ANN_MLP> annX = ANN_MLP::load("ann/ann_param_X"); // 加载神经网络-多层感知器

	string idResult = classifyCharacter(ann, annX, dst_mat); // 分类数字字符
	return idResult;
}
