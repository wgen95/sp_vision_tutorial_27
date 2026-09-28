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
    while (1) {
        // 调用相机读取图像
        
        unsigned int nMsec = 100;
        cv::Mat img = my_cam.read(nMsec);
        if (img.empty()) {
            continue; 
        }
        
        //cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);
        
        auto  armors = s.detect(img);

        //cv::Mat display_img;
        //cv::cvtColor(gray_img, display_img, cv::COLOR_GRAY2BGR);
        if (!armors.empty()) 
        { 
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
                std::string color_str =auto_aim:: COLORS[armor.color];
                std::string armor_str = auto_aim::ARMOR_NAMES[armor.name];
                std::string info =  color_str + " " + armor_str  ;

                tools::draw_points(img, armor.points, {0, 255, 0},3); // 画装甲板关键点
                tools::draw_text(img, info, text_pos, {0, 255, 0});  
                std::cout << "识别到一个装甲板！" << std::endl;
            }
        } 
        else {
             std::cout << "当前画面没有装甲板" << std::endl;
        }

        cv::resize(img, img , cv::Size(640, 480));
        cv::imshow("img", img);

        // cv::resize(img, img , cv::Size(640, 480));
        // cv::imshow("img", img);
        if (cv::waitKey(1) == 'q') {
                 break;
        }

    }

    return 0;
}