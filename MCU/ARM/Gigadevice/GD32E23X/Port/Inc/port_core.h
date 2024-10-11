
#ifndef PORT_CORE_H_
#define PORT_CORE_H_

#include "port_exti.h"
#include "port_flash.h"
#include "gd32e23x.h"

#define HDL_VTOR_TAB_ALIGN         256  //(2 << SCB_VTOR_TBLOFF_Pos)

#define HDL_INTERRUPT_PRV_SIZE       4

typedef enum {
  HDL_NVIC_EXCEPTION_NonMaskableInt    = NonMaskableInt_IRQn,   /*!< Non maskable interrupt                                 */
  HDL_NVIC_EXCEPTION_HardFault         = HardFault_IRQn,        /*!< Hardfault interrupt                                      */
  HDL_NVIC_EXCEPTION_SVCall            = SVCall_IRQn,            /*!< SV call interrupt                           */
  HDL_NVIC_EXCEPTION_PendSV            = PendSV_IRQn,            /*!< Pend SV interrupt                           */
  HDL_NVIC_EXCEPTION_SysTick           = SysTick_IRQn,           /*!< System tick interrupt                       */
  /* interruput numbers */
  HDL_NVIC_IRQ0_WWDGT                  = WWDGT_IRQn,                 /*!< window watchdog timer interrupt                          */
  HDL_NVIC_IRQ1_LVD                    = LVD_IRQn,                   /*!< LVD through EXTI line detect interrupt                   */
  HDL_NVIC_IRQ2_RTC                    = RTC_IRQn,                   /*!< RTC through EXTI line interrupt                          */
  HDL_NVIC_IRQ3_FMC                    = FMC_IRQn,                   /*!< FMC interrupt                                            */
  HDL_NVIC_IRQ4_RCU                    = RCU_IRQn,                   /*!< RCU interrupt                                            */
  HDL_NVIC_IRQ5_EXTI0_1                = EXTI0_1_IRQn,               /*!< EXTI line 0 and 1 interrupts                             */
  HDL_NVIC_IRQ6_EXTI2_3                = EXTI2_3_IRQn,               /*!< EXTI line 2 and 3 interrupts                             */
  HDL_NVIC_IRQ7_EXTI4_15               = EXTI4_15_IRQn,              /*!< EXTI line 4 to 15 interrupts                             */
  HDL_NVIC_IRQ8                        = 8,
  HDL_NVIC_IRQ9_DMA_CH0                = DMA_Channel0_IRQn,          /*!< DMA channel 0 interrupt                                  */
  HDL_NVIC_IRQ10_DMA_CH1_2             = DMA_Channel1_2_IRQn,        /*!< DMA channel 1 and channel 2 interrupts                   */
  HDL_NVIC_IRQ11_DMA_CH3_4             = DMA_Channel3_4_IRQn,        /*!< DMA channel 3 and channel 4 interrupts                   */
  HDL_NVIC_IRQ12_ADC_CMP               = ADC_CMP_IRQn,               /*!< ADC, CMP interrupts                                      */
  HDL_NVIC_IRQ13_TIMER0_BRK_UP_TRG_COM = TIMER0_BRK_UP_TRG_COM_IRQn, /*!< TIMER0 break, update, trigger and commutation interrupts */
  HDL_NVIC_IRQ14_TIMER0_CH             = TIMER0_Channel_IRQn,        /*!< TIMER0 channel capture compare interrupts                */
  HDL_NVIC_IRQ15                       = 15,
  HDL_NVIC_IRQ16_TIMER2                = TIMER2_IRQn,                /*!< TIMER2 interrupt                                         */
  HDL_NVIC_IRQ17_TIMER5                = TIMER5_IRQn,                /*!< TIMER5 interrupt                                         */
  HDL_NVIC_IRQ18                       = 18,
  HDL_NVIC_IRQ19_TIMER13               = TIMER13_IRQn,               /*!< TIMER13 interrupt                                        */
  HDL_NVIC_IRQ20_TIMER14               = TIMER14_IRQn,               /*!< TIMER14 interrupt                                        */
  HDL_NVIC_IRQ21_TIMER15               = TIMER15_IRQn,               /*!< TIMER15 interrupt                                        */
  HDL_NVIC_IRQ22_TIMER16               = TIMER16_IRQn,               /*!< TIMER16 interrupt                                        */
  HDL_NVIC_IRQ23_I2C0_EV               = I2C0_EV_IRQn,               /*!< I2C0 event interrupt                                     */
  HDL_NVIC_IRQ24_I2C1_EV               = I2C1_EV_IRQn,               /*!< I2C1 event interrupt                                     */
  HDL_NVIC_IRQ25_SPI0                  = SPI0_IRQn,                  /*!< SPI0 interrupt                                           */
  HDL_NVIC_IRQ26_SPI1                  = SPI1_IRQn,                  /*!< SPI1 interrupt                                           */
  HDL_NVIC_IRQ27_USART0                = USART0_IRQn,                /*!< USART0 interrupt                                         */
  HDL_NVIC_IRQ28_USART1                = USART1_IRQn,                /*!< USART1 interrupt                                         */
  HDL_NVIC_IRQ29                       = 29,
  HDL_NVIC_IRQ30                       = 30,
  HDL_NVIC_IRQ31                       = 31,
  HDL_NVIC_IRQ32_I2C0_ER               = I2C0_ER_IRQn,               /*!< I2C0 error interrupt                                     */
  HDL_NVIC_IRQ33                       = 33,
  HDL_NVIC_IRQ34_I2C1_ER               = I2C1_ER_IRQn,               /*!< I2C1 error interrupt                                     */
  HDL_NVIC_IRQ35                       = 35,
  HDL_NVIC_IRQ36                       = 36,
  HDL_NVIC_IRQ37                       = 37,
  HDL_NVIC_IRQ38                       = 38,
  HDL_NVIC_IRQ39                       = 39,
  HDL_NVIC_IRQ40                       = 40,
  HDL_NVIC_IRQ41                       = 41,
  HDL_NVIC_IRQ42                       = 42,
  HDL_NVIC_IRQ43                       = 43,
  HDL_NVIC_IRQ44                       = 44,
  HDL_NVIC_IRQ45                       = 45,
  HDL_NVIC_IRQ46                       = 46,
  HDL_NVIC_IRQ47                       = 47,
  HDL_NVIC_IRQ48                       = 48,
} hdl_nvic_irq_n_t;

typedef struct {
  hdl_nvic_irq_n_t irq_type;
  uint8_t priority_group;
  uint8_t priority;
  PRIVATE(hw, HDL_INTERRUPT_PRV_SIZE);
} hdl_interrupt_t;

/* GD32E230 interrupt vector
  .vector = {
    &_estack,
    &reset_handler,
    &nmi_handler,
    &hard_fault_handler,
    ((void *)0),
    ((void *)0),
    ((void *)0),
    ((void *)0),
    ((void *)0),
    ((void *)0),
    ((void *)0),
    &svc_handler,
    ((void *)0),
    ((void *)0),
    &pend_sv_handler,
    &systick_handler,
    &wwdgt_handler, 
    &lvd_handler,
    &rtc_handler,
    &fmc_handler,
    &rcu_handler,
    &exti0_1_handler,
    &exti2_3_IRQHandler,
    &exti4_15_handler,
    &irq_n_handler,
    &dma_channel0_handler,
    &dma_channel1_2_handler,
    &dma_channel3_4_handler,
    &adc_cmp_handler,
    &timer0_brk_up_trg_com_handler,
    &timer0_channel_handler,
    &irq_n_handler,
    &timer2_handler,
    &timer5_handler,
    &irq_n_handler,
    &timer13_handler,
    &timer14_handler,
    &timer15_handler,
    &timer16_handler,
    &i2c0_ev_handler,
    &i2c1_ev_handler,
    &spi0_handler,
    &spi1_handler,
    &usart0_handler,
    &usart1_handler,
    &irq_n_handler,
    &irq_n_handler,
    &irq_n_handler,
    &i2c0_er_handler,
    &irq_n_handler,
    &i2c1_er_handler,
  }
 */
typedef struct {
  uint32_t prio_bits;
  hdl_interrupt_t **interrupts;
  uint8_t irq_latency; /* processor ensures that a minimum of irq_latency+1 hclk cycles exist between an interrupt becoming pended */
  void *vector;
} hdl_interrupt_controller_config_t;

#define hdl_interrupts(...) ((hdl_interrupt_t *[]){__VA_ARGS__, NULL})

typedef struct{
  uint32_t flash_latency;
} hdl_core_config_t;

extern void *_estack;
extern void *_sidata, *_sdata, *_edata;
extern void *_sbss, *_ebss;

void call_isr(hdl_nvic_irq_n_t irq, uint32_t event);

void reset_handler();
void irq_n_handler();
void nmi_handler();
void hard_fault_handler();
void svc_handler();
void pend_sv_handler();
void systick_handler();

void wwdgt_handler();
void lvd_handler();
void rtc_handler();
void fmc_handler();
void rcu_handler();
void exti0_1_handler();
void exti2_3_IRQHandler();
void exti4_15_handler();
void dma_channel0_handler();
void dma_channel1_2_handler();
void dma_channel3_4_handler();
void adc_cmp_handler();
void timer0_brk_up_trg_com_handler();
void timer0_channel_handler();
void timer2_handler();
void timer5_handler();
void timer13_handler();
void timer14_handler();
void timer15_handler();
void timer16_handler();
void i2c0_ev_handler();
void i2c1_ev_handler();
void spi0_handler();
void spi1_handler();
void usart0_handler();
void usart1_handler();
void i2c0_er_handler();
void i2c1_er_handler();

#endif /* PORT_CORE_H_ */ 
