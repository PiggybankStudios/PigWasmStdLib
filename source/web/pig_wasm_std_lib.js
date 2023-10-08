// ======================= Start of pig_wasm_std_lib.js =======================
/*
File:   pig_wasm_std_lib.js
Author: Taylor Robbins
Date:   10\07\2023
Description:
	** Contains a bunch of client-side code that is required for PigWasm StdLib to function properly
	** This includes a bunch of functions that need to be imported to the wasm module for it to be able to
	** communicate with the browser for basic functionality (like stdout, memory allocation, etc.)
	** The application should combine this javascript file with their own file(s) to create a "combined.js"
	** file which will be included by index.html
*/

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
var globalCanvas = null;
var globalGlContext = null;
var globalWasmMemory = null;
var globalWasmModule = null;

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
async function loadWasmModule(filePath, environment)
{
	console.log("Loading " + filePath + "...");
	let result = null;
	try
	{
		const fetchPromise = fetch(filePath);
		const wasmModule = await WebAssembly.instantiateStreaming(
			fetchPromise,
			{ env: environment }
		);
		result = wasmModule.instance;
		console.log("Loaded module exports:", result.exports);
	}
	catch (exception)
	{
		console.error("Failed to load WASM module from \"" + filePath + "\":", exception);
	}
	return result;
}

function wasmPntrToJsString(memory, ptr)
{
	const codes = [];
	const buf = new Uint8Array(memory.buffer);
	
	let cIndex = 0;
	while (true)
	{
		const char = buf[ptr + cIndex];
		if (!char) { break; }
		codes.push(char);
		cIndex++;
	}
	
	//TODO: Can we do something else? If we do our own UTF-8 parsing maybe?
	return String.fromCharCode(...codes);
}

// +--------------------------------------------------------------+
// |                        API Functions                         |
// +--------------------------------------------------------------+
function jsAbort(messageStrPntr)
{
	let messageStr = wasmPntrToJsString(globalWasmMemory, messageStrPntr);
	console.error("Abort:", messageStr);
	throw new Error(messageStr);
}

function jsAssertFailure(filePathPntr, fileLineNum, funcNamePntr, messageStrPntr)
{
	let filePath = wasmPntrToJsString(globalWasmMemory, filePathPntr);
	let funcName = wasmPntrToJsString(globalWasmMemory, funcNamePntr);
	let messageStr = wasmPntrToJsString(globalWasmMemory, messageStrPntr);
	let outputMessage = "Assertion failed! (" + messageStr + ") is not true! In " + filePath + ":" + fileLineNum + " " + funcName + "(...)";
	console.error(outputMessage);
	throw new Error(outputMessage);
}

function jsGrowMemory(numPages)
{
	// console.log("Memory growing by " + numPages + " pages");
	globalWasmMemory.grow(numPages);
}

function jsTestFunction()
{
	console.log("TestFunction was called!");
}

function jsPrintNumber(labelStrPntr, number)
{
	let labelStr = wasmPntrToJsString(globalWasmMemory, labelStrPntr);
	console.log(labelStr + ": " + number + " (0x" + number.toString(16) + ")");
}

function jsPrintFloat(labelStrPntr, number)
{
	let labelStr = wasmPntrToJsString(globalWasmMemory, labelStrPntr);
	console.log(labelStr + ": " + number);
}

function jsPrintString(strPntr)
{
	let str = wasmPntrToJsString(globalWasmMemory, strPntr);
	console.log("String:", str);
}

apiFuncs = {
	jsAbort: jsAbort,
	jsAssertFailure: jsAssertFailure,
	jsGrowMemory: jsGrowMemory,
	jsTestFunction: jsTestFunction,
	jsPrintNumber: jsPrintNumber,
	jsPrintFloat: jsPrintFloat,
	jsPrintString: jsPrintString,
};

// +--------------------------------------------------------------+
// |                        Main Functions                        |
// +--------------------------------------------------------------+
function PigWasm_AcquireCanvas(canvasWidth, canvasHeight)
{
	canvas = document.getElementsByTagName("canvas")[0];
	// console.log(canvas);
	
	// set the display size of the canvas.
	canvas.style.width = canvasWidth + "px";
	canvas.style.height = canvasHeight + "px";
	
	// set the size of the drawingBuffer
	var devicePixelRatio = window.devicePixelRatio || 1;
	canvas.width = canvasWidth * devicePixelRatio;
	canvas.height = canvasHeight * devicePixelRatio;
	
	// canvasContainer = document.getElementById("canvas_container");
	// console.assert(canvasContainer != null, "Couldn't find canvas container DOM element!");
	return canvas;
}

function PigWasm_CreateGlContext(canvas)
{
	canvasContextGl = canvas.getContext("webgl2");
	if (canvasContextGl === null) { console.error("Unable to initialize WebGL render context. Your browser or machine may not support it :("); return null; }
	// console.log(canvasContextGl);
	return canvasContextGl;
}

function PigWasm_InitMemory(initialMemPageCount)
{
	wasmMemory = new WebAssembly.Memory({ initial: initialMemPageCount });
	return wasmMemory
}

async function PigWasm_Init(wasmMemory, initialMemPageCount, wasmFilePath)
{
	let wasmEnvironment =
	{
		memory: wasmMemory,
		...apiFuncs,
	};
	
	console.log("Before loading wasm module we have " + wasmMemory.buffer.byteLength);
	wasmModule = await loadWasmModule(wasmFilePath, wasmEnvironment);
	console.log("After loading wasm module we now have " + wasmMemory.buffer.byteLength);
	console.log("WasmModule:", wasmModule);
	
	wasmModule.exports.InitStdLib(initialMemPageCount);
	
	return wasmModule;
}

// ======================== End of pig_wasm_std_lib.js ========================