#pragma once

#include <ExcelSlicer.h>

using namespace std;

class ExcelSlicerEmListener : public ExcelSlicerListener {
public:
    void onStartDocument(const ExcelSlicer& excelSlicer) override;
    void onEndDocument(const ExcelSlicer& excelSlicer) override;
    void onParsedSliceData(const ExcelSlicer& excelSlicer, const string& slicedData, bool isEnded) override;
};