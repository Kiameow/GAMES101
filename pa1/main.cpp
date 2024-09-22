#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 
                 0, 1, 0, -eye_pos[1], 
                 0, 0, 1, -eye_pos[2], 
                 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    const double rotation_radians = MY_PI * rotation_angle / 180.0;
    model << cos(rotation_radians), -sin(rotation_radians),  0,  0, 
              sin(rotation_radians), cos(rotation_radians),  0,  0, 
              0,                     0,                      1,  0, 
              0,                     0,                      0,  1;
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    const auto n = zNear;
    const auto f = zFar;
    const auto t = tan(MY_PI * eye_fov / 180.0 / 2.0) * n;
    const auto r = aspect_ratio * t;
    const auto l = -r;
    const auto b = -t;

    Eigen::Matrix4f perspToOrtho;
    perspToOrtho << n,          0,         0,        0,
                    0,          n,         0,        0,
                    0,          0,         n+f,      - n*f,
                    0,          0,         1,        0;
    Eigen::Matrix4f orthoProj;
    orthoProj << 2/(r-l),   0,         0,       -(r+l)/(r-l),
                 0,         2/(t-b),   0,       -(t+b)/(t-b),
                 0,         0,         2/(n-f), -(f+n)/(f-n),
                 0,         0,         0,       1;

    projection = orthoProj * perspToOrtho;
    return projection;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the rotation matrix for the given axis and angle.
    // Then return it.
    const auto unitAxis = axis.normalized();
    const auto nX = unitAxis[0];
    const auto nY = unitAxis[1];
    const auto nZ = unitAxis[2];
    const auto radians = MY_PI * angle / 180.0;

    Eigen::Matrix3f K;
    K << 0, -nZ, nY,
         nZ, 0, -nX,
         -nY, nX, 0;

    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();

    Eigen::Matrix3f RodriguesRotation = I + sin(radians) * K + (1 - cos(radians)) * K * K;
    rotation.topLeftCorner<3, 3>() = RodriguesRotation;

    return rotation;
}

void setAxis(Eigen::Vector3f& axis, float x, float y, float z) 
{
    axis[0] = x;
    axis[1] = y;
    axis[2] = z;
}

int main(int argc, const char** argv)
{
    // std::cout << "tan45: " << tan(45) << "\n";
    // std::cout << "sin30: " << sin(30) << "\n";
    // std::cout << "cos60: " << cos(60) << "\n";
    // std::cout << "tan45: " << tan(MY_PI / 4) << "\n";
    bool ENTER_ANY_AXIS_MODE = false;
    float angle = 0;
    float deltaAngle = 10;
    bool command_line = false;
    std::string filename = "output.png";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--anyaxis") {
            std::cout << "Entering any axis mode" << std::endl;
            ENTER_ANY_AXIS_MODE = true;
            break;
        }
    }
    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    float axisX, axisY, axisZ = 0.0f;
    if (ENTER_ANY_AXIS_MODE) {
        std::cout << "Please enter the rotation axis (seperated by space): " << std::endl;
        std::cin >> axisX >> axisY >> axisZ;
    }
    Vector3f axis = {axisX, axisY, axisZ};

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (ENTER_ANY_AXIS_MODE) {
        r.set_model(get_rotation(axis, angle));
    } else {
        r.set_model(get_model_matrix(angle));
    }
    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            if (ENTER_ANY_AXIS_MODE) 
                r.update_model_delta(get_rotation(axis, -deltaAngle));
            else
                r.update_model_delta(get_model_matrix(-deltaAngle));
        }
        else if (key == 'd') {
            if (ENTER_ANY_AXIS_MODE) 
                r.update_model_delta(get_rotation(axis, deltaAngle));
            else
                r.update_model_delta(get_model_matrix(deltaAngle));
        }
        else if (key == 's') {
            if (ENTER_ANY_AXIS_MODE) {
                std::cout << "Please enter the rotation axis (seperated by space): " << std::endl;
                std::cin >> axisX >> axisY >> axisZ;
                setAxis(axis, axisX, axisY, axisZ);
            }
        }
    }

    return 0;
}