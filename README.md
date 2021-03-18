English


README.md

As the economy develops and the flow of people increases, identification and authentication of ID cards are becoming more frequent. This technology has played a significant role in facilitating people's lives and improving the quality of consumer experience. In the network environment such as large-scale public platforms, third-party payment platforms, and second-hand car trading platforms, user registration is an indispensable link, and at the same time, there are huge user groups for information input. OCR recognition technology based on mobile terminals has also played an important role in handling licenses and various other registration matters, solving the inefficiency of manually entering information in the process of user real-name registration.

Use process: 1. To obtain the ID card picture, you can choose the picture that has been stored in the computer, or you can open the camera to shoot on site. It should be noted here that when taking pictures, try to keep the ID facing the camera. 2. Click the recognition button, one-key recognition. The identification result is the original ID code, including birthplace, birthday, age, gender, and checksum (to check whether the ID number is legal).

The environment used in this project is as follows: 1. A computer with Win10 system installed, and the integrated development environment VS2017, computer vision library Opencv3.4.5 and Microsoft basic class library MFC are used. Different versions of the environment need to be adjusted accordingly, including the SDK version, platform toolset and additional dependencies. The version that comes with your computer shall prevail. 2. Equipped with a high-definition camera. If the resolution of the camera is too low, the recognition rate of the collected ID card will drop to a certain extent. 3. There are 5 ID card test samples in the IDCard folder, which are for learning purposes only. If there is any infringement, please contact to delete it. 4. The ann folder contains the trained neural network-multilayer perceptron, and Number_char.rar contains the training sample of the ID number, including the numbers 0-9, and the Roman numeral X, which can be trained by yourself. 5. cities.txt contains the first 6 digits of the country's provinces, cities and counties and their corresponding ID cards, which are used to match the place of birth.

The following is an introduction to the resource files: 1.headers.h contains most of the header files needed by the project, as well as the declaration of custom functions and the definition of classes. 2.training.cpp realizes the function of building and training a neural network, which includes calculating the gray image features and histogram features of the extracted samples. 3. The whole process of identification.cpp identification, including image preprocessing, ID number area positioning and digital character cutting, as well as character feature extraction. 4.identifyIdCardDlg.cpp realizes the process of extracting personal information, as well as the design of the visual interface, including buttons such as turning on the camera and taking pictures and their corresponding event processing functions.

Finally, I wish you 0error and 0warning!


随着经济发展和人流量增多，身份证的识别认证日趋频繁。这项技术在便捷人们的生活和提高消费者的体验质量上发挥了不小的作用。在大型公众平台、第三方支付平台、二手车交易平台等网络环境下，用户注册是必不可少的环节，同时又有庞大的用户群体来进行信息输入。在办理执照和其他各种登记的事务中，基于移动终端的OCR识别技术也发挥了重要的作用，解决了用户实名注册过程中手动录入信息的低效问题。

使用流程：
1.获取身份证图片，可以选择电脑中已经存储好的照片，也可以打开摄像头现场拍摄。这里需要注意的是，拍照时尽量让证件正对摄像头。
2.点击识别按钮，一键识别。识别结果为身份证原码，包括出生地，生日，年龄，性别，校验和（检验身份证号码是否合法）。

这个项目所使用的环境如下：
1.安装了Win10系统的电脑，且使用到了集成开发环境VS2017、计算机视觉库Opencv3.4.5和微软基础类库MFC。不同版本的环境需要作出对应的调整，包括SDK版本、平台工具集和附加依赖项，以您的电脑自带的版本为准。
2.配备高清摄像头，如果摄像头的分辨率过低，采集的身份证识别率会有一定程度的下降。
3.IDCard文件夹下有5张身份证测试样本，仅供学习之用，若有侵权，请联系删除。
4.ann文件夹下为训练好的神经网络-多层感知器，Number_char.rar里是身份证号码的训练样本，包括数字0-9，以及罗马数字X，可自行训练。
5.cities.txt包含了全国的省市县及其对应的身份证前6位号码，用于匹配出生地。

以下是资源文件的介绍：
1.headers.h 包含了项目需要的大部分头文件，还包括自定义函数的声明和类的定义。
2.training.cpp 实现了搭建并训练神经网络的功能，其中包括计算提取样本的灰度图特征和直方图特征。
3.identification.cpp识别的整个过程，包括图像的预处理，身份证号码区域的定位和数字字符的切割，也包括字符的特征提取。
4.identifyIdCardDlg.cpp 实现提取个人信息的过程，还有可视化界面的设计，包括打开摄像头和拍照等按钮及其对应的事件处理函数。

最后，祝你0error，0warning！








