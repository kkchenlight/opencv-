/*
 * @Author: kkchen
 * @Date: 2022-02-25 19:45:36
 * @LastEditors: kkchen
 * @LastEditTime: 2022-02-27 13:03:14
 * @Email: 1649490996@qq.com
 * @Description: 用于opencv进行等比例缩放使用
 */
#include<functional>
#include<vector>
#include<string>
#include<chrono>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace chrono;

void timeTest(function<void()>& fun, string& userData){
    auto start = steady_clock::now();
    fun();
    auto end   = steady_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    std::cout << userData << " cost: " << duration.count() / 1000.0 << " ms" << std::endl;
}


void opencvKernel(Mat& srcData, Mat& dstData, Size& srcSize, Size& dstSize){

    float scaleOri = srcSize.height * 1.0 / srcSize.width;
    float scaleDst = dstSize.height * 1.0 / dstSize.width;
    Size tmpSize;
    Rect tmpRect;
    Mat tmpMat;
    if(scaleOri > scaleDst){  
        tmpSize.height = dstSize.height;
        tmpSize.width  = srcSize.width * (dstSize.height * 1.0 / srcSize.height); 
        tmpRect.x = (dstSize.width - tmpSize.width) / 2;
        tmpRect.y = 0;
        tmpRect.width = tmpSize.width;
        tmpRect.height = tmpSize.height;
    }else{
        std::cout << "1 " << endl;
        tmpSize.width = dstSize.width;
        tmpSize.height = srcSize.height * (dstSize.width * 1.0 / srcSize.width);
        tmpRect.y = (dstSize.height - tmpSize.height) / 2;
        tmpRect.x = 0;
        tmpRect.width = tmpSize.width;
        tmpRect.height = tmpSize.height;
    }
    resize(srcData, tmpMat, tmpSize);
    addWeighted(dstData(tmpRect), 0, tmpMat, 1, 0, dstData(tmpRect));
}


void handedKernel(Mat& srcData, Mat& dstData, Size& srcSize, Size& dstSize)
{
    float scaleOri = srcSize.height * 1.0 / srcSize.width;
    float scaleDst = dstSize.height * 1.0 / dstSize.width;
    Size tmpSize;
    Rect tmpRect;
    Mat tmpMat;
    if(scaleOri > scaleDst){  
        tmpSize.height = dstSize.height;
        tmpSize.width  = srcSize.width * (dstSize.height * 1.0 / srcSize.height); 
        tmpRect.x = (dstSize.width - tmpSize.width) / 2;
        tmpRect.y = 0;
        tmpRect.width = tmpSize.width;
        tmpRect.height = tmpSize.height;
    }else{
        tmpSize.width = dstSize.width;
        tmpSize.height = srcSize.height * (dstSize.width * 1.0 / srcSize.width);
        tmpRect.y = (dstSize.height - tmpSize.height) / 2;
        tmpRect.x = 0;
        tmpRect.width = tmpSize.width;
        tmpRect.height = tmpSize.height;
    }
    resize(srcData, tmpMat, tmpSize);
    for(int i = 0; i < tmpSize.height; i++){
        uint8_t* dstPtr = dstData(tmpRect).data + i * dstData.step;
        uint8_t* srcPtr = tmpMat.data + i * tmpMat.step;
        memcpy(dstPtr, srcPtr, tmpMat.step);
    }
}




int main(int argc, char* argv[]){

    //get imgPath;
    string imgPath = argv[1];

    //get image width and height
    Mat oriMat = imread(imgPath);
    int imgHeight = oriMat.rows;
    int imgWidth  = oriMat.cols;
    Size oriSize = oriMat.size();
    
    //set scaled width and height
    Mat dstMat = Mat(1080, 1920, CV_8UC3);
    Size dstSize = dstMat.size();

    //lanuch the kernel
    function<void()> fun1 = bind(handedKernel, oriMat, dstMat, oriSize, dstSize);
    string exName = "opencvKernel";
    timeTest(fun1, exName);
    cout << "hit up done" << endl;
    timeTest(fun1, exName);
    
    /*function<void()> fun2 = bind(handedKernel, oriMat, dstMat, oriSize, dstSize);
    string exName2 = "handedKernel";
    timeTest(fun2, exName2);*/

    cv::imwrite("./resize.jpg", dstMat);
    return 0;
}
