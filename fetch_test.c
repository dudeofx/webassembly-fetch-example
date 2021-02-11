#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <emscripten.h>
#include <emscripten/fetch.h>

enum {
   FETCH_NOP,
   FETCH_LOAD,
   FETCH_SAVE,
   FETCH_CMD_COUNT
};

typedef struct _tFetchWorkOrder tFetchWorkOrder;
struct _tFetchWorkOrder {
   void (*JSHandler)(char *);
   int                  id;
   int                  cmd;
   char                *data;
   emscripten_fetch_t  *fetch_struct;
   tFetchWorkOrder     *next;
};

tFetchWorkOrder *ticket_wheel =  NULL;
int              ticket_uid = 0x1000;

//-----------------------------------------------------------------------
void RemoveWorkOrder(tFetchWorkOrder *wk) {
   if (ticket_wheel == wk) {
      ticket_wheel = wk->next;
      free(wk);
   } else {
      tFetchWorkOrder *tmp = ticket_wheel;
      while (tmp->next != wk) tmp = tmp->next;
      tmp->next = wk->next;
      free(wk);
   }
}
//-----------------------------------------------------------------------
void FetchTransferHandler(emscripten_fetch_t *fetch) {
   tFetchWorkOrder *wk = (tFetchWorkOrder *) fetch->userData;
 
   switch (wk->cmd) {
      case FETCH_LOAD:
         wk->JSHandler((char *) fetch->data);
         break;
      case FETCH_SAVE:
         wk->JSHandler((char *) fetch->data);
         break;
   }
   emscripten_fetch_close(fetch);
   RemoveWorkOrder(wk);
}
//-----------------------------------------------------------------------
void FetchErrorHandler(emscripten_fetch_t *fetch) {
   tFetchWorkOrder *wk = (tFetchWorkOrder *) fetch->userData;
   switch (wk->cmd) {
      case FETCH_LOAD:
         wk->JSHandler("");
         break;
      case FETCH_SAVE:
         wk->JSHandler(fetch->statusText);
         break;
   }
   emscripten_fetch_close(fetch);
   RemoveWorkOrder(wk);
}
//-----------------------------------------------------------------------
void SubmitWorkOrder(int cmd, char *data, void (*JSHandler)(char *)) {
   if (cmd <= FETCH_NOP) return;
   if (cmd >= FETCH_CMD_COUNT) return;

   tFetchWorkOrder *order = malloc(sizeof(tFetchWorkOrder));

   order->JSHandler = JSHandler;
   order->id = ticket_uid++;
   order->cmd = cmd;
   order->data = data;
   order->fetch_struct = NULL;

   order->next = ticket_wheel;
   ticket_wheel = order;
    
   emscripten_fetch_attr_t attr;
   emscripten_fetch_attr_init(&attr);

   switch (cmd) {
      case FETCH_LOAD:
         attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
         strcpy(attr.requestMethod, "GET");
         break;
      case FETCH_SAVE: // non standard approach so just keep it to ourselves
         attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
         strcpy(attr.requestMethod, "SPECIAL");
         attr.requestData = data;
         attr.requestDataSize = strlen(data)+1;
         break;
   };
   attr.onsuccess = FetchTransferHandler;
   attr.onerror = FetchErrorHandler;
   attr.userData = order;

   order->fetch_struct = emscripten_fetch(&attr, "storage_handler.php");
}
//-----------------------------------------------------------------------
EMSCRIPTEN_KEEPALIVE 
void LoadData(void (*JSHandler)(char *)) {
   SubmitWorkOrder(FETCH_LOAD, NULL, JSHandler);
}
//-----------------------------------------------------------------------
EMSCRIPTEN_KEEPALIVE
void SaveData(char *data, void (*JSHandler)(char *)) {
   SubmitWorkOrder(FETCH_SAVE, data, JSHandler);
}

//-----------------------------------------------------------------------
int main() {
   // alert DOM side when they can start using the APIs
   EM_ASM(
      if (typeof Module.OnUtilsReady !== 'undefined') {
         Module.OnUtilsReady();
      }
   );
}
//-----------------------------------------------------------------------

