#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "Common.h"
#include "ExcelSlicer.h"
#include "MemParseHandlers.h"
#include "XML2JsonParser.h"

using namespace std;

static const char* gMemBufId = "memBufId";
static int gMemBufCnt = 0;

#define EnableParseTimeDuration 0

static unsigned gInitializedXMLPlatform = 0;

int InitializeXMLPlatformIfNeccesary() {
    if (!gInitializedXMLPlatform) {
        try {
            XMLPlatformUtils::Initialize();
        }
        catch (const XMLException& toCatch) {
            XERCES_STD_QUALIFIER cerr << "Error during initialization! Message:\n"
                << StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
            return -1;
        }
    }
    gInitializedXMLPlatform++;
    return 0;
}

int TerminateXMLPlatform() {
    // Should terminate even if error occured
    gInitializedXMLPlatform--;
    if (!gInitializedXMLPlatform) {
        XMLPlatformUtils::Terminate();
    }
    return 0;
}

int ParseXml2Json(const ReadFileInfo& readFileInfo, HandlerBase* handler) {
    SAXParser::ValSchemes valScheme = SAXParser::Val_Auto;
    bool doNamespaces = false;
    bool doSchema = false;
    bool schemaFullChecking = false;
    InitializeXMLPlatformIfNeccesary();

    //  Create a SAX parser object. Then, according to what we were told on
    //  the command line, set it to validate or not.
    SAXParser *parser = new SAXParser;
    parser->setValidationScheme(valScheme);
    parser->setDoNamespaces(doNamespaces);
    parser->setDoSchema(doSchema);
    parser->setHandleMultipleImports(true);
    parser->setValidationSchemaFullChecking(schemaFullChecking);
    parser->setExitOnFirstFatalError(false); // parse xml file as far as it can, do not exit on invalide charactor

    //  Create our SAX handler object and install it on the parser, as the
    //  document and error handlers.
    parser->setDocumentHandler(handler);
    parser->setErrorHandler(handler);

    //  Get the starting time and kick off the parse of the indicated
    //  file. Catch any exceptions that might propogate out of it.
    unsigned long duration;
    int errorCount = 0;
    int errorCode = 0;
    try
    {
#if EnableParseTimeDuration
        const unsigned long startMillis = XMLPlatformUtils::getCurrentMillis();
#endif
        if (ParseXmlType_FromFile == readFileInfo.mParseXmlType) {
            parser->parse(readFileInfo.mFileFullPath.c_str());
        } else if (ParseXmlType_FromMemory == readFileInfo.mParseXmlType) {
            //  Create MemBufferInputSource from the buffer containing the XML
            //  statements.
            shared_ptr<MemBufInputSource> memBufIS(new MemBufInputSource
            (
                (const XMLByte*)readFileInfo.mXmlFileContent.c_str()
                , readFileInfo.mXmlFileContent.length()
                , gMemBufId + (gMemBufCnt++)
                , false
            ));
            memBufIS->setCopyBufToStream(false); // not copy old buffer, use it directly, for performance
            parser->parse(*memBufIS);
        }
#if EnableParseTimeDuration
        const unsigned long endMillis = XMLPlatformUtils::getCurrentMillis();
        duration = endMillis - startMillis;
#endif
        errorCount = parser->getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
        XERCES_STD_QUALIFIER cerr << "[Error] OutOfMemoryException parsing: "
            << readFileInfo.mFileFullPath << XERCES_STD_QUALIFIER endl;
        errorCode = -5;
    }
    catch (const XMLException& e)
    {
        XERCES_STD_QUALIFIER cerr << "\n[Error] Exception parsing: " << readFileInfo.mFileFullPath
            << "\nException message:  \n"
            << StrX(e.getMessage()) << "\n" << XERCES_STD_QUALIFIER endl;
        errorCode = -4;
    }

#if EnableParseTimeDuration
    if (!errorCount) {
        XERCES_STD_QUALIFIER cout << "\nFinished parsing: "
            << readFileInfo.mFileFullPath
            << "\n\n"
            << "Parsing took " << duration << " ms\n" << XERCES_STD_QUALIFIER endl;
    }
#endif

    // Delete the parser itself.  Must be done prior to calling Terminate, below.
    delete parser;
    
    if (errorCount > 0)
        errorCode = -4;

    // Should terminate even if error occured
    TerminateXMLPlatform();

    return errorCode;
}

int ParseXml2Json(string& jsonUtf8Str, const ReadFileInfo& readFileInfo) {
    MemParseHandlers handlers;
    ExcelSlicer excelSlicer;
    handlers.setWillSaveTransformResult(readFileInfo.mWillSaveTransformResult);
    if (readFileInfo.mWillSlice && readFileInfo.mSliceSize > 0) {
        handlers.setLisener(&excelSlicer);
        excelSlicer.setReadFileInfo(readFileInfo);
    }

    int ret = ParseXml2Json(readFileInfo, &handlers);
    if (ret) {
        cerr << "[Error] ParseXml2Json error, ret: " << ret << endl;
    }
    jsonUtf8Str = std::move(handlers.jsonUtf8String());

    return ret;
}

int GetFileContentWithNoSlice(string& jsonUtf8Str, const string& fileFullPath, const string& fileId,
    const string& relativePath) {
    return GetFileContent(jsonUtf8Str, fileFullPath, fileId, relativePath, false, SliceType_WordDocument, 0);
}

int GetFileContent(string& jsonUtf8Str, const string& fileFullPath, 
    const string& fileId, const string& relativePath,
    bool willSlice, SliceType sliceType) {
    return GetFileContent(jsonUtf8Str, fileFullPath, fileId, relativePath, willSlice, sliceType, DefaultSliceRowNum);
}

int GetFileContent(string& jsonUtf8Str, const string& fileFullPath, 
    const string& fileId, const string& relativePath,
    bool willSlice, SliceType sliceType, unsigned int sliceSize) {
    return GetFileContent(jsonUtf8Str, fileFullPath, fileId, relativePath, willSlice, sliceType, sliceSize, true);
}

int GetFileContent(string& jsonUtf8Str, const string& fileFullPath, 
    const string& fileId, const string& relativePath,
    bool willSlice, SliceType sliceType, unsigned int sliceSize, bool willSaveTransformResult) {
    ReadFileInfo readFileInfo;
    readFileInfo.mParseXmlType = ParseXmlType_FromFile;
    readFileInfo.mFileFullPath = fileFullPath;
    readFileInfo.mFileId       = fileId;
    readFileInfo.mRelativePath = relativePath;
    readFileInfo.mWillSlice    = willSlice;
    readFileInfo.mSliceType    = sliceType;
    readFileInfo.mSliceSize    = sliceSize;
    readFileInfo.mWillSaveTransformResult   = willSaveTransformResult;

    // xml to json
    int ret = ParseXml2Json(jsonUtf8Str, readFileInfo);
    if (ret) {
        cerr << "[Error] parse xml to json: " << readFileInfo.mFileFullPath << std::endl;
        return -1;
    }
    return 0;
}