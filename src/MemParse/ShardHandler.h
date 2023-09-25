#pragma once

#include <xercesc/sax/AttributeList.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <stack>
#include <string.h>

#include "Common.h"
#include "JsonTransformer.h"

XERCES_CPP_NAMESPACE_USE

using namespace std;

class ShardHandler;

class ShardHandlerListener {
public:
    virtual void onStartDocument(const ShardHandler& excelShardr) {}
    virtual void onEndDocument(const ShardHandler& excelShardr) {}
    virtual void onStartSheetDataElement(const ShardHandler& excelShardr,
        const string& header, const string& simpleTail) {}
    virtual void onEndSheetDataElement(const ShardHandler& excelShardr) {}
    virtual void onParsedShardData(const ShardHandler& excelShardr,
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

class ShardHandler : public JsonTransformerLisener {
public:
    static int readNextShard(string& jsonUtf8Str, bool& isShardEnded,
        const string& fileFullPath, ShardType shardType, unsigned int shardSize);

    ShardHandler() = default;
    ~ShardHandler() = default;

    void initIfNeccesary(const string& fileFullPath, const string& startTagForShard,
        size_t shardSize = DefaultShardRowNum);
    int readNextShard(string& jsonUtf8Str, bool& isShardEnded);

    // interface for JsonTransformerLisener
    void startDocument() override;
    void endDocument() override;
    void startElement(const string& tagStr,
        AttributeList& attributes, const string& jsonStrOfThisTag) override;
    // void endElement(const string& tagStr, const string& jsonStr) override;
    void endElement2(const string& tagStr,
        const string& jsonStr, const HandlerExtraInfo& extraInfo) override;
    void characters(const string& chars) override;

    // void setReadFileInfo(const ReadFileInfo& readFileInfo) { mReadFileInfo = readFileInfo; }
    // const ReadFileInfo& getReadFileInfo() const { return mReadFileInfo; }

private:
    inline void appendCurrShardData(const string& jsonStr);
    inline void resetCurrShardData();
    inline void onParsedShardData(
        bool isEnded, const HandlerExtraInfo& extraInfo);
    int deleteThisSherdDataFromFile(size_t startPosLeft);
    void initSimpleXmlHead();

    static std::map<std::string, shared_ptr<ShardHandler>> mMapOfShardHandlers;

    string mFileFullPath;
    string mStartTagForShard;
    size_t mShardSize;

    // for shard
    ShardState mShardState = ShardState_InitState;
    uint32_t mCurrRowIdxForShard = 0;
    string mCurrRowShard;                   // json string stream of utf8 to output

    uint32_t mDepthContentTag = 0;
    bool mInited = false;

    // shared_ptr<ShardHandlerListener> mShardHandlerListener;
    // ReadFileInfo mReadFileInfo;

    // ShardType mShardType = ShardType_WordDocument;
    // bool mWillStartParsedContentTag = false; // whether start parsing contentTag for word's document.xml

    string mSimpleHeadOfXml;
    std::queue<string> mTagQueue;
    bool mSharedEnded = false;
};
