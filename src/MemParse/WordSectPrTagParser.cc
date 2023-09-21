#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>

#include <iostream>
#include <string>
#include <regex>

#include "Common.h"
#include "MemParseHandlers.h"
#include "XML2JsonParser.h"

using namespace std;

const string StrStartTag = "<w:document";
const string StrEndTag = "</w:document>";
const string StrBrack = ">";

namespace {

int getAllTagOfSectPrForWordDocument(string& outTagArrays, const string& content) {
    const string StrStartTag = "<w:sectPr";
    const string StrEndTag = "</w:sectPr>";
    const size_t LenStartTag = StrStartTag.length();
    const size_t LenEndTag = StrEndTag.length();
    size_t lenTotalContent = content.length();
    size_t currStartTagPos;

    for (size_t currPos = 0; currPos < lenTotalContent;) {
        //  search first <w:sectPr> from currPos of content
        //  if not found; then:
        //      search end
        //      return data;
        //  record currStartTagPos of <w:sectPr>;
        //  search first </w:sectPr> from current <w:sectPr>'s tail of content
        //  if not found; then:
        //      handle error;
        //      return;
        //  // found; then:
        //  get string from currStartTagPos to the end of current </w:sectPr>
        //  set currPos = the next position of end of current </w:sectPr>
        //  continue;

        // search first <w:sectPr> from currPos of content
        currPos = content.find(StrStartTag, currPos);
        if (currPos == string::npos) {
            // search end
            return 0;
        }
        // record currStartTagPos of <w:sectPr>;
        currStartTagPos = currPos;

        //  search first </w:sectPr> from current <w:sectPr>'s tail of content
        currPos = content.find(StrEndTag, currPos + LenStartTag);
        if (currPos == string::npos) {
            cout << "Err: can't find </w:sectPr> for <w:sectPr> of pos: " << currStartTagPos << endl;
            // FIXME, handle error;
            return -1;
        }
        //  get string from currStartTagPos to the end of current </w:sectPr>
        currPos = currPos + LenEndTag;
        string subString = content.substr(
            currStartTagPos, currPos - currStartTagPos);
        // cout << "Debug: subString: " << subString << endl;
        outTagArrays += subString;
    }
    return 0;
}

int getHeadForWordDocument(string& outTagArrays, const string& content) {
    auto currPos = content.find(StrStartTag);
    if (currPos == string::npos) {
        return -1;
    }
    currPos = content.find(StrBrack, currPos);
    if (currPos == string::npos) {
        return -1;
    }
    outTagArrays = content.substr(0, currPos+1);
    return 0;
}

int getSectPrTagStrForWordDocument(string& outStr, const string& fileName) {
    string content;
    int ret = readFileIntoString(content, fileName);
    if (ret) {
        cerr << "Err: readFileIntoString: " << fileName << endl;
        return ret;
    }
    if (!content.length()) {
        cout << "Err: no content: " << fileName << endl;
        return -1;
    }
    DurationTimer dt("getAllTagOfSectPrForWordDocument");

    ret = getHeadForWordDocument(outStr, content);
    if (ret) {
        cout << "Err: no <w:document>: " << fileName << endl;
        return ret;
    }
    ret = getAllTagOfSectPrForWordDocument(outStr, content);
    if (ret) {
        cout << "Err: get <w:sectPr>: " << fileName << endl;
        return ret;
    }
    outStr += StrEndTag;
    // cout << "all: " << outStr << endl;
    return 0;
}

} // namespace

// Get json array of </w:sectPr> for Word's word/document.xml
int GetDocumentSectPrArray(string& jsonStr, const string& fileName) {
    // get <w:sectPr> array of xml content
    string xmlStr;
    int ret = getSectPrTagStrForWordDocument(xmlStr, fileName);
    if (ret) {
        cout << "Err: get <w:sectPr> for xml: " << fileName << endl;
        return ret;
    }
    if (xmlStr.length() <= 0) {
        return 0;
    }

    // get json for xml content
    MemParseHandlers handlers;
    ReadFileInfo readFileInfo;
    readFileInfo.mParseXmlType = ParseXmlType_FromMemory;
    readFileInfo.mXmlFileContent = xmlStr;
    DurationTimer dt("ParseXml2Json");

    ret = ParseXml2Json(readFileInfo, &handlers);
    if (ret) {
        cout << "[Error] ParseXml2Json error, fileName: " << fileName << endl;
    } else {
        jsonStr = std::move(handlers.jsonUtf8String());
    }
    return ret;
}