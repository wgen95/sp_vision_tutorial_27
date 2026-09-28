#include "camera.hpp"

Camera::Camera() 
{
    MV_CC_DEVICE_INFO_LIST device_list;
    int ret = MV_CC_EnumDevices(MV_USB_DEVICE, &device_list);
    if (ret != MV_OK) {
        throw std::runtime_error("MV_CC_EnumDevices failed: " + std::to_string(ret));
    }
    if (device_list.nDeviceNum == 0) {
        throw std::runtime_error("No camera device found.");
    }

    ret = MV_CC_CreateHandle(&handle_, device_list.pDeviceInfo[0]);
    if (ret != MV_OK) {
        throw std::runtime_error("MV_CC_CreateHandle failed: " + std::to_string(ret));
    }

    ret = MV_CC_OpenDevice(handle_);
    if (ret != MV_OK) {
        handle_ = nullptr;
        throw std::runtime_error("MV_CC_OpenDevice failed: " + std::to_string(ret));
    }
    opened_ = true;

    MV_CC_SetEnumValue(handle_, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_CONTINUOUS);
    MV_CC_SetEnumValue(handle_, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
    MV_CC_SetEnumValue(handle_, "GainAuto", MV_GAIN_MODE_OFF);
    MV_CC_SetFloatValue(handle_, "ExposureTime", 10000);
    MV_CC_SetFloatValue(handle_, "Gain", 20);
    MV_CC_SetFrameRate(handle_, 60);
    ret = MV_CC_StartGrabbing(handle_);
    if (ret != MV_OK) {
        MV_CC_CloseDevice(handle_);
        MV_CC_DestroyHandle(handle_);
        handle_ = nullptr;
        throw std::runtime_error("MV_CC_StartGrabbing failed: " + std::to_string(ret));          
    }
    grabbing_ = true;
}

cv::Mat Camera::read(unsigned int nMsec)
{
    MV_FRAME_OUT raw;
    int ret = MV_CC_GetImageBuffer(handle_, &raw, nMsec);
    if (ret != MV_OK) {
        return  cv::Mat();   
    }
    
    cv::Mat img =  transfer(raw);

    ret = MV_CC_FreeImageBuffer(handle_, &raw);
    if (ret != MV_OK) {
      return cv::Mat();
    }
    return img;
}

cv::Mat Camera::transfer(MV_FRAME_OUT& raw)
{
    MV_CC_PIXEL_CONVERT_PARAM cvt_param;
    cv::Mat img(cv::Size(raw.stFrameInfo.nWidth, raw.stFrameInfo.nHeight), CV_8U, raw.pBufAddr);

    cvt_param.nWidth = raw.stFrameInfo.nWidth;
    cvt_param.nHeight = raw.stFrameInfo.nHeight;

    cvt_param.pSrcData = raw.pBufAddr;
    cvt_param.nSrcDataLen = raw.stFrameInfo.nFrameLen;
    cvt_param.enSrcPixelType = raw.stFrameInfo.enPixelType;

    cvt_param.pDstBuffer = img.data;
    cvt_param.nDstBufferSize = img.total() * img.elemSize();
    cvt_param.enDstPixelType = PixelType_Gvsp_BGR8_Packed;

    auto pixel_type = raw.stFrameInfo.enPixelType;
    const static std::unordered_map<MvGvspPixelType, cv::ColorConversionCodes> type_map = {
      {PixelType_Gvsp_BayerGR8, cv::COLOR_BayerGR2RGB},
      {PixelType_Gvsp_BayerRG8, cv::COLOR_BayerRG2RGB},
      {PixelType_Gvsp_BayerGB8, cv::COLOR_BayerGB2RGB},
      {PixelType_Gvsp_BayerBG8, cv::COLOR_BayerBG2RGB}};
    cv::cvtColor(img, img, type_map.at(pixel_type));
    
    return img;
}
Camera::~Camera()
{
    if (handle_)
    {
        int ret = MV_CC_StopGrabbing(handle_);
        if (ret != MV_OK) {
            std::cout<<"StopGrabbing is wrong"<<std::endl;
            
        }
        grabbing_ = false;
        ret = MV_CC_CloseDevice(handle_);
        if (ret != MV_OK) {
            std::cout<<"CloseDevice is wrong"<<std::endl;    
        }
        opened_ = false;
        ret = MV_CC_DestroyHandle(handle_);
        if (ret != MV_OK) {
            std::cout<<"DestoryHandle is wrong"<<std::endl;
        }
        handle_ = nullptr;
    }   
}
     