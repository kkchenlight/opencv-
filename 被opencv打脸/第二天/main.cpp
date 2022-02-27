/*
 * @Author: kkchen
 * @Date: 2022-02-27 15:31:09
 * @LastEditors: kkchen
 * @LastEditTime: 2022-02-27 18:59:18
 * @Email: 1649490996@qq.com
 * @Description: 主要比较基于整形的缩放实验
 */
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <opencv2/opencv.hpp>
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

void handeFloatKernel(Mat& srcMat, Mat& dstMat, Size srcSize, Size dstSize){

    //get the scale
    float widthScale  = srcSize.width * (1.0 / dstSize.width);
    float heightScale = srcSize.height * (1.0 / dstSize.height);
    //start the loop
    for(int i = 0; i < dstSize.height; i++){
        float srcHeightIndex = i * heightScale;
        if(srcHeightIndex < 1){
            srcHeightIndex = 1;
        }
        for(int j = 0; j < dstSize.width; j++){
            float srcWidthIndex = j * widthScale;
            if(srcWidthIndex < 1){
                srcWidthIndex = 1;
            }
            uint8_t* dstPtr = dstMat.data + j + i * dstMat.step;
            
            //caculate the (x, y) -> (x+1, y) direction 
            int xyYIndex = floor(srcHeightIndex); // xyYIndex is the Y index of point(x, y)
            int xyXIndex = floor(srcWidthIndex);// xyXIndex is the X index of point(x,y)
            int x1yX1Index = ceil(srcWidthIndex); // x1yX1Index is the X index of point(x + 1, y)
            uint8_t* srcXY = srcMat.data + xyYIndex * srcMat.step + xyXIndex;
            uint8_t* srcX1Y = srcMat.data + xyYIndex * srcMat.step + x1yX1Index;
            uint8_t tmpXX1YAve = srcXY[0] + (srcX1Y[0] - srcXY[0]) * (srcWidthIndex - xyXIndex);

            //caculate the (x, y + 1) -> (x + 1, y + 1) direction
            int xy1Y1Index = ceil(srcHeightIndex); //xy1Y1Index is the Y index of point(x, y + 1);
            int xy1XIndex = floor(srcWidthIndex);// xy1XIndex is the X index of point(x, y + 1)
            int x1y1X1Index = ceil(srcWidthIndex);// x1yX1Index is the X1 index of point(x + 1, y + 1);
            uint8_t* srcXY1 = srcMat.data + xy1Y1Index * srcMat.step + xy1XIndex;
            uint8_t* srcX1Y1= srcMat.data + xy1Y1Index * srcMat.step + x1y1X1Index;
            uint8_t tmpXX1Y1Ave = srcXY1[0] + (srcX1Y[0] - srcX1Y1[0]) * (srcWidthIndex - x1y1X1Index);

            //caculate the tmpXX1Y1Ave and tmpXX1YAve for dstPtr
            dstPtr[0] = tmpXX1YAve + (tmpXX1Y1Ave - tmpXX1YAve) * (srcHeightIndex - xyYIndex);
        }
    }
}

void handeIntKernel(Mat& srcMat, Mat& dstMat, Size srcSize, Size dstSize){
    int32_t widthScale = srcSize.width * 1024 / dstSize.width;
    int32_t heightScale = srcSize.height * 1024/ dstSize.height;

     for(int i = 0; i < dstSize.height; i++){
        int32_t srcHeightTmpIndex = i * heightScale;
        int32_t srcHeightIndex = srcHeightTmpIndex >> 10;
        int32_t srcHeightDif   = srcHeightTmpIndex - srcHeightIndex * 1024;
        if(srcHeightIndex < 1){
            srcHeightIndex = 1;
        }
        for(int j = 0; j < dstSize.width; j++){
            int32_t srcWidthTmpIndex = j * widthScale;
            int32_t srcWidthIndex = srcWidthTmpIndex >> 10;
            int32_t srcWidthDif   = srcWidthTmpIndex - srcWidthIndex * 1024;
            if(srcWidthIndex < 1){
                srcWidthIndex = 1;
            }
            //cout << "srcHeightDif = " << srcHeightDif << "srcWidthDif = " << srcWidthDif << endl;
            uint8_t* dstPtr = dstMat.data + j + i * dstMat.step;   
            //caculate the (x, y) -> (x+1, y) direction 
            int xyYIndex = srcHeightIndex; // xyYIndex is the Y index of point(x, y)
            int xyXIndex = srcWidthIndex;// xyXIndex is the X index of point(x,y)
            int x1yX1Index = srcWidthIndex + 1; // x1yX1Index is the X index of point(x + 1, y)
            uint8_t* srcXY = srcMat.data + xyYIndex * srcMat.step + xyXIndex;
            uint8_t* srcX1Y = srcMat.data + xyYIndex * srcMat.step + x1yX1Index;
            uint8_t tmpXX1YAve = srcXY[0] + (((srcX1Y[0] - srcXY[0]) * srcWidthDif) >> 10);

            //caculate the (x, y + 1) -> (x + 1, y + 1) direction
            int xy1Y1Index = srcHeightIndex + 1; //xy1Y1Index is the Y index of point(x, y + 1);
            int xy1XIndex = srcWidthIndex;// xy1XIndex is the X index of point(x, y + 1)
            int x1y1X1Index = srcWidthIndex + 1;// x1yX1Index is the X1 index of point(x + 1, y + 1);
            uint8_t* srcXY1 = srcMat.data + xy1Y1Index * srcMat.step + xy1XIndex;
            uint8_t* srcX1Y1= srcMat.data + xy1Y1Index * srcMat.step + x1y1X1Index;
            uint8_t tmpXX1Y1Ave = srcXY1[0] + (((srcX1Y[0] - srcX1Y1[0]) * srcWidthDif) >> 10);

            //caculate the tmpXX1Y1Ave and tmpXX1YAve for dstPtr
            dstPtr[0] = tmpXX1YAve + (((tmpXX1Y1Ave - tmpXX1YAve) * srcHeightDif) >> 10);
        }
    }
}

int main(int argc, char* argv[]){
    
    //get input image and convert to gray
    string imgPath = argv[1];
    Mat oriMat     = imread(imgPath);
    Mat grayOriMat;
    cvtColor(oriMat, grayOriMat, COLOR_BGR2GRAY);

    //set the ori size and dst size;
    Size oriSize = cv::Size(grayOriMat.cols, grayOriMat.rows);
    Size dstSize = cv::Size(1920, 1920);
    cout << "oriSize" << oriSize << endl;

    //set dstMat
    Mat dstMat = Mat(dstSize, CV_8UC1);

    //bind the function
    function<void()> fun = bind(handeIntKernel, grayOriMat, dstMat, oriSize, dstSize);
    function<void()> fun1 = bind(handeFloatKernel, grayOriMat, dstMat, oriSize, dstSize);


    string exName1 = "hit up";
    timeTest(fun, exName1);

    string exName2 = "Int 实现";
    timeTest(fun, exName2);

    string exName3 = "Float实现";
    timeTest(fun1, exName3);


    cv::imwrite("./result.jpg", dstMat);
    cv::imwrite("./gray.jpg", grayOriMat);

    return 0;
}

