
importScripts('xml2json_bin.js');

Module = {};
Module.onRuntimeInitialized = function() {
    var response = {
        command: "onWorkerReady",
    }
    console.log("Module.onRuntimeInitialized: ")
    postMessage(response);
    // let MinBuffSizeToRead = (1024*1024*10);
    // let MinBuffSizeToRead = (1024*1024*5);
    // let MaxBuffSizeToRead = (2 * MinBuffSizeToRead);
    // console.log("Module.onRuntimeInitialized: MinBuffSizeToRead: ", MinBuffSizeToRead);
    // console.log("Module.onRuntimeInitialized: MaxBuffSizeToRead: ", MaxBuffSizeToRead);
    // Module.setMinBuffSizeToRead(MinBuffSizeToRead);
    // Module.setMaxBuffSizeToRead(MaxBuffSizeToRead);
}

// console.log = function() {}

// default row number for shard
var DefaultShardSize = 1;
const ShardType_WordDocument = 0;
const ShardType_ExcelSheetx = 1;

addEventListener("message", (message) => {
  var handlers = {
    "readAsJSON": readAsJSON,
    "readNextShard": readNextShard,
    "shardingParse": shardingParse,
    "convertXML2JSON": convertXML2JSON,
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
    // content = content.replaceAll("\"", "\\\"");
    // content = content.replaceAll("\\", "\\\\");
    // content = content.replaceAll("\'", "\\\'");
    // content = content.replaceAll("\n", "\\n");
    // content = content.replaceAll("\r", "\\r");

    // content = content.replaceAll("\f", "\\f");
    // content = content.replaceAll("\v", "\\v");
    // content = content.replaceAll("\t", "\\t");
    // content = content.replaceAll("", "");

    return content;
}

function readAsJSON(param) {
    if (!param?.relativePath.endsWith("\.xml")) {
        console.error("Err: worker readAsJSON not xml file: ", param);
        return;
    }
    let shardInfo = getShardInfo(param);
    let ret;
    try {
        ret = Module.readAsJSON(param.fileFullPath,
            param.fileId, param.relativePath, shardInfo.willShard,
            shardInfo.shardType, shardInfo.shardSize, shardInfo.willSaveTransformResult);
    } catch (error) {
        console.error("Err: Module.readAsJSON param:",
            param, error.stack);
    }

    let fileContent = ret?.fileContent;
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
    let ret;
    try {
        console.log("worker readNextShard, param: ", param);
        ret = Module.readNextShard(param.fileFullPath,
            param.shardType, param.shardSize);
    } catch (error) {
        console.error("Err: Module.readNextShard param:",
            param, error.stack);
    }
    let fileContent = ret?.fileContent;
    if (fileContent?.length > 0) {
        try {
            fileContent = new TextDecoder().decode(fileContent);
            fileContent = JSON.parse(fileContent)
        } catch (err) {
            fileContent = {}
            console.error("parse json error: ", param, err, fileContent)
        }
    } else {
        fileContent = {}
    }

    let isShardEnded = !ret || !!ret?.isShardEnded;
    var response = {
        command: "onReadNextShardComplete",
        param: param,
        result: {
            fileContent: fileContent,
            isShardEnded: isShardEnded,
        },
    }
    postMessage(response);
    // delete native memory to avoid memory leak
    Module.freeNativeString(ret?.nativeStringPointer);
    if (isShardEnded) {
        deleteFileInternal(param, false);
    }
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

function convertXML2JSON(param) {
    let ret;
    try {
        ret = Module.convertXML2JSON(param?.xmlContent);
    } catch (error) {
        console.error("Err: Module.convertXML2JSON param:",
            param, error.stack);
    }
    let jsonContent = ret?.jsonContent;
    if (jsonContent?.length > 0) {
        try {
            jsonContent = new TextDecoder().decode(jsonContent);
            jsonContent = JSON.parse(jsonContent)
        } catch (err) {
            console.error("parse json error: ", param, err, jsonContent)
        }
    } else {
        jsonContent = {}
    }

    var response = {
        command: "onConvertXML2JSONComplete",
        param: param,
        result: {
            jsonContent: jsonContent,
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

function deleteFileInternal(param, willInform) {
    if (!param) {
        console.error("Err: deleteFileInternal: param is null");
        return;
    }
    param.willInform = willInform;
    deleteFile(param);
}

function deleteFile(param) {
    if (!param) {
        console.error("Err: worker deleteFile: param is null");
        return;
    }
    Module.deleteFile(param?.fileFullPath);
    if (param?.willInform == undefined || !!param?.willInform) {
        var response = {
            command: "onDeleteFileComplete",
            param: param,
        }
        postMessage(response);
    }
}

function getShardInfo(param) {
    if (!param) {
        return;
    }
    return {
        willShard: param.willShard,
        shardType: param.shardType,
        shardSize: param?.shardSize ? param?.shardSize : DefaultShardSize,
        willSaveTransformResult: !param.willShard && param.willSaveTransformResult,
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
