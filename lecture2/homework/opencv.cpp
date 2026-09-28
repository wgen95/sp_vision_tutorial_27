#include "io/camera.hpp"
#include "tasks/yolo.hpp"
#include "opencv2/opencv.hpp"
#include "tools/img_tools.hpp"
#include<iostream>
int main()
{
    // 初始化相机、yolo类
    Camera my_cam;
    auto_aim:: YOLO s("../configs/yolo.yaml") ;
    std:: cout <<"hello,world"<<std::endl;
    while (1) {
        // 调用相机读取图像
        
        unsigned int nMsec = 100;
        cv::Mat img = my_cam.read(nMsec);
        if (img.empty()) {
            continue; // 如果图像为空，跳过
        }
        // 调用yolo识别opencv标志
        auto  armors = s.detect(img);

        if (!armors.empty()) 
        { // 💡 非常重要：一定要判断是否为空，否则后续取数据会崩溃
            for (const auto& armor : armors)    
            { 
                cv::Point2f top_left = armor.points[0];
                cv::Point2f top_right = armor.points[3];
                cv::Point2f bottom_left = armor.points[1];
                cv::Point2f top_center = (top_left + top_right) / 2.0f;
                cv::Point2f half_left_diff = (bottom_left - top_left) / 2.0f;

                cv::Point2f I_need = top_center + half_left_diff; 
                //std::cout<<armor.points<<std::endl;
                cv::Point text_pos(static_cast<int>(I_need.x), static_cast<int>(I_need.y));
                std::string info = armor.color + armor.name;

                tools::draw_points(img, armor.points, {0, 255, 0}); // 画装甲板关键点
                tools::draw_text(img, ,info, text_pos, {0, 255, 0});  
                std::cout << "识别到一个装甲板！" << std::endl;
            }
        } 
        else {
             std::cout << "当前画面没有装甲板" << std::endl;
        }

        cv::resize(img, img , cv::Size(640, 480));
        cv::imshow("img", img);
        if (cv::waitKey(0) == 'q') 
            break;
        
        // 显示图像
        // cv::resize(img, img , cv::Size(640, 480));
        // cv::imshow("img", img);
        // if (cv::waitKey(0) == 'q') {
        //     // break;
        // }
    // }

    return 0;
}