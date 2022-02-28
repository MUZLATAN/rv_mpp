#pragma once

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <opencv2/opencv.hpp>


namespace whale {
namespace vision {
using rtsp_callback_t = std::function<void(cv::Mat&)>;

}	// namespace vision
}	// namespace whale