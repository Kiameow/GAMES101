// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <iostream>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    const auto A = _v[0];
    const auto B = _v[1];
    const auto C = _v[2];
    const auto z = _v[0].z();

    const auto vecAB = B - A;
    const auto vecBC = C - B;
    const auto vecCA = A - C;

    const auto vecAP = Vector3f(x, y, z) - A;
    const auto vecBP = Vector3f(x, y, z) - B;
    const auto vecCP = Vector3f(x, y, z) - C;

    const auto crossAB = vecAB.cross(vecAP);
    const auto crossBC = vecBC.cross(vecBP);
    const auto crossCA = vecCA.cross(vecCP);

    if ((crossAB.z() < 0 && crossBC.z() < 0 && crossCA.z() < 0) || 
        (crossAB.z() > 0 && crossBC.z() > 0 && crossCA.z() > 0) )
    {
        return true;
    } 
    else 
    {
        return false;
    }

}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    auto xMin = int(INFINITY);
    auto xMax = int(-INFINITY);
    auto yMin = int(INFINITY);
    auto yMax = int(-INFINITY);

    for (int i = 0; i < 3; ++i)
    {
        xMin = xMin > std::floor(v[i].x()) ? std::floor(v[i].x()) : xMin;
        xMax = xMax < std::ceil(v[i].x()) ? std::ceil(v[i].x()) : xMax;
        yMin = yMin > std::floor(v[i].y()) ? std::floor(v[i].y()) : yMin;
        yMax = yMax < std::ceil(v[i].y()) ? std::ceil(v[i].y()) : yMax;
    }

    std::vector<std::vector<Eigen::Vector3f>> MSAA_frame_buf(
        width * height, 
        std::vector<Eigen::Vector3f>(MSAA_times * MSAA_times, Eigen::Vector3f{0, 0, 0})
    );

    
    for (int x = xMin; x <= xMax; ++x) 
    {
        for (int y = yMin; y <= yMax; ++y) 
        {
            if (insideTriangle(x + 0.5f, y + 0.5f, t.v))
            {
                // get the interpolated z value
                // need to judge if the z_buffer(depth_buf) is smaller than current pixel
                // need to get the color of the triangle at the current pixel
                // pass the point position and the color to the set_pixel function
                auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;

                if (z_interpolated < depth_buf[get_index(x, y)])
                {
                    for (int i = 0; i < MSAA_times * MSAA_times; ++i)
                    {
                        float cx = x + 1.0f / MSAA_times * (i % MSAA_times) + 1.0f / MSAA_times / 2;
                        float cy = y + 1.0f / MSAA_times * (i / MSAA_times) + 1.0f / MSAA_times / 2;

                        if (insideTriangle(cx, cy, t.v))
                        {
                            MSAA_frame_buf[get_index(x, y)][i] = t.getColor();
                        }
                    }
                    depth_buf[get_index(x, y)] = z_interpolated;
                    frame_buf[get_index(x, y)] = calc_color(MSAA_frame_buf[get_index(x, y)]);
                }
                set_pixel(Eigen::Vector3f(x, y, z_interpolated), frame_buf[get_index(x, y)]);
            }
        }
    }

    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
}

void rst::rasterizer::set_MSAA_times(int times)
{
    MSAA_times = times;
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

Eigen::Vector3f rst::rasterizer::calc_color(std::vector<Eigen::Vector3f>& colors)
{
    Eigen::Vector3f color = Eigen::Vector3f{0, 0, 0};
    for (auto& col : colors)
    {
        color.x() += col.x();
        color.y() += col.y();
        color.z() += col.z();
    }
    color /= colors.size();
    return color;
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on