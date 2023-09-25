#include <iostream>
#include <string>
// #include <locale>
// #include <codecvt>

#include "ExcelSharder.h"
#include "ExcelSharderEmListener.h"

// for shard xml file(sheet1.xml, sheet2.xml, etc.) of .xlsx
const string ShardStr_SheetData     = "sheetData";
const string ShardStr_Row           = "row";
const string ShardStrForWord_WBody  = "w:body";

#define DepthOfStartTagForShard     1

#define EnableShardWithDepthOnly    1

void ExcelSharder::startDocument() {
    mShardState = ShardState_InitState;
    mCurrRowIdxForShard = 0;
    mCurrRowShard.clear();
    mWillStartParsedContentTag = false;
    mShardType = mReadFileInfo.mShardType;
    mDepthContentTag = 0;

    // for shard xml file(sheet1.xml, sheet2.xml, etc.) of .xlsx
    if (ShardType_ExcelSheetx == mShardType) {
        mStartTagForShard = ShardStr_SheetData;
        mContentTagForShard = ShardStr_Row;
    } else if (ShardType_WordDocument == mShardType) {
        mStartTagForShard = ShardStrForWord_WBody;
    } else {
        cout << "Err: wrong ShardType: " << mShardType << endl;
    }
    mExcelSharderListener = make_shared<ExcelSharderEmListener>();
    mExcelSharderListener->onStartDocument(*this);
}

void ExcelSharder::endDocument() {
    if (mExcelSharderListener != nullptr) {
        mExcelSharderListener->onEndDocument(*this);
    }
}

void ExcelSharder::startElement(const string& tagStr,
    AttributeList& attributes, const string& jsonStrOfThisTag) {
    if (ShardType_WordDocument == mShardType) {
        if (mWillStartParsedContentTag) {
            mContentTagForShard = tagStr;
            // cout << "ExcelSharder::startElement: mContentTagForShard: "
            //     << mContentTagForShard << endl;
            mWillStartParsedContentTag = false;
        }
    }
    switch (mShardState) {
        case ShardState_InitState:
            if (mStartTagForShard == tagStr) {
                mShardState = ShardState_StartedSheetDataElement;
#if EnableShardWithDepthOnly
                mDepthContentTag = DepthOfStartTagForShard;
#else
                if (ShardType_ExcelSheetx == mShardType) {
#if 0
                    if (mExcelSharderListener != nullptr) {
                        string jsonStrOfTail;
                        mExcelSharderListener->onStartSheetDataElement(*this, str, jsonStrOfTail);
                    }
#endif
                } else if (ShardType_WordDocument == mShardType) {
                    mWillStartParsedContentTag = true;
                }
#endif
            }
            break;
        case ShardState_StartedRowElement:
#if EnableShardWithDepthOnly
            mDepthContentTag++;
#else
            if (mContentTagForShard == tagStr) {
                mDepthContentTag++;
            }
#endif
            appendCurrShardData(jsonStrOfThisTag);
            break;
        case ShardState_EndedRowElement:
            // fallthrough
        case ShardState_StartedSheetDataElement:
#if EnableShardWithDepthOnly
            if (DepthOfStartTagForShard == mDepthContentTag) {
                mShardState = ShardState_StartedRowElement;
                mDepthContentTag++;
                appendCurrShardData(jsonStrOfThisTag);
            } else {
                cerr << "Err: depth is error for file: " << mReadFileInfo.mFileFullPath;
            }
#else
            if (mContentTagForShard == tagStr) {
                mShardState = ShardState_StartedRowElement;
                mDepthContentTag++;
                appendCurrShardData(jsonStrOfThisTag);
            }
#endif
            break;
        case ShardState_EndedSheetDataElement:
            break;
        default:
            break;
    }
}

void ExcelSharder::appendCurrShardData(const string& jsonStr) {
    mCurrRowShard += jsonStr;
}

void ExcelSharder::resetCurrShardData() {
    mCurrRowShard.clear();
}

static void replaceStartingSubstring(std::string& original,
    const std::string& toReplace, const std::string& replacement) {
    if (original.find(toReplace) == 0) {
        original.replace(0, toReplace.length(), replacement);
    }
}

void ExcelSharder::onParsedShardData(bool isEnded) {
    if (!mCurrRowShard.empty()) {
        // TODO, for performance, change head at first <row> for this shard
        if (mCurrRowIdxForShard <= mReadFileInfo.mShardSize) {
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

    if (mExcelSharderListener != nullptr) {
        mExcelSharderListener->onParsedShardData(*this, mCurrRowShard, isEnded);
    }
    resetCurrShardData();
}

void ExcelSharder::endElement(const string& endTagStr, const string& jsonStrOfThisTag) {
    switch (mShardState) {
        case ShardState_InitState:
            break;
        case ShardState_StartedRowElement:
            appendCurrShardData(jsonStrOfThisTag);
#if EnableShardWithDepthOnly
            mDepthContentTag--;

            if (DepthOfStartTagForShard == mDepthContentTag) {
                mShardState = ShardState_EndedRowElement;
                mCurrRowIdxForShard++;
                if ((mCurrRowIdxForShard % mReadFileInfo.mShardSize) == 0) {
                    onParsedShardData(false);
                }
            }
#else
            if (mContentTagForShard == endTagStr) {
                mDepthContentTag--;

                if (!mDepthContentTag) {
                    mShardState = ShardState_EndedRowElement;
                    mCurrRowIdxForShard++;
                    if ((mCurrRowIdxForShard % mReadFileInfo.mShardSize) == 0) {
                        onParsedShardData(false);
                    }
                }
            }
#endif
            break;
        case ShardState_StartedSheetDataElement:
            // fallthrough
        case ShardState_EndedRowElement:
            if (mStartTagForShard == endTagStr) {
                mShardState = ShardState_EndedSheetDataElement;
                onParsedShardData(true);
                if (mExcelSharderListener != nullptr) {
                    mExcelSharderListener->onEndSheetDataElement(*this);
                }
            }
            break;
        case ShardState_EndedSheetDataElement:
            break;
        default:
            break;
    }
}

void ExcelSharder::characters(const string& chars) {
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