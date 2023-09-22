#include <iostream>
#include <string>
// #include <locale>
// #include <codecvt>

#include "ExcelSlicer.h"
#include "ExcelSlicerEmListener.h"

// for slice xml file(sheet1.xml, sheet2.xml, etc.) of .xlsx
const string SliceStr_SheetData     = "sheetData";
const string SliceStr_Row           = "row";
const string SliceStrForWord_WBody  = "w:body";

#define DepthOfStartTagForSlice     1

#define EnableSliceWithDepthOnly    1

void ExcelSlicer::startDocument() {
    mSliceState = SliceState_InitState;
    mCurrRowIdxForSlice = 0;
    mCurrRowSlice.clear();
    mWillStartParsedContentTag = false;
    mSliceType = mReadFileInfo.mSliceType;
    mDepthContentTag = 0;

    // for slice xml file(sheet1.xml, sheet2.xml, etc.) of .xlsx
    if (SliceType_ExcelSheetx == mSliceType) {
        mStartTagForSlice = SliceStr_SheetData;
        mContentTagForSlice = SliceStr_Row;
    } else if (SliceType_WordDocument == mSliceType) {
        mStartTagForSlice = SliceStrForWord_WBody;
    } else {
        cout << "Err: wrong SliceType: " << mSliceType << endl;
    }
    mExcelSlicerListener = make_shared<ExcelSlicerEmListener>();
    mExcelSlicerListener->onStartDocument(*this);
}

void ExcelSlicer::endDocument() {
    if (mExcelSlicerListener != nullptr) {
        mExcelSlicerListener->onEndDocument(*this);
    }
}

void ExcelSlicer::startElement(const string& tagStr,
    AttributeList& attributes, const string& jsonStrOfThisTag) {
    if (SliceType_WordDocument == mSliceType) {
        if (mWillStartParsedContentTag) {
            mContentTagForSlice = tagStr;
            // cout << "ExcelSlicer::startElement: mContentTagForSlice: "
            //     << mContentTagForSlice << endl;
            mWillStartParsedContentTag = false;
        }
    }
    switch (mSliceState) {
        case SliceState_InitState:
            if (mStartTagForSlice == tagStr) {
                mSliceState = SliceState_StartedSheetDataElement;
#if EnableSliceWithDepthOnly
                mDepthContentTag = DepthOfStartTagForSlice;
#else
                if (SliceType_ExcelSheetx == mSliceType) {
#if 0
                    if (mExcelSlicerListener != nullptr) {
                        string jsonStrOfTail;
                        mExcelSlicerListener->onStartSheetDataElement(*this, str, jsonStrOfTail);
                    }
#endif
                } else if (SliceType_WordDocument == mSliceType) {
                    mWillStartParsedContentTag = true;
                }
#endif
            }
            break;
        case SliceState_StartedRowElement:
#if EnableSliceWithDepthOnly
            mDepthContentTag++;
#else
            if (mContentTagForSlice == tagStr) {
                mDepthContentTag++;
            }
#endif
            appendCurrSliceData(jsonStrOfThisTag);
            break;
        case SliceState_EndedRowElement:
            // fallthrough
        case SliceState_StartedSheetDataElement:
#if EnableSliceWithDepthOnly
            if (DepthOfStartTagForSlice == mDepthContentTag) {
                mSliceState = SliceState_StartedRowElement;
                mDepthContentTag++;
                appendCurrSliceData(jsonStrOfThisTag);
            } else {
                cerr << "Err: depth is error for file: " << mReadFileInfo.mFileFullPath;
            }
#else
            if (mContentTagForSlice == tagStr) {
                mSliceState = SliceState_StartedRowElement;
                mDepthContentTag++;
                appendCurrSliceData(jsonStrOfThisTag);
            }
#endif
            break;
        case SliceState_EndedSheetDataElement:
            break;
        default:
            break;
    }
}

void ExcelSlicer::appendCurrSliceData(const string& jsonStr) {
    mCurrRowSlice += jsonStr;
}

void ExcelSlicer::resetCurrSliceData() {
    mCurrRowSlice.clear();
}

static void replaceStartingSubstring(std::string& original,
    const std::string& toReplace, const std::string& replacement) {
    if (original.find(toReplace) == 0) {
        original.replace(0, toReplace.length(), replacement);
    }
}

void ExcelSlicer::onParsedSliceData(bool isEnded) {
    if (!mCurrRowSlice.empty()) {
        // TODO, for performance, change head at first <row> for this slice
        if (mCurrRowIdxForSlice <= mReadFileInfo.mSliceSize) {
            // return json array, for the first slice
            replaceStartingSubstring(mCurrRowSlice,
                ContentPrefixStr, StartBracketStr);
        } else {
            // return json array, for non first slice
            replaceStartingSubstring(mCurrRowSlice,
                CommaStr, StartBracketStr);
        }
        mCurrRowSlice += CloseBracketStr;
    }
    // cout << "mCurrRowSlice: " << mCurrRowSlice << endl;

    if (mExcelSlicerListener != nullptr) {
        mExcelSlicerListener->onParsedShardData(*this, mCurrRowSlice, isEnded);
    }
    resetCurrSliceData();
}

void ExcelSlicer::endElement(const string& endTagStr, const string& jsonStrOfThisTag) {
    switch (mSliceState) {
        case SliceState_InitState:
            break;
        case SliceState_StartedRowElement:
            appendCurrSliceData(jsonStrOfThisTag);
#if EnableSliceWithDepthOnly
            mDepthContentTag--;

            if (DepthOfStartTagForSlice == mDepthContentTag) {
                mSliceState = SliceState_EndedRowElement;
                mCurrRowIdxForSlice++;
                if ((mCurrRowIdxForSlice % mReadFileInfo.mSliceSize) == 0) {
                    onParsedSliceData(false);
                }
            }
#else
            if (mContentTagForSlice == endTagStr) {
                mDepthContentTag--;

                if (!mDepthContentTag) {
                    mSliceState = SliceState_EndedRowElement;
                    mCurrRowIdxForSlice++;
                    if ((mCurrRowIdxForSlice % mReadFileInfo.mSliceSize) == 0) {
                        onParsedSliceData(false);
                    }
                }
            }
#endif
            break;
        case SliceState_StartedSheetDataElement:
            // fallthrough
        case SliceState_EndedRowElement:
            if (mStartTagForSlice == endTagStr) {
                mSliceState = SliceState_EndedSheetDataElement;
                onParsedSliceData(true);
                if (mExcelSlicerListener != nullptr) {
                    mExcelSlicerListener->onEndSheetDataElement(*this);
                }
            }
            break;
        case SliceState_EndedSheetDataElement:
            break;
        default:
            break;
    }
}

void ExcelSlicer::characters(const string& chars) {
    switch (mSliceState) {
        case SliceState_InitState:
            break;
        case SliceState_StartedSheetDataElement:
            break;
        case SliceState_StartedRowElement:
            appendCurrSliceData(chars);
            break;
        case SliceState_EndedRowElement:
            break;
        case SliceState_EndedSheetDataElement:
            break;
        default:
            break;
    }
}