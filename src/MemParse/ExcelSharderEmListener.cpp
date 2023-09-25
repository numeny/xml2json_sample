#include <iostream>
#include <string>
#if defined(EMSCRIPTEN)
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

#include "ExcelSharderEmListener.h"

namespace {

#if defined(EMSCRIPTEN)
EM_JS(void, startDocument, (const char *fileId, const char *relativePath), {
  if (typeof onStartDocument === "function") {
    onStartDocument(fileId, relativePath);
  }
});
EM_JS(void, endDocument, (const char *fileId, const char *relativePath), {
  if (typeof onEndDocument === "function") {
    onEndDocument(fileId, relativePath);
  }
});
EM_JS(void, parsedShardData, (const char *fileId, const char *relativePath,
  const char* sharddData, bool isEnded), {
  if (typeof onParsedShardData === "function") {
    onParsedShardData(fileId, relativePath, sharddData, isEnded);
  }
});
#endif

}

void ExcelSharderEmListener::onStartDocument(const ExcelSharder& excelShardr) {
    // cout << "ExcelSharderEmListener::onStartDocument" << endl;
#if defined(EMSCRIPTEN)
    const ReadFileInfo &readFileInfo = excelShardr.getReadFileInfo();
    startDocument(readFileInfo.mFileId.c_str(), readFileInfo.mRelativePath.c_str());
#endif
}

void ExcelSharderEmListener::onEndDocument(const ExcelSharder& excelShardr) {
    // cout << "ExcelSharderEmListener::onEndDocument" << endl;
#if defined(EMSCRIPTEN)
    const ReadFileInfo &readFileInfo = excelShardr.getReadFileInfo();
    endDocument(readFileInfo.mFileId.c_str(), readFileInfo.mRelativePath.c_str());
#endif
}

void ExcelSharderEmListener::onParsedShardData(
    const ExcelSharder& excelShardr, const string& sharddData, bool isEnded) {
    // cout << "ExcelSharderEmListener::onParsedShardData"
    //   << ", sharddData:" << sharddData << ", \n isEnded: " << isEnded << endl;
#if defined(EMSCRIPTEN)
    const ReadFileInfo &readFileInfo = excelShardr.getReadFileInfo();
    parsedShardData(readFileInfo.mFileId.c_str(),
      readFileInfo.mRelativePath.c_str(), sharddData.c_str(), isEnded);    
#endif
}
