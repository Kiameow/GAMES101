#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;
bool anti_aliasing = false;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    std::vector<cv::Point2f> remain_points = control_points;
    std::vector<cv::Point2f> new_points;
    while (remain_points.size() > 1) 
    {
        new_points.clear();
        for(int i = 0; i < remain_points.size() - 1; i++) 
        {
            const cv::Point2f new_point = remain_points[i] * (1 - t) + remain_points[i + 1] * t;
            new_points.push_back(new_point);
        }
        remain_points = new_points;
    }
    
    return remain_points[0];

}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm
    for (float t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = recursive_bezier(control_points, t);
        if (anti_aliasing)
        {
            const auto base_x = std::floor(point.x);
            const auto base_y = std::floor(point.y);
            const auto x_diff = point.x - base_x;
            const auto y_diff = point.y - base_y;

            const auto min_x = x_diff < 0.5f ? base_x - 1 : base_x;
            const auto min_y = y_diff < 0.5f ? base_y - 1 : base_y;
            const auto max_x = x_diff < 0.5f ? base_x : base_x + 1;
            const auto max_y = y_diff < 0.5f ? base_y : base_y + 1;

            std::queue<float> dist_squared_queue;
            float min_dist_squared = 2.0f;
            for (const auto x : {min_x, max_x})
            {
                for (const auto y : {min_y, max_y})
                {
                    const auto dist_squared = 
                        std::pow(x - point.x, 2) + 
                        std::pow(y - point.y, 2);
                    dist_squared_queue.push(dist_squared);
                    if (dist_squared < min_dist_squared)
                    {
                        min_dist_squared = dist_squared;
                    }
                }
            }
            
            for (const auto x : {min_x, max_x})
            {
                for (const auto y : {min_y, max_y})
                {
                    const auto intensity = min_dist_squared / dist_squared_queue.front();
                    dist_squared_queue.pop();
                    window.at<cv::Vec3b>(y, x)[1] = std::max( static_cast<uchar>(255 * intensity), 
                                                              window.at<cv::Vec3b>(y, x)[1]);
                }
            }
        } else {
            window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
        }
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);
    bool show_control_points = true;

    int key = -1;
    // key 27 means ESC
    while (key != 27) 
    {
        window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
        cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
        // key 114 means r, revert the last step
        if (key == 114)
        {
            control_points.pop_back();            
        }
        // key 99 means c, clear all points
        if (key == 99)
        {
            control_points.clear();
        }
        // key 104 means h, toggle the visibility of the control points
        if (key == 104)
        {
            show_control_points = !show_control_points;
        }
        // key 101 means e, enable the anti-aliasing    
        if (key == 101)
        {
            anti_aliasing = !anti_aliasing;
        }

        if (show_control_points)
        {
            for (auto &point : control_points) 
            {
                cv::circle(window, point, 3, {255, 255, 255}, 3);
            }
        }

        if (control_points.size() >= 4) 
        {
            //naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve_anti_aliasing.png", window);
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
