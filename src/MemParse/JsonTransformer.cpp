#include <iostream>
#include <string>
// #include <locale>
// #include <codecvt>

#include "JsonTransformer.h"

const string TagPrefixStr           = "{\"t\":\"";      // tagName
const string NamespacePrefixStr     = ",\"ns\":\"";     // namespace for this tagName or attribute
const string AttrPrefixStr          = ",\"as\":[";      // attrs for this tagName
const string NamePrefixStrOfAttr    = "{\"ak\":\"";     // key of attrs for this tagName
const string ValuePrefixStrOfAttr   = "\",\"av\":\"";   // value of attrs for this tagName
const string ContentPrefixStr       = ",\"cs\":[";      // contents(children) for this tagName
const string TextPrefixStr          = ",\"tx\":\"";     // text for this tagName
const string QuotationStr           = "\"";
const string CommaStr               = ",";
const string StartBracketStr        = "[";
const string CloseBracketStr        = "]";
const string CloseCurlyBracketStr   = "}";
const string EnterStr               = "\n";
const string IndentsStr             = "    ";
const string BlanksStr              = "\f\v\r\t\n ";
const string ColonStr               = ":";

// Enable output json string's indentation
#define EnableFormatJsonStream  0

#define WithMyStringCat 1

namespace {

#if 0
void XMLChToUtf8(string& outUtf8, const XMLCh* const xmlChs)
{
    // performance is a problem
#if WithMyStringCat
    outUtf8 = wstring_convert<codecvt_utf8<char16_t>, char16_t >{}.to_bytes(xmlChs);
#endif
}
#else
void XMLChToUtf8(string& utf8str, const XMLCh* u16str, bool escapedEnter = true) {
#if WithMyStringCat
    if (nullptr == u16str) {
        return;
    }
    char tmpChar;
    for (size_t i = 0; u16str[i] != u'\0';) {
        char32_t codepoint = u16str[i++];

        if (codepoint >= 0xD800 && codepoint <= 0xDBFF && u16str[i] >= 0xDC00 && u16str[i] <= 0xDFFF) {
            // Surrogate pair handling
            codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (u16str[i++] - 0xDC00);
        }
        if (codepoint <= 0x7F) {
            tmpChar = static_cast<char>(codepoint);
            if ((codepoint >= 0x00 && codepoint <= 0x08)
                || (codepoint >= 0x0B && codepoint <= 0x0C)
                || (codepoint >= 0x0E && codepoint <= 0x1F)
                || codepoint == 0x7F) {
                // controle charactor, ignore, \\x00-\\x08\\x0b-\\x0c\\x0e-\\x1f\\x7f
                continue;
            }
            switch (tmpChar) {
            // handle escaped chars
            case '\"':
                utf8str += "\\\"";
                break;
            case '\\':
                utf8str += "\\\\";
                break;
            default:
                if (escapedEnter) {
                    // handle escaped chars
                    switch (tmpChar) {
                        case '\n': // "\n" --> "\\n"
                            utf8str += "\\n";
                            break;
                        case '\r': // "\r" --> "\\r"
                            utf8str += "\\r";
                            break;
                        default:
                            utf8str += tmpChar;
                            break;
                    }
                } else {
                    utf8str += tmpChar;
                }
                break;
            }
        }
        else if (codepoint <= 0x7FF) {
            utf8str += static_cast<char>((codepoint >> 6) | 0xC0);
            utf8str += static_cast<char>((codepoint & 0x3F) | 0x80);
        }
        else if (codepoint <= 0xFFFF) {
            utf8str += static_cast<char>((codepoint >> 12) | 0xE0);
            utf8str += static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
            utf8str += static_cast<char>((codepoint & 0x3F) | 0x80);
        }
        else if (codepoint <= 0x10FFFF) {
            utf8str += static_cast<char>((codepoint >> 18) | 0xF0);
            utf8str += static_cast<char>(((codepoint >> 12) & 0x3F) | 0x80);
            utf8str += static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
            utf8str += static_cast<char>((codepoint & 0x3F) | 0x80);
        }
    }
#endif
}
#endif

// trim special character in the front and end of @str
void trim(string& str)
{
#if WithMyStringCat
    str.erase(0, str.find_first_not_of(BlanksStr));
    str.erase(str.find_last_not_of(BlanksStr) + 1);
#endif
}

void addNamespaceToTagOrAttrIfNeccessary(string& jsonStr, const string& tagOrAttrbuteStr) {
    auto colonPos = tagOrAttrbuteStr.find(ColonStr);
    if (colonPos == string::npos) {
        jsonStr += tagOrAttrbuteStr;
        return;
    }
    // namespace exists
    string strNamespace = tagOrAttrbuteStr.substr(0, colonPos);
    string tagNameOnly = tagOrAttrbuteStr.substr(
        colonPos+1, tagOrAttrbuteStr.length() - colonPos);
    jsonStr += tagNameOnly;
    if (strNamespace.length() > 0) {
        jsonStr += QuotationStr;
        jsonStr += NamespacePrefixStr;
        jsonStr += strNamespace;
    }
}

// Add ", attr: [] "
void attrs2JsonStr(string& jsonStr, const AttributeList& attributes,
    bool& hasAttributeOfPreserveSpace) {
    XMLSize_t totalSize = attributes.getLength();
    if (totalSize > 0) {
        // ADD ", attr: ["
        jsonStr += AttrPrefixStr;
    }
    for (XMLSize_t idx = 0; idx < totalSize; idx++) {
        if (idx > 0) {
            jsonStr += CommaStr;
        }

        string strName;
        string strVal;
        XMLChToUtf8(strName, attributes.getName(idx));
        XMLChToUtf8(strVal, attributes.getValue(idx));
        if (strName == "xml:space" && strVal == "preserve") {
            hasAttributeOfPreserveSpace = true;
        }
        // ADD type
        jsonStr += NamePrefixStrOfAttr;

        addNamespaceToTagOrAttrIfNeccessary(jsonStr, strName);

        // ADD val
        jsonStr += ValuePrefixStrOfAttr;
        jsonStr += strVal;

        jsonStr += QuotationStr;
        jsonStr += CloseCurlyBracketStr;
    }
    if (totalSize > 0) {
        // ADD "]"
        jsonStr += CloseBracketStr;
    }
}

// Add " { tag: xx, attr: [{},{}]"
void tagAndAttrs2JsonStr(string& jsonStr, const string& tagStr,
    AttributeList& attributes, bool& hasAttributeOfPreserveSpace) {
    jsonStr += TagPrefixStr;
    addNamespaceToTagOrAttrIfNeccessary(jsonStr, tagStr);

    jsonStr += QuotationStr;
    attrs2JsonStr(jsonStr, attributes, hasAttributeOfPreserveSpace);
}

bool replaceStr(string& originalString, const string &searchString, const string& replacementString) {
    size_t pos = 0;
    while ((pos = originalString.find(searchString, pos)) != std::string::npos) {
        originalString.replace(pos, searchString.length(), replacementString);
        pos += replacementString.length();
    }
    return 0;
}

// Add ", text: xxx "
bool appendTextWithPrefix(string& jsonStr, const XMLCh* const textStr,
    bool withPrefix, shared_ptr<StackElement> element) {
    if (nullptr == textStr) {
        // cout << "[Error] JsonTransformer::appendText(nullptr)" << endl;
        return false;
    }

    string strText;
    XMLChToUtf8(strText, textStr, false);

    if (!element || !element->mHasAttributeOfPreserveSpace) {
        // reserve the space in "tx",
        // when no attribute of xml:space="preserve"
        string strTextTmp(strText);
        if (withPrefix) {
            trim(strTextTmp);
        }
        if (strTextTmp.length() <= 0) {
            return false;
        }
    }

    // handle escaped chars
    replaceStr(strText, "\n", "\\n");
    replaceStr(strText, "\r", "\\r");
    replaceStr(strText, "\f", "\\f");
    replaceStr(strText, "\v", "\\v");
    replaceStr(strText, "\t", "\\t");

    if (withPrefix) {
        jsonStr += TextPrefixStr;
    }
    jsonStr += strText;
    // appendJsonStream(QuotationStr);
    return true;
}

} // namespace

void JsonTransformer::startDocument() {
    stack<shared_ptr<StackElement>> tmp;
    mStack.swap(tmp);
    mJsonStream = string();
    if (mJsonTransformerLisener) {
        mJsonTransformerLisener->startDocument();
    }
}

void JsonTransformer::endDocument() {
    if (mJsonTransformerLisener) {
        mJsonTransformerLisener->endDocument();
    }
}

void JsonTransformer::appendJsonStream(const string& jsonStr) {
    if (mWillSaveTransformResult) {
#if WithMyStringCat
        mJsonStream += jsonStr;
#endif
    }
}

void JsonTransformer::appendIndents() {
    // ADD format space
    size_t size = mStack.size();
    string spacePre;
    appendJsonStream(EnterStr);
    for(size_t idx = 0; idx < mStack.size(); idx++) {
        spacePre += IndentsStr;
    }
    appendJsonStream(spacePre);
}

void JsonTransformer::startElement(const XMLCh* const name, AttributeList& attributes) {
    // 1. switch (stack has element and stateMachineOfLastElementInStack) {
    //       NoContent: Add ", content: ["; set WithContent to top stack element;
    //       WithContent: Add ","
    //    }
    // 2. Add " { tag: xx, attr: mm "
    // 3. push stack new element;
    string tagStr, jsonStrOfThisTag;

    size_t size = mStack.size();
    if (size > 0) {
        shared_ptr<StackElement> element = mStack.top();
        if (nullptr != element) {
            switch (element->mStackElementState) {
                case NoContent:
                    jsonStrOfThisTag += ContentPrefixStr;
                    element->mStackElementState = WithContent;
                    break;
                case WithContent:
                    jsonStrOfThisTag += CommaStr;
                    break;
                default:
                    break;
            }
        }
    }

    // if this element is ShardStr_SheetData
    // set ShardState is Started_SheetData

    // Add " { tag: xx, attr: [{},{}]"
    XMLChToUtf8(tagStr, name);

    bool hasAttributeOfPreserveSpace = false; // has attribute: xml:space="preserve"
    tagAndAttrs2JsonStr(jsonStrOfThisTag, tagStr,
        attributes, hasAttributeOfPreserveSpace);
    appendJsonStream(jsonStrOfThisTag);

    // push stack new element;
    shared_ptr<StackElement> newElement(
        new StackElement(hasAttributeOfPreserveSpace));
    mStack.push(newElement);

    if (mJsonTransformerLisener) {
        mJsonTransformerLisener->startElement(tagStr, attributes, jsonStrOfThisTag);
    }
}

void JsonTransformer::endElement(const XMLCh* const name) {
    // 1. switch (stack has element and stateMachineOfLastElementInStack) {
    //     WithContent: Add "]"
    //
    // 2. pop stack top element;
    // 3. Add " }"

    shared_ptr<StackElement> element = mStack.top();
    if (nullptr == element) {
        // error, FIXME
        return;
    }
    string tagStr, jsonStr;
    switch (element->mStackElementState) {
        case WithContent:
            // WithContent: Add "]"
            jsonStr += CloseBracketStr;
            break;
        case InCharacter:
            // InCharacter: Add "\""
            jsonStr += QuotationStr;
        default:
            break;
    }
    XMLChToUtf8(tagStr, name);
	// if (name != TopStatckElement) {
    //     // FIXME, error
    // }
    mStack.pop();

    if (EnableFormatJsonStream) {
        appendIndents();
    }

    // Add " }"
    jsonStr += CloseCurlyBracketStr;

    appendJsonStream(jsonStr);

    if (mJsonTransformerLisener) {
        mJsonTransformerLisener->endElement(tagStr, jsonStr);
    }
}

void JsonTransformer::characters(const XMLCh* const chars, const XMLSize_t length) {
    if (chars == nullptr) {
        return;
    }
    // characters() callback may be called many time for following case:
    // <f>ROUND((INT([4]����!F$9&amp;&quot;/&quot;&amp;[4]����!H$9&amp;&quot;/&quot;&amp;[4]����!J$9)-K99)/365,2)</f>
    shared_ptr<StackElement> element = mStack.top();
    if (nullptr == element) {
        // error, FIXME
        return;
    }
    string jsonStr;
    bool ret = false;
    switch (element->mStackElementState) {
        case NoContent:
            // Add ", text: xxx "
            ret = appendTextWithPrefix(jsonStr, chars, true, element);
            break;
        case InCharacter:
            // Add "xxx"
            ret = appendTextWithPrefix(jsonStr, chars, false, element);
            break;
        default:
            break;
    }

    if (ret) {
        element->mStackElementState = InCharacter;
    }
    if (jsonStr.length() <= 0) {
        return;
    }
    appendJsonStream(jsonStr);

    if (mJsonTransformerLisener) {
        mJsonTransformerLisener->characters(jsonStr);
    }
}
