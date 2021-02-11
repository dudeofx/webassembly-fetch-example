# webassembly-fetch-example
A working example of using emscripten's fetch API to save and load raw data from the cloud. The WASM code maintains a simple queue to handle asyncronous code. The back-end is a simple PHP script. The data is saved and loaded from a file as opposed to using a SQL database.


NOTE: To make it as simple as possible when saving data, I take everything on the request body and just dump it to a file. I created a propietary method called 'SPECIAL'. I was going to use 'POST' but I didn't want to complicate things with encoding data or worry about other specifications POST or PUT would requiere.

## Quick Start
Run build.sh to compile the example. This will create two files: 
  * fetch_test.wasm 
  * fetch_test.js

To see the example run publish the four files below into your webserver
  * fetch_test.wasm 
  * fetch_test.js
  * index.html
  * storage_handler.php

NOTE: Make sure the php script has write access to the directory its in. I accomplished that in linux with:
    sudo chgrp www-data .
    sudo chmod . 775 
If you don't like to that, the PHP script should be simple enought to edit and make it work to your liking.

## Tracing the API logic flow
  1. Browser retrieves index.html
  2. index.html sets up the DOM and imports functions LoadData() and SaveData() from fetch_test.js
  3. LoadData() or SaveData() prepare and submit a tFetchWorkOrder to keep track of the transaction. They both call emscripten\_fetch() to start the transaction.
  4. emscripten\_fetch() communicates with the server. On the server side storage\_handler.php attends the request.
  5. storage\_handler.php either reads or writes to data.txt depending on the request.
  6. When the transaction is finished emscripten\_fetch() calls a callback function on the WASM side. These callback functions were provisioned when the tFetchWorkOrder was submitted. They are FetchTransferHandler() and FetchErrorHandler()
  7. The FetchXxxxHandler() functions then call another callback function on the JavaScript side (defined in index.html). These callbacks where provisioned when LoadData() or SaveData() are called. 
  8. The JavaScript callbacks do the final work.
  9. Callbacks are required due to the asyncronous nature of the internet. There are two sets of callbacks because some things need to be taken cared of on the WASM side and some things need to be taken cared of on the JavaScript side.
