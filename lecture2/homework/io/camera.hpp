#include "hikrobot/include/MvCameraControl.h"
#include <opencv2/opencv.hpp>

class Camera
{
public:     
    Camera();        
    cv::Mat read(unsigned int nMsec = 100);
    ~Camera();
private:
    void* handle_ = nullptr;
    bool opened_ = false;
    bool grabbing_ = false;
    cv::Mat transfer(MV_FRAME_OUT& raw);
    void cleanup();                   
};

