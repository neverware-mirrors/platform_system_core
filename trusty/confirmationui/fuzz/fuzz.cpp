/*
 * Copyright (C) 2020 The Android Open Source Project
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

#undef NDEBUG

#include <assert.h>
#include <log/log.h>
#include <stdlib.h>
#include <trusty/fuzz/utils.h>
#include <unistd.h>

using android::trusty::fuzz::TrustyApp;

#define TIPC_DEV "/dev/trusty-ipc-dev0"
#define CONFIRMATIONUI_PORT "com.android.trusty.confirmationui"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    static uint8_t buf[TIPC_MAX_MSG_SIZE];

    TrustyApp ta(TIPC_DEV, CONFIRMATIONUI_PORT);
    auto ret = ta.Connect();
    if (!ret.ok()) {
        android::trusty::fuzz::Abort();
    }

    /* Send message to confirmationui server */
    ret = ta.Write(data, size);
    if (!ret.ok()) {
        return -1;
    }

    /* Read message from confirmationui server */
    ret = ta.Read(&buf, sizeof(buf));
    if (!ret.ok()) {
        return -1;
    }

    return 0;
}
