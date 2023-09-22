#include <string.h>

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

#include "MemParseHandlers.h"

// 解析xml大小1.16G
// t2 = 31.1272  Memory: 9G
// t2 = 32.3432  Memory: 9G
// t2 = 30.7645  Memory: 8G

// 解析xml大小150M
// t2 = 3.96557
// t2 = 3.92425
// t2 = 3.93356

// 解析xml大小150M，Nodejs
// test: 9.276s
// test: 7.323s
// test: 7.449s

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

void MemParseHandlers::endElement(const XMLCh* const name) {
#if WithMyParser
    mJsonTransformer.endElement(name);
#endif
}

void MemParseHandlers::endElement2(const XMLCh* const name, const HandlerExtraInfo& extraInfo)
{
    // throw UserInterruption();
    // XMLCh fTotalSizeOfContentHasRead = name[0];
    XMLCh fTotalSizeOfContentHasRead = extraInfo.mTotalSizeOfContentHasRead;
    // std::cout << "MemParseHandlers::endElement: " << fTotalSizeOfContentHasRead << std::endl;
    std::cout << "MemParseHandlers::endElement: " << gXMLInMemBuf[fTotalSizeOfContentHasRead] << std::endl;
    printf("MemParseHandlers::endElement: %s\n\n\n", &gXMLInMemBuf[fTotalSizeOfContentHasRead]);
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
