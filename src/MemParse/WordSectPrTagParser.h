#include <string>

using namespace std;
class WordSectPrTagParser {
public:
    // static int GetDocumentSectPrArray(string& jsonStr, const string& fileName);
    WordSectPrTagParser(const string& fileName) : mFileName(fileName) {}
    ~WordSectPrTagParser() = default;
    int getSectPrTagStrForWordDocument(string& outStr);

private:
    int getAllTagOfSectPrForWordDocument(size_t& nextSearchPos);
    int getContentBetweenStr(size_t nextSearchPos, const string& startStr, const string& endStr,
        bool willSearchCloseBrack = false);
    int getHeadForWordDocument(size_t& nextSearchPos);
    int isLastTagSectPr(bool& isLastTagSectPr);

    string mFileName;
    string mFileContent;

    string mXmlStrToParse;
};