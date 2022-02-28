#include <stdio.h>
#include <chrono>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <memory>
#include <opencv2/opencv.hpp>
#include "RtspMppImpl.h"
#include <iostream>

#include "rknn_api.h"

/**
 * 功能描述: 获取模型指针和大小
 * 
 * 输入参数： filename 
 * 输入参数： model_size 
 * 返回参数：true/false
 */
bool load_model(const char *filename, rknn_context* pCtx)
{
    auto start = std::chrono::steady_clock::now();

    printf(" ======== Load model from %s ========\n", filename);

    // 1. 获取模型指针和大小
    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr)
    {
        printf("fopen %s fail!\n", filename);
        return false;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char *)malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if (model_len != fread(model, 1, model_len, fp))
    {
        printf("fread %s fail!\n", filename);
        free(model);
        return false;
    }

    if (fp)
    {
        fclose(fp);
    }
    
    // 2. 加载模型
    int ret = rknn_init(pCtx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM);
    if (ret < 0)
    {
        printf("rknn_init fail! ret=%d\n", ret);
        return false;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<float, std::milli> du = end - start;
    printf("Load model Cost %.2f ms\n", du.count());

    return true;
}

bool get_input(rknn_context ctx, void** input_data, int* pInputSize)
{
    //  获取输入属性
    rknn_tensor_attr input_attr;
    input_attr.index = 0;
    int ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &input_attr, sizeof(input_attr));
    if (ret < 0)
    {
        printf("rknn_query RKNN_QUERY_INPUT_ATTR failed! ret=%d\n", ret);
        return false;
    }

    //      获取输入Tensor shape
    int input_size = 1;
    for (int i = 0; i < input_attr.n_dims; i++)
    {
        // printf("DIM[%d]: %d\n", i, input_attr.dims[i]);
        input_size *= input_attr.dims[i];
    }
    *pInputSize = input_size;

    // 4. 设置输入数据
    *input_data = malloc(input_size);

    return true;
}

bool get_outputs(rknn_context ctx, rknn_output** pOutput, int* pOutputNum)
{
    rknn_input_output_num io_num;
    int ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret < 0)
    {
        printf("rknn_query RKNN_QUERY_IN_OUT_NUM failed! ret=%d\n", ret);
        return false;
    }

    int output_num = io_num.n_output;
    *pOutputNum = output_num;
    *pOutput = (rknn_output*)malloc(sizeof(rknn_output) * output_num);

    rknn_output* output = *pOutput;
    for (int i = 0; i < output_num; i++)
    {
        rknn_tensor_attr output_attr;
        output_attr.index = i;
        ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &output_attr, sizeof(output_attr));
        if (ret < 0)
        {
            printf("rknn_query RKNN_QUERY_INPUT_ATTR failed! ret=%d\n", ret);
            return false;
        }
        
        // 计算size
        int size = 1;
        for (int k = 0; k < output_attr.n_dims; k++)
        {
            size *= output_attr.dims[k];
        }

        output[i].index = i;
        output[i].want_float = 1;
        output[i].is_prealloc = 1;
        output[i].buf = malloc(sizeof(float) * size);
        output[i].size = sizeof(float) * size;
    }

    return true;
}

bool inference(rknn_context ctx, void* input_data, int input_size, rknn_output* outputs, int output_num, float* cost_time)
{
    int ret = 0;

    // 3. 获取输入输出属性
    auto start = std::chrono::steady_clock::now();

    // 4. 设置输入数据
    memset(input_data, 0, input_size);
    rknn_input inputs[1];
    inputs[0].index = 0;
    inputs[0].buf = input_data;
    inputs[0].size = input_size;
    inputs[0].pass_through = 1;
    // pass_through = 0时需要设置以下值，RKNN会对输入数据做转换
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].fmt = RKNN_TENSOR_NCHW;
    ret = rknn_inputs_set(ctx, 1, inputs);
    if (ret < 0)
    {
        printf("rknn_input_set failed! ret=%d\n", ret);
        free(input_data);
        return false;
    }

    // 5. 推理
    ret = rknn_run(ctx, NULL);
    if (ret < 0)
    {
        printf("rknn_run failed! ret=%d\n", ret);
        return false;
    }

    // 6. 获取输出
    ret = rknn_outputs_get(ctx, output_num, outputs, NULL);
    if (ret < 0)
    {
        printf("rknn_outputs_get failed! ret=%d\n", ret);
        return false;
    }

    // 7. 释放输出
    // rknn_outputs_release(ctx, output_num, outputs);
    // free(outputs);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<float, std::milli> du = end - start;
    printf("[Thread: %u]  Inference Cost %.2f ms\n", std::this_thread::get_id(), du.count());

    *cost_time = du.count();

    return true;
}

class TestRVMPP{
public:
    TestRVMPP(){}
    ~TestRVMPP(){}

public:
    int dataCallback(cv::Mat& image){
        mImg = image.clone();
        return 0;
    }

public:
    cv::Mat mImg;

};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s [model_path] [loop=100] [thread_num=4]\n", argv[0]);
        return -1;
    }

    const char* model_path = argv[1];
    int loop = 100;
    if (argc >= 3)
    {
        loop = atoi(argv[2]);
    }

    int thread_num = 1;
    if (argc >= 4)
    {
        thread_num = atoi(argv[3]);
    }


    std::shared_ptr<whale::vision::RtspMppImpl> rtsp_impl_ = std::make_shared<whale::vision::RtspMppImpl>("");

    int ret = rtsp_impl_->init("rtsp://admin:whale123@192.168.10.31:554/h264/ch39/main/av_stream");
    if (0 != ret ){
        std::cout<<"rtsp init failed"<<std::endl;
        return 0;
    }
    std::cout<<"hahahha"<<std::endl;

    TestRVMPP trvmpp;
    rtsp_impl_->set(cv::CAP_PROP_BUFFERSIZE, 3);
    rtsp_impl_->set(cv::CAP_PROP_FPS, 15);
    rtsp_impl_->setDataCallback(std::bind(&TestRVMPP::dataCallback, &trvmpp, std::placeholders::_1), "WXX");
    cv::Mat img;
    ret = rtsp_impl_->read(img);

    for (int n = 0; n < thread_num; n++)
    {
        std::thread t1([&] {
            rknn_context ctx;
            int input_size;
            void* input_data;
            int output_num;
            rknn_output* outputs;

            if (!load_model(model_path, &ctx))
            {
                printf("Load model failed!\n");
                rknn_destroy(ctx);
                return -1;
            }
            input_data = trvmpp.mImg.data;
            input_size = trvmpp.mImg.cols*trvmpp.mImg.rows*3;
            printf(" ======== Get Input ======== \n");
            if (!get_input(ctx, &input_data, &input_size))
            {
                printf("Get input failed!\n");
                free(input_data);
                return -1;
            }

            printf(" ======== Get Output ======== \n");
            if (!get_outputs(ctx, &outputs, &output_num))
            {
                printf("Get output failed!\n");
                rknn_outputs_release(ctx, output_num, outputs);
                free(outputs);
                return -1;
            }

            printf(" ======== Inference ========\n");
            float avg_time = 0.0f;
            for (int i = 0; i < loop; i++)
            {

                
                bool ret = rtsp_impl_->read(img);
                std::cout<<trvmpp.mImg.cols<<"  "<<trvmpp.mImg.rows<<std::endl;
                if (trvmpp.mImg.cols <10 || trvmpp.mImg.rows < 10)
                    continue;
                float cost_time;
                if (!inference(ctx, trvmpp.mImg.data, trvmpp.mImg.cols*trvmpp.mImg.rows*3, outputs, output_num, &cost_time))
                {
                    printf("Inference failed!\n");
                    return -1;
                }
                avg_time += cost_time;
            }
            avg_time /= loop;
            printf(" ======== AVG time: %.2fms ========\n", avg_time);
            
            // 7. 释放输出
            rknn_outputs_release(ctx, output_num, outputs);
            rknn_destroy(ctx);

        });
        t1.detach();
    }

    std::this_thread::sleep_until(std::chrono::time_point<std::chrono::system_clock>::max());
    return 0;
}