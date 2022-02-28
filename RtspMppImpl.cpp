//
// Created by z on 2020/5/8.
//




#include "RtspMppImpl.h"

#include <unistd.h>
#include <iostream>


int whale::vision::RtspMppImpl::init(std::string rtsp_addr_) {
    inited_ = false;
	if (pFormatCtx_) {
		avformat_close_input(&pFormatCtx_);
		avformat_free_context(pFormatCtx_);
		pFormatCtx_ = NULL;
	}
	if (packet_) {
		av_free(packet_);
		packet_ = NULL;
	}

	//设置缓存大小,1080p可将值跳到最大
	av_dict_set(&options_, "buffer_size", "1024000", 0);
	// tcp open
	av_dict_set(&options_, "rtsp_transport", "tcp", 0);
	// set timeout unit: us
	av_dict_set(&options_, "stimeout", "5000000", 0);
	// set max delay
	av_dict_set(&options_, "max_delay", "500000", 0);

	pFormatCtx_ = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx_, rtsp_addr_.c_str(), NULL, &options_) !=
			0) {
        std::cout << "Couldn't open input stream.\n";
		return -1;
	}

	// get video stream info
	if (avformat_find_stream_info(pFormatCtx_, NULL) < 0) {
		std::cout << "Couldn't find stream information.\n";
		return -1;
	}

	// find stream video codec type
	unsigned int i = 0;
	for (i = 0; i < pFormatCtx_->nb_streams; i++) {
		if (pFormatCtx_->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex_ = i;
			codec_id_ = pFormatCtx_->streams[i]->codec->codec_id;
			break;
		}
	}
	std::cout<<"video index is: "<<videoindex_;

	if (videoindex_ == -1) {
        std::cout << "Didn't find a video stream.";
		return -1;
	}

	packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));

	if (AV_CODEC_ID_H264 == codec_id_) {
		MppDecode::getInstance()->setMppDecodeType(MPP_VIDEO_CodingAVC);
		std::cout << "video stream decode type is h264\n";
	} else if (AV_CODEC_ID_HEVC == codec_id_) {
		MppDecode::getInstance()->setMppDecodeType(MPP_VIDEO_CodingHEVC);
			std::cout << codec_id_ << " ,video stream decode type is h265";
	}

	int ret = MppDecode::getInstance()->initMppMtx();
	if (ret == 0) {
		std::cout<<"init is true"<<std::endl;
		inited_ = true;
	}

	return 0;


}

void whale::vision::RtspMppImpl::restart() {

}

bool whale::vision::RtspMppImpl::read(cv::Mat& image) {
	if (!inited_) {
		std::cout << " ffmpeg rtsp stream init failed.\n";
		return false;
	}

	if (av_read_frame(pFormatCtx_, packet_) < 0) {
		std::cout << " av read frame error..\n";
		return false;
	}
	if (packet_->stream_index == videoindex_) {
		MppDecode::getInstance()->decodeMtx(packet_, image, m_strSn);
		bool bRet = true;
		// if (1 != packet_->flags) {	// 注意 MppDecode单例方式只支持全I帧，后续支持带P帧的不能用单例的方式
		// 	std::cout << " ffmpeg not I frame\n";
		// 	bRet = false;
		// }
		av_packet_unref(packet_); 
		return bRet;
	}
	av_packet_unref(packet_);
	
	return true;
}

