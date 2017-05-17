#include "grabber.hpp"

void Grabber::InitOpenNI()
{
    auto rc = openni::OpenNI::initialize();
    if (rc != openni::STATUS_OK)
    {
        printf("Initialize failed\n%s\n", openni::OpenNI::getExtendedError());
        exit(0);
    }
}

void Grabber::InitDevice()
{
    device_ = new openni::Device();
    auto rc = device_->open(openni::ANY_DEVICE);
    if (rc != openni::STATUS_OK)
    {
        printf("Couldn't open device\n%s\n", openni::OpenNI::getExtendedError());
        exit(0);
    }
}

void Grabber::setColorStreamCrop(int originX, int originY, int width, int height)
{
    if(bool can_crop = color_stream_->isCroppingSupported()){
        printf("Color stream can be cropped!\n");    
    }
    else{
        printf("Color stream can not be cropped!");
        exit(0);
    }
    auto rc = color_stream_->setCropping(originX, originY, width, height);
    if (rc != openni::STATUS_OK){
        printf("Couldn't crop!\n%s\n", openni::OpenNI::getExtendedError());
        exit(0);
    }   
}

void Grabber::getCameraGain()
{
    openni::CameraSettings* settings = color_stream_->getCameraSettings();
    if (settings != nullptr) {
        printf("Gain of RGB camera is: %d\n", settings->getGain());
    }
    else{        
        printf("Camera Settings yielded nullptr\n");
        exit(0);
    }    
}

void Grabber::InitDepthStream()
{
    depth_stream_ = new openni::VideoStream();

    // Create depth stream from device
    if (device_->getSensorInfo(openni::SENSOR_DEPTH) != nullptr)
    {
        auto rc = depth_stream_->create(*device_, openni::SENSOR_DEPTH);
        if (rc != openni::STATUS_OK)
        {
            printf("Couldn't create depth stream\n%s\n", openni::OpenNI::getExtendedError());
            exit(0);
        }
    }

    // Get info about depth sensor
    const openni::SensorInfo& sensor_info       = *device_->getSensorInfo(openni::SENSOR_DEPTH);
    const openni::Array<openni::VideoMode>& arr = sensor_info.getSupportedVideoModes();

    // Look for VGA mode in depth sensor and set it for depth stream
    for (int i = 0; i < arr.getSize(); ++i)
    {
        const openni::VideoMode& vmode = arr[i];
        if (vmode.getPixelFormat() == openni::PIXEL_FORMAT_DEPTH_1_MM &&
            vmode.getResolutionX() == 640 &&
            vmode.getResolutionY() == 480)
        {
            depth_stream_->setVideoMode(vmode);
            break;
        }
    }

    // Start the depth stream
    auto rc = depth_stream_->start();
    if (rc != openni::STATUS_OK)
    {
        printf("Couldn't start the depth stream\n%s\n", openni::OpenNI::getExtendedError());
        exit(0);
    }

    depth_frame_ = new openni::VideoFrameRef();
}

void Grabber::InitColorStream()
{
    color_stream_ = new openni::VideoStream();

    if (device_->getSensorInfo(openni::SENSOR_COLOR) != nullptr)
    {
        auto rc = color_stream_->create(*device_, openni::SENSOR_COLOR);
        if (rc != openni::STATUS_OK)
        {
            printf("Couldn't create color stream\n%s\n", openni::OpenNI::getExtendedError());
            exit(0);
        }
    }

    // Get info about color sensor
    const openni::SensorInfo& sensor_info       = *device_->getSensorInfo(openni::SENSOR_COLOR);
    const openni::Array<openni::VideoMode>& arr = sensor_info.getSupportedVideoModes();

    // Look for VGA mode and set it for color stream
    for (int i = 0; i < arr.getSize(); ++i)
    {
        const openni::VideoMode& vmode = arr[i];
        if (
            vmode.getResolutionX() == 640 &&
            vmode.getResolutionY() == 480)
        {
            color_stream_->setVideoMode(vmode);
            break;
        }
    }

    // Note: Doing image registration earlier than this seems to fail
    if (device_->isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR))
    {
        auto rc = device_->setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
        if (rc == openni::STATUS_OK)
            std::cout << "Depth to color image registration set success\n";
        else
            std::cout << "Depth to color image registration set failed\n";
    }
    else
    {
        std::cout << "Depth to color image registration is not supported!!!\n";
    }
        

    // Start color stream
    auto rc = color_stream_->start();
    if (rc != openni::STATUS_OK)
    {
        printf("Couldn't start the color stream\n%s\n", openni::OpenNI::getExtendedError());
        exit(0);
    }

    color_frame_ = new openni::VideoFrameRef();
}

void Grabber::CapturePsenseDepthFrame()
{
    auto rc = depth_stream_->readFrame(depth_frame_);
    if (rc != openni::STATUS_OK)
    {
        printf("Read failed!\n%s\n", openni::OpenNI::getExtendedError());
    }

    if (depth_frame_->getVideoMode().getPixelFormat() != openni::PIXEL_FORMAT_DEPTH_1_MM && depth_frame_->getVideoMode().getPixelFormat() != openni::PIXEL_FORMAT_DEPTH_100_UM)
    {
        printf("Unexpected frame format\n");
    }

    // Get pointer to Primesense depth frame
    openni::DepthPixel* dev_buf_ptr = (openni::DepthPixel*) depth_frame_->getData();

    // Copy frame data to OpenCV mat
    cv::Mat depth_mat(depth_frame_->getHeight(), depth_frame_->getWidth(), CV_16U, dev_buf_ptr);

    cv::Mat disp_mat = ChangeDepthForDisplay(depth_mat);

    cv::imshow("Depth", disp_mat);
}

void Grabber::CapturePsenseColorFrame()
{
    // Read from stream to frame
    auto rc = color_stream_->readFrame(color_frame_);
    if (rc != openni::STATUS_OK)
    {
        printf("Read failed!\n%s\n", openni::OpenNI::getExtendedError());
    }

    // Pointer to Primesense color frame
    openni::RGB888Pixel* dev_buf_ptr = (openni::RGB888Pixel*) color_frame_->getData();

    // Make mat from camera data
    cv::Mat color_mat(color_frame_->getHeight(), color_frame_->getWidth(), CV_8UC3, dev_buf_ptr);
    // Convert to BGR format for OpenCV
    cv::cvtColor(color_mat, color_mat, CV_RGB2BGR);

    cv::imshow("Color", color_mat);
}

void Grabber::Run()
{
    openni::VideoStream* streams[] = {depth_stream_, color_stream_};
    
   while (true)
   {
       int readyStream = -1;
       
       auto rc = openni::OpenNI::waitForAnyStream(streams, 2, &readyStream, 2000);
       if (rc != openni::STATUS_OK)
       {
           printf("Wait failed! (timeout is %d ms)\n%s\n", 2000, openni::OpenNI::getExtendedError());
           break;
       }

       switch (readyStream)
       {
       case 0:
        CapturePsenseDepthFrame();
           break;
       case 1:
        CapturePsenseColorFrame();
           break;
       default:
           printf("Unxpected stream\n");
       }

        char c = cv::waitKey(10);
        if ('q' == c)
            break;
        else if ('g' == c)
            this->getCameraGain();
            
   }
}

cv::Mat Grabber::ChangeDepthForDisplay(const cv::Mat& mat)
{
    assert(CV_16U == mat.type());

    const float depth_near = 500;
    const float depth_far  = 5000;

    const float alpha = 255.0 / (depth_far - depth_near);
    const float beta  = - depth_near * alpha;

    cv::Mat fmat;
    mat.convertTo(fmat, CV_32F);

    for (int r = 0; r < mat.rows; ++r)
    {
        for (int c = 0; c < mat.cols; ++c)
        {
            float v = fmat.at<float>(r, c) * alpha + beta;
            
            if (v > 255) v = 255;
            if (v < 0)   v = 0;

            fmat.at<float>(r, c) = v;
        }
    }

    cv::Mat bmat;
    fmat.convertTo(bmat, CV_8U);

    cv::Mat cmat;
    cv::cvtColor(bmat, cmat, CV_GRAY2BGR);
    cv::applyColorMap(cmat, cmat, cv::COLORMAP_OCEAN);

    return cmat;
}