#pragma once

#include <string>
#include <sys/time.h>

using namespace std;

#define DebugXml2Json 1

#define DefaultShardRowNum 1

typedef enum {
    ParseXmlType_FromMemory,
    ParseXmlType_FromFile,
    ParseXmlType_Max,
} ParseXmlType;

typedef enum {
    ShardType_WordDocument,
    ShardType_ExcelSheetx,
    ShardType_Max,
} ShardType;

struct ReadFileInfo {
    std::string mFileFullPath;           // full path of xml file to read
    std::string mFileId;                 // fileId for unzip file
    std::string mRelativePath;           // relative path of xml file in unzip directory
    ParseXmlType mParseXmlType = ParseXmlType_FromFile;
    std::string mXmlFileContent;         // content of xml file to read, used when mParseXmlType is ParseXmlType_FromMemory

    bool mWillShard = false;        // will the file to read be shardd?
    unsigned int mShardSize = DefaultShardRowNum;    // 1024; // row number for slicing
    ShardType mShardType = ShardType_WordDocument;
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
extern int deleteAllFile(const string& filePath);

extern void setMinBuffSizeToRead(unsigned int buffSize);
extern void setMaxBuffSizeToRead(unsigned int buffSize);