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

emscripten::val getTransformFile(const string& fileFullPath,
    const string& fileId, const string& relativePath,
    bool willSlice, int sliceType, unsigned int sliceSize, bool willSaveTransformResult) {
    DurationTimer dt("getTransformFile: " + fileFullPath);
    string *jsonUtf8Str = new string();
    // readFileIntoString(*jsonUtf8Str, fileFullPath);
    int ret = GetFileContent(*jsonUtf8Str, fileFullPath, fileId,
        relativePath, willSlice, (SliceType)sliceType,
        sliceSize, willSaveTransformResult);
    if (ret) {
        cerr << "Err: getTransformFile" << endl;
    }
    try {
        if (!std::filesystem::remove(fileFullPath)) {
            cerr << "Err: delete file:" << fileFullPath << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Err: delete file:" << fileFullPath << ", exception: " << e.what() << std::endl;
    }

	emscripten::val rst = emscripten::val::object();
    auto buff = emscripten::val(emscripten::typed_memory_view(
            jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("fileContent", buff);
    rst.set("nativePointer", reinterpret_cast<int>(jsonUtf8Str));
    return rst;

    // return emscripten::val(emscripten::typed_memory_view(
    //     jsonUtf8Str->length(), jsonUtf8Str->c_str()));
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
    auto buff = emscripten::val(emscripten::typed_memory_view(
            jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("fileContent", buff);
    rst.set("nativeStringPointer", reinterpret_cast<int>(jsonUtf8Str));
    return rst;

    // return emscripten::val(emscripten::typed_memory_view(
    //     jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    // auto buff = emscripten::val::global("ArrayBuffer").new_(10);
    // return buff;
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
    auto buff = emscripten::val(emscripten::typed_memory_view(
            jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("fileContent", buff);
    rst.set("nativeStringPointer", reinterpret_cast<int>(jsonUtf8Str));
    return rst;

    // return emscripten::val(emscripten::typed_memory_view(
    //     jsonUtf8Str->length(), jsonUtf8Str->c_str()));
}

void freeNativeString(int nativePointer) {
  delete (string*)nativePointer;
}

void freeNativePointer(int nativePointer) {
  delete (void*)nativePointer;
}

EMSCRIPTEN_BINDINGS(my_module22) {
  emscripten::function("writeXmlFile", &writeXmlFile);
  emscripten::function("getTransformFile", &getTransformFile);
  emscripten::function("getDocumentSectPrArray", &getDocumentSectPrArray);
  emscripten::function("getExcelSheetHeadAndTail", &getExcelSheetHeadAndTail);
  emscripten::function("freeNativeString", &freeNativeString);
}
#endif