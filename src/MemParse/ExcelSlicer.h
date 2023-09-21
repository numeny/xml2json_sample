#pragma once

#include <xercesc/sax/AttributeList.hpp>

#include <iostream>
#include <memory>
#include <stack>
#include <string.h>

#include "Common.h"
#include "JsonTransformer.h"

XERCES_CPP_NAMESPACE_USE

using namespace std;

class ExcelSlicer;

class ExcelSlicerListener {
public:
    virtual void onStartDocument(const ExcelSlicer& excelSlicer) {}
    virtual void onEndDocument(const ExcelSlicer& excelSlicer) {}
    virtual void onStartSheetDataElement(const ExcelSlicer& excelSlicer,
        const string& header, const string& simpleTail) {}
    virtual void onEndSheetDataElement(const ExcelSlicer& excelSlicer) {}
    virtual void onParsedSliceData(const ExcelSlicer& excelSlicer,
        const string& slicedData, bool isEnded) {}
};

typedef enum {
    SliceState_InitState,
    SliceState_StartedSheetDataElement,
    SliceState_StartedRowElement,
    SliceState_EndedRowElement,
    SliceState_EndedSheetDataElement,
    SliceState_Max,
} SliceState;

class ExcelSlicer : public JsonTransformerLisener {
public:
    ExcelSlicer() = default;
    ~ExcelSlicer() = default;
    // interface for JsonTransformerLisener
    void startDocument();
    void endDocument();
    void startElement(const string& tagStr, AttributeList& attributes, const string& jsonStrOfThisTag);
    void endElement(const string& tagStr, const string& jsonStr);
    void characters(const string& chars);

    void setReadFileInfo(const ReadFileInfo& readFileInfo) { mReadFileInfo = readFileInfo; }
    const ReadFileInfo& getReadFileInfo() const { return mReadFileInfo; }

private:
    inline void appendCurrSliceData(const string& jsonStr);
    inline void resetCurrSliceData();
    inline void onParsedSliceData(bool isEnded);

    // for slice
    SliceState mSliceState = SliceState_InitState;
    uint32_t mCurrRowIdxForSlice = 0;
    string mCurrRowSlice;                   // json string stream of utf8 to output

    string mStartTagForSlice;
    string mContentTagForSlice;
    uint32_t mDepthContentTag = 0;

    shared_ptr<ExcelSlicerListener> mExcelSlicerListener;
    ReadFileInfo mReadFileInfo;

    SliceType mSliceType = SliceType_WordDocument;

    bool mWillStartParsedContentTag = false; // whether start parsing contentTag for word's document.xml
};
