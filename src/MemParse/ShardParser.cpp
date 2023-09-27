#include <iostream>
#include <string>
// #include <locale>
// #include <codecvt>
#include <fstream>

#include "MemParseHandlers.h"
#include "ShardHandler.h"
#include "ShardParser.h"
#include "XML2JsonParser.h"

size_t MinBuffSizeToRead = (1024*1024*10);
size_t MaxBuffSizeToRead = (2 * MinBuffSizeToRead);

std::map<std::string, shared_ptr<ShardParser>> ShardParser::mMapOfShardParsers;

int ShardParser::readNextShard(string& jsonUtf8Str, bool& isShardEnded,
    const string& fileFullPath, ShardType shardType, unsigned int shardSize) {
    shared_ptr<ShardParser> shardParser;
    if (mMapOfShardParsers.find(fileFullPath) != mMapOfShardParsers.end()) {
        shardParser = mMapOfShardParsers[fileFullPath];
        if (!shardParser) {
            cerr << "Err: ShardParser::readNextShard: null handler." << endl;
            return -1;
        }
    } else {
        shardParser = make_shared<ShardParser>();
        shardParser->initIfNeccesary(fileFullPath, shardType, shardSize);
        mMapOfShardParsers[fileFullPath] = shardParser;
    }
    int ret = shardParser->readNextShard(jsonUtf8Str, isShardEnded);
    if (ret) {
        cerr << "Err: ShardParser::readNextShard: read" << fileFullPath << endl;
    }
    if (isShardEnded) {
        if (mMapOfShardParsers.find(fileFullPath) != mMapOfShardParsers.end()) {
            mMapOfShardParsers.erase(fileFullPath);
        }
        // mSharedEnded = false;
#if defined(EMSCRIPTEN)
        int ret2 = deleteAllFile(fileFullPath);
        if (ret2) {
            cerr << "Err: ShardParser::readNextShard delete file: " << fileFullPath << endl;
        }
#endif
    }
    return ret;
}

int ShardParser::readNextShard(string& jsonUtf8Str, bool& isShardEnded) {
    DurationTimer dt("ShardParser::readNextShard");
    refreshInputBuff();

    // parse xml
    ReadFileInfo readFileInfo;
    readFileInfo.mParseXmlType = ParseXmlType_FromMemory;
    readFileInfo.mXmlFileContent = mInputBuff;
    readFileInfo.mWillSaveTransformResult   = false;

    MemParseHandlers handlers;
    handlers.setWillSaveTransformResult(false);
    handlers.setLisener(mShardHandler.get());

    int ret = ParseXml2Json(readFileInfo, &handlers);
    if ((RetCode_UserInterruption == ret)) {
        ret = 0;
        // cout << "[Debug] ParseXml2Json UserInterruption " << endl;
    } else if (ret) {
        cerr << "Err: ParseXml2Json, ret: " << ret << endl;
    }
    deleteThisShardDataFromBuff(mShardHandler->totalSizeOfContentHasRead());

    isShardEnded = mShardHandler->isSharedEnded();
    jsonUtf8Str = std::move(mShardHandler->currShardData());
    return ret;
}

void ShardParser::initIfNeccesary(const string& fileFullPath,
    ShardType shardType, size_t shardSize) {
    if (mShardHandler) {
        return;
    }
    mFileFullPath = fileFullPath;
    mCurrPosToReadFromFile = 0;
    mIsEof = false;
    mShardHandler = make_shared<ShardHandler>();
    mShardHandler->initIfNeccesary(shardType, shardSize);
}

int ShardParser::deleteThisShardDataFromBuff(size_t startPosLeft) {
    DurationTimer dt("deleteThisShardDataFromBuff");
    if (mSimpleHeadOfXml.empty()) {
        mSimpleHeadOfXml = mShardHandler->simpleHeadOfXml();
        if (mSimpleHeadOfXml.empty()) {
            cerr << "Err: deleteThisShardDataFromBuff, simple headOf xml is empty: "
                << mFileFullPath << endl;
        }
    }
    mInputBuff = mSimpleHeadOfXml +
        mInputBuff.substr(startPosLeft, mInputBuff.length() - startPosLeft);
    return 0;
}

int ShardParser::readFileBlockIntoString(std::string &contentStr,
    const string& fileName, size_t from, size_t sizeToRead) {
    DurationTimer timer("readFileBlockIntoString");

    std::ifstream inputFile(fileName.c_str(),
        std::ios::in | std::ios::out | std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Err: open fileName:" << fileName << std::endl;
        return -1;
    }

    inputFile.seekg(from, std::ios::beg);
    if (inputFile.eof()) {
        mIsEof = true;
        inputFile.close();
        return 0;
    }

    char* fileContent = new char[sizeToRead+1];
    if (!fileContent) {
        std::cerr << "Err: memory out when read file:" << fileName << std::endl;
        inputFile.close();
        return -1;
    }
    inputFile.read(fileContent, sizeToRead);
    if (inputFile.eof()) {
        mIsEof = true;
        // std::cout << "Debug: readFileBlockIntoString, read eof:" << fileName << std::endl;
    } else if (inputFile.fail()) {
        std::cerr << "Err: readFileBlockIntoString, reading file:" << fileName << std::endl;
    }
    std::streamsize bytesRead = inputFile.gcount();
    if (bytesRead <= 0) {
        mIsEof = true;
        // std::cout << "Debug: readFileBlockIntoString: read bytes: "
        //     << bytesRead << ", finish reading file:" << mFileFullPath << std::endl;
    }
    mCurrPosToReadFromFile += bytesRead;
    fileContent[bytesRead] = 0;
    inputFile.close();
    contentStr = std::move(string(fileContent, bytesRead));
    delete[] fileContent;
    return 0;
}

void ShardParser::refreshInputBuff() {
    DurationTimer timer("refreshInputBuff");
    if (mInputBuff.length() >= MinBuffSizeToRead
        /*FIXME, || mShardHandler->isSharedEnded()*/ || mIsEof) {
        return;
    }
    // refresh data to MaxBuffSizeToRead
    // read content within MaxBuffSizeToRead from mCurrPosToReadFromFile
    size_t sizeToRead = MaxBuffSizeToRead - mInputBuff.length();
    string strToRead;
    int ret = readFileBlockIntoString(strToRead,
        mFileFullPath, mCurrPosToReadFromFile, sizeToRead);
    if (ret) {
        std::cerr << "Err: readFileBlockIntoString:" << mFileFullPath << std::endl;
    }
    mInputBuff += strToRead;
}

void setMinBuffSizeToRead(unsigned int buffSize) {
  MinBuffSizeToRead = buffSize;
}

void setMaxBuffSizeToRead(unsigned int buffSize) {
  MaxBuffSizeToRead = buffSize;
}