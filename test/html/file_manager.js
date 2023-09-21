
importScripts('xml2json_bin.js');
importScripts('queue.js');

Module = {};
Module.onRuntimeInitialized = function() {
    var response = {
        command: "onWorkerReady",
    }
    console.log("Module.onRuntimeInitialized: ")

    postMessage(response);
}

// console.log = function() {}

// default row number for slice
var DefaultSliceRowNum = 1;
const SliceType_WordDocument = 0;
const SliceType_ExcelSheetx = 1;

addEventListener("message", (message) => {
  var handlers = {
    "getTransformFile": getTransformFile,
    "getDocumentSectPrArray": getDocumentSectPrArray,
    "getExcelSheetHeadAndTail": getExcelSheetHeadAndTail,
    "writeFile": writeFile,
  };  

  var request = message.data;
  console.log("worker receive message: ", request)
  var func = handlers[request.command]
  if (func === undefined) {
      console.error("[Error] worker no handler for command: " + request.command);
      return;
  }
  return func(request.param);
});

function handleEscapedChars(content) {
    // console.log("worker handleEscapedChars none - 0");
    // \f\v\r\t\n
    // content = content.replaceAll("\n", "\\n");
    // content = content.replaceAll("\r", "\\r");

    // content = content.replaceAll("\f", "\\f");
    // content = content.replaceAll("\v", "\\v");
    // content = content.replaceAll("\t", "\\t");
    // content = content.replaceAll("", "");

    return content;
}

function getTransformFile(param) {
    if (!param) {
        console.error("Err: worker getTransformFile no param.");
        return;
    }
    if (!param?.relativePath.endsWith("\.xml")) {
        // FIXME, TODO
        return;
    }
    let sliceInfo = getSliceInfo(param);
    let fileContent;
    let ret;
    try {
        ret = Module.getTransformFile(param.fileFullPath,
            param.fileId, param.relativePath, param.willSlice,
            param.sliceType, param.sliceRowNum, !param.willSlice && param.willSaveTransformResult);
        fileContent = ret.fileContent;
    } catch (error) {
        console.error("Err: Module.getTransformFile param:",
            param, error.stack);
    }
    if (fileContent?.length > 0) {
        try {
            fileContent = new TextDecoder().decode(fileContent);
            fileContent = JSON.parse(fileContent)
        } catch (err) {
            console.error("parse json error: ", param, fileContent, err)
        }
    } else {
        fileContent = {}
    }
    var response = {
        command: "onGetTransformFileComplete",
        param: param,
        result: {
            hasSlice: sliceInfo.willSlice,
            fileContent: fileContent,
        },
    }
    postMessage(response);
    Module.freeNativeString(ret?.nativeStringPointer);
}

function classof(obj){
    if(typeof(obj)==="undefined")return "undefined";
    if(obj===null)return "Null";
    var res = Object.prototype.toString.call(obj).match(/^\[object\s(.*)\]$/)[1];
    if(res==="Object"){
        res = obj.constructor.name;
        if(typeof(res)!='string' || res.length==0){
            if(obj instanceof jQuery)return "jQuery";// jQuery build stranges Objects
            if(obj instanceof Array)return "Array";// Array prototype is very sneaky
            return "Object";
        }
    }
    return res;
}
function getDocumentSectPrArray(param) {
    let fileContent;
    let ret;
    try {
        ret = Module.getDocumentSectPrArray(param.fileFullPath);
        fileContent = ret?.fileContent;
    } catch (error) {
        console.error("Err: Module.getDocumentSectPrArray param:",
            param, error.stack);
    }
    if (fileContent?.length > 0) {
        try {
            fileContent = new TextDecoder().decode(fileContent)
            fileContent = JSON.parse(fileContent)
        } catch (err) {
            console.error("parse json error: ", param, err, fileContent)
        }
    } else {
        fileContent = {}
    }

    var response = {
        command: "onGetDocumentSectPrArrayComplete",
        param: param,
        result: {
            fileContent: fileContent,
        },
    }
    postMessage(response);
    Module.freeNativeString(ret?.nativeStringPointer);    
}

function getExcelSheetHeadAndTail(param) {
    let fileContent;
    let ret;
    try {
        ret = Module.getExcelSheetHeadAndTail(param.fileFullPath);
        fileContent = ret?.fileContent;
    } catch (error) {
        console.error("Err: Module.getExcelSheetHeadAndTail param:",
            param, error.stack);
    }
    if (fileContent?.length > 0) {
        fileContent = new TextDecoder().decode(fileContent)
        try {
            fileContent = JSON.parse(fileContent)
        } catch (err) {
            console.error("parse json error: ", param, err, fileContent)
        }
    } else {
        fileContent = {}
    }

    var response = {
        command: "onGetExcelSheetHeadAndTailComplete",
        param: param,
        result: {
            fileContent: fileContent,
        },
    }
    postMessage(response);
    Module.freeNativeString(ret?.nativeStringPointer);
}

function writeFile(param) {
    var docFileName = 'f.xml';
    var ret = Module.writeXmlFile(docFileName, param);
    var response = {
        command: "onWriteFileComplete",
        param: param,
    };
    postMessage(response);
}


function getSliceInfo(param) {
    if (!param) {
        return;
    }
    return {
        willSlice: param.willSlice,
        sliceType: param.sliceType,
        sliceRowNum: param?.sliceRowNum ? param?.sliceRowNum : DefaultSliceRowNum,
    }
}

function onParsedSliceData(fileId, relativePath, slicedData, isEnded) {
    // console.log("In js.onParsedSliceData: fileId: "
    //     + fileId + ", relativePath: " + relativePath);
    // console.log("In js.onParsedSliceData: isEnded: "
    //     + isEnded + ", slicedData: " + slicedData);
    // console.log("worker js.onParsedSliceData: isEnded: " + isEnded);

    var fileContent = UTF8ToString(slicedData);
    var response = {
        command: "onParsedSliceDataComplete",
        param: {
            fileId: UTF8ToString(fileId),
            relativePath: UTF8ToString(relativePath),
        },
        result: {
            fileContent: fileContent,
            isEnded: isEnded,
        },
    };
    postMessage(response);
}

function onEndDocument(fileId, relativePath) {
    var response = {
        command: "onEndDocumentComplete",
        param: {
            fileId: UTF8ToString(fileId),
            relativePath: UTF8ToString(relativePath),
        },
    };
    postMessage(response);
}
