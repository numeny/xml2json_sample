#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <stack>
#include <string.h>

#include <xercesc/sax/AttributeList.hpp>

#include "Xml2JsonCommon.h"
#include "JsonTransformer.h"

XERCES_CPP_NAMESPACE_USE

using namespace std;

typedef enum {
    ShardState_InitState,
    ShardState_StartedSheetDataElement,
    ShardState_StartedRowElement,
    ShardState_EndedRowElement,
    ShardState_EndedSheetDataElement,
    ShardState_Max,
} ShardState;

class ShardHandler : public JsonTransformerLisener {
public:
    ShardHandler() = default;
    ~ShardHandler() = default;

    void initIfNeccesary(ShardType shardType, size_t shardSize = DefaultShardRowNum);

    // interface for JsonTransformerLisener
    void startDocument() override;
    void endDocument() override;
    void startElement(const string& tagStr,
        AttributeList& attributes, const string& jsonStrOfThisTag) override;
    // void endElement(const string& tagStr, const string& jsonStr) override;
    void endElement2(const string& tagStr,
        const string& jsonStr, const HandlerExtraInfo& extraInfo) override;
    void characters(const string& chars) override;

    bool isSharedEnded() { return mSharedEnded; }
    const string& currShardData() { return mCurrRowShard; }
    const string& simpleHeadOfXml() {return mSimpleHeadOfXml; }
    size_t totalSizeOfContentHasRead() { return mTotalSizeOfContentHasRead; }
private:
    inline void appendCurrShardData(const string& jsonStr);
    inline void resetCurrShardData();
    inline void onParsedShardData(
        bool isEnded, const HandlerExtraInfo& extraInfo);
    int deleteThisShardDataFromBuff(size_t startPosLeft);
    void initSimpleXmlHead();

    // input params
    size_t mShardSize;
    string mStartTagForShard;

    // for shard
    ShardState mShardState = ShardState_InitState;
    uint32_t mCurrRowIdxForShard = 0;
    string mCurrRowShard;               // json string stream of utf8 to output
    uint32_t mDepthContentTag = 0;

    string mSimpleHeadOfXml;
    std::queue<string> mTagQueue;
    bool mSharedEnded = false;
    size_t mTotalSizeOfContentHasRead = 0;

    stack<string> mShardStack;
};
