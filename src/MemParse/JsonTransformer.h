#pragma once

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/HandlerBase.hpp>

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

class JsonTransformer;

class JsonTransformerLisener {
public:
    virtual void startDocument() {}
    virtual void endDocument() {}
    virtual void startElement(const string& tagStr,
        AttributeList& attributes, const string& jsonStrOfThisTag) {}
    virtual void endElement(const string& tagStr, const string& jsonStr) {}
    virtual void endElement2(const string& tagStr,
        const string& jsonStr, const HandlerExtraInfo& extraInfo) {}
    virtual void characters(const string& chars) {}
};

typedef enum {
    NoContent,
    InCharacter,
    WithContent
} StackElementState;

struct StackElement {
    StackElement(const string& tagName)
        : mStackElementState(NoContent),
          mTagName(tagName) {
    }
    StackElementState mStackElementState;
    string mTagName;
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
    // void endElement(const XMLCh* const name);
    void endElement2(const XMLCh* const name, const HandlerExtraInfo&);

    void characters(const XMLCh* const chars, const XMLSize_t length);
    string& jsonUtf8String() { return mJsonStream; }

    stack<shared_ptr<StackElement>>& currStateStack() { return mStack; }


    void setLisener(JsonTransformerLisener* lisener) { mJsonTransformerLisener = lisener; }
    void setWillSaveTransformResult(bool willSaveTransformResult) { mWillSaveTransformResult = willSaveTransformResult; }

private:
    inline void appendJsonStream(const string& jsonStr);

    void appendIndents();

    string mJsonStream;                     // json string stream of utf8 to output
    stack<shared_ptr<StackElement>> mStack; // the state stack for cat json content

    // weak_ptr<JsonTransformerLisener> mJsonTransformerLisener;
    JsonTransformerLisener* mJsonTransformerLisener = nullptr; // FIXME, FIXME

    bool mWillSaveTransformResult = true;
};

