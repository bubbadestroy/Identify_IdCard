#include "headers.h"

// 获取训练矩阵和标签矩阵，并保存进xml文件
void getAnnXML() {
  FileStorage fs("ann_xml.xml", FileStorage::WRITE);
  if (!fs.isOpened()) {
    cout << "Failed to open xml file." << endl;
  }

  Mat trainData;                             // 训练矩阵
  Mat classes = Mat::zeros(550, 1, CV_8UC1); // 标签矩阵,数字0-9，X 每种50个样本
  Mat img_read;
  Mat dst_feature; // 存放训练样本的特征
  char path[90];   // 存放训练样本的文件路径

  // X
  for (int i = 0; i < 11; i++) {
    for (int j = 1; j <= 50; ++j) {
      sprintf_s(path, "Number_char/%d/%d (%d).png", i, i, j);
      img_read = imread(path, 1);
      cvtColor(img_read, img_read, COLOR_BGR2GRAY); // RGB转灰度
      calcGradientFeat(img_read, dst_feature);      // 计算样本的特征

      trainData.push_back(dst_feature);      // 特征提取至训练矩阵
      classes.at<uchar>(i * 50 + j - 1) = i; // 标签提取至标签矩阵
    }
  }
  cout << "特征计算及提取完成！" << endl;

  // 训练矩阵和标签矩阵写入xml文件
  fs << "TrainingData" << trainData;
  fs << "classes" << classes;
  fs.release(); // 析构类对象，并关闭文件

  cout << "训练矩阵和标签矩阵写入xml文件完成！" << endl;
}

// 计算样本的特征
void calcGradientFeat(const Mat &imgSrc, Mat &out) {
  vector<float> feat;

  // 第二类特征-灰度值比
  // 8 * 2个特征值
  Mat image;
  resize(imgSrc, image, Size(8, 16)); // 图片统一大小，8列16行

  // 滤波去噪，x方向和y方向
  float mask[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}}; // Box模板
  Mat y_mask = Mat(3, 3, CV_32F, mask) / 8;                // 除8是为了归一化
  Mat x_mask = y_mask.t();                                 // 转置

  // 卷积运算，参数分别为：输入图像、输出图像、目标图像深度、卷积核
  Mat sobelX, sobelY;
  filter2D(image, sobelX, CV_32F, x_mask);
  filter2D(image, sobelY, CV_32F, y_mask);

  sobelX = abs(sobelX);
  sobelY = abs(sobelY);

  // 计算整个图像的灰度值之和
  float totleValueX = sumMatValue(sobelX);
  float totleValueY = sumMatValue(sobelY);

  // 图像划分为2*4共8个格子，每个格子4*4个像素点，计算每个格子灰度值和与图像灰度值之比
  Mat subImageX;
  Mat subImageY;
  for (int i = 0; i < image.rows; i = i + 4) {
    for (int j = 0; j < image.cols; j = j + 4) {
      subImageX = sobelX(Rect(j, i, 4, 4));
      feat.push_back(sumMatValue(subImageX) / totleValueX);
      subImageY = sobelY(Rect(j, i, 4, 4));
      feat.push_back(sumMatValue(subImageY) / totleValueY);
    }
  }

  // 第一类特征-灰度值和
  // 32个特征值
  Mat imageGray;
  cv::resize(imgSrc, imageGray, Size(4, 8)); // 图片统一大小，4列8行
  Mat p = imageGray.reshape(1, 1);           // reshape(int cn, int rows=0) cn表示通道数量，rows表示矩阵行数
  p.convertTo(p, CV_32FC1);                  // 类型转换

  for (int i = 0; i < p.cols; i++) {
    feat.push_back(p.at<float>(i));
  }

  // 第三类特征-水平、垂直直方图
  // 8+16个特征
  Mat vhist = projectHistogram(imgSrc, 1); // 水平直方图
  Mat hhist = projectHistogram(imgSrc, 0); // 垂直直方图
  for (int i = 0; i < vhist.cols; i++)
    feat.push_back(vhist.at<float>(i));
  for (int i = 0; i < hhist.cols; i++)
    feat.push_back(hhist.at<float>(i));

  // 写入
  out = Mat::zeros(1, (int)feat.size(), CV_32F);
  for (int i = 0; i < feat.size(); i++)
    out.at<float>(i) = feat[i];
}

// 计算灰度值和
float sumMatValue(const Mat &image) {
  float sumValue = 0;
  int r = image.rows;
  int c = image.cols;
  if (image.isContinuous()) // 若内存相连，则置为一行多列
  {
    c = r * c;
    r = 1;
  }

  for (int i = 0; i < r; i++) {
    const uchar *linePtr = image.ptr<uchar>(i);
    for (int j = 0; j < c; j++) {
      sumValue += linePtr[j];
    }
  }

  return sumValue;
}

// 计算直方图
Mat projectHistogram(const Mat &img, int t) {
  Mat lowData;
  cv::resize(img, lowData, Size(8, 16));
  int sz = (t) ? lowData.rows : lowData.cols; // t=1计算水平，t=0计算垂直
  Mat mhist = Mat::zeros(1, sz, CV_32F);

  for (int j = 0; j < sz; j++) {
    Mat data = (t) ? lowData.row(j) : lowData.col(j);
    mhist.at<int>(j) = countNonZero(data); // 返回灰度值不为0的像素数。黑底白字，即属于数字的像素点总数
  }

  double min, max;
  minMaxLoc(mhist, &min, &max); // 寻找最大最小值
  // 数据类型转换，参数分别为：目标矩阵、目标矩阵和源矩阵数据类型一致，尺度变换因子（归一化），尺度变换后的偏移量
  if (max > 1)
    mhist.convertTo(mhist, -1, 1.0f / max, 0);

  return mhist;
}

// 训练神经网络
void trainAnn(int nlayers, int numCharacters) {
  Ptr<ANN_MLP> ann = ANN_MLP::create(); // 创建神经网络-多层感知器

  // 从文件获取训练矩阵，标签矩阵
  Mat trainData, classes;
  FileStorage fs;
  fs.open("ann_xml.xml", FileStorage::READ);
  fs["TrainingData"] >> trainData;
  fs["classes"] >> classes;

  Mat layerSizes(1, 3, CV_32SC1);         // 3层神经网络
  layerSizes.at<int>(0) = trainData.cols; // 输入层的神经元结点数-每个样本的特征数量
  layerSizes.at<int>(1) = nlayers;        // 隐藏层的神经元结点数-
  layerSizes.at<int>(2) = numCharacters;  // 输出层的神经元结点数-0~9,X

  ann->setLayerSizes(layerSizes);
  ann->setTrainMethod(ANN_MLP::BACKPROP, 0.1, 0.1);                       // 反向传播算法、权梯度项强度（一般为0.1）、动量项强度（一般为0.1）
  ann->setActivationFunction(ANN_MLP::SIGMOID_SYM);                       // 激活函数
  ann->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 5000, 0.01)); // 迭代算法的终止准则，后两项参数为最大迭代次数和结果精确度

  // 为输出结果贴上标签
  Mat trainClasses;
  trainClasses.create(trainData.rows, numCharacters, CV_32FC1);
  for (int i = 0; i < trainData.rows; i++) // 每一个训练样本
  {
    for (int k = 0; k < trainClasses.cols; k++) // 每一种可能的结果
    {
      if (k == (int)classes.at<uchar>(i)) // 训练样本的标签与结果对应
      {
        trainClasses.at<float>(i, k) = 1;
      }

      else
        trainClasses.at<float>(i, k) = 0;
    }
  }

  cout << "正在训练。。。" << endl;
  ann->train(trainData, ml::ROW_SAMPLE, trainClasses);
  cout << "训练完成！！！" << endl;
  ann->save("/ann/ann_param_X"); // 保存MLP为可执行文件
}
