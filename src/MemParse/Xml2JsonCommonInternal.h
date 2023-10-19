#pragma once

// for shard xml file(excel: xl/worksheets/sheetxxx.xml, word: word/document.xml, etc.)
const string ShardStr_SheetData         = "sheetData";
const string ShardStr_Row               = "row";
const string ShardStrForWord_WBody      = "w:body";
const string ShardStrForWord_WBody_StartTag  = "<w:body";
const string ShardStrForWord_WBody_EndTag  = "</w:body>";

const int RetCode_UserInterruption = 100;
struct UserInterruption {
    int mRetCode = RetCode_UserInterruption;
};