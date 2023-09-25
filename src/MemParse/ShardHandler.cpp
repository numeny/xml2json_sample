#include <iostream>
#include <string>
// #include <locale>
// #include <codecvt>
#include <fstream>

#include "MemParseHandlers.h"
#include "ShardHandler.h"
#include "XML2JsonParser.h"

// for shard xml file(sheet1.xml, sheet2.xml, etc.) of .xlsx
const string ShardStr_SheetData     = "sheetData";
const string ShardStr_Row           = "row";
const string ShardStrForWord_WBody  = "w:body";

#define DepthOfStartTagForShard     1

std::map<std::string, shared_ptr<ShardHandler>> ShardHandler::mMapOfShardHandlers;

int ShardHandler::readNextShard(string& jsonUtf8Str, bool& isShardEnded,
    const string& fileFullPath, ShardType shardType, unsigned int shardSize) {
    shared_ptr<ShardHandler> shardHandler;
    if (mMapOfShardHandlers.find(fileFullPath) != mMapOfShardHandlers.end()) {
        shardHandler = mMapOfShardHandlers[fileFullPath];
        if (!shardHandler) {
            cerr << "Err: ShardHandler::ReadNextShard: null handler." << endl;
            return -1;
        }
    } else {
        string startTagForShard = (ShardType_WordDocument == shardType) ? "w:body" :
            (ShardType_ExcelSheetx == shardType) ? "sheetData" : "";
        if (startTagForShard.empty()) {
            cerr << "Err: ShardHandler::ReadNextShard: wrong shard type: "
                << shardType << endl;
        }
        shardHandler = make_shared<ShardHandler>();
        shardHandler->initIfNeccesary(fileFullPath, startTagForShard, shardSize);
        mMapOfShardHandlers[fileFullPath] = shardHandler;
    }
    return shardHandler->readNextShard(jsonUtf8Str, isShardEnded);
}

int ShardHandler::readNextShard(string& jsonUtf8Str, bool& isShardEnded) {
    // parse xml from file
    ReadFileInfo readFileInfo;
    readFileInfo.mParseXmlType = ParseXmlType_FromFile;
    readFileInfo.mFileFullPath = mFileFullPath;
    // readFileInfo.mFileId       = fileId;
    // readFileInfo.mRelativePath = relativePath;
    // readFileInfo.mWillShard    = true;
    // readFileInfo.mShardType    = mShardType;
    // readFileInfo.mShardSize    = mShardSize;
    readFileInfo.mWillSaveTransformResult   = false;

    MemParseHandlers handlers;
    handlers.setWillSaveTransformResult(false);
    handlers.setLisener(this);

    int ret = ParseXml2Json(readFileInfo, &handlers);
    if ((RetCode_UserInterruption == ret)) {
        ret = 0;
        cout << "[Debug] ParseXml2Json UserInterruption " << endl;
    } else if (ret) {
        cout << "[Error] ParseXml2Json error, ret: " << ret << endl;
    }
    isShardEnded = mSharedEnded;
    jsonUtf8Str = std::move(mCurrRowShard);
    return ret;
}

void ShardHandler::initIfNeccesary(const string& fileFullPath,
    const string& startTagForShard, size_t shardSize) {
    if (!mFileFullPath.empty()) {
        return;
    }
    mFileFullPath = fileFullPath;
    mStartTagForShard = startTagForShard;
    mShardSize = shardSize;
}

void ShardHandler::startDocument() {
    resetCurrShardData();
    mShardState = ShardState_InitState;
    mCurrRowIdxForShard = 0;
    mCurrRowShard.clear();                   // json string stream of utf8 to output

    mDepthContentTag = 0;
    // mShardHandlerListener = make_shared<ShardHandlerEmListener>();
    // mShardHandlerListener->onStartDocument(*this);
}

void ShardHandler::endDocument() {
    // if (mShardHandlerListener != nullptr) {
    //     mShardHandlerListener->onEndDocument(*this);
    // }
    mSharedEnded = true;
}

void ShardHandler::startElement(const string& tagStr,
    AttributeList& attributes, const string& jsonStrOfThisTag) {
    switch (mShardState) {
        case ShardState_InitState:
            if (mSimpleHeadOfXml.empty()) {
                mTagQueue.push("<" + tagStr + ">");
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
                cerr << "Err: depth is error for file: " << mFileFullPath;
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

void ShardHandler::onParsedShardData(
    bool isEnded, const HandlerExtraInfo& extraInfo) {
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

    deleteThisSherdDataFromFile(extraInfo.mTotalSizeOfContentHasRead);
    mSharedEnded = isEnded;
    // if (mShardHandlerListener != nullptr) {
    //     mShardHandlerListener->onParsedShardData(*this, mCurrRowShard, isEnded);
    // }
    // interrupt the parsing of xml file
    cout << "Debug: onParsedShardData will throw UserInterruption(): " << endl;

    throw UserInterruption();
}

void ShardHandler::initSimpleXmlHead() {
    if (!mSimpleHeadOfXml.empty()) {
        return;
    }

    while(!mTagQueue.empty()) {
        string elem = mTagQueue.front();
        mSimpleHeadOfXml += elem;
        mTagQueue.pop();
    }
    return;
}

#include <cstdio>

static int writeXmlFile2(const string& fileName, const string& fileContent) {
    cout << "File writing start: " << fileName << std::endl;
    std::ofstream outputFile(fileName);
    if (!outputFile.is_open()) {
        std::cerr << "Unable to open the file." << std::endl;
        return 1;
    }

    outputFile << fileContent;

    outputFile.close();

    cout << "Debug: File writing complete: " << fileName << std::endl;

    return 0;
}

int ShardHandler::deleteThisSherdDataFromFile(size_t startPosLeft) {
    // concat with simple header of xml and left file content
    // and write to file
    string allContentStr;

    int ret = readFileIntoString(allContentStr, mFileFullPath);
    if (ret) {
        cerr << "Err: deleteThisSherdDataFromFile, read err: "
            << mFileFullPath << endl;
    }

    // printf("Debug: deleteThisSherdDataFromFile: startPosLeft: %ld\n%s\n\n\n",
    //     startPosLeft, &allContentStr[startPosLeft]);

    if (mSimpleHeadOfXml.empty()) {
        cerr << "Err: deleteThisSherdDataFromFile, simple headOf xml is empty: "
            << mFileFullPath << endl;
    }
    string outContentStr(mSimpleHeadOfXml);
    outContentStr += allContentStr.substr(startPosLeft, allContentStr.length() - startPosLeft);
    ret = writeXmlFile2(mFileFullPath, outContentStr);
    if (ret) {
        cerr << "Err: writeXmlFile2 err: " << mFileFullPath << endl;
    }
    return ret;
}

void ShardHandler::endElement2(const string& endTagStr,
    const string& jsonStrOfThisTag, const HandlerExtraInfo& extraInfo) {
    switch (mShardState) {
        case ShardState_InitState:
            if (mSimpleHeadOfXml.empty()) {
                mTagQueue.push("</" + endTagStr + ">");
            }
            break;
        case ShardState_StartedRowElement:
            appendCurrShardData(jsonStrOfThisTag);
            mDepthContentTag--;

            if (DepthOfStartTagForShard == mDepthContentTag) {
                mShardState = ShardState_EndedRowElement;
                mCurrRowIdxForShard++;
                if ((mCurrRowIdxForShard % mShardSize) == 0) {
                    onParsedShardData(false, extraInfo);
                }
            }
            break;
        case ShardState_StartedSheetDataElement:
            // fallthrough
        case ShardState_EndedRowElement:
            if (mStartTagForShard == endTagStr) {
                mShardState = ShardState_EndedSheetDataElement;
                onParsedShardData(true, extraInfo);
                // if (mShardHandlerListener != nullptr) {
                //     mShardHandlerListener->onEndSheetDataElement(*this);
                // }
            } else {
                cerr << "Err: end tag is error for file: " << mFileFullPath;
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