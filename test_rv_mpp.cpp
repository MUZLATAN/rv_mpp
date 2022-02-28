#include <chrono>
#include <memory>
#include <opencv2/opencv.hpp>
#include "RtspMppImpl.h"
#include <iostream>


class TestRVMPP{
public:
    TestRVMPP(){}
    ~TestRVMPP(){}

public:
    int dataCallback(cv::Mat& image){
        mImg = image.clone();
        return 0;
    }

public:
    cv::Mat mImg;

};

int main()
{

    TestRVMPP trvmpp;
    std::shared_ptr<whale::vision::RtspMppImpl> rtsp_impl_ = std::make_shared<whale::vision::RtspMppImpl>("");

    int ret = rtsp_impl_->init("rtsp://admin:admin@192.168.110.67/h264/ch39/main/av_stream");
    if (0 != ret ){
        std::cout<<"rtsp init failed"<<std::endl;
        return 0;
    }
    std::cout<<"hahahha"<<std::endl;

    rtsp_impl_->set(cv::CAP_PROP_BUFFERSIZE, 3);
    rtsp_impl_->set(cv::CAP_PROP_FPS, 15);
    rtsp_impl_->setDataCallback(std::bind(&TestRVMPP::dataCallback, &trvmpp, std::placeholders::_1), "WXX");

    int idx_count = 0;

    while(idx_count++ <10){
        cv::Mat img;
        bool ret = rtsp_impl_->read(img);
        std::cout<<trvmpp.mImg.cols<<"  "<<trvmpp.mImg.rows<<std::endl;
        
    }

    return 0;
}