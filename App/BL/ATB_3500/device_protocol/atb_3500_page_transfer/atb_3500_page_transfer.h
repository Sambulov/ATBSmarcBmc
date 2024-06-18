/*!
    \file atb_3500_page_transfer.h
    \author a.bodin
    
    \version 2024-06-05, V1.0.0, This file describe major protocol
 */

#ifndef ATB_3500_PAGE_TRANSFER_H_
#define ATB_3500_PAGE_TRANSFER_H_

#if defined(ATB_3500)


#define ATB_3500_SECTOR_DATA_SIZE                   ((uint32_t)32)
#define ATB_3500_PAGE_TRANSFER_READ_SECTOR_SIZE     ((uint32_t)150)
#define ATB_3500_PAGE_TRANSFER_WRITE_SECTOR_SIZE    ((uint32_t)150)


#define ATB_3500_PAGE_TRANSFER_TOTAL_SECTOR_SIZE (ATB_3500_PAGE_TRANSFER_READ_SECTOR_SIZE +         \
                                                        ATB_3500_PAGE_TRANSFER_WRITE_SECTOR_SIZE)

#pragma pack(1)

typedef enum {
    ATB_3500_SECTOR_TRANSFER_SM_INITIAL = (uint8_t)0,
    ATB_3500_SECTOR_TRANSFER_SM_PROCCEDING = (uint8_t)1,
    ATB_3500_SECTOR_TRNASFER_SM_COMPLETED = (uint8_t)2,
    ATB_3500_SECTOR_TRNASFER_SM_FAILED = (uint8_t)3,
} atb_3500_sector_transfer_state_machine_e;

/* This function have to be implement by user */
typedef uint8_t (*sector_transfer_client_request)(void *arg, const uint8_t *data, int16_t lenght);
typedef uint8_t (*sector_transfer_server_answer)(void *arg, uint8_t *data, int16_t lenght);

typedef struct {
    /* Public */
    sector_transfer_client_request client_request;
    sector_transfer_server_answer server_answer;
    /* Private */
    uint8_t state_machine;                  /* atb_3500_sector_transfer_state_machine_e */
    uint8_t current_destination_port;
    uint8_t current_sequence_number;
    uint8_t acknoledgment_number;

} atb3500_sector_transfer_handler_t;

typedef struct {
    uint8_t read_sector[ATB_3500_PAGE_TRANSFER_READ_SECTOR_SIZE];
    uint8_t write_sector[ATB_3500_PAGE_TRANSFER_WRITE_SECTOR_SIZE];
} atb_3500_page_transfer_t;

typedef struct {
    uint8_t destination_port;        /* The port number of the receiving application */
    uint8_t sequence_number;         /* Used to identify transaction number */
    uint8_t acknoledgment_number;    /* Used to acknowledge data transferring (server have to send this number + 1)*/
    uint8_t
    flag_fault            : 1,       /* Used to notify issue with transferring */
    dummy                 : 7;       /* 0 - default value */
    uint8_t data[ATB_3500_SECTOR_DATA_SIZE];                /* Data sector */
} atb_3500_segment_struct_t;

typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t buff[30];
} atb_3500_sector_spi_buff_t;

typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t buff[30];
} atb_3500_sector_i2c_buff_t;

typedef struct {
    uint8_t cmd;
    uint8_t buff[31];
} atb_3500_sector_usart_buff_t;

typedef struct {
    atb_3500_segment_struct_t spi_sector;      /* 36 bytes */
    atb_3500_segment_struct_t i2c_secor;       /* 36 bytes */
    atb_3500_segment_struct_t usart_sector;    /* 36 bytes */
    /* Available for writing */
    /* X1 connector */
    uint8_t do_lte_modem_reset;                /* 1 byte */
    uint8_t do_pci_switch;                     /* 1 byte */
    uint8_t do_external_output_1;              /* 1 byte */
    uint8_t do_external_output_2;              /* 1 byte */ /* 4 bytes */
    /* LEDs */
    uint8_t do_led_0_0;                        /* 1 byte */
    uint8_t do_led_0_1;                        /* 1 byte */
    uint8_t do_led_0_2;                        /* 1 byte */
    uint8_t do_led_1_0;                        /* 1 byte */ /* 4 bytes */
    uint8_t do_led_1_1;                        /* 1 byte */  
    uint8_t do_led_1_2;                        /* 1 byte */
    uint8_t do_led_2_0;                        /* 1 byte */
    uint8_t do_led_2_1;                        /* 1 byte */ /* 4 bytes */
    uint8_t do_led_2_2;                        /* 1 byte */
    /* Watch dog */
    uint8_t resert_watch_dog;                  /* 1 byte */
    uint8_t dummy[26];
    uint16_t crc16;             
} atb_3500_page_transfer_read_sector_t;

typedef struct {
    atb_3500_segment_struct_t spi_sector;      /* 36 bytes */
    atb_3500_segment_struct_t i2c_secor;       /* 36 bytes */
    atb_3500_segment_struct_t usart_sector;    /* 36 bytes */
    uint8_t di_module_addres_1;                /* 1 byte */
    uint8_t di_module_addres_2;                /* 1 byte */
    uint8_t di_module_addres_3;                /* 1 byte */
    uint8_t di_module_addres_4;                /* 1 byte */   /* 4 bytes */
    uint8_t di_module_addres_5;                /* 1 byte */
    uint8_t di_external_periphery_irq_1;       /* 1 byte */
    uint8_t di_external_periphery_irq_2;       /* 1 byte */
    /* Feedback, host is adjusting this pins */
    /* X1 connector */
    uint8_t do_lte_modem_reset;                /* 1 byte */ /* 4 bytes */
    uint8_t do_pci_switch;                     /* 1 byte */
    uint8_t do_external_output_1;              /* 1 byte */
    uint8_t do_external_output_2;              /* 1 byte */
    /* LEDs */
    uint8_t do_led_0_0;                        /* 1 byte */ /* 4 bytes */
    uint8_t do_led_0_1;                        /* 1 byte */
    uint8_t do_led_0_2;                        /* 1 byte */
    uint8_t do_led_1_0;                        /* 1 byte */
    uint8_t do_led_1_1;                        /* 1 byte */ /* 4 bytes */
    uint8_t do_led_1_2;                        /* 1 byte */
    uint8_t do_led_2_0;                        /* 1 byte */
    uint8_t do_led_2_1;                        /* 1 byte */
    uint8_t do_led_2_2;                        /* 1 byte */ /* 4 bytes */
    uint8_t dummy[18];
    uint16_t cnt_alive;
    uint16_t crc16;  
} atb_3500_page_transfer_write_sector_t;
#pragma pack(4)

_Static_assert(sizeof(atb_3500_page_transfer_write_sector_t) == ATB_3500_PAGE_TRANSFER_WRITE_SECTOR_SIZE, "Sector size must me equal ATB_3500_PAGE_TRANSFER_WRITE_SECTOR_SIZE and align 4");
_Static_assert(sizeof(atb_3500_page_transfer_read_sector_t) == ATB_3500_PAGE_TRANSFER_READ_SECTOR_SIZE, "Sector size must me equal ATB_3500_PAGE_TRANSFER_READ_SECTOR_SIZE and align 4");


#endif /* ATB_3500 */
#endif /* ATB_3500_PAGE_TRANSFER_H_ */