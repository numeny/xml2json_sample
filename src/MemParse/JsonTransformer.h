#pragma once

#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>

#include <iostream>
#include <memory>
#include <stack>
#include <string.h>

XERCES_CPP_NAMESPACE_USE

using namespace std;

extern const string ContentPrefixStr;
extern const string CommaStr;
extern const string StartBracketStr;
extern const string CloseBracketStr;

class JsonTransformerLisener {
public:
    virtual void startDocument() {}
    virtual void endDocument() {}
    virtual void startElement(const string& tagStr,
        AttributeList& attributes, const string& jsonStrOfThisTag) {}
    virtual void endElement(const string& tagStr, const string& jsonStr) {}
    virtual void characters(const string& chars) {}
    virtual void setSAXParser(SAXParser* parser) {}
};

typedef enum {
    NoContent,
    InCharacter,
    WithContent
} StackElementState;

struct StackElement {
    StackElement() : mStackElementState(NoContent),
        mHasAttributeOfPreserveSpace(false) {}
    StackElement(bool hasAttributeOfPreserveSpace) : mStackElementState(NoContent),
        mHasAttributeOfPreserveSpace(hasAttributeOfPreserveSpace) {}
    StackElementState mStackElementState;
    bool mHasAttributeOfPreserveSpace = false; // has attribute: xml:space="preserve"
};

// JsonTransformer accept xml's SAX input and trasform it to json string of utf8
class JsonTransformer
{
public:
    JsonTransformer() { }
    ~JsonTransformer() { }
    void startDocument();
    void endDocument();
    void startElement(const XMLCh* const name, AttributeList& attributes);
    void endElement(const XMLCh* const name);

    void characters(const XMLCh* const chars, const XMLSize_t length);
    string& jsonUtf8String() { return mJsonStream; }

    void setLisener(JsonTransformerLisener* lisener) { mJsonTransformerLisener = lisener; }
    void setWillSaveTransformResult(bool willSaveTransformResult) { mWillSaveTransformResult = willSaveTransformResult; }
    void setSAXParser(SAXParser* parser) {
        if (mJsonTransformerLisener) {
            mJsonTransformerLisener->setSAXParser(parser);
        }
    }

private:
    inline void appendJsonStream(const string& jsonStr);

    void appendIndents();

    string mJsonStream;                     // json string stream of utf8 to output
    stack<shared_ptr<StackElement>> mStack; // the state stack for cat json content

    // weak_ptr<JsonTransformerLisener> mJsonTransformerLisener;
    JsonTransformerLisener* mJsonTransformerLisener = nullptr; // FIXME, FIXME

    bool mWillSaveTransformResult = true;
};

