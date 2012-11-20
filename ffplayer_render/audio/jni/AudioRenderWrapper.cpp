/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define TAG "AudioRenderWrapper"
#include <utils/Log.h>
#include <media/AudioTrack.h>
#include <media/AudioSystem.h>
#include <utils/Errors.h>

#include <binder/MemoryHeapBase.h>
#include <binder/MemoryBase.h>

#include "AudioRenderWrapper.h"

using namespace android;
#define CHECK_AUDIO_TRACK(track) \
    do { \
        if(NULL == track) \
        return ANDROID_AUDIOTRACK_RESULT_JNI_EXCEPTION; \
    } while (0)
//struct audiotrack_fields_t {
static AudioTrack* track;
//sp<MemoryHeapBase>memHeap;
//sp<MemoryBase>memBase;
//};
//static struct audiotrack_fields_t audio;

/*
   static AudioTrack* getNativeAudioTrack(JNIEnv* env, jobject jaudioTrack) {
   jclass clazz = env->FindClass("android/media/AudioTrack");
   jfieldID field_track = env->GetFieldID(clazz, "mNativeTrackInJavaObj", "I");
   if(field_track == NULL) {
   return NULL;
   }
   return (AudioTrack *) env->GetIntField(jaudioTrack, field_track);
   }
 */

/*
   static bool allocSharedMem(int sizeInBytes) {
   memHeap = new MemoryHeapBase(sizeInBytes);
   if (memHeap->getHeapID() < 0) {
   return false;
   }
   memBase = new MemoryBase(memHeap, 0, sizeInBytes);
   return true;
   }
 */

int AudioTrackWrapper_Register() {
    __android_log_print(ANDROID_LOG_INFO, TAG, "registering audio track");
    track = new AudioTrack();
    CHECK_AUDIO_TRACK(track);
    __android_log_print(ANDROID_LOG_INFO, TAG, "AudioTrack registered");
    return ANDROID_AUDIOTRACK_RESULT_SUCCESS;
}

int AudioTrackWrapper_Start() {
    //__android_log_print(ANDROID_LOG_INFO, TAG, "starting audio track");
    CHECK_AUDIO_TRACK(track);
    track->start();
    return ANDROID_AUDIOTRACK_RESULT_SUCCESS;
}

int AudioTrackWrapper_Set(int streamType,
                          uint32_t sampleRate,
                          int format,
                          int channels,
                          audio_track_wrapper_cbf_t cbf,
                          void *user) {
    CHECK_AUDIO_TRACK(track);

    __android_log_print(ANDROID_LOG_INFO, TAG, "Configure audio track, streamType:%d, sampleRate:%d, channels:%d, format:%d cbf:%p, user:%p",
                        streamType, sampleRate, format, channels, cbf, user);

    status_t ret = track->set(streamType,
                              sampleRate,
                              format,
                              channels,
                              0,
                              0,
                              cbf,
                              user);

    if (ret != NO_ERROR) {
        return ANDROID_AUDIOTRACK_RESULT_ERRNO;
    }
    return ANDROID_AUDIOTRACK_RESULT_SUCCESS;
}

int AudioTrackWrapper_Flush() {
    if(track == NULL) {
        return ANDROID_AUDIOTRACK_RESULT_ALLOCATION_FAILED;
    }
    track->flush();
    return ANDROID_AUDIOTRACK_RESULT_SUCCESS;
}

int AudioTrackWrapper_Stop() {
    if(track == NULL) {
        return ANDROID_AUDIOTRACK_RESULT_ALLOCATION_FAILED;
    }
    track->stop();
    return ANDROID_AUDIOTRACK_RESULT_SUCCESS;
}

int AudioTrackWrapper_Reload() {
    if(track == NULL) {
        return ANDROID_AUDIOTRACK_RESULT_ALLOCATION_FAILED;
    }
    if(track->reload() != NO_ERROR) {
        return ANDROID_AUDIOTRACK_RESULT_ERRNO;
    }
    return ANDROID_AUDIOTRACK_RESULT_SUCCESS;
}

int AudioTrackWrapper_Unregister() {
    __android_log_print(ANDROID_LOG_INFO, TAG, "unregistering audio track");
    if(!track->stopped()) {
        track->stop();
    }
    //memBase.clear();
    //memHeap.clear();
    free(track);
    track = NULL;
    __android_log_print(ANDROID_LOG_INFO, TAG, "AudioTrack unregistered");
    return ANDROID_AUDIOTRACK_RESULT_SUCCESS;
}

int AudioTrackWrapper_Write(void *buffer, int buffer_size) {
    // give the data to the native AudioTrack object (the data starts at the offset)
    ssize_t written = 0;
    // regular write() or copy the data to the AudioTrack's shared memory?
    if (track->sharedBuffer() == 0) {
        written = track->write(buffer, buffer_size);
    } else {
        // writing to shared memory, check for capacity
        if ((size_t)buffer_size > track->sharedBuffer()->size()) {
            __android_log_print(ANDROID_LOG_INFO, TAG, "buffer size was too small");
            buffer_size = track->sharedBuffer()->size();
        }
        memcpy(track->sharedBuffer()->pointer(), buffer, buffer_size);
        written = buffer_size;
    }
    return written;
}
