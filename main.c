//
//  Project:         1801VP1-065 emulator using REmulator hardware
//  File:            main.c
//  Author:          Sonic_Amiga
//
//  Compiled Using:  STM32CubeIDE Version: 1.4.0
//                   Build: 7511_20200720_0928 (UTC)
//
//  Processor:       STM32F205
//  Board:           RE-mulator 1801PE2/PP1
//_____________________________________________________________________________
//
#include "c205.h"
#include "debug.h"
#include "ioport.h"

//_____________________________________________________________________________
//
struct AppState app_config;
unsigned int    app_copy[1024];

#define IO_OUTPUT               bPIO_MODE_OUTPUT
#define IO_INPUT                bPIO_MODE_INPUT
#define IO_AFUNC                bPIO_MODE_AFUNC
#define IO_INPUT_PULL           (bPIO_MODE_INPUT | 0x10)
#define IO_INPUT_ANALOG         bPIO_MODE_ANALOG

#define IO_PORT_SETUP_ENTRY(port)                           \
{                                                           \
    Type        = 0;                                        \
    Data        = port << 16;                               \
    Mode        = (bPIO_MODE_INPUT << 0)                    \
                | (bPIO_MODE_INPUT << 2)                    \
                | (bPIO_MODE_INPUT << 4)                    \
                | (bPIO_MODE_INPUT << 6)                    \
                | (bPIO_MODE_INPUT << 8)                    \
                | (bPIO_MODE_INPUT << 10)                   \
                | (bPIO_MODE_INPUT << 12)                   \
                | (bPIO_MODE_INPUT << 14)                   \
                | (bPIO_MODE_INPUT << 16)                   \
                | (bPIO_MODE_INPUT << 18)                   \
                | (bPIO_MODE_INPUT << 20)                   \
                | (bPIO_MODE_INPUT << 22)                   \
                | (bPIO_MODE_INPUT << 24)                   \
                | (bPIO_MODE_INPUT << 26)                   \
                | (bPIO_MODE_INPUT << 28)                   \
                | (bPIO_MODE_INPUT << 30);                  \
                                                            \
    Attr        = (bPIO_OUTPUT_2MHZ << 0)                   \
                | (bPIO_OUTPUT_2MHZ << 2)                   \
                | (bPIO_OUTPUT_2MHZ << 4)                   \
                | (bPIO_OUTPUT_2MHZ << 6)                   \
                | (bPIO_OUTPUT_2MHZ << 8)                   \
                | (bPIO_OUTPUT_2MHZ << 10)                  \
                | (bPIO_OUTPUT_2MHZ << 12)                  \
                | (bPIO_OUTPUT_2MHZ << 14)                  \
                | (bPIO_OUTPUT_2MHZ << 16)                  \
                | (bPIO_OUTPUT_2MHZ << 18)                  \
                | (bPIO_OUTPUT_2MHZ << 20)                  \
                | (bPIO_OUTPUT_2MHZ << 22)                  \
                | (bPIO_OUTPUT_2MHZ << 24)                  \
                | (bPIO_OUTPUT_2MHZ << 26)                  \
                | (bPIO_OUTPUT_2MHZ << 28)                  \
                | (bPIO_OUTPUT_2MHZ << 30);                 \
                                                            \
    Pull        = (bPIO_PULL_DOWN << 0)                     \
                | (bPIO_PULL_DOWN << 2)                     \
                | (bPIO_PULL_DOWN << 4)                     \
                | (bPIO_PULL_DOWN << 6)                     \
                | (bPIO_PULL_DOWN << 8)                     \
                | (bPIO_PULL_DOWN << 10)                    \
                | (bPIO_PULL_DOWN << 12)                    \
                | (bPIO_PULL_DOWN << 14)                    \
                | (bPIO_PULL_DOWN << 16)                    \
                | (bPIO_PULL_DOWN << 18)                    \
                | (bPIO_PULL_DOWN << 20)                    \
                | (bPIO_PULL_DOWN << 22)                    \
                | (bPIO_PULL_DOWN << 24)                    \
                | (bPIO_PULL_DOWN << 26)                    \
                | (bPIO_PULL_DOWN << 28)                    \
                | (bPIO_PULL_DOWN << 30);                   \
}

#define IO_PORT_SETUP_LEAVE(port)                           \
{                                                           \
    DBG_ASSERT(port == (Data >> 16), "Invalid port setup"); \
    IO_PORT(port)->sPIO_TYPE  = (Type & 0xFFFF);            \
    IO_PORT(port)->sPIO_ODR   = (Data & 0xFFFF);            \
    IO_PORT(port)->sPIO_MODE  = Mode;                       \
    IO_PORT(port)->sPIO_PUPD  = Pull;                       \
    IO_PORT(port)->sPIO_SPEED = Attr;                       \
}

#define IO_PORT_SETUP(name, mode, data, type)               \
{                                                           \
    unsigned int shift;                                     \
                                                            \
    Type |= type << IO_MASK_N(name);                        \
    shift = IO_MASK_N(name)*2;                              \
    Pull &= ~(bPIO_PULL_MASK<<shift);                       \
    Pull |= (bPIO_PULL_NONE<<shift);                        \
                                                            \
    switch(mode)                                            \
    {                                                       \
        case IO_OUTPUT:                                     \
        {                                                   \
            Mode |= bPIO_MODE_OUTPUT<<shift;                \
            switch(name & IO_ATTR_MSK)                      \
            {                                               \
                case IO_ATTR_2MHZ:                          \
                {                                           \
                    Attr |= bPIO_OUTPUT_2MHZ<<shift;        \
                    break;                                  \
                }                                           \
                case IO_ATTR_25MHZ:                         \
                {                                           \
                    Attr |= bPIO_OUTPUT_25MHZ<<shift;       \
                    break;                                  \
                }                                           \
                case IO_ATTR_50MHZ:                         \
                {                                           \
                    Attr |= bPIO_OUTPUT_50MHZ<<shift;       \
                    break;                                  \
                }                                           \
                case IO_ATTR_100MHZ:                        \
                {                                           \
                    Attr |= bPIO_OUTPUT_100MHZ<<shift;      \
                    break;                                  \
                }                                           \
            }                                               \
            Data |= (data & 0x01) << (shift/2);             \
            break;                                          \
        }                                                   \
        case IO_INPUT:                                      \
        {                                                   \
            Mode |= bPIO_MODE_INPUT<<shift;                 \
            break;                                          \
        }                                                   \
        case IO_INPUT_PULL:                                 \
        {                                                   \
            Pull &= ~(bPIO_PULL_MASK<<shift);               \
            Mode |= bPIO_MODE_INPUT<<shift;                 \
            if (data & 0x01)                                \
            {                                               \
                Pull |= bPIO_PULL_UP<<shift;                \
            }                                               \
            else                                            \
            {                                               \
                Pull |= bPIO_PULL_DOWN<<shift;              \
            }                                               \
            break;                                          \
        }                                                   \
        case IO_INPUT_ANALOG:                               \
        {                                                   \
            Mode |= bPIO_MODE_ANALOG<<shift;                \
            break;                                          \
        }                                                   \
        case bPIO_MODE_AFUNC:                               \
        {                                                   \
            Mode |= bPIO_MODE_AFUNC<<shift;                 \
            break;                                          \
        }                                                   \
        default:                                            \
        {                                                   \
            DBG_ASSERT(FALSE, "Invalid mode setting");      \
            break;                                          \
        }                                                   \
    }                                                       \
    DBG_ASSERT((Data >> 16) == (IO_PORT_N(name)<<8),        \
                                    "Invalid port setup");  \
    DBG_ASSERT(IO_MASK_N(name) < 16, "Invalid pin mask");   \
}

//_____________________________________________________________________________
//
// Функции для инициализации периферийных регистров
//
static void app_init_port(void)
{
    unsigned int Mode, Data, Pull, Attr, Type;

    //
    // Разрешаем работу Compemsation Cell чтобы
    // корректно отрабатывало ограничение
    // скорости работы входов портов
    //
    *RCC_APB2EN |= bRCC_SYSCFGEN;
    *RCC_APB2LP |= bRCC_SYSCFGEN;
    __DSB();
    __ISB();
    *SYSCFG_CMPCR = bSYSCFG_CMP_PD;
    while((*SYSCFG_CMPCR & bSYSCFG_READY) == 0);
    //
    // Настройка порта A
    //
    IO_PORT_SETUP_ENTRY(IO_PORT_A);
    {
        IO_PORT_SETUP(_CFG0,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_CFG1,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_CFG2,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_CFG3,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_CFG4,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_CFG5,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_TXD,     IO_OUTPUT,      1, 0);
        IO_PORT_SETUP(_RXD,     IO_INPUT,       0, 0);
        IO_PORT_SETUP(_READY,   IO_OUTPUT,      1, 1);
        // We can't have JTAG, but we can have SWD, keep it
        IO_PORT_SETUP(_TMS,     IO_AFUNC,       0, 0);
        IO_PORT_SETUP(_TCK,     IO_AFUNC,       0, 0);
        IO_PORT_SETUP(_TDI,     IO_INPUT_PULL,  1, 0);
    }
    IO_PORT_SETUP_LEAVE(IO_PORT_A);
    //
    // Настройка порта B
    //
    IO_PORT_SETUP_ENTRY(IO_PORT_B);
    {
        IO_PORT_SETUP(_AD0,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD1,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD2,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD3,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD4,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD5,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD6,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD7,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD8,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD9,     IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD10,    IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD11,    IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD12,    IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD13,    IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD14,    IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_AD15,    IO_OUTPUT,      1, 1);
    }
    IO_PORT_SETUP_LEAVE(IO_PORT_B);
    //
    // Настройка порта C
    //
    IO_PORT_SETUP_ENTRY(IO_PORT_C);
    {
        IO_PORT_SETUP(_RPLY,    IO_OUTPUT,      1, 1);
        IO_PORT_SETUP(_SYNC,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_DIN,     IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_SEL0,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_SEL1,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_SEL2,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_SEL3,    IO_INPUT_PULL,  1, 0);
        IO_PORT_SETUP(_Q32_IN,  IO_INPUT_PULL,  0, 0);
        IO_PORT_SETUP(_Q32_OUT, IO_INPUT_PULL,  0, 0);
        IO_PORT_SETUP(_USEL0,   IO_OUTPUT,      0, 1);
        IO_PORT_SETUP(_USEL1,   IO_OUTPUT,      0, 1);
    }
    IO_PORT_SETUP_LEAVE(IO_PORT_C);
    //
    // Перестраховочная настройка неиспользуемых портов
    //
    IO_PORT_SETUP_ENTRY(IO_PORT_D);
    IO_PORT_SETUP_LEAVE(IO_PORT_D);
    IO_PORT_SETUP_ENTRY(IO_PORT_E);
    IO_PORT_SETUP_LEAVE(IO_PORT_E);
    IO_PORT_SETUP_ENTRY(IO_PORT_F);
    IO_PORT_SETUP_LEAVE(IO_PORT_F);
    IO_PORT_SETUP_ENTRY(IO_PORT_G);
    IO_PORT_SETUP_LEAVE(IO_PORT_G);

    IO_PORT_SETUP_ENTRY(IO_PORT_H);
    {
        IO_PORT_SETUP(_OSC_IN,  IO_INPUT_PULL,  0, 0);
        IO_PORT_SETUP(_OSC_OUT, IO_INPUT_PULL,  0, 0);
    }
    IO_PORT_SETUP_LEAVE(IO_PORT_H);

    *PWR_CR = bPWR_PVD_27
            | bPWR_PVDE
            | bPWR_CSBF
            | bPWR_CWUF;
}

//_____________________________________________________________________________
//
// Приблизительная задержка на указанное число микросекунд
// Требует корректной настройки системного таймера
//
static void io_delay_us(unsigned int us)
{
    unsigned int stop, cur, prev, div;
    //
    // Системный таймер настроен на частоту 1 кГц, значит 1 мкс
    // соответствует примерно 1/1000 от ограничителя счетчика
    // Также учитываем что счетчик декрементируется - инвертируем
    //
    div  = *SC_STLOAD + 1;
    prev = div - *SC_STVAL;
    stop = prev + (us * div)/1000;

    while ((cur = div - *SC_STVAL) < stop)
    {
        if (cur < prev)         //
        {                       // проверим не произошел
            if (stop < div)     // ли перенос таймера
            {                   //
                break;          //
            }
            stop -= div;
        }
        prev = cur;
    }
}

static void io_delay_ms(unsigned int ms)
{
    if (ms != 0)
    {
        do
        {
            io_delay_us(1000);
            ms--;
        }
        while(ms);
    }
}

//_____________________________________________________________________________
//

static const unsigned int baud_table[13] =
{
	50, 75, 100, 150, 200, 300, 600, 1200, 2400, 4800, 9600, 19200, 56700
};

static void app_read_config(void)
{
	unsigned int cfg = IO_PORT(_CFG0)->sPIO_IDR;
	unsigned short base = (cfg & IO_MASK(_CFG4)) ? 0176560 : 0177560;
	unsigned int baud_idx = (cfg >> 4) & 0x0F; // _CFG3..._CFG0
	unsigned int baud = (baud_idx < 13) ? baud_table[baud_idx] : 115200;
    // We can't use _CFG5 here because it activates reflashing mode in the bootloader

	app_config.base = base;
	app_config.rxs  = 0;
	app_config.txs  = 0;
	app_config.txd  = 0;

    uart_init(baud);
    dbg_printf("\r\nPorts initialized, address 0%o, serial %d 8n2\r\n",
    		   base, baud);
}

//_____________________________________________________________________________
//
static void app_start_proc(void)
{
    unsigned char *start, *end;
    APP_LOOP_PTR addr;

    //
    // Copy our main program to RAM in order to get maximum performance
    //
    start = (unsigned char*)(((unsigned int)&app_rom_loop) & ~1);
    end   = (unsigned char*)(((unsigned int)&app_end_loop) & ~1);
    bsp_memcpy(app_copy, start, end - start + 2*sizeof(unsigned int));
    //
    // Deassert external reset signal
    //
    IO_PORT_ODC(_READY);
    //
    // Run our emulation loop
    //
#if RUN_IN_ROM
    // Run the loop in flash memory - for debugging.
    addr = &app_rom_loop;
#else
    //
    addr = (APP_LOOP_PTR)(((unsigned int)app_copy) | 1);
#endif
    addr(UART_BASE, IO_PORT(_AD), IO_PORT(_SYNC), &app_config);
}

//_____________________________________________________________________________
//
// Модуль выполнения основной задачи - фактически функция main()
//
void app_start(void)
{
    //
    // Assert reset signal for our PDP-11
    //
    IO_SET_MODE(_READY);
    IO_PORT_ODS(_READY);

    app_init_port();
    //
    // Подождем пока выполниться надежный сброс и
    // установяться стабильные уровни на входах
    // выбора конфигурации
    //
    io_delay_ms(2);
//
//  {
//      unsigned int tmp;
//
//      tmp = *RCC_CFG;
//      tmp &= ~bRCC_MCO_MASK;
//      tmp &= ~bRCC_MCO_PRE_MASK;
//      tmp |= bRCC_MCO_PLL;
//      tmp |= bRCC_MCO_PRE4;
//      *RCC_CFG = tmp;
//      IO_SET_MODE(_MCO);
//  }
//
    //
    // Read and apply configuration from jumpers
    //
    app_read_config();
    //
    // Run the main loop
    //
    app_start_proc();
    //
    // Нет возврата
    //
    DBG_ASSERT(FALSE, "Invalid return from main loop");
    for(;;);
}
