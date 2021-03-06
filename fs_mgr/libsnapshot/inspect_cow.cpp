//
// Copyright (C) 2020 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include <android-base/logging.h>
#include <android-base/unique_fd.h>
#include <libsnapshot/cow_reader.h>

namespace android {
namespace snapshot {

void MyLogger(android::base::LogId, android::base::LogSeverity severity, const char*, const char*,
              unsigned int, const char* message) {
    if (severity == android::base::ERROR) {
        fprintf(stderr, "%s\n", message);
    } else {
        fprintf(stdout, "%s\n", message);
    }
}

static void usage(void) {
    LOG(ERROR) << "Usage: inspect_cow [-s] <COW_FILE>";
}

static bool Inspect(const std::string& path, bool silent) {
    android::base::unique_fd fd(open(path.c_str(), O_RDONLY));
    if (fd < 0) {
        PLOG(ERROR) << "open failed: " << path;
        return false;
    }

    CowReader reader;
    if (!reader.Parse(fd)) {
        LOG(ERROR) << "parse failed: " << path;
        return false;
    }

    CowHeader header;
    if (!reader.GetHeader(&header)) {
        LOG(ERROR) << "could not get header: " << path;
        return false;
    }
    CowFooter footer;
    bool has_footer = false;
    if (reader.GetFooter(&footer)) has_footer = true;

    if (!silent) {
        std::cout << "Major version: " << header.major_version << "\n";
        std::cout << "Minor version: " << header.minor_version << "\n";
        std::cout << "Header size: " << header.header_size << "\n";
        std::cout << "Footer size: " << header.footer_size << "\n";
        std::cout << "Block size: " << header.block_size << "\n";
        std::cout << "\n";
        if (has_footer) {
            std::cout << "Total Ops size: " << footer.op.ops_size << "\n";
            std::cout << "Number of Ops: " << footer.op.num_ops << "\n";
            std::cout << "\n";
        }
    }

    auto iter = reader.GetOpIter();
    while (!iter->Done()) {
        const CowOperation& op = iter->Get();

        if (!silent) std::cout << op << "\n";

        iter->Next();
    }

    return true;
}

}  // namespace snapshot
}  // namespace android

int main(int argc, char** argv) {
    int ch;
    bool silent = false;
    while ((ch = getopt(argc, argv, "s")) != -1) {
        switch (ch) {
            case 's':
                silent = true;
                break;
            default:
                android::snapshot::usage();
        }
    }
    android::base::InitLogging(argv, android::snapshot::MyLogger);

    if (argc < optind + 1) {
        android::snapshot::usage();
        return 1;
    }

    if (!android::snapshot::Inspect(argv[optind], silent)) {
        return 1;
    }
    return 0;
}
