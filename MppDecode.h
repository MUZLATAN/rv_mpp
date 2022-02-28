//
// Created by z on 2020/5/8.
//

#pragma once


#include <string.h>
#include <map>

#include "utils.h"
#include "rk_mpi.h"
#include "rk_mpi_cmd.h"
#include "rk_vdec_cfg.h"
#include "mpp_log.h"
#include "mpp_mem.h"
#include "mpp_env.h"
#include "mpp_time.h"
#include "mpp_common.h"
#include "mpp_frame.h"
#include "mpp_buffer_impl.h"
#include "mpp_frame_impl.h"
#include "mpp_trie.h"
#include "mpp_dec_cfg.h"

#include <libavformat/avformat.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <mutex>
#include "Callback.h"


typedef void* MppTrie;

typedef struct MppDecCfgImpl_t {
    RK_S32              size;
    MppTrie             api;
    MppDecCfgSet        cfg;
} MppDecCfgImpl;



#define ROK 0
#define RERR -1

#define MAX_DEC_ERROR_NUM (30) // 错误帧数超过当前值则重新初始化MPP
#define MAX_DEC_ERROR_TRY_NUM (5) // 超过当前次数未能成功发送，则丢弃当前帧

namespace whale {
namespace vision {
class MppDecode {
  private:
	MppDecode()
			: ctx(NULL),
				mpi(NULL),
				eos(0),
				packet(NULL),
				frame(NULL),
				param(NULL),
				need_split(1),
				type(MPP_VIDEO_CodingAVC),
				frm_grp(NULL),
				pkt_grp(NULL),
				mpi_cmd(MPP_CMD_BASE),
				frame_count(1),
				frame_num(0),
				max_usage(0),
				fp_output(NULL),
				m_nDecPutErrNum(0),
				m_bInit(false)  {}
	~MppDecode() { deInitMpp(); }

 public:
	static MppDecode* getInstance() {
		static MppDecode Decode;
		return &Decode;
	}

	/**
	 * set type for h264 or h265 decode
	 * MPP_VIDEO_CodingAVC  ---> h264
	 * MPP_VIDEO_CodingHEVC ---> h265
	 */
	void setMppDecodeType(MppCodingType t) { type = t; }
	void setMppFp(FILE* fp) { fp_output = fp; }

	int initMpp();
	int initMppMtx();

	int decode(AVPacket* av_packet, cv::Mat& image, const std::string &p_sn);
	int decodeMtx(AVPacket* av_packet, cv::Mat& image, const std::string &p_sn);

	//转码, 将yuv转码为rgb的格式
	void YUV420SP2Mat(cv::Mat& image);

	void deInitMpp();
	void setDataCallback(const rtsp_callback_t& cb, const std::string p_sn) { 
		if (m_callBackMap.find(p_sn) == m_callBackMap.end()) {
			m_callBackMap.insert(std::make_pair(p_sn, cb)); 
			m_strSnVec.push_back(p_sn); 
		}
		return;
	}

 private:
	MppCtx ctx;
	MppApi* mpi;
	RK_U32 eos;
	MppBufferGroup frm_grp;
	MppBufferGroup pkt_grp;
	MppPacket packet;
	MppFrame frame;
	MppParam param;
	RK_U32 need_split;
	MpiCmd mpi_cmd;
	RK_S32 frame_count;
	RK_S32 frame_num;
	size_t max_usage;
	MppCodingType type;
	FILE* fp_output;

	rtsp_callback_t cb_;
	std::map<std::string, rtsp_callback_t> m_callBackMap;
	std::vector<std::string> m_strSnVec;
	int m_nDecPutErrNum;
	volatile bool m_bInit;
 	std::mutex 	 m_QueMtx;
 public:
	cv::Mat rgbImg;
};
}	 // namespace vision
}	 // namespace whale

