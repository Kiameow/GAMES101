//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColorBilinear(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        // rounding to nearest integer
        auto u_left = std::floor(u_img);
        auto v_top = std::floor(v_img);
        auto u_center = std::min((u_img - u_left >= 0.5f ? u_left + 1 : u_left), float(width - 1));
        auto v_center = std::min((v_img - v_top >= 0.5f ? v_top + 1 : v_top), float(height - 1));

        auto color_top_left = image_data.at<cv::Vec3b>(v_center - 0.5f, u_center - 0.5f);
        auto color_top_right = image_data.at<cv::Vec3b>(v_center - 0.5f, u_center + 0.5f);
        auto color_bottom_left = image_data.at<cv::Vec3b>(v_center + 0.5f, u_center - 0.5f);
        auto color_bottom_right = image_data.at<cv::Vec3b>(v_center + 0.5f, u_center + 0.5f);

        // s is the interpolation factor of horizontal direction
        auto s = u_img - (u_center - 0.5f);
        // t is the interpolation factor of vertical direction
        auto t = v_img - (v_center - 0.5f);

        auto color = (1 - s) * (1 - t) * color_top_left + 
                    s * (1 - t) * color_top_right + 
                    (1 - s) * t * color_bottom_left + 
                    s * t * color_bottom_right;

        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

};
#endif //RASTERIZER_TEXTURE_H
