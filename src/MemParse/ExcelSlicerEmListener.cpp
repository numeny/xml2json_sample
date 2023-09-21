#include <iostream>
#include <string>
#if defined(EMSCRIPTEN)
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

#include "ExcelSlicerEmListener.h"

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
EM_JS(void, parsedSliceData, (const char *fileId, const char *relativePath,
  const char* slicedData, bool isEnded), {
  if (typeof onParsedSliceData === "function") {
    onParsedSliceData(fileId, relativePath, slicedData, isEnded);
  }
});
#endif

}

void ExcelSlicerEmListener::onStartDocument(const ExcelSlicer& excelSlicer) {
    // cout << "ExcelSlicerEmListener::onStartDocument" << endl;
#if defined(EMSCRIPTEN)
    const ReadFileInfo &readFileInfo = excelSlicer.getReadFileInfo();
    startDocument(readFileInfo.mFileId.c_str(), readFileInfo.mRelativePath.c_str());
#endif
}

void ExcelSlicerEmListener::onEndDocument(const ExcelSlicer& excelSlicer) {
    // cout << "ExcelSlicerEmListener::onEndDocument" << endl;
#if defined(EMSCRIPTEN)
    const ReadFileInfo &readFileInfo = excelSlicer.getReadFileInfo();
    endDocument(readFileInfo.mFileId.c_str(), readFileInfo.mRelativePath.c_str());
#endif
}

void ExcelSlicerEmListener::onParsedSliceData(
    const ExcelSlicer& excelSlicer, const string& slicedData, bool isEnded) {
    // cout << "ExcelSlicerEmListener::onParsedSliceData"
    //   << ", slicedData:" << slicedData << ", \n isEnded: " << isEnded << endl;
#if defined(EMSCRIPTEN)
    const ReadFileInfo &readFileInfo = excelSlicer.getReadFileInfo();
    parsedSliceData(readFileInfo.mFileId.c_str(),
      readFileInfo.mRelativePath.c_str(), slicedData.c_str(), isEnded);    
#endif
}
