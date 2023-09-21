#pragma once

#include <string>
#include <xercesc/sax/HandlerBase.hpp>

#include "Common.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

extern int ParseXml2Json(const ReadFileInfo& readFileInfo, HandlerBase* handler);

extern int ParseXml2Json(string& jsonUtf8Str, const ReadFileInfo& readFileInfo);

extern int GetFileContentWithNoSlice(string& jsonUtf8Str, const string& fileFullPath,
    const string& fileId, const string& relativePath);

extern int GetFileContent(string& jsonUtf8Str, const string& fileFullPath,
    const string& fileId, const string& relativePath,
    bool willSlice, SliceType sliceType, unsigned int sliceSize);

extern int GetFileContent(string& jsonUtf8Str, const string& fileFullPath, 
    const string& fileId, const string& relativePath, bool willSlice, SliceType sliceType);

extern int GetFileContent(string& jsonUtf8Str, const string& fileFullPath, 
    const string& fileId, const string& relativePath,
    bool willSlice, SliceType sliceType, unsigned int sliceSize, bool willSaveTransformResult);

extern int GetDocumentSectPrArray(
    string& jsonUtf8Str, const string& fileFullPath);

extern int GetExcelSheetHeadAndTail(
    string& jsonStr, const string& fileName);
