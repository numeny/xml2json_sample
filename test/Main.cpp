#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include <ctime>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "XML2JsonParser.h"
#include "Common.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

const char*  gXMLInMemBuf =
"\
<?xml version='1.0' encoding='UTF-8' standalone='yes'?>\n\
<worksheet xmlns='http://schemas.openxmlformats.org/spreadsheetml/2006/main' xmlns:r='http://schemas.openxmlformats.org/officeDocument/2006/relationships' xmlns:xdr='http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing' xmlns:x14='http://schemas.microsoft.com/office/spreadsheetml/2009/9/main' xmlns:mc='http://schemas.openxmlformats.org/markup-compatibility/2006' xmlns:etc='http://www.wps.cn/officeDocument/2017/etCustomData'>\n\
    <sheetPr>\n\
        <pageSetUpPr fitToPage='1'/>\n\
    </sheetPr>\n\
    <dimension ref='A1:CE100'/>\n\
    <sheetViews>\n\
        <sheetView zoomScale='70' zoomScaleNormal='70' zoomScaleSheetLayoutView='50' workbookViewId='0'>\n\
            <selection activeCell='D47' sqref='A1:CD100'/>\n\
        </sheetView>\n\
    </sheetViews>\n\
    <sheetFormatPr defaultColWidth='9' defaultRowHeight='15.75' customHeight='1'/>\n\
    <cols>\n\
        <col min='1' max='1' width='6.16153846153846' style='14' customWidth='1'/>\n\
        <col min='83' max='16384' width='9' style='11'/>\n\
    </cols>\n\
    <sheetData>mmm</sheetData>\n\
    <legacyDrawing />\n\
</worksheet>\n\
";

using namespace std;

static bool endsWith(const std::string& str, const std::string& ending) {
    if (str.length() >= ending.length()) {
        return (str.compare(str.length() - ending.length(), ending.length(), ending) == 0);
    } else {
        return false;
    }
}

#define DebugJsonStr 1

#if !defined(EMSCRIPTEN)

int main(int argC, char* argV[])
{
    // string xmlFilePath = "/mnt/c/Users/docs/doc/机械设备_机车车辆_中文件/xl/worksheets/sheet2.xml"; // default xml file path
    // string xmlFilePath = "/mnt/c/Users/docs/doc/sheet1_1G.xml"; // default xml file path
    // string xmlFilePath = "/mnt/c/Users/docs/doc/Big.docx/word/document.xml"; // default xml file path
    string xmlFilePath = "/mnt/c/Users/docs/doc/c.xlsx/xl/worksheets/sheet1.xml"; // default xml file path
    // string xmlFilePath = "/mnt/c/Users/docs/doc/P2.docx/word/document.xml"; // default xml file path
    // string xmlFilePath = "/mnt/c/Users/docs/doc/smal.xlsx/small/xl/worksheets/sheet1.xml"; // default xml file path

    // "Usage:"
    // "       command [0|1|2|3] [xxx]"
    // " 0 - parse xml from memory(xml string), the same as no parameter"
    // " 1 - parse directly one xml of @xxx (default is @xmlFilePath) to json string."
    // " 2 - read one xml of @xxx (default is @xmlFilePath)
    //       and use memory cache to parse (default is @xmlFilePath) to json string."
    // " 3 - parse all xml file and save to json file for a directory @xxx."
    bool parseFromMem = true;
    bool parseFromFileDirectly = true;
    bool parseFromFileSlice = true;
    bool getSectPrArrayForWordDocument = false;
    bool getHeadAndTailForExcelSheet = false;
    if (argC >= 2) {
        if ("0" == string(argV[1])) {
            parseFromMem = true;
        } else if ("1" == string(argV[1])) {
            parseFromMem = false;
            parseFromFileDirectly = true;
            parseFromFileSlice = false;
        } else if ("2" == string(argV[1])) {
            parseFromMem = false;
            parseFromFileDirectly = true;
            parseFromFileSlice = true;
        } else if ("3" == string(argV[1])) {
            parseFromMem = false;
            parseFromFileDirectly = false;
            parseFromFileSlice = false;
        } else if ("4" == string(argV[1])) {
            getSectPrArrayForWordDocument = true;
        } else if ("5" == string(argV[1])) {
            getHeadAndTailForExcelSheet = true;
        } else {
            parseFromMem = false;
            parseFromFileDirectly = true;
            parseFromFileSlice = false;
        }
        if (argC >= 3) {
            xmlFilePath = string(argV[2]);
        }
    }

    string jsonUtf8Str; // output json string
    ReadFileInfo readFileInfo;
    if (getSectPrArrayForWordDocument) {
        int ret = GetDocumentSectPrArray(jsonUtf8Str, xmlFilePath);
#if DebugJsonStr
        cout << "Parse json string: " << jsonUtf8Str << endl;
#else
        cout << "Parse json string: ended" << endl;
#endif
        return 0;
    }
    if (getHeadAndTailForExcelSheet) {
        int ret = GetExcelSheetHeadAndTail(jsonUtf8Str, xmlFilePath);
#if DebugJsonStr
        cout << "Parse json string: " << jsonUtf8Str << endl;
#else
        cout << "Parse json string: ended" << endl;
#endif
        return 0;
    }
    if (parseFromMem) {
        readFileInfo.mParseXmlType = ParseXmlType_FromMemory;
        readFileInfo.mXmlFileContent = gXMLInMemBuf;
        int ret = ParseXml2Json(jsonUtf8Str, readFileInfo);
#if DebugJsonStr
        cout << "Parse json string: " << jsonUtf8Str << endl;
#else
        cout << "Parse json string: ended" << endl;
#endif
        return 0;
    }

    cout << "Parsing xml file: " << xmlFilePath << endl;

    static unsigned long t0, t1, t2;
    t0 = XMLPlatformUtils::getCurrentMillis();
    SliceType sliceType = SliceType_WordDocument;
    if (xmlFilePath.find("word/document.xml") != string::npos) {
        sliceType = SliceType_WordDocument;
    } else if (xmlFilePath.find("xl/worksheets/sheet") != string::npos) {
        sliceType = SliceType_ExcelSheetx;
    }

    if (parseFromFileDirectly) {
        int ret = GetFileContent(jsonUtf8Str, xmlFilePath, "fileId",
            xmlFilePath, parseFromFileSlice, sliceType/*, 1*/);
        t1 = XMLPlatformUtils::getCurrentMillis();
        cout << "Parse time(ms) : " << (t1 - t0) <<endl;
    } else {
        string str;
        int ret = readFileIntoString(str, xmlFilePath);
        t1 = XMLPlatformUtils::getCurrentMillis();
        readFileInfo.mParseXmlType = ParseXmlType_FromMemory;
        readFileInfo.mXmlFileContent = str;
        ret = ParseXml2Json(jsonUtf8Str, readFileInfo);
        t2 = XMLPlatformUtils::getCurrentMillis();
        cout << "Read file time(ms) : " << (t1 - t0) <<endl;
        cout << "Parse time(ms) : " << (t2 - t1) <<endl;
    }
#if DebugJsonStr
        cout << "Parse json string: " << jsonUtf8Str << endl;
#else
        cout << "Parse json string: ended" << endl;
#endif
    return 0;
}
#endif
