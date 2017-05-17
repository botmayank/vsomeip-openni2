///////////////////////////////////////////////////////////////////////////////
//
// Simple program that reads depth and color (RGB) images from Primensense
// camera using OpenNI2 and displays them using OpenCV.
//
// Ashwin Nanjappa
///////////////////////////////////////////////////////////////////////////////

#include "grabber.hpp"

int main()
{
    Grabber grabber;
    grabber.InitOpenNI();
    grabber.InitDevice();
    grabber.InitDepthStream();
    grabber.InitColorStream();
    grabber.Run();

    return 0;
}
