#include <fstream>
#include <iostream>
#include <cstring>
#include <regex>
#include <string>

#include "ExcelSharder.h"
#include "MemParseHandlers.h"
#include "XML2JsonParser.h"

using namespace std;
namespace {
int readDataOfFile(string& outString, ifstream& inputFile, int readDataSize) {
    // TODO, for performance
#if 0
    outString.resize(readDataSize + 1);
    char* retBuffer = (char *)outString.data();

    inputFile.read(retBuffer, readDataSize);
    retBuffer[readDataSize] = '\0';
    if (inputFile.eof() || inputFile.fail()) {
        return -1;
    }
#else
    char* retBuffer = (char *)new char[readDataSize+1];
    inputFile.read(retBuffer, readDataSize);
    retBuffer[readDataSize] = '\0';
    if (inputFile.eof() || inputFile.fail()) {
        return -1;
    }
    outString = string(retBuffer);
#endif
    delete[] retBuffer;

    // cout << "hasFound: true: outString: " << outString << endl;
    // cout << "hasFound: true: len(outString): " << outString.length() << endl;
    return 0;
}
#define ErrCode_Unkown      -1
#define ErrCode_NotFound    -2

int getHeadBeforeStartSheetDataElement(string& outString, ifstream& inputFile) {
    const string StrSheetData = "<sheetData>";
    int SheetDataLen = StrSheetData.length();

    inputFile.seekg(0, ios::beg);
    char *buffer = new char[SheetDataLen+1];

    bool hasFound = false;
    int ret = 0;
    while (!inputFile.eof() && !inputFile.fail()) {
        inputFile.read(buffer, SheetDataLen);
        if (inputFile.eof()) {
            ret = ErrCode_NotFound;
            break;
        }
        if (inputFile.fail()) {
            cout << "Err: read file error" << endl;
            ret = ErrCode_Unkown;
            break;
        }
        buffer[SheetDataLen] = '\0';
        if (string(buffer) == StrSheetData) {
            hasFound = true;
            break;
        }
        inputFile.seekg(1 - SheetDataLen, ios::cur);
    }

    if (hasFound) {
        streampos currPos = inputFile.tellg();
        inputFile.seekg(0, ios::beg);
        ret = readDataOfFile(outString, inputFile, currPos);
    }

    delete[] buffer;
    return ret;
}

int getTailAfterEndSheetDataElement(string& outString,
    ifstream& inputFile, const streampos& fileSize) {
    // FIXME, handle <sheetData/>, empty excel
    const string StrSheetData = "</sheetData>";
    int SheetDataLen = StrSheetData.length();

    inputFile.seekg(-SheetDataLen, ios::end);
    char *buffer = new char[SheetDataLen+1];

    bool hasFound = false;
    int ret = 0;
    while (!inputFile.eof() && !inputFile.fail()) {
        inputFile.read(buffer, SheetDataLen);
        if (inputFile.eof()) {
            cout << "Warn: read file error, no " << StrSheetData << endl;
            ret = ErrCode_NotFound;
            break;
        }
        if (inputFile.fail()) {
            cout << "Err: read file error" << endl;
            ret = ErrCode_Unkown;
            break;
        }
        buffer[SheetDataLen] = '\0';
        if (string(buffer) == StrSheetData) {
            hasFound = true;
            break;
        }
        inputFile.seekg(-SheetDataLen-1, ios::cur);
    }

    if (hasFound) {
        inputFile.seekg(-SheetDataLen, ios::cur);
        streampos currPos = inputFile.tellg();

        ret = readDataOfFile(outString, inputFile, fileSize - currPos);
    }

    delete[] buffer;
    return ret;
}

bool findStartEndTagOfSheetDataForExcel(const string& content) {
    string mStartEndTagRegExp = "<sheetData */>";

    regex pattern(mStartEndTagRegExp);
    sregex_iterator iter(content.begin(), content.end(), pattern);
    sregex_iterator end;
    return iter != end;
}

int getContentOfHeadAndTailForExcel(string& outStringOfHeadAndTail, const string& fileName) {
    DurationTimer dt("getContentOfHeadAndTailForExcel");
    ifstream inputFile(fileName.c_str(), ios::binary);
    if (!inputFile) {
        cerr << "[Error] open fileName: " << fileName << endl;
        return ErrCode_Unkown;
    }
    inputFile.seekg(0, ios::end);
    streampos fileSize = inputFile.tellg();
    string outStringOfHead, outStringOfTail;
    int ret = getHeadBeforeStartSheetDataElement(outStringOfHead, inputFile);
    if (ErrCode_NotFound == ret) { // not found "<sheetData>"
        ret = readFileIntoString(outStringOfHeadAndTail, fileName);
        if (ret) {
            cerr << "Err: readFileIntoString: " << fileName << endl;
        }

        if (findStartEndTagOfSheetDataForExcel(outStringOfHeadAndTail)) { // found "<sheetData/>"
            // return the whole file content
        } else { // not found "<sheetData/>"
            ret = ErrCode_NotFound;
        }
    } else if (ret) {
        cout << "[Error] getHeadBeforeStartSheetDataElement error: " << fileName << endl;
    } else {
        ret = getTailAfterEndSheetDataElement(outStringOfTail, inputFile, fileSize);
        if (ret) {
            cerr << "[Error] getTailAfterEndSheetDataElement error: " << fileName << endl;
        }
        outStringOfHeadAndTail = std::move(outStringOfHead + outStringOfTail);
    }

    inputFile.close();

    return ret;
}
}

// Get json of head and tail for Excel's xl/worksheets/sheetxxx.xml
int GetExcelSheetHeadAndTail(string& jsonStr, const string& fileName) {
    // Get xml content of head and tail
    string xmlStr;
    int ret = getContentOfHeadAndTailForExcel(xmlStr, fileName);
    if (ret) {
        cerr << "Err: get for xml: " << fileName << endl;
        return ret;
    }
    if (xmlStr.length() <= 0) {
        return 0;
    }
    DurationTimer dt("ParseXml2Json");

    // get json for xml content
    MemParseHandlers handlers;
    ReadFileInfo readFileInfo;
    readFileInfo.mParseXmlType = ParseXmlType_FromMemory;
    readFileInfo.mXmlFileContent = xmlStr;

    ret = ParseXml2Json(readFileInfo, &handlers);
    if (ret) {
        cerr << "Err: GetExcelSheetHeadAndTail error, fileName: " << fileName << endl;
    } else {
        jsonStr = std::move(handlers.jsonUtf8String());
    }
    return ret;
}
