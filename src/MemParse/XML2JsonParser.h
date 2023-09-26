#pragma once

#include <string>
#include <xercesc/sax/HandlerBase.hpp>

#include "Common.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

const int RetCode_UserInterruption = 100;
class UserInterruption {
public:
    int mRetCode = RetCode_UserInterruption;
};

extern int ParseXml2Json(const ReadFileInfo& readFileInfo, HandlerBase* handler);

extern int ParseXml2Json(string& jsonUtf8Str, const ReadFileInfo& readFileInfo);

extern int GetFileContentWithNoShard(string& jsonUtf8Str, const string& fileFullPath,
    const string& fileId, const string& relativePath);

extern int GetFileContent(string& jsonUtf8Str, const string& fileFullPath,
    const string& fileId, const string& relativePath,
    bool willShard, ShardType shardType, unsigned int shardSize);

extern int GetFileContent(string& jsonUtf8Str, const string& fileFullPath, 
    const string& fileId, const string& relativePath, bool willShard, ShardType shardType);

extern int GetFileContent(string& jsonUtf8Str, const string& fileFullPath, 
    const string& fileId, const string& relativePath,
    bool willShard, ShardType shardType, unsigned int shardSize, bool willSaveTransformResult);

extern int ConvertXML2JSON(string& jsonStr, const string& xmlContent);

extern int GetDocumentSectPrArray(
    string& jsonUtf8Str, const string& fileFullPath);

extern int GetExcelSheetHeadAndTail(
    string& jsonStr, const string& fileName);
