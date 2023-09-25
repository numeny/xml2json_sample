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

#include "ShardHandler.h"
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

emscripten::val readAsJSON(const string& fileFullPath,
    const string& fileId, const string& relativePath,
    bool willShard, int shardType, unsigned int shardSize, bool willSaveTransformResult) {
    DurationTimer dt("readAsJSON: " + fileFullPath);
    string *jsonUtf8Str = new string();
    // readFileIntoString(*jsonUtf8Str, fileFullPath);
    int ret = GetFileContent(*jsonUtf8Str, fileFullPath, fileId,
        relativePath, willShard, (ShardType)shardType,
        shardSize, willSaveTransformResult);
    if (ret) {
        cerr << "Err: readAsJSON" << endl;
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
  delete reinterpret_cast<char*>(nativePointer);
}

emscripten::val readNextShard(const string& fileFullPath,
    int shardType, unsigned int shardSize) {
    cout << "in readNextShard()-1" << endl;
    string *jsonUtf8Str = new string();

    // int ret = GetNextShardData(jsonUtf8Str,
    //     fileFullPath, (ShardType)shardType, shardSize);
    // int ret = ReadNextShard(*jsonUtf8Str,
    //     fileFullPath, (ShardType)shardType, shardSize);
    bool isShardEnded;

    int ret = ShardHandler::readNextShard(*jsonUtf8Str, isShardEnded,
        fileFullPath, (ShardType)shardType, shardSize);
    if (ret) {
        cerr << "Err: readNextShard" << endl;
    }
	emscripten::val rst = emscripten::val::object();
    auto buff = emscripten::val(emscripten::typed_memory_view(
            jsonUtf8Str->length(), jsonUtf8Str->c_str()));
    rst.set("fileContent", buff);
    rst.set("isShardEnded", isShardEnded);
    rst.set("nativeStringPointer", reinterpret_cast<int>(jsonUtf8Str));
    return rst;
    // return emscripten::val(emscripten::typed_memory_view(
    //     jsonUtf8Str.length(), jsonUtf8Str.c_str()));
 }

// int deleteFile(const std::wstring &fileIdWstring) {
//     string fileId = U_TO_UTF8(fileIdWstring);
//     if (!fs::exists(fileId)/* && fs::is_directory(fileId)*/) {
//         return 0;
//     }
//     int ret = 0;
//     try {
//         if (!fs::remove_all(fileId)) {
//             ret = -1;
//             std::cerr << "Err: remove_all:" << fileId << std::endl;
//         }
//     } catch (const std::exception& e) {
//         ret = -1;
//         std::cerr << "Err: remove_all:" << fileId << ", exception: " << e.what() << std::endl;
//     }
//     return ret;
// }

// void deleteFiles(const std::wstring &fileId){
// 	NSDirectory::DeleteDirectory(result_folder+fileId);
// }

EMSCRIPTEN_BINDINGS(my_module22) {
  emscripten::function("writeXmlFile", &writeXmlFile);
  emscripten::function("readAsJSON", &readAsJSON);
  emscripten::function("readNextShard", &readNextShard);
  emscripten::function("getDocumentSectPrArray", &getDocumentSectPrArray);
  emscripten::function("getExcelSheetHeadAndTail", &getExcelSheetHeadAndTail);
  emscripten::function("freeNativeString", &freeNativeString);
}
#endif