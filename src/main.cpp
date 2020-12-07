/**
* Copyright 2020 Huawei Technologies Co., Ltd
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.

* File main.cpp
* Description: dvpp sample main func
*/

#include <iostream>
#include <stdlib.h>
#include <dirent.h>

#include "object_detect.h"
#include "utils.h"
#include <chrono>
using namespace std::chrono;

using namespace std;

namespace {
uint32_t kModelWidth = 416;
uint32_t kModelHeight = 416;
//const char* kModelPath = "../model/yolov3.om";
const char* kModelPath = "../model/yolov3_tf.om";
}


//string videoFile = "rtsp://admin:xxxx@10.0.1xxxx.148:554";
string videoFile = "../data/detection.mp4";
int main(int argc, char *argv[]) {
    //Check the input when the application executes, which takes the path to the input video file
//    if((argc < 2) || (argv[1] == nullptr)){
//        ERROR_LOG("Please input: ./main <image_dir>");
//        return FAILED;
//    }

    //Instantiate the target detection class with the parameters of the classification model path and the required width and height of the model input
    ObjectDetect detect(kModelPath, kModelWidth, kModelHeight);
    //Initializes the ACL resource for categorical reasoning, loads the model and requests the memory used for reasoning input
    Result ret = detect.Init();
    if (ret != SUCCESS) {
        ERROR_LOG("Classification Init resource failed");
        return FAILED;
    }

    //Use Opencv to open the video file
//    string videoFile = string(argv[1]);
    printf("open %s\n", videoFile.c_str());
    cv::VideoCapture capture(videoFile);
    if (!capture.isOpened()) {
        cout << "Movie open Error" << endl;
        return FAILED;
    }

    cout << "width = " <<  capture.get(3) << endl;
    cout << "height = " <<  capture.get(4) << endl;
    cout << "frame_fps = " <<  capture.get(5) << endl;
    cout << "frame_nums = " <<  capture.get(7) << endl;

    //Frame by frame reasoning
    while(1) {
        // 1 Read a frame of an image
        cv::Mat frame;
        if (!capture.read(frame)) {
            INFO_LOG("Video capture return false");
            break;
        }
        // 2 The frame image is preprocessed
        high_resolution_clock::time_point start = high_resolution_clock::now();
        Result ret = detect.Preprocess(frame);
        if (ret != SUCCESS) {
            ERROR_LOG("Read file %s failed, continue to read next",
                      videoFile.c_str());
            continue;
        }
        high_resolution_clock::time_point end = high_resolution_clock::now();
        duration<double, std::milli> time_span = end - start;
        cout << "\nPreProcess time " << time_span.count() << "ms" << endl;

        // 3 The preprocessed images are fed into model reasoning and the reasoning results are obtained
        aclmdlDataset* inferenceOutput = nullptr;
        start = high_resolution_clock::now();
        ret = detect.Inference(inferenceOutput);
        if ((ret != SUCCESS) || (inferenceOutput == nullptr)) {
            ERROR_LOG("Inference model inference output data failed");
            return FAILED;
        }
        end = high_resolution_clock::now();
        time_span = end - start;
        cout << "Inference time " << time_span.count() << "ms" << endl;

        // 4 Parses the inference output and sends the inference class, location, confidence, and image to the Presenter Server for display
        start = high_resolution_clock::now();
        ret = detect.Postprocess(frame, inferenceOutput);
        if (ret != SUCCESS) {
            ERROR_LOG("Process model inference output data failed");
            return FAILED;
        }
        end = high_resolution_clock::now();
        time_span = end - start;
        cout << "PostProcess time " << time_span.count() << "ms" << endl;
    }

    INFO_LOG("Execute video object detection success");
    return SUCCESS;
}
