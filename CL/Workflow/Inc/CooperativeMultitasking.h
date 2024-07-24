#ifndef COOPERATIVE_MT_H_INCLUDED
#define COOPERATIVE_MT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define CO_ROUTINE_DESC_SIZE  28

typedef struct {
  CL_PRIVATE(CO_ROUTINE_DESC_SIZE);
} Coroutine_t;

typedef uint8_t (*CoroutineHandler_t)(Coroutine_t *pxThis, uint8_t bCancel, void *pxArg);

void vCoroutineAdd(Coroutine_t *pcCoRBuffer, CoroutineHandler_t pfHandler, void *pxArg);
void vCoroutineSetContext(Coroutine_t *pxCoR, void* pxArg);
void vCoroutineCancel(Coroutine_t *pxCoR);
void vCoroutineTerminate(Coroutine_t *pxCoR);

uint32_t ulCooperativeScheduler(uint8_t bCancelAll);

/*!
  Snake notation
*/

typedef Coroutine_t coroutine_t;
typedef CoroutineHandler_t coroutine_handler_t;

void coroutine_add(coroutine_t *cor_buf, coroutine_handler_t handler, void *arg);
void coroutine_set_context(coroutine_t *cor, void* arg);
void coroutine_cancel(coroutine_t *cor);
void coroutine_terminate(coroutine_t *cor);

uint32_t cooperative_scheduler(uint8_t cancel_all);

#ifdef __cplusplus
}
#endif

#endif /* COOPERATIVE_MT_H_INCLUDED */ 
