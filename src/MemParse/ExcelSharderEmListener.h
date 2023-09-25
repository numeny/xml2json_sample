#pragma once

#include <ExcelSharder.h>

using namespace std;

class ExcelSharderEmListener : public ExcelSharderListener {
public:
    void onStartDocument(const ExcelSharder& excelShardr) override;
    void onEndDocument(const ExcelSharder& excelShardr) override;
    void onParsedShardData(const ExcelSharder& excelShardr, const string& sharddData, bool isEnded) override;
};