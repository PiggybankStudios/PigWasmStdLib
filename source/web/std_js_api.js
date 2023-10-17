// ======================= Start of pig_wasm_std_lib.js =======================
/*
File:   std_js_api.js
Author: Taylor Robbins
Date:   10\07\2023
Description:
	** Contains a bunch of client-side code that is required for PigWasm StdLib to function properly
	** This includes a bunch of functions that need to be imported to the wasm module for it to be able to
	** communicate with the browser for basic functionality (like stdout, memory allocation, etc.)
	** The application should combine this javascript file with their own file(s) to create a "combined.js"
	** file which will be included by index.html
*/

const WASM_PAGE_SIZE = (64 * 1024); //64kB

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
var stdGlobals =
{
	heapBase: 0,
	canvas: null,
	glContext: null,
	wasmMemory: null,
	wasmModule: null,
};

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
async function loadWasmModule(filePath, environment)
{
	// console.log("Loading " + filePath + "...");
	let result = null;
	try
	{
		const fetchPromise = fetch(filePath);
		const wasmModule = await WebAssembly.instantiateStreaming(
			fetchPromise,
			{ env: environment }
		);
		result = wasmModule.instance;
		// console.log("Loaded module exports:", result.exports);
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
function jsStdAbort(messageStrPntr, exitCode)
{
	let messageStr = wasmPntrToJsString(stdGlobals.wasmMemory, messageStrPntr);
	let exitStr = "Abort [" + exitCode + "]: " + messageStr;
	console.error(exitStr);
	throw new Error(exitStr);
}

function jsStdAssertFailure(filePathPntr, fileLineNum, funcNamePntr, messageStrPntr)
{
	let filePath = wasmPntrToJsString(stdGlobals.wasmMemory, filePathPntr);
	let funcName = wasmPntrToJsString(stdGlobals.wasmMemory, funcNamePntr);
	let messageStr = wasmPntrToJsString(stdGlobals.wasmMemory, messageStrPntr);
	let outputMessage = "Assertion failed! (" + messageStr + ") is not true! In " + filePath + ":" + fileLineNum + " " + funcName + "(...)";
	console.error(outputMessage);
	throw new Error(outputMessage);
}

function jsStdGetHeapSize()
{
	return stdGlobals.wasmMemory.buffer.byteLength - stdGlobals.heapBase;
}

function jsStdGrowMemory(numPages)
{
	// console.log("Memory growing by " + numPages + " pages");
	stdGlobals.wasmMemory.grow(numPages);
}

jsStdApiFuncs = {
	jsStdAbort: jsStdAbort,
	jsStdAssertFailure: jsStdAssertFailure,
	jsStdGrowMemory: jsStdGrowMemory,
	jsStdGetHeapSize: jsStdGetHeapSize,
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
	stdGlobals.canvas = canvas;
	return canvas;
}

function PigWasm_CreateGlContext(canvas)
{
	canvasContextGl = canvas.getContext("webgl2");
	if (canvasContextGl === null) { console.error("Unable to initialize WebGL render context. Your browser or machine may not support it :("); return null; }
	// console.log(canvasContextGl);
	stdGlobals.glContext = canvasContextGl;
	return canvasContextGl;
}

function PigWasm_InitMemory(initialMemPageCount)
{
	wasmMemory = new WebAssembly.Memory({ initial: initialMemPageCount });
	stdGlobals.wasmMemory = wasmMemory;
	return wasmMemory
}

async function PigWasm_Init(wasmMemory, initialMemPageCount, wasmFilePath, appApiFuncs)
{
	apiFuncs = {
		...jsStdApiFuncs,
		...appApiFuncs,
	}
	
	let wasmEnvironment =
	{
		memory: wasmMemory,
		...apiFuncs,
	};
	
	// console.log("Before loading wasm module we have " + wasmMemory.buffer.byteLength);
	wasmModule = await loadWasmModule(wasmFilePath, wasmEnvironment);
	// console.log("After loading wasm module we now have " + wasmMemory.buffer.byteLength);
	// console.log("WasmModule:", wasmModule);
	
	wasmModule.exports.InitStdLib(initialMemPageCount);
	
	stdGlobals.heapBase = wasmModule.exports.GetStackBase();
	stdGlobals.heapBase += 1024; //1kB should account for any innacuracy in our crude method of calculating the stack base
	if ((stdGlobals.heapBase % WASM_PAGE_SIZE) != 0)
	{
		stdGlobals.heapBase += WASM_PAGE_SIZE - (stdGlobals.heapBase % WASM_PAGE_SIZE); //align to a page barrier
	}
	
	stdGlobals.wasmModule = wasmModule;
	return wasmModule;
}

// ======================== End of pig_wasm_std_lib.js ========================