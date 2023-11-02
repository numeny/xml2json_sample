#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/time.h>

#if defined(EMSCRIPTEN)
#include <emscripten.h>
#include <emscripten/bind.h>
#include <filesystem>
#endif

#include "ShardParser.h"
#include "Xml2JsonCommon.h"
#include "XML2JsonParser.h"

using namespace std;

const std::wstring source_folder = L"/source_file/";
const std::wstring result_folder = L"/trans_result/";
const std::string compress_folder = "/compress_folder/";

int writeFile() {
    std::ofstream outputFile("example.txt");

    if (!outputFile.is_open()) {
        std::cerr << "Unable to open the file." << std::endl;
        return 1;
    }

    outputFile << "Hello, world!" << std::endl;
    outputFile << "This is a sample file." << std::endl;

    outputFile.close();

    cout << "File writing complete: example.txt" << std::endl;

    return 0;
}

#include <cstdio>

int writeXmlFile(const string& fileName, const string& fileContent) {
    cout << "File writing start: " << fileName << std::endl;
    std::ofstream outputFile(fileName);
    if (!outputFile.is_open()) {
        std::cerr << "Unable to open the file." << std::endl;
        return 1;
    }

    outputFile << fileContent;

    outputFile.close();

    cout << "File writing complete: " << fileName << std::endl;

    return 0;
}


#if defined(EMSCRIPTEN)

emscripten::val readRawData(const string& fileFullPath) {
    DurationTimer dt("readRawData: " + fileFullPath);
    string *jsonUtf8Str = new string();
    int ret = readFileIntoString(*jsonUtf8Str, fileFullPath);
    if (ret) {
        cerr << "Err: readRawData" << endl;
    }
    // cout << "c++ readRawData: jsonUtf8Str: " << jsonUtf8Str->length() << endl;

	emscripten::val rst = emscripten::val::object();
    auto buff = emscripten::val(
        emscripten::typed_memory_view(jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("fileContent", buff);
    rst.set("nativeStringPointer", reinterpret_cast<int>(jsonUtf8Str));
    return rst;
}

emscripten::val readAsJSON(const string& fileFullPath,
    const string& fileId, const string& relativePath,
    bool willShard, int shardType, unsigned int shardSize, bool willSaveTransformResult) {
    DurationTimer dt("readAsJSON: " + fileFullPath);
    string *jsonUtf8Str = new string();
    int ret = GetFileContent(*jsonUtf8Str, fileFullPath, fileId,
        relativePath, willShard, (ShardType)shardType,
        shardSize, willSaveTransformResult);
    if (ret) {
        cerr << "Err: readAsJSON" << endl;
    }

	emscripten::val rst = emscripten::val::object();
    auto buff = emscripten::val(
        emscripten::typed_memory_view(jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("fileContent", buff);
    rst.set("nativePointer", reinterpret_cast<int>(jsonUtf8Str));
    return rst;
}

emscripten::val getDocumentSectPrArray(const string& fileFullPath) {
    DurationTimer dt("getDocumentSectPrArray: " + fileFullPath);
    cout << "in getDocumentSectPrArray()" << endl;
    string *jsonUtf8Str = new string();
    int ret = GetDocumentSectPrArray(*jsonUtf8Str, fileFullPath);
    if (ret) {
        cerr << "Err: getDocumentSectPrArray: " << fileFullPath << endl;
    }

    emscripten::val rst = emscripten::val::object();
    auto buff = emscripten::val(
        emscripten::typed_memory_view(jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("fileContent", buff);
    rst.set("nativeStringPointer", reinterpret_cast<int>(jsonUtf8Str));
    return rst;
}

emscripten::val getExcelSheetHeadAndTail(const string& fileFullPath) {
    DurationTimer dt("getExcelSheetHeadAndTail: " + fileFullPath);
    cout << "in getExcelSheetHeadAndTail()" << fileFullPath << endl;

    string *jsonUtf8Str = new string();
    int ret = GetExcelSheetHeadAndTail(*jsonUtf8Str, fileFullPath);
    if (ret) {
        cerr << "Err: getExcelSheetHeadAndTail: " << fileFullPath << endl;
    }
	emscripten::val rst = emscripten::val::object();
    auto buff = emscripten::val(
        emscripten::typed_memory_view(jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("fileContent", buff);
    rst.set("nativeStringPointer", reinterpret_cast<int>(jsonUtf8Str));
    return rst;
}

emscripten::val convertXML2JSON(const string& xmlContent) {
    string *jsonUtf8Str = new string();
    int ret = ConvertXML2JSON(*jsonUtf8Str, xmlContent);
    if (ret) {
        cerr << "Err: ConvertXML2JSON: " << xmlContent << endl;
    }
    emscripten::val rst = emscripten::val::object();
    auto buff = emscripten::val(
        emscripten::typed_memory_view(jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("jsonContent", buff);
    rst.set("nativeStringPointer", reinterpret_cast<int>(jsonUtf8Str));
    return rst;
}

void freeNativeString(int nativePointer) {
  delete (string*)nativePointer;
}

void freeNativePointer(int nativePointer) {
  delete reinterpret_cast<char*>(nativePointer);
}

emscripten::val readNextShard(const string& fileFullPath,
    int shardType, unsigned int shardSize) {
    string *jsonUtf8Str = new string();
    bool isShardEnded;

    int ret = ShardParser::readNextShard(*jsonUtf8Str, isShardEnded,
        fileFullPath, (ShardType)shardType, shardSize);
    if (ret) {
        cerr << "Err: readNextShard" << endl;
    }
	emscripten::val rst = emscripten::val::object();
    auto buff = emscripten::val(
        emscripten::typed_memory_view(jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("fileContent", buff);
    rst.set("isShardEnded", isShardEnded);
    rst.set("nativeStringPointer", reinterpret_cast<int>(jsonUtf8Str));

    return rst;
}

void deleteFile(const string& fileFullPath) {
    auto ret = deleteAllFile(fileFullPath);
    if (ret) {
        cerr << "Err: deleteFile: " << fileFullPath << endl;
    }
}

EMSCRIPTEN_BINDINGS(my_module22) {
  emscripten::function("writeXmlFile", &writeXmlFile);
  emscripten::function("readRawData", &readRawData);
  emscripten::function("readAsJSON", &readAsJSON);
  emscripten::function("readNextShard", &readNextShard);
  emscripten::function("deleteFile", &deleteFile);
  emscripten::function("convertXML2JSON", &convertXML2JSON);
  emscripten::function("getDocumentSectPrArray", &getDocumentSectPrArray);
  emscripten::function("getExcelSheetHeadAndTail", &getExcelSheetHeadAndTail);
  emscripten::function("freeNativeString", &freeNativeString);
  emscripten::function("freeNativePointer", &freeNativePointer);
  emscripten::function("setMinBuffSizeToRead", &setMinBuffSizeToRead);
  emscripten::function("setMaxBuffSizeToRead", &setMaxBuffSizeToRead);
}
#endif