#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "Common.h"

using namespace std;

int readFileIntoString(std::string &contentStr, const string& fileName) {
    DurationTimer timer("readFileIntoString");

    std::ifstream inputFile(fileName.c_str(), std::ios::ate);

    if (!inputFile.is_open()) {
        std::cerr << "Err: open fileName:" << fileName << std::endl;
        return -1;
    }

    std::streampos fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    char* fileContent = new char[fileSize];
    if (!fileContent) {
        std::cerr << "Err: memory out when read file:" << fileName << std::endl;
        return -1;
    }

    inputFile.read(fileContent, fileSize);
    if (!inputFile) {
        std::cerr << "Err: reading file:" << fileName << std::endl;
    }
    inputFile.close();
#if 0
    try {
        if (!std::filesystem::remove(fileName)) {
            std::cerr << "Err: delete file:" << fileName << std::endl;
            // return -1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Err: delete file:" << fileName << ", exception: " << e.what() << std::endl;
        // return -1;
    }
#endif
    contentStr = std::move(string(fileContent, fileSize));

    delete[] fileContent;
    return 0;
}

int readFileIntoString2(char **contentStr, size_t& outFileSize, const string& fileName) {
    DurationTimer timer("readFileIntoString");

    std::ifstream inputFile(fileName.c_str(), std::ios::ate);

    if (!inputFile.is_open()) {
        std::cerr << "Err: open fileName:" << fileName << std::endl;
        return -1;
    }

    std::streampos fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    char* fileContent = new char[fileSize];
    if (!fileContent) {
        std::cerr << "Err: memory out when read file:" << fileName << std::endl;
        return -1;
    }

    inputFile.read(fileContent, fileSize);
    if (!inputFile) {
        std::cerr << "Err: reading file:" << fileName << std::endl;
    }
    inputFile.close();
#if 0
    std::cout << "[Debug] readFileIntoString2 delete file:" << fileName << std::endl;
    try {
        if (!std::filesystem::remove(fileName)) {
            std::cerr << "Err: delete file:" << fileName << std::endl;
            // return -1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Err: delete file:" << fileName << ", exception: " << e.what() << std::endl;
        // return -1;
    }
#endif
    *contentStr = fileContent;
    outFileSize = fileSize;

    // delete[] fileContent;
    return 0;
}

#define EnableTimeCalculation 1

DurationTimer::DurationTimer() {
#if EnableTimeCalculation
    gettimeofday(&mStart, nullptr);
#endif
}

DurationTimer::DurationTimer(const string& tag): mTag(tag) {
#if EnableTimeCalculation
    gettimeofday(&mStart, nullptr);
#endif
}

DurationTimer::~DurationTimer() {
#if EnableTimeCalculation
    gettimeofday(&mEnd, nullptr);
    printDuration();
#endif
}

void DurationTimer::printDuration() {
#if EnableTimeCalculation
    double elapsedTime;
    elapsedTime = getTimeInterval(mStart, mEnd);
    cout << "[" << mTag << "]Time elapsed: " << elapsedTime << " seconds" << endl;
#endif
}

double DurationTimer::getTimeInterval(struct timeval startTime, struct timeval endTime) {
    double startSeconds = startTime.tv_sec + startTime.tv_usec * 1e-6;
    double endSeconds = endTime.tv_sec + endTime.tv_usec * 1e-6;
    return endSeconds - startSeconds;
}
