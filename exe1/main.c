#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "mpu6050.h"

const int I2C_CHIP_ADDRESS = 0x68;
const int I2C_SDA_GPIO = 20;
const int I2C_SCL_GPIO = 21;

void i2c_task(void *p) {
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(I2C_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_GPIO);
    gpio_pull_up(I2C_SCL_GPIO);

    // 1) Reinicia o dispositivo (colocando 1 no bit 7 do registrador 0x6B)
    uint8_t buf[2];
    buf[0] = MPUREG_PWR_MGMT_1;
    buf[1] = 1 << 7; // bit7 = DEVICE_RESET
    i2c_write_blocking(i2c_default, I2C_CHIP_ADDRESS, buf, 2, false);

    // 2) Configurar acelerômetro para ±4g
    //    Passos: ler ACCEL_CONFIG, limpar bits [4:3], setar para '01' (4g), escrever de volta

    // Ler o registrador ACCEL_CONFIG (0x1C)
    uint8_t reg = MPUREG_ACCEL_CONFIG;
    i2c_write_blocking(i2c_default, I2C_CHIP_ADDRESS, &reg, 1, true);   // envia qual registrador quer ler
    i2c_read_blocking(i2c_default, I2C_CHIP_ADDRESS, &reg, 1, false);  // lê o valor atual

    // Limpa bits [4:3] (AFS_SEL) e coloca '01' (que representa ±4g)
    // Bits [4:3] = 0x18 em hexadecimal = (1<<4) | (1<<3)
    // Para ±4g: bits [4:3] = 0b01 = (1<<3)
    reg &= ~0x18;   // zera bits 4 e 3
    reg |= (1 << 3); // coloca bit3 em 1 (±4g)

    // Agora escrevemos de volta
    buf[0] = MPUREG_ACCEL_CONFIG;
    buf[1] = reg;
    i2c_write_blocking(i2c_default, I2C_CHIP_ADDRESS, buf, 2, false);

    while (1) {
        // Sua lógica de leitura do sensor pode ficar aqui
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    xTaskCreate(i2c_task, "i2c task", 4095, NULL, 1, NULL);
    vTaskStartScheduler();

    while (true) {
    }
}
