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

class ExcelSharder;

class ExcelSharderListener {
public:
    virtual void onStartDocument(const ExcelSharder& excelShardr) {}
    virtual void onEndDocument(const ExcelSharder& excelShardr) {}
    virtual void onStartSheetDataElement(const ExcelSharder& excelShardr,
        const string& header, const string& simpleTail) {}
    virtual void onEndSheetDataElement(const ExcelSharder& excelShardr) {}
    virtual void onParsedShardData(const ExcelSharder& excelShardr,
        const string& sharddData, bool isEnded) {}
};

typedef enum {
    ShardState_InitState,
    ShardState_StartedSheetDataElement,
    ShardState_StartedRowElement,
    ShardState_EndedRowElement,
    ShardState_EndedSheetDataElement,
    ShardState_Max,
} ShardState;

class ExcelSharder : public JsonTransformerLisener {
public:
    ExcelSharder() = default;
    ~ExcelSharder() = default;
    // interface for JsonTransformerLisener
    void startDocument() override;
    void endDocument() override;
    void startElement(const string& tagStr, AttributeList& attributes,
        const string& jsonStrOfThisTag) override;
    void endElement(const string& tagStr, const string& jsonStr) override;
    void characters(const string& chars) override;

    void setReadFileInfo(const ReadFileInfo& readFileInfo) { mReadFileInfo = readFileInfo; }
    const ReadFileInfo& getReadFileInfo() const { return mReadFileInfo; }

private:
    inline void appendCurrShardData(const string& jsonStr);
    inline void resetCurrShardData();
    inline void onParsedShardData(bool isEnded);

    // for shard
    ShardState mShardState = ShardState_InitState;
    uint32_t mCurrRowIdxForShard = 0;
    string mCurrRowShard;                   // json string stream of utf8 to output

    string mStartTagForShard;
    string mContentTagForShard;
    uint32_t mDepthContentTag = 0;

    shared_ptr<ExcelSharderListener> mExcelSharderListener;
    ReadFileInfo mReadFileInfo;

    ShardType mShardType = ShardType_WordDocument;

    bool mWillStartParsedContentTag = false; // whether start parsing contentTag for word's document.xml
};
