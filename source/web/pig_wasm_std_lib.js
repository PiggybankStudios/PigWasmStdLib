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
// |                        API Functions                         |
// +--------------------------------------------------------------+
function TestFunction()
{
	console.log("TestFunction was called!");
}

apiFuncs = {
	TestFunction: TestFunction,
};

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

async function PigWasm_Init(wasmFilePath, initialMemPageCount)
{
	wasmMemory = new WebAssembly.Memory({ initial: initialMemPageCount });
	let wasmEnvironment =
	{
		memory: wasmMemory,
		...apiFuncs,
	};
	
	console.log("Before loading wasm module we have " + wasmMemory.buffer.byteLength);
	wasmModule = await loadWasmModule(wasmFilePath, wasmEnvironment);
	console.log("After loading wasm module we now have " + wasmMemory.buffer.byteLength);
	console.log("WasmModule:", wasmModule);
	
	wasmModule.exports.InitPigWasmStdLib(initialMemPageCount);
	
	return wasmModule;
}

// ======================== End of pig_wasm_std_lib.js ========================