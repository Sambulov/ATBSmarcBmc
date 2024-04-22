#include "hdl_portable.h"
#include "Macros.h"

typedef enum {
    GD_ADC_STATE_MACHINE_DISABLE,
    GD_ADC_STATE_MACHINE_CALIBRATE,
    GD_ADC_STATE_MACHINE_OK,
    GD_ADC_STATE_MACHINE_FAULT,
} gd_adc_state_machine_e;

static uint8_t internal_dma_is_init = 0;

int8_t _gd_adc_calibrate(hdl_timer_t *timer, uint32_t time_stamp, uint32_t timeout){
    static uint8_t flag1 = 0;
    static uint8_t flag2 = 0;
    /* reset the selected ADC calibration register */
    if(!flag1){
        flag1 = 1;
        ADC_CTL1 |= (uint32_t)ADC_CTL1_RSTCLB;
    }
    if ((ADC_CTL1 & ADC_CTL1_RSTCLB)){
        if (TIME_ELAPSED(time_stamp, timeout, hdl_timer_get(timer)))
            return HDL_MODULE_INIT_FAILED;
        else
            return HDL_MODULE_INIT_ONGOING;
    }

    if(!flag2){
        flag2 = 1;
        ADC_CTL1 |= ADC_CTL1_CLB;
    }
    /* enable ADC calibration process */
    if ((ADC_CTL1 & ADC_CTL1_CLB)){
        if (TIME_ELAPSED(time_stamp, timeout, hdl_timer_get(timer)))
            return HDL_MODULE_INIT_FAILED;
        else
            return HDL_MODULE_INIT_ONGOING;
    }
    flag1 = 0;
    flag2 = 0;
    return HDL_MODULE_INIT_OK;
}

hdl_module_state_t hdl_adc(void *desc, uint8_t enable){
    static uint32_t time_stamp = 0; /* TODO: Use private var */
    static gd_adc_state_machine_e state_machine = GD_ADC_STATE_MACHINE_DISABLE;
    hdl_adc_t *hdl_adc = (hdl_adc_t *)desc;

    if(hdl_adc->module.reg == NULL || 
    hdl_adc->module.dependencies == NULL || 
    hdl_adc->module.dependencies[0] == NULL ||
        hdl_adc->module.dependencies[1] == NULL || 
        hdl_adc->module.dependencies[2] == NULL || 
            hdl_adc->sources == NULL || 
            hdl_adc->sources[0] == NULL)

        return HDL_MODULE_INIT_FAILED;
    /* We can find timer in our dependencies */
    hdl_timer_t *hdl_timer = (hdl_timer_t *)hdl_adc->module.dependencies[1];
    /* TODO: SEE ADC_REGULAR_INSERTED_CHANNEL */
    if(enable) {
        switch (state_machine){
            case GD_ADC_STATE_MACHINE_DISABLE:
                rcu_periph_clock_enable(RCU_ADC);
                rcu_adc_clock_config(RCU_ADCCK_IRC28M);
                /* pointer points to the first element in the array */
                hdl_adc_source_t **adc_channel_source = hdl_adc->sources;
                int16_t channel_element_number = 0;
                while (*adc_channel_source != NULL) {
                    /* Config routine sequence */
                    adc_regular_channel_config((uint8_t)channel_element_number, (uint8_t)(*adc_channel_source)->channel_number, (uint32_t)(*adc_channel_source)->channel_sample_time);
                    adc_channel_source++;
                    channel_element_number++;
                }
                /* routine sequence lenght */
                

                adc_channel_length_config(ADC_REGULAR_CHANNEL, channel_element_number);
                //adc_discontinuous_mode_config(ADC_REGULAR_CHANNEL, channel_element_number);
                adc_data_alignment_config(hdl_adc->data_alignment);
                adc_resolution_config((uint32_t)hdl_adc->resolution);
                adc_external_trigger_source_config(ADC_REGULAR_CHANNEL, hdl_adc->start_triger );

                if (hdl_adc->mode == ADC_OPERATION_MODE_SINGLE_SCAN)
                    adc_special_function_config(ADC_SCAN_MODE, ENABLE);
                else //(hdl_adc->mode == ADC_OPERATION_MODE_CONTINUOS_SCAN)
                    adc_special_function_config(ADC_CONTINUOUS_MODE, ENABLE);

                /* TODO: Issue with INSERTED channel */     

                adc_enable();
                state_machine = GD_ADC_STATE_MACHINE_CALIBRATE;
            case GD_ADC_STATE_MACHINE_CALIBRATE:
                if(time_stamp == 0)
                    time_stamp = hdl_timer_get(hdl_timer); 
                hdl_module_state_t res = _gd_adc_calibrate(hdl_timer, time_stamp, hdl_adc->init_timeout);
                if(res == HDL_MODULE_INIT_OK){
                    adc_dma_mode_enable();
                    state_machine = GD_ADC_STATE_MACHINE_OK;
                    return HDL_MODULE_INIT_OK;
                }
                else if(res == HDL_MODULE_INIT_ONGOING){
                    return HDL_MODULE_INIT_ONGOING;
                }
                else{
                    state_machine = GD_ADC_STATE_MACHINE_FAULT;
                    return HDL_MODULE_INIT_FAILED;
                }
            case GD_ADC_STATE_MACHINE_FAULT:
                return HDL_MODULE_INIT_FAILED;
            case GD_ADC_STATE_MACHINE_OK:
                return HDL_MODULE_INIT_OK;
        }
    }
    else{
        time_stamp = 0;
        internal_dma_is_init = 0;
        state_machine = GD_ADC_STATE_MACHINE_DISABLE;
        adc_disable();
        adc_dma_mode_disable();
        rcu_periph_clock_disable(RCU_ADC);
        return HDL_MODULE_DEINIT_OK;
    }
}

uint8_t hdl_adc_start(hdl_adc_t *hdl_adc, void *buff){
    hdl_dma_t* hdl_dma = (hdl_dma_t *)hdl_adc->module.dependencies[2];
    if(!internal_dma_is_init) {
        internal_dma_is_init = 1;
        /* pointer points to the first element in the array */
        hdl_adc_source_t **adc_channel_source = hdl_adc->sources;
        int16_t channel_element_number = 0;
        while (*adc_channel_source != NULL){
            adc_channel_source++;
            channel_element_number++;
        }
        /* Config DMA struct */
        hdl_dma_config_t config;
        config.memory_addr = (uint32_t)buff;
        config.memory_width = HDL_DMA_SIZE_OF_MEMORY_16_BIT;
        config.memory_inc = HDL_DMA_INCREMENT_ON;
        config.periph_addr = (uint32_t)(&ADC_RDATA);
        config.periph_inc = HDL_DMA_INCREMENT_OFF;
        config.periph_width = HDL_DMA_SIZE_OF_MEMORY_16_BIT;
        config.amount = channel_element_number;
        config.direction = HDL_DMA_DIRECTION_P2M;
        /* TODO: What is mode the best? */
        /* TODO: get argument from desc */
        config.dma_mode = HDL_DMA_MODE_CIRCULAR;
        config.priority = 0;
        hdl_dma_config(hdl_dma, &config, hdl_adc->dma_channel);
        /* TODO: if channel == 1 */
        /* TODO: if channel !=0 || !=1  return FALSE*/
        dma_channel_enable(hdl_adc->dma_channel);
    }

    if(hdl_adc->start_triger != HDL_ADC_TRIGER_SOFTWARE){
        
        adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);
    }
    else
        adc_software_trigger_enable(ADC_REGULAR_CHANNEL);

    return 0;
    
    return 1;
}
