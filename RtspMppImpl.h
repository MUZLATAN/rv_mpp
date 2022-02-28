/*
 * @Author: your name
 * @Date: 2021-11-30 16:06:40
 * @LastEditTime: 2021-12-21 14:45:55
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /louts_oem/whcode/cv/algo_mini/include/RtspMppImpl.h
 */
//
// Created by z on 2020/5/8.
//

#pragma once

extern "C" {
#include <libavformat/avformat.h>
};

#include <memory>
#include <opencv2/core/core.hpp>

#include "MppDecode.h"
#include "Callback.h"


namespace whale {
namespace vision {


class RtspMppImpl {
 public:
	RtspMppImpl(const std::string &rtsp_addr)

			:    rtsp_addr_(rtsp_addr),
			inited_(false),
				pFormatCtx_(NULL),
				options_(NULL),
				packet_(NULL),
				videoindex_(-1),
				codec_id_(AV_CODEC_ID_NONE)
				// mppdec_(std::make_shared<MppDecode>(),
				{
		av_register_all();
		avformat_network_init();
	}

	~RtspMppImpl() {
		av_free(packet_);
		avformat_close_input(&pFormatCtx_);
	}

	int init(std::string rtsp_addr_);
	bool open(const std::string &rtsp_addr) {}

	void reopen() {}

	bool read(cv::Mat &image);
	void set(int option, int value) {}
	bool isOpened() {}
	void release() {}

	int getFrame(cv::Mat &image);

	void setDataCallback(const rtsp_callback_t &cb, const std::string p_sn) {
		MppDecode::getInstance()->setDataCallback(cb, p_sn);
		m_strSn = p_sn;
	}
	void restart();

 private:
    int inited_;
	AVFormatContext *pFormatCtx_;
	AVDictionary *options_;
	int videoindex_;
	AVPacket *packet_;
	enum AVCodecID codec_id_;
	// std::shared_ptr<MppDecode> mppdec_;
    int64_t last_decode_time_;
	const int mpp_exception_decode_time_window_ = 5 * 60 * 1000;
	const int decode_gop_time_window_ = 60 * 1000;
	std::string m_strSn;
	std::string rtsp_addr_;
};

}}