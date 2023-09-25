
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

// default row number for shard
var DefaultShardRowNum = 1;
const ShardType_WordDocument = 0;
const ShardType_ExcelSheetx = 1;

addEventListener("message", (message) => {
  var handlers = {
    "readAsJSON": readAsJSON,
    "readNextShard": readNextShard,
    "shardingParse": shardingParse,
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

function readAsJSON(param) {
    if (!param) {
        console.error("Err: worker readAsJSON no param.");
        return;
    }
    if (!param?.relativePath.endsWith("\.xml")) {
        // FIXME, TODO
        return;
    }
    let shardInfo = getShardInfo(param);
    let fileContent;
    let ret;
    try {
        ret = Module.readAsJSON(param.fileFullPath,
            param.fileId, param.relativePath, param.willShard,
            param.shardType, param.shardRowNum, !param.willShard && param.willSaveTransformResult);
        fileContent = ret.fileContent;
    } catch (error) {
        console.error("Err: Module.readAsJSON param:",
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
        command: "onReadAsJSONComplete",
        param: param,
        result: {
            hasShard: shardInfo.willShard,
            fileContent: fileContent,
        },
    }
    postMessage(response);
    Module.freeNativeString(ret?.nativeStringPointer);
}

function readNextShard(param) {
    let fileContent;
    let ret;
    try {
        console.log("worker readNextShard, param: ", param);
        console.log("worker readNextShard, fileFullPath: ", param.fileFullPath);
        console.log("worker readNextShard, shardType: ", param.shardType);
        console.log("worker readNextShard, shardSize: ", param.shardSize);

        ret = Module.readNextShard(param.fileFullPath,
            param.shardType, param.shardSize);
        fileContent = ret?.fileContent;
    } catch (error) {
        console.error("Err: Module.readNextShard param:",
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
        command: "onReadNextShardComplete",
        param: param,
        result: {
            fileContent: fileContent,
            isShardEnded: !ret || !!ret?.isShardEnded,
        },
    }
    postMessage(response);
    Module.freeNativeString(ret?.nativeStringPointer);
}

function classof(obj) {
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

function shardingParse(param) {
    let shardType = getShardType(param);
    let retRes = false;
    let fileContent;
    if (ShardType_WordDocument == shardType
        || ShardType_ExcelSheetx == shardType) {
        let ret;
        try {
            if (ShardType_WordDocument == shardType) {
                ret = Module.getDocumentSectPrArray(param.fileFullPath);
            } else if (ShardType_ExcelSheetx == shardType) {
                ret = Module.getExcelSheetHeadAndTail(param.fileFullPath);
            }
        } catch (error) {
            console.error("Err: Module.shardingParse param:",
                param, error.stack);
        }
        fileContent = ret?.fileContent;
        if (fileContent?.length > 0) {
            try {
                fileContent = new TextDecoder().decode(fileContent);
                fileContent = JSON.parse(fileContent);
                // ===================
                // =================
                // ============= move to bamboo
                retRes = true;
            } catch (err) {
                console.error("parse json error: ", param, err, fileContent);
            }
        } else {
            fileContent = {}
        }
        // delete native memory to avoid memory leak
        Module.freeNativeString(ret?.nativeStringPointer);
    }

    var response = {
        command: "onShardingParseComplete",
        param: param,
        result: {
            ret: retRes,
            fileContent: fileContent,
        },
    }
    postMessage(response);
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


function getShardInfo(param) {
    if (!param) {
        return;
    }
    return {
        willShard: param.willShard,
        shardType: param.shardType,
        shardRowNum: param?.shardRowNum ? param?.shardRowNum : DefaultShardRowNum,
    }
}
function getShardType(param) {
    return param.shardType;
}

function onParsedShardData(fileId, relativePath, shardedData, isEnded) {
    // console.log("In js.onParsedShardData: fileId: "
    //     + fileId + ", relativePath: " + relativePath);
    // console.log("In js.onParsedShardData: isEnded: "
    //     + isEnded + ", shardedData: " + shardedData);
    // console.log("worker js.onParsedShardData: isEnded: " + isEnded);

    var fileContent = UTF8ToString(shardedData);
    var response = {
        command: "onParsedShardDataComplete",
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
