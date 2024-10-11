#include "hdl_portable.h"

#if defined(__CORE_CM23_H_DEPENDANT)

typedef struct {
  hdl_nvic_irq_n_t irq_type;
  uint8_t priority_group;
  uint8_t priority;
  struct {
    hdl_event_t event;
  } private;
} hdl_nvic_interrupt_private_t;

HDL_ASSERRT_STRUCTURE_CAST(hdl_nvic_interrupt_private_t, hdl_interrupt_t, HDL_INTERRUPT_PRV_SIZE, hdl_core.h);

void __libc_init_array();
void main();

void nmi_handler()                      { call_isr(HDL_NVIC_EXCEPTION_NonMaskableInt, 0); }
void hard_fault_handler()               { call_isr(HDL_NVIC_EXCEPTION_HardFault, 0); }
void pend_sv_handler()                  { call_isr(HDL_NVIC_EXCEPTION_PendSV, 0); }
void systick_handler()                  { call_isr(HDL_NVIC_EXCEPTION_SysTick, 0); }

/* Returns the current value of the Link Register (LR). */ 
__attribute__( ( always_inline ) ) __STATIC_INLINE uint32_t __get_LR(void)  { 
  register uint32_t result; 
  __ASM volatile ("MOV %0, LR\n" : "=r" (result) ); 
  return(result); 
} 

void call_isr(hdl_nvic_irq_n_t irq, uint32_t event) {
  hdl_interrupt_controller_config_t *ic = (hdl_interrupt_controller_config_t *)((uint32_t *)SCB->VTOR)[0];
  hdl_interrupt_t **isrs = ic->interrupts;
  if(isrs != NULL) {
    while (*isrs != NULL) {
      if((*isrs)->irq_type == irq) {
        hdl_nvic_interrupt_private_t *isr = (hdl_nvic_interrupt_private_t *)*isrs;
        if(!hdl_event_raise(&isr->private.event, ic, event))
          NVIC_DisableIRQ(irq);
        return;
      }
      isrs++;
    }
  }
  //If you get stuck here, your code is missing some interrupt request. see interrupts in MIG file.
	asm("bkpt 255");
  for(;;) ;
}

// __attribute__((naked)) void switch_to_psp(void) {
//   // change PSP
//   asm("LDR R0, =_psp_stack");
//   asm("MSR PSP, R0");
//   // set SPSEL bit
//   asm("MRS R1, CONTROL");
//   asm("ORR R1, R1, #2");
//   asm("MSR CONTROL, R1");
//   // return
//   asm("BX LR");
// }

// __attribute__ ((naked)) void switch_to_unprivileged(void) {
//   // set nPRIV bit
//   asm("MRS R0, CONTROL");
//   asm("ORR R0, R0, #1");
//   asm("MSR CONTROL, r0");
//   // return
//   asm("BX LR");
// }

__attribute__((naked, noreturn)) void reset_handler() {
	asm ("ldr r0, =_estack");
	asm ("mov r13, r0");
	void **pSource, **pDest;
	for (pSource = &_sidata, pDest = &_sdata; pDest != &_edata; pSource++, pDest++)
	  *pDest = *pSource;
  for (pDest = &_sbss; pDest != &_ebss; pDest++)
    *pDest = 0;
  __libc_init_array();
  main();
  for (;;) ;
}

void svc_handler() {
  register uint32_t result;
  //__ASM volatile ("MRS %0, msp" : "=r" (result) );
  asm ("MRS            R1, MSP");
  //__ASM volatile ("MOV %0, LR\n" : "=r" (result) );
  asm ("MOV            R3, LR");
  //if(lr & 4) {
  asm ("LDR            R2, =4");
  asm ("AND            R3, R2");
  asm ("CBZ            R3, test");
  asm ("MRS            R1, PSP");
  asm ("test:\n\
        ADD            R1, R1, #40");
  asm ("MOV            %0, R1" : "=r" (result));
  result = *(uint32_t*)result;
  result = (uint8_t)(*(uint16_t*)(result - 2));
  call_isr(HDL_NVIC_EXCEPTION_SVCall, result);
}

void irq_n_handler() {
  uint32_t prio = -1;
  IRQn_Type irq = 0;
  for(uint32_t i = 0; i < sizeof(NVIC->IABR)/sizeof(NVIC->IABR[0]); i++) {
    uint32_t iabr = NVIC->IABR[i];
    while(iabr) {
      uint8_t bit = 31 - __CLZ(iabr);
      IRQn_Type cur_irq = (32 * i) + bit;
      uint32_t cur_prio = NVIC_GetPriority(cur_irq);
      if(cur_prio < prio) {
        irq = cur_irq;
        prio = cur_prio;
      }
      iabr &= ~(1 << bit);
    }
  }
  call_isr(irq, 0);
}

hdl_module_state_t hdl_interrupt_controller(void *desc, uint8_t enable) {
  if(enable) {
    hdl_interrupt_controller_t *nvic = (hdl_interrupt_controller_t *)desc;
    /* NVIC_SetPriorityGrouping   not available for Cortex-M23 */
    SYSCFG_CPU_IRQ_LAT = nvic->config->irq_latency;
    if(nvic->config->vector != NULL)
      SCB->VTOR = (uint32_t)nvic->config->vector;
    return HDL_MODULE_ACTIVE; 
  }
  else {
    //TODO: disable nvic
  }
  return HDL_MODULE_UNLOADED;
}

uint8_t hdl_interrupt_request(hdl_interrupt_controller_t *ic, const hdl_interrupt_t *isr, hdl_delegate_t *delegate) {
  if((hdl_state(&ic->module) != HDL_MODULE_ACTIVE) || (ic->config->interrupts == NULL) || (delegate == NULL) || (isr == NULL))
    return HDL_FALSE;
  hdl_nvic_interrupt_private_t *_isr = (hdl_nvic_interrupt_private_t *)isr;
  hdl_event_subscribe(&_isr->private.event, delegate);
  uint32_t prio = ((_isr->priority_group << (8U - ic->config->prio_bits)) | 
                  (_isr->priority & (0xFF >> ic->config->prio_bits)) & 
                  0xFFUL);
  uint32_t shift = _BIT_SHIFT(_isr->irq_type);
  volatile uint32_t *ipr = (_isr->irq_type < 0)? &(SCB->SHPR[_SHP_IDX(_isr->irq_type)]):
                                                    &(NVIC->IPR[_IP_IDX(_isr->irq_type)]);
  /* set priority for interrupt */
  *ipr = (*ipr & ~(0xFFUL << shift)) | (prio << shift);
  /* interrupt enable */
  if(_isr->irq_type < 0) {
    switch (_isr->irq_type) {
      case HDL_NVIC_EXCEPTION_SysTick:
        SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;                  /* Enable SysTick IRQ */
        break;
      case HDL_NVIC_EXCEPTION_PendSV:
        // TODO: enable if possible, ;
        break;
      case HDL_NVIC_EXCEPTION_SVCall:
        // TODO: enable if possible, ;
        break;
      case HDL_NVIC_EXCEPTION_HardFault:
        // TODO: enable if possible, ;
        break;
      case HDL_NVIC_EXCEPTION_NonMaskableInt:
        // TODO: enable if possible, ;
        break;
      default:
        return HDL_FALSE;
    }
  }
  else {
    NVIC_EnableIRQ(_isr->irq_type);
  }
  return HDL_TRUE;
}

#endif
