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

class ShardHandler;

class ShardParser : public JsonTransformerLisener {
public:
    static int readNextShard(string& jsonUtf8Str, bool& isShardEnded,
        const string& fileFullPath, ShardType shardType, unsigned int shardSize);

    ShardParser() = default;
    ~ShardParser() = default;

    void initIfNeccesary(const string& fileFullPath, ShardType shardType,
        size_t shardSize = DefaultShardRowNum);
    int readNextShard(string& jsonUtf8Str, bool& isShardEnded);

private:
    void refreshInputBuff();
    int readFileBlockIntoString(std::string &contentStr,
        const string& fileName, size_t from, size_t sizeToRead);
    int deleteThisShardDataFromBuff(size_t startPosLeft);

    // map from fileFullPath to ShardParser
    static std::map<std::string, shared_ptr<ShardParser>> mMapOfShardParsers;

    string mFileFullPath;

    string mInputBuff;
    size_t mCurrPosToReadFromFile = 0;  // current position to read from file
    bool mIsEof = false;                // is EOF?
    string mSimpleHeadOfXml;

    shared_ptr<ShardHandler> mShardHandler;
};
