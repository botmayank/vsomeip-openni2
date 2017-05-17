#ifndef _GRABBER_H_
#define _GRABBER_H_

#include <opencv2/opencv.hpp>
#include <OpenNI.h>

class Grabber
{
public:
    void InitOpenNI();
    void InitDevice();
    void InitDepthStream();
    void InitColorStream();
    void setColorStreamCrop(int originX, int originY, int width, int height);
    int getCameraGain();
    void Run();

private:
    void CapturePsenseDepthFrame();
    void CapturePsenseColorFrame();
    cv::Mat ChangeDepthForDisplay(const cv::Mat& mat);

    openni::Device*        device_;
    openni::VideoStream*   depth_stream_;
    openni::VideoStream*   color_stream_;
    openni::VideoFrameRef* depth_frame_;
    openni::VideoFrameRef* color_frame_;
};


#endif //GRABBER_HPP