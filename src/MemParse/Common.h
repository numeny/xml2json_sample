#pragma once

#include <string>
#include <sys/time.h>

using namespace std;

#define DebugXml2Json 1

#define DefaultSliceRowNum 1

typedef enum {
    ParseXmlType_FromMemory,
    ParseXmlType_FromFile,
    ParseXmlType_Max,
} ParseXmlType;

typedef enum {
    SliceType_WordDocument,
    SliceType_ExcelSheetx,
    SliceType_Max,
} SliceType;

struct ReadFileInfo {
    std::string mFileFullPath;           // full path of xml file to read
    std::string mFileId;                 // fileId for unzip file
    std::string mRelativePath;           // relative path of xml file in unzip directory
    ParseXmlType mParseXmlType = ParseXmlType_FromFile;
    std::string mXmlFileContent;         // content of xml file to read, used when mParseXmlType is ParseXmlType_FromMemory

    bool mWillSlice = false;        // will the file to read be sliced?
    unsigned int mSliceSize = DefaultSliceRowNum;    // 1024; // row number for slicing
    SliceType mSliceType = SliceType_WordDocument;
    bool mWillSaveTransformResult = true;
};

class DurationTimer {
public:
    DurationTimer();
    DurationTimer(const std::string& tag);
    ~DurationTimer();
private:
    void printDuration();
    double getTimeInterval(struct timeval startTime, struct timeval endTime);
    struct timeval mStart, mEnd;
    std::string mTag;
};

extern int readFileIntoString(std::string &contentStr, const std::string& fileName);
extern int readFileIntoString2(char **contentStr, size_t& outFileSize, const string& fileName);