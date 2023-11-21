#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>

#include <iostream>
#include <string>
#include <regex>

#include "Xml2JsonCommon.h"
#include "MemParseHandlers.h"
#include "Xml2JsonCommonInternal.h"
#include "XML2JsonParser.h"
#include "WordSectPrTagParser.h"

using namespace std;
namespace {

const string StrDocumentStartTag    = "<w:document";
const string StrDocumentEndTag      = "</w:document>";
const string StrCloseBrack          = ">";
const string StrSectPrTag           = "w:sectPr";
const string StrSectPrTag2           = "sectPr";
const string StrSectPrStartTag      = "<w:sectPr";
const string StrSectPrEndTag        = "</w:sectPr>";

} // namespace

int WordSectPrTagParser::isLastTagSectPr(bool& isLastTagSectPr) {
    isLastTagSectPr = false;
    auto currPos = mFileContent.rfind(ShardStrForWord_WBody_EndTag);
    if (currPos == string::npos) {
        // for case "<w:body/>"
        return 0;
    }
    currPos = mFileContent.rfind("</", currPos - 1);
    if (currPos == string::npos) {
        cerr << "Err: can't find </w:body>'s previous tag" << endl;
        return -1;
    }
    auto currPos2 = mFileContent.find(StrCloseBrack, currPos);
    if (currPos2 == string::npos) {
        cerr << "Err: can't find previous endTag" << endl;
        return -1;
    }
    string subString = mFileContent.substr(
        // currPos + 2, (currPos2 - 1) - (currPos + 2) + 1);
        currPos + 2, currPos2 - currPos - 2);
    isLastTagSectPr = ((subString == StrSectPrTag) || (subString == StrSectPrTag2));
    return 0;
}

int WordSectPrTagParser::getAllTagOfSectPrForWordDocument(size_t& nextSearchPos) {
    const size_t LenStartTag = StrSectPrStartTag.length();
    const size_t LenEndTag = StrSectPrEndTag.length();
    size_t lenTotalContent = mFileContent.length();
    size_t currStartTagPos;
    string sectPrArrayStr;
    size_t currPos;
    for (currPos = nextSearchPos; currPos < lenTotalContent;) {
        //  search first <w:sectPr> from currPos of mFileContent
        //  if not found; then:
        //      search end
        //      return data;
        //  record currStartTagPos of <w:sectPr>;
        //  search first </w:sectPr> from current <w:sectPr>'s tail of mFileContent
        //  if not found; then:
        //      handle error;
        //      return;
        //  // found; then:
        //  get string from currStartTagPos to the end of current </w:sectPr>
        //  set currPos = the next position of end of current </w:sectPr>
        //  continue;

        // search first <w:sectPr> from currPos of mFileContent
        currPos = mFileContent.find(StrSectPrStartTag, currPos);
        if (currPos == string::npos) {
            // search end
            break;
        }
        // record currStartTagPos of <w:sectPr>;
        currStartTagPos = currPos;

        //  search first </w:sectPr> from current <w:sectPr>'s tail of mFileContent
        currPos = mFileContent.find(StrSectPrEndTag, currPos + LenStartTag);
        if (currPos == string::npos) {
            cerr << "Err: can't find </w:sectPr> for <w:sectPr> of pos: " << currStartTagPos << endl;
            return -1;
        }
        nextSearchPos = currPos;
        //  get string from currStartTagPos to the end of current </w:sectPr>
        currPos = currPos + LenEndTag;
        string subString = mFileContent.substr(
            currStartTagPos, currPos - currStartTagPos);
        sectPrArrayStr += subString;
    }
    bool isTagSectPr = false;
    auto ret = isLastTagSectPr(isTagSectPr);
    if (ret) {
        cerr << "Err: get isLastTagSectPr: " << mFileName << endl;
        return ret;
    }
    mXmlStrToParse += string(ShardStrForWord_WBody_StartTag + " lastSect=")
        + (isTagSectPr ? "\"true\"" : "\"false\"") + StrCloseBrack;
    mXmlStrToParse += sectPrArrayStr;
    mXmlStrToParse += ShardStrForWord_WBody_EndTag;// "</w:body>"

    return 0;
}

int WordSectPrTagParser::getContentBetweenStr(
    size_t nextSearchPos, const string& startStr,
    const string& endStr, bool willSearchCloseBrack) {
    auto startPos = mFileContent.find(startStr, nextSearchPos);
    if (startPos == string::npos) {
        // for case: <w:body/>
        return -2;
    }
    if (willSearchCloseBrack) {
        startPos = mFileContent.find(StrCloseBrack, startPos);
        if (startPos == string::npos) {
            cerr << "Err: get sectPr's brothers: no close brack." << endl;
            return -1;
        }
    }
    auto endPos = mFileContent.find(endStr, startPos);
    if (endPos == string::npos) {
        cerr << "Err: get sectPr's brothers: no: " << endStr << endl;
        return -1;
    }
    endPos--;
    if (willSearchCloseBrack) {
        startPos++;
    } else {
        startPos = startPos + startStr.length();
    }
    mXmlStrToParse += mFileContent.substr(startPos, endPos - startPos + 1);
    return 0;
}

int WordSectPrTagParser::getHeadForWordDocument(size_t& nextSearchPos) {
    auto currPos = mFileContent.find(StrDocumentStartTag);
    if (currPos == string::npos) {
        cerr << "Err: get sectPr: no <w:document>" << endl;
        return -1;
    }
    currPos = mFileContent.find(StrCloseBrack, currPos);
    if (currPos == string::npos) {
        cerr << "Err: get sectPr: no <w:document> close" << endl;
        return -1;
    }
    mXmlStrToParse = mFileContent.substr(0, currPos + 1);
    nextSearchPos = currPos + 1;
    return 0;
}

int WordSectPrTagParser::getSectPrTagStrForWordDocument(string& outStr) {
    int ret = readFileIntoString(mFileContent, mFileName);
    if (ret) {
        cerr << "Err: readFileIntoString: " << mFileName << endl;
        return ret;
    }
    if (!mFileContent.length()) {
        cout << "Err: get sectPr: no content: " << mFileName << endl;
        return -1;
    }
    DurationTimer dt("getAllTagOfSectPrForWordDocument");
    size_t nextSearchPos;
    ret = getHeadForWordDocument(nextSearchPos);
    if (ret) {
        cout << "Err: get <w:sectPr>, no <w:document>: " << mFileName << endl;
        return ret;
    }
    // get <w:body>'s previous brothers
    ret = getContentBetweenStr(0, StrDocumentStartTag,
        ShardStrForWord_WBody_StartTag, true);
    if (ret == -2) {
        // for case: <w:body/>
        return 0;
    }
    if (ret) {
        cout << "Err: get <w:sectPr>: get body's pre brothers : " << mFileName << endl;
        return ret;
    }
    // get <w:sectPr> array
    ret = getAllTagOfSectPrForWordDocument(nextSearchPos);
    if (ret) {
        cout << "Err: get <w:sectPr>: invalid sectPr: " << mFileName << endl;
        return ret;
    }

    // get </w:body>'s post brothers
    ret = getContentBetweenStr(nextSearchPos,
        ShardStrForWord_WBody_EndTag, StrDocumentEndTag);
    if (ret == -2) {
        // for case: <w:body/>
        return 0;
    }
    if (ret) {
        cout << "Err: get <w:sectPr>: get body's post brothers: " << mFileName << endl;
        return ret;
    }
    mXmlStrToParse += StrDocumentEndTag;
    outStr = std::move(mXmlStrToParse);
    // cout << "all: " << outStr << endl;
    return 0;
}

// Get json array of </w:sectPr> for Word's word/document.xml
int GetDocumentSectPrArray(string& jsonStr, const string& fileName) {
    // get <w:sectPr> array of xml content
    string xmlStr;
    WordSectPrTagParser sectPrTagParser(fileName);
    int ret = sectPrTagParser.getSectPrTagStrForWordDocument(xmlStr);
    if (ret) {
        cerr << "Err: get <w:sectPr> for xml: " << fileName << endl;
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
        cerr << "Err: get SectPr: ParseXml2Json error, fileName: " << fileName << endl;
    } else {
        jsonStr = std::move(handlers.jsonUtf8String());
    }
    return ret;
}