#include <iostream>
#include <string>
// #include <locale>
// #include <codecvt>
#include <fstream>

#include "MemParseHandlers.h"
#include "ShardHandler.h"
#include "Xml2JsonCommonInternal.h"
#include "XML2JsonParser.h"

#define DepthOfStartTagForShard     1

void ShardHandler::initIfNeccesary(ShardType shardType, size_t shardSize) {
    if (!mStartTagForShard.empty()) {
        return;
    }
    mShardSize = shardSize;
    mStartTagForShard = (ShardType_WordDocument == shardType) ? ShardStrForWord_WBody :
        (ShardType_ExcelSheetx == shardType) ? ShardStr_SheetData : "";
    if (mStartTagForShard.empty()) {
        cerr << "Err: ShardParser::ReadNextShard: wrong shard type: "
            << shardType << endl;
    }

    mShardState = ShardState_InitState;
    mCurrRowIdxForShard = 0;
    mCurrRowShard.clear();
    mDepthContentTag = 0;
    mSimpleHeadOfXml.clear();
    mSharedEnded = false;
    mTotalSizeOfContentHasRead = 0;
}

void ShardHandler::startDocument() {
    mShardState = ShardState_InitState;
    mCurrRowIdxForShard = 0;
    mCurrRowShard.clear();
    mDepthContentTag = 0;
    // mSimpleHeadOfXml.clear(); // Init only once
    mSharedEnded = false;
    mTotalSizeOfContentHasRead = 0;
}

void ShardHandler::endDocument() {
    mSharedEnded = true;
}

void ShardHandler::startElement(const string& tagStr,
    AttributeList& attributes, const string& jsonStrOfThisTag) {
    switch (mShardState) {
        case ShardState_InitState:
            if (mSimpleHeadOfXml.empty()) {
                mShardStack.push(tagStr);
            }
            if (mStartTagForShard == tagStr) {
                mShardState = ShardState_StartedSheetDataElement;
                mDepthContentTag = DepthOfStartTagForShard;
                initSimpleXmlHead();
            }
            break;
        case ShardState_StartedRowElement:
            mDepthContentTag++;
            appendCurrShardData(jsonStrOfThisTag);
            break;
        case ShardState_EndedRowElement:
            // fallthrough
        case ShardState_StartedSheetDataElement:
            if (DepthOfStartTagForShard == mDepthContentTag) {
                mShardState = ShardState_StartedRowElement;
                mDepthContentTag++;
                appendCurrShardData(jsonStrOfThisTag);
            } else {
                cerr << "Err: depth is error." << endl;
            }
            break;
        case ShardState_EndedSheetDataElement:
            break;
        default:
            break;
    }
}

void ShardHandler::appendCurrShardData(const string& jsonStr) {
    mCurrRowShard += jsonStr;
}

void ShardHandler::resetCurrShardData() {
    mCurrRowShard.clear();
}

static void replaceStartingSubstring(std::string& original,
    const std::string& toReplace, const std::string& replacement) {
    if (original.find(toReplace) == 0) {
        original.replace(0, toReplace.length(), replacement);
    }
}

void ShardHandler::onParsedShardData(bool isEnded) {
    if (!mCurrRowShard.empty()) {
        // TODO, for performance, change head at first <row> for this shard
        if (mCurrRowIdxForShard <= mShardSize) {
            // return json array, for the first shard
            replaceStartingSubstring(mCurrRowShard,
                ContentPrefixStr, StartBracketStr);
        } else {
            // return json array, for non first shard
            replaceStartingSubstring(mCurrRowShard,
                CommaStr, StartBracketStr);
        }
        mCurrRowShard += CloseBracketStr;
    }
    // cout << "mCurrRowShard: " << mCurrRowShard << endl;

    if (mSAXParser) {
        mTotalSizeOfContentHasRead = mSAXParser->totalSizeOfContentHasRead();
    }
    cout << "ShardHandler::onParsedShardData: mTotalSizeOfContentHasRead - 2: " << mTotalSizeOfContentHasRead << endl;

    mSharedEnded = isEnded;

    // interrupt the parsing of xml file
    throw UserInterruption();
}

void ShardHandler::initSimpleXmlHead() {
    if (!mSimpleHeadOfXml.empty()) {
        return;
    }
    stack<string> tmpShardStack;
    while (!mShardStack.empty()) {
        tmpShardStack.push(mShardStack.top());
        mShardStack.pop();
    }
    while (!tmpShardStack.empty()) {
        mSimpleHeadOfXml += "<" + tmpShardStack.top() + ">";
        tmpShardStack.pop();
    }
    return;
}

void ShardHandler::endElement(const string& endTagStr,
    const string& jsonStrOfThisTag) {
    switch (mShardState) {
        case ShardState_InitState:
            if (mSimpleHeadOfXml.empty()) {
                mShardStack.pop();
            }
            break;
        case ShardState_StartedRowElement:
            appendCurrShardData(jsonStrOfThisTag);
            mDepthContentTag--;

            if (DepthOfStartTagForShard == mDepthContentTag) {
                mShardState = ShardState_EndedRowElement;
                mCurrRowIdxForShard++;
                if ((mCurrRowIdxForShard % mShardSize) == 0) {
                    onParsedShardData(false);
                }
            }
            break;
        case ShardState_StartedSheetDataElement:
            // fallthrough
        case ShardState_EndedRowElement:
            if (mStartTagForShard == endTagStr) {
                mShardState = ShardState_EndedSheetDataElement;
                onParsedShardData(true);
            } else {
                cerr << "Err: end tag is error for file." << endl;
            }
            break;
        case ShardState_EndedSheetDataElement:
            break;
        default:
            break;
    }
}

void ShardHandler::characters(const string& chars) {
    switch (mShardState) {
        case ShardState_InitState:
            break;
        case ShardState_StartedSheetDataElement:
            break;
        case ShardState_StartedRowElement:
            appendCurrShardData(chars);
            break;
        case ShardState_EndedRowElement:
            break;
        case ShardState_EndedSheetDataElement:
            break;
        default:
            break;
    }
}

void ShardHandler::setSAXParser(SAXParser* parser) {
    mSAXParser = parser;
}
