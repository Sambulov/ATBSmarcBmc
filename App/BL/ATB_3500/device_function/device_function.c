/*!
    \file device_function.c
    \author a.bodin
    
    \version 2024-06-04, V1.0.0, This file describe functions, which using, in the business logic
 */
#if defined (ATB_3500)

#include "device_logic.h"

#define ADC_FILTER_LEN                  (uint16_t)(10)
#define ADC_FILTER_SAMPLE_TIME_MS       (uint32_t)(1)

#define TIMEOUT_MS_POE_24                ((uint32_t) 5000)
#define MAX_BOOT_BOOT_RETRY              ((int8_t)5)

/*!
    \brief          Function returns difference between timestamp and current ms counters
    \param[in]      time_stamp_ms
    \return         difference in ms
 */
uint32_t get_ms_time_from(uint32_t time_stamp_ms) {
    return hdl_timer_get(get_object_timer_ms()) - time_stamp_ms;
}
/*!
    \brief          Get current ms counter value
 */
uint32_t get_ms_time (void) {
  return hdl_timer_get(get_object_timer_ms());
}

/*!
    \brief          Initial struct init
    \param[in]      od - pointer to struct
    \return         void
 */
void device_object_distonary_init_default(object_dictionary_t *od) {
    memset(od, 0, sizeof(object_dictionary_t));
    /* Intit filter for adc */
    od->counter_of_boot_retry = MAX_BOOT_BOOT_RETRY;
    for(uint16_t i = 0; i < ADC_CHANNEL_AMOUNT; i++) {
        filter_mean_init(&od->adc_filter[i], ADC_FILTER_LEN, ADC_FILTER_SAMPLE_TIME_MS, &get_ms_time_from, &get_ms_time);
    }     
}

/*!
    \brief          Init all hardware module
    \param[in]      h - pointer to struct
    \return         
      \retval         DL_STATUS_SUCCES
      \retval         DL_STATUS_ERROR
 */
device_logic_status_e device_hardware_init(object_dictionary_t *h) {
    /***********************************************************
     *               TIMERS
    ***********************************************************/
    hdl_enable(HDL_MODULE_WRAP(get_object_timer_ms()));
    /***********************************************************
     *               SMARC POWER UP and SMARC GPIO
     ***********************************************************/
    hdl_enable(HDL_MODULE_WRAP(get_object_do_smarc_reset()));
    hdl_enable(HDL_MODULE_WRAP(get_object_di_smarc_reset_feedback()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_smarc_boot_0()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_smarc_boot_0()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_smarc_boot_2()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_smarc_button()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_smarc_irq_1()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_smarc_irq_2()));
    /***********************************************************
     *                      LED
     ***********************************************************/
    hdl_enable(HDL_MODULE_WRAP(get_object_do_led_0_0()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_led_0_1()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_led_0_2()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_led_1_0()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_led_1_1()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_led_1_2()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_led_2_0()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_led_2_1()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_led_2_2()));
    /***********************************************************
     *                  POWER ADJUST
     ***********************************************************/
    hdl_enable(HDL_MODULE_WRAP(get_object_do_24v_power_on()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_5v_power_on()));
    hdl_enable(HDL_MODULE_WRAP(get_object_di_power_fault()));
    hdl_enable(HDL_MODULE_WRAP(get_object_di_power_good()));
    /***********************************************************
     *                        Other
    ***********************************************************/
    hdl_enable(HDL_MODULE_WRAP(get_object_do_pci_switch()));
    hdl_enable(HDL_MODULE_WRAP(get_object_do_lte_reset()));
    /***********************************************************
     *                  ADC
     ***********************************************************/
    hdl_enable(HDL_MODULE_WRAP(get_object_gpio_ain_0()));
    hdl_enable(HDL_MODULE_WRAP(get_object_gpio_ain_1()));
    hdl_enable(HDL_MODULE_WRAP(get_object_gpio_ain_2()));
    hdl_enable(HDL_MODULE_WRAP(get_object_gpio_ain_3()));
    hdl_enable(HDL_MODULE_WRAP(get_object_gpio_ain_4()));
    hdl_enable(HDL_MODULE_WRAP(get_object_gpio_ain_5()));
    hdl_enable(HDL_MODULE_WRAP(get_object_adc()));

    while (!hdl_init_complete()) {
        cooperative_scheduler(false);
    }
    return DL_STATUS_SUCCESS;
}

/*!
    \brief          Update and filter adc value
    \param[in]      od - pointer to struct
    \return         void
 */
void device_adc_proc(object_dictionary_t* od) {
    uint16_t adc_raw_value = hdl_adc_get_data(get_object_adc(), get_object_adc_24v());
    uint16_t adc_calc_value = 0;
    uint8_t adc_filter_is_full_occupy = 1;
    /* Channel 0 */
    if( (adc_raw_value != HDL_ADC_INVALID_VALUE) && (od->adc_filter[ATB3500_ADC_24V].ok) ) {
        adc_calc_value = adc_raw_value * ADC_CHANNEL_0_VOLTAGE_DIVIDER * ADC_REFERENCE_VOLTAGE_MV / ADC_RESOLUTION;
        filter_mean(&od->adc_filter[ATB3500_ADC_24V], adc_calc_value);
        if(!(filter_mean_is_full(&od->adc_filter[ATB3500_ADC_24V])))
            adc_filter_is_full_occupy = 0;
    }
    else
        adc_filter_is_full_occupy = 0;
    /* Channel 1 */
    adc_raw_value = hdl_adc_get_data(get_object_adc(), get_object_adc_24v_poe());
    if( (adc_raw_value != HDL_ADC_INVALID_VALUE) && (od->adc_filter[ATB3500_ADC_24V_POE].ok) ) {
        adc_calc_value = adc_raw_value * ADC_CHANNEL_1_VOLTAGE_DIVIDER * ADC_REFERENCE_VOLTAGE_MV / ADC_RESOLUTION;
        filter_mean(&od->adc_filter[ATB3500_ADC_24V_POE], adc_calc_value);
        if(!(filter_mean_is_full(&od->adc_filter[ATB3500_ADC_24V_POE])))
            adc_filter_is_full_occupy = 0;
    }
    else
        adc_filter_is_full_occupy = 0;
    /* Channel 2 */
    adc_raw_value = hdl_adc_get_data(get_object_adc(), get_object_adc_5v());
    if( (adc_raw_value != HDL_ADC_INVALID_VALUE) && (od->adc_filter[ATB3500_ADC_5V].ok) ) {
        adc_calc_value = adc_raw_value * ADC_CHANNEL_2_VOLTAGE_DIVIDER * ADC_REFERENCE_VOLTAGE_MV / ADC_RESOLUTION;
        filter_mean(&od->adc_filter[ATB3500_ADC_5V], adc_calc_value);
        if(!(filter_mean_is_full(&od->adc_filter[ATB3500_ADC_5V])))
            adc_filter_is_full_occupy = 0;
    }
    else
        adc_filter_is_full_occupy = 0;
    /* Channel 3 */
    adc_raw_value = hdl_adc_get_data(get_object_adc(), get_object_adc_3v3());
    if( (adc_raw_value != HDL_ADC_INVALID_VALUE) && (od->adc_filter[ATB3500_ADC_3V3].ok) ) {
        adc_calc_value = adc_raw_value * ADC_CHANNEL_3_VOLTAGE_DIVIDER * ADC_REFERENCE_VOLTAGE_MV / ADC_RESOLUTION;
        filter_mean(&od->adc_filter[ATB3500_ADC_3V3], adc_calc_value);
        if(!(filter_mean_is_full(&od->adc_filter[ATB3500_ADC_3V3])))
            adc_filter_is_full_occupy = 0;
    }
    else
        adc_filter_is_full_occupy = 0;
    /* Channel 4 */
    adc_raw_value = hdl_adc_get_data(get_object_adc(), get_object_adc_2v5());
    if( (adc_raw_value != HDL_ADC_INVALID_VALUE) && (od->adc_filter[ATB3500_ADC_2V5].ok) ) {
        adc_calc_value = adc_raw_value * ADC_CHANNEL_4_VOLTAGE_DIVIDER * ADC_REFERENCE_VOLTAGE_MV / ADC_RESOLUTION;
        filter_mean(&od->adc_filter[ATB3500_ADC_2V5], adc_calc_value);
        if(!(filter_mean_is_full(&od->adc_filter[ATB3500_ADC_2V5])))
            adc_filter_is_full_occupy = 0;
    }
    else
        adc_filter_is_full_occupy = 0;
    /* Channel 5 */
    adc_raw_value = hdl_adc_get_data(get_object_adc(), get_object_adc_1v8());
    if( (adc_raw_value != HDL_ADC_INVALID_VALUE) && (od->adc_filter[ATB3500_ADC_1V8].ok) ) {
        adc_calc_value = adc_raw_value * ADC_CHANNEL_5_VOLTAGE_DIVIDER * ADC_REFERENCE_VOLTAGE_MV / ADC_RESOLUTION;
        filter_mean(&od->adc_filter[ATB3500_ADC_1V8], adc_calc_value);
        if(!(filter_mean_is_full(&od->adc_filter[ATB3500_ADC_1V8])))
            adc_filter_is_full_occupy = 0;
    }
    else
        adc_filter_is_full_occupy = 0;
    if(adc_filter_is_full_occupy)
        od->sb.sb_adc_include_valid_data = 1;
    else 
        od->sb.sb_adc_include_valid_data = 0;
}

/*!
    \brief          This function check power status
 */
void device_check_power_status(object_dictionary_t *od) {
    static uint32_t poe_24v_check_time_stamp = 0;
    if(!od->sb.sb_24v_poe_is_enable) {
        od->error.dl_error_poe_fault = 0;
        od->error.dl_error_poe_not_good = 0;
        poe_24v_check_time_stamp = get_ms_time();
    }
    else {
        if( (hdl_gpio_read(get_object_di_power_fault())) == (!get_object_di_power_fault()->inactive_default) ) {
            if(get_ms_time_from(poe_24v_check_time_stamp) >= TIMEOUT_MS_POE_24) {
                od->error.dl_error_poe_fault = 1;
            }
        }
        if( (hdl_gpio_read(get_object_di_power_good())) == (get_object_di_power_good()->inactive_default) ) {
            if(get_ms_time_from(poe_24v_check_time_stamp) >= TIMEOUT_MS_POE_24) {
                od->error.dl_error_poe_not_good = 1;
            }
        }
    }
}
/*!
    \brief          This function describe relay control
 */
void device_relay_control(object_dictionary_t *od) {
    if(od->error.dl_error_poe_fault || od->error.dl_error_poe_not_good) {
        hdl_gpio_write(get_object_do_relay_1(), !get_object_do_relay_1()->inactive_default);
        hdl_gpio_write(get_object_do_relay_1(), !get_object_do_relay_1()->inactive_default);
    }
    else {
        hdl_gpio_write(get_object_do_relay_1(), get_object_do_relay_1()->inactive_default);
        hdl_gpio_write(get_object_do_relay_1(), !get_object_do_relay_1()->inactive_default); 
    }
}

void device_smarc_irq_proc(object_dictionary_t *od) {
    if(od->error.dl_error_poe_fault)
        hdl_gpio_write(get_object_do_smarc_irq_1(), !get_object_do_smarc_irq_1()->inactive_default);
    else
        hdl_gpio_write(get_object_do_smarc_irq_1(), get_object_do_smarc_irq_1()->inactive_default);
    if(od->error.dl_error_poe_not_good)
        hdl_gpio_write(get_object_do_smarc_irq_2(), !get_object_do_smarc_irq_2()->inactive_default);
    else
        hdl_gpio_write(get_object_do_smarc_irq_2(), get_object_do_smarc_irq_2()->inactive_default);
}

uint8_t device_choose_smarc_boot(object_dictionary_t *od) {
    return SUCCESS;
}

#endif /* ATB3500 */