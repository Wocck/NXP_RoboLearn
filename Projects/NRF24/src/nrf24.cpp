#include "nrf24.h"

// Rejestry nRF24L01
#define CONFIG_REG 0x00
#define STATUS_REG 0x07
#define RX_ADDR_P0 0x0A
#define SETUP_AW 0x03
#define RF_CH 0x05
#define RF_SETUP 0x06
#define EN_RXADDR 0x02
#define RX_PW_P0 0x11
#define R_RX_PAYLOAD 0x61
#define FLUSH_RX 0xE2
#define EN_AA 0x01
#define TX_ADDR 0x10

// Maski i wartości
#define CONFIG_DEFAULT 0x0F
#define RF_SETUP_DEFAULT 0x26
#define EN_RXADDR_DEFAULT 0x01
#define ADDR_WIDTH_DEFAULT 0x03
#define RF_CH_DEFAULT 76
#define PAYLOAD_SIZE_DEFAULT sizeof(DataPacket)
#define EN_AA_DEFAULT 0x00
#define STATUS_CLEAR 0x70
#define RX_DR_FLAG 0x40
#define PIPE_ADDR {0xCC, 0xCE, 0xCC, 0xCE, 0xCC}

// Konstruktor klasy NRF24
NRF24::NRF24(const struct device* spi) : gpio_dev_1(nullptr), spi_dev(nullptr) {
    // Wywołanie set_device w konstruktorze
    if (spi && set_device(spi) == 0) {
        printk("SPI device set successfully\n");
    } else {
        printk("Failed to set SPI device in constructor\n");
    }

    // Inicjalizacja konfiguracji SPI
    spi_cfg.frequency = 100000;
    spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE | SPI_HOLD_ON_CS;
    spi_cfg.slave = 0U;
    spi_cfg.cs.gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(lpspi1), cs_gpios);
    spi_cfg.cs.delay = 0U;
}

void NRF24::irq_handler(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
    NRF24* obj = CONTAINER_OF(cb, NRF24, irq_callback);
    k_work_submit(&obj->irq_work);
}

void NRF24::process_irq_work(struct k_work* work) {
    NRF24* obj = CONTAINER_OF(work, NRF24, irq_work);
    obj->handle_irq();
}

void NRF24::handle_irq() {
    while (true) {
        uint8_t fifo_status;
        if (read_register(0x17, &fifo_status, 1) != 0) { // FIFO_STATUS register
            printk("Failed to read FIFO_STATUS register\n");
            break;
        }

        if (fifo_status & 0x01) { // RX_EMPTY bit
            break; // FIFO RX is empty, exit
        }

        struct DataPacket packet;
        if (receive_payload(&packet) == 0) {
            current_packet = packet;
        } else {
            printk("Failed to receive payload\n");
        }

        // Clear RX_DR flag
        uint8_t clear_flags = 0x40;
        write_register(0x07, &clear_flags, 1);
    }
}

int NRF24::set_device(const struct device* spi) {
    if (!spi) {
        printk("SPI device is not provided\n");
        return -1;
    }
    spi_dev = spi;
    gpio_dev_1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));

    if (!gpio_dev_1) {
        printk("GPIO_1 device not ready\n");
        return -1;
    }

    // Konfiguracja pinu CE
    int ret = gpio_pin_configure(gpio_dev_1, CE_GPIO_PIN, GPIO_OUTPUT_LOW);
    if (ret < 0) {
        printk("Failed to configure CE pin\n");
        return ret;
    }

    k_sleep(K_MSEC(10));
    return 0;
}

int NRF24::write_register(uint8_t reg, const uint8_t* data, size_t len) {
    uint8_t cmd = 0x20 | (reg & 0x1F);
    tx_buf[0] = cmd;
    memcpy(&tx_buf[1], data, len);

    struct spi_buf spi_tx = { .buf = tx_buf, .len = len + 1 };
    struct spi_buf spi_rx = { .buf = rx_buf, .len = len + 1 };

    struct spi_buf_set tx = { .buffers = &spi_tx, .count = 1 };
    struct spi_buf_set rx = { .buffers = &spi_rx, .count = 1 };

    return spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
}

// Funkcja do odczytu z rejestru
int NRF24::read_register(uint8_t reg, uint8_t* data, size_t len) {
    uint8_t cmd = reg & 0x1F;
    tx_buf[0] = cmd;
    memset(&tx_buf[1], 0xFF, len);

    struct spi_buf spi_tx = { .buf = tx_buf, .len = len + 1 };
    struct spi_buf spi_rx = { .buf = rx_buf, .len = len + 1 };

    struct spi_buf_set tx = { .buffers = &spi_tx, .count = 1 };
    struct spi_buf_set rx = { .buffers = &spi_rx, .count = 1 };

    int ret = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
    if (ret == 0) {
        memcpy(data, &rx_buf[1], len);
    }
    return ret;
}

void NRF24::reset(uint8_t reg) {
    gpio_pin_set(gpio_dev_1, CE_GPIO_PIN, 0);
    k_sleep(K_MSEC(2));

    if (reg == STATUS_REG) {
        uint8_t status_reset = 0x00;
        write_register(STATUS_REG, &status_reset, 1);
    } else {
        // Reset podstawowych rejestrów nRF24L01
        uint8_t config_reset = 0x08;
        write_register(CONFIG_REG, &config_reset, 1);

        uint8_t en_aa_reset = 0x3F;
        write_register(EN_AA, &en_aa_reset, 1);

        uint8_t en_rxaddr_reset = 0x03;
        write_register(EN_RXADDR, &en_rxaddr_reset, 1);

        uint8_t addr_width_reset = 0x03;
        write_register(SETUP_AW, &addr_width_reset, 1);

        uint8_t rf_ch_reset = 0x02;
        write_register(RF_CH, &rf_ch_reset, 1);

        uint8_t rf_setup_reset = 0x0E;
        write_register(RF_SETUP, &rf_setup_reset, 1);

        // Reset adresów RX i TX
        uint8_t rx_addr_p0_reset[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
        write_register(RX_ADDR_P0, rx_addr_p0_reset, 5);

        uint8_t tx_addr_reset[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
        write_register(TX_ADDR, tx_addr_reset, 5);

        // Reset rozmiaru ładunków dla wszystkich kanałów
        uint8_t payload_size_reset = 0x00;
        write_register(RX_PW_P0, &payload_size_reset, 1);

        // Wyczyść bufor RX
        uint8_t flush_command = FLUSH_RX;
        write_register(flush_command, nullptr, 0);

        // Wyczyść wszystkie flagi STATUS
        uint8_t status_clear = STATUS_CLEAR;
        write_register(STATUS_REG, &status_clear, 1);
    }
}

int NRF24::init() {
    gpio_pin_set(gpio_dev_1, CE_GPIO_PIN, 0);

    // Initialize nRF24L01 as Receiver
    uint8_t rf_addr_width = ADDR_WIDTH_DEFAULT;
    write_register(SETUP_AW, &rf_addr_width, 1);

    uint8_t en_rxaddr = EN_RXADDR_DEFAULT;
    write_register(EN_RXADDR, &en_rxaddr, 1);

    uint8_t rx_addr[5] = PIPE_ADDR;
    write_register(RX_ADDR_P0, rx_addr, 5);

    uint8_t payload_size = PAYLOAD_SIZE_DEFAULT;
    write_register(RX_PW_P0, &payload_size, 1);

    uint8_t rf_ch = RF_CH_DEFAULT;
    write_register(RF_CH, &rf_ch, 1);

    uint8_t rf_setup = RF_SETUP_DEFAULT;
    write_register(RF_SETUP, &rf_setup, 1);

    uint8_t en_aa = EN_AA_DEFAULT;
    write_register(EN_AA, &en_aa, 1);

    uint8_t config = CONFIG_DEFAULT;
    write_register(CONFIG_REG, &config, 1);

    uint8_t status_clear = STATUS_CLEAR;
    write_register(STATUS_REG, &status_clear, 1);

    gpio_pin_set(gpio_dev_1, CE_GPIO_PIN, 1);
    k_sleep(K_MSEC(2));
    return 0;
}

int NRF24::receive_payload(DataPacket* packet) {
    uint8_t status;
    if (read_register(STATUS_REG, &status, 1) != 0) {
        return -1;
    }

    if (status & 0x40) {
        uint8_t cmd = R_RX_PAYLOAD;
        size_t payload_size = PAYLOAD_SIZE_DEFAULT;

        // Prepare TX buffer with command and dummy bytes
        tx_buf[0] = cmd;
        memset(&tx_buf[1], 0xFF, payload_size);

        struct spi_buf spi_tx = { .buf = tx_buf, .len = payload_size + 1 };
        struct spi_buf spi_rx = { .buf = rx_buf, .len = payload_size + 1 };

        struct spi_buf_set tx = { .buffers = &spi_tx, .count = 1 };
        struct spi_buf_set rx = { .buffers = &spi_rx, .count = 1 };

        int ret = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
        if (ret == 0) {
            memcpy(packet, &rx_buf[1], payload_size);

            // Clear RX_DR flag
            uint8_t clear = 0x40;
            write_register(STATUS_REG, &clear, 1);
        }
        return ret;
    }
    return -1;
}

int NRF24::configure_irq() {
    gpio_dev_irq = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    if (!gpio_dev_irq) {
        printk("IRQ GPIO device not ready\n");
        return -1;
    }

    // Configure IRQ pin as input with pull-up
    int ret = gpio_pin_configure(gpio_dev_irq, IRQ_GPIO_PIN, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Failed to configure IRQ pin\n");
        return ret;
    }

    // Configure IRQ pin interrupt
    ret = gpio_pin_interrupt_configure(gpio_dev_irq, IRQ_GPIO_PIN, GPIO_INT_EDGE_TO_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure IRQ pin interrupt\n");
        return ret;
    }

    // Initialize work queue
    k_work_init(&irq_work, process_irq_work);

    // Initialize GPIO callback
    gpio_init_callback(&irq_callback, irq_handler, BIT(IRQ_GPIO_PIN));
    gpio_add_callback(gpio_dev_irq, &irq_callback);

    printk("IRQ pin interrupt configured\n");
    return 0;
}

void NRF24::test_registers() {
    uint8_t value;
    printk("Testing nRF24L01 Registers...\n");

    // Read CONFIG register
    if (read_register(CONFIG_REG, &value, 1) == 0) {
        printk("CONFIG register (0x00): 0x%02X\n", value);
    } else {
        printk("Failed to read CONFIG register\n");
    }

    // Read EN_RXADDR register
    if (read_register(EN_RXADDR, &value, 1) == 0) {
        printk("EN_RXADDR register (0x02): 0x%02X\n", value);
    } else {
        printk("Failed to read EN_RXADDR register\n");
    }

    // Read RF_CH register
    if (read_register(RF_CH, &value, 1) == 0) {
        printk("RF_CH register (0x05): 0x%02X\n", value);
    } else {
        printk("Failed to read RF_CH register\n");
    }

    // Read RF_SETUP register
    if (read_register(RF_SETUP, &value, 1) == 0) {
        printk("RF_SETUP register (0x06): 0x%02X\n", value);
    } else {
        printk("Failed to read RF_SETUP register\n");
    }

    // Read STATUS register
    if (read_register(STATUS_REG, &value, 1) == 0) {
        printk("STATUS register (0x07): 0x%02X\n", value);
    } else {
        printk("Failed to read STATUS register\n");
    }

    // Read RX_ADDR_P0 register
    uint8_t rx_addr[5];
    if (read_register(RX_ADDR_P0, rx_addr, 5) == 0) {
        printk("RX_ADDR_P0 register (0x0A): ");
        for (int i = 0; i < 5; i++) {
            printk("0x%02X ", rx_addr[i]);
        }
        printk("\n");
    } else {
        printk("Failed to read RX_ADDR_P0 register\n");
    }

    // Read RX_PW_P0 register
    if (read_register(RX_PW_P0, &value, 1) == 0) {
        printk("RX_PW_P0 register (0x11): 0x%02X\n", value);
    } else {
        printk("Failed to read RX_PW_P0 register\n");
    }

    if (read_register(SETUP_AW, &value, 1) == 0) {
        printk("Addr Width register (0x03): 0x%02X\n", value);
    } else {
        printk("Failed to read RX_PW_P0 register\n");
    }

    printk("Register test complete.\n");
}

DataPacket NRF24::get_current_packet() const {
    return current_packet;
}


