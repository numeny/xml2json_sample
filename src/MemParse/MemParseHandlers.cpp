#include <string.h>

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

#include "MemParseHandlers.h"

#define WithMyParser 1

void MemParseHandlers::startDocument() {
#if WithMyParser
    mJsonTransformer.startDocument();
#endif
}

void MemParseHandlers::endDocument() {
#if WithMyParser
    mJsonTransformer.endDocument();
#endif
}
extern const char*  gXMLInMemBuf;
// ---------------------------------------------------------------------------
//  MemParseHandlers: Implementation of the SAX DocumentHandler interface
// ---------------------------------------------------------------------------
void MemParseHandlers::startElement(
    const XMLCh* const name, AttributeList& attributes) {
#if WithMyParser
    mJsonTransformer.startElement(name, attributes);
#endif
}

// void MemParseHandlers::endElement(const XMLCh* const name) {
// #if WithMyParser
//     mJsonTransformer.endElement(name);
// #endif
// }

void MemParseHandlers::endElement2(const XMLCh* const name, const HandlerExtraInfo& extraInfo)
{
#if WithMyParser
    mJsonTransformer.endElement2(name, extraInfo);
#endif
    // // throw UserInterruption();
    // // XMLCh fTotalSizeOfContentHasRead = name[0];
    // XMLCh fTotalSizeOfContentHasRead = extraInfo.mTotalSizeOfContentHasRead;
    // // std::cout << "MemParseHandlers::endElement: " << fTotalSizeOfContentHasRead << std::endl;
    // std::cout << "MemParseHandlers::endElement: " << gXMLInMemBuf[fTotalSizeOfContentHasRead] << std::endl;
    // printf("MemParseHandlers::endElement: %s\n\n\n", &gXMLInMemBuf[fTotalSizeOfContentHasRead]);
}

void MemParseHandlers::characters(
    const XMLCh* const chars, const XMLSize_t length) {
#if WithMyParser
    mJsonTransformer.characters(chars, length);
#endif
}

// ---------------------------------------------------------------------------
//  MemParseHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void MemParseHandlers::error(const SAXParseException& e) {
    XERCES_STD_QUALIFIER cerr << "\nError at (file " << StrX(e.getSystemId())
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "): " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void MemParseHandlers::fatalError(const SAXParseException& e) {
    XERCES_STD_QUALIFIER cerr << "\nFatal Error at (file " << StrX(e.getSystemId())
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "): " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void MemParseHandlers::warning(const SAXParseException& e) {
    XERCES_STD_QUALIFIER cerr << "\nWarning at (file " << StrX(e.getSystemId())
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "): " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}
