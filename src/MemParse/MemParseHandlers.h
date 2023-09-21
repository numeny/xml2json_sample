#pragma once

#include <string.h>
#include <stdlib.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "JsonTransformer.h"

#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif

XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

class MemParseHandlers : public HandlerBase
{
public:
    MemParseHandlers() {}
    ~MemParseHandlers() {}

    //  Handlers for the SAX DocumentHandler interface
    void startDocument();
    void endDocument();
    void startElement(const XMLCh* const name, AttributeList& attributes);
    void endElement(const XMLCh* const name);
    void characters(const XMLCh* const chars, const XMLSize_t length);
    void setLisener(JsonTransformerLisener* lisener) { mJsonTransformer.setLisener(lisener); }
    void setWillSaveTransformResult(bool willSaveTransformResult) {
        mJsonTransformer.setWillSaveTransformResult(willSaveTransformResult);
    }

    //  Handlers for the SAX ErrorHandler interface
    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);

    // FIXME
    JsonTransformer& jsonTransformer() {
        return mJsonTransformer;
    }
    // FIXME
    std::string& jsonUtf8String() {
        return mJsonTransformer.jsonUtf8String();
    }

private:
    JsonTransformer mJsonTransformer;
};


// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of XMLCh data to local code page for display.
// ---------------------------------------------------------------------------
class StrX
{
public :
    StrX(const XMLCh* const toTranscode) {
        // Call the private transcoding method
        fLocalForm = XMLString::transcode(toTranscode);
    }
    ~StrX() {
        XMLString::release(&fLocalForm);
    }
    const char* localForm() const {
        return fLocalForm;
    }
private:
    char* fLocalForm;
};

inline XERCES_STD_QUALIFIER ostream& operator<<(XERCES_STD_QUALIFIER ostream& target, const StrX& toDump) {
    target << toDump.localForm();
    return target;
}
