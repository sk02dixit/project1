#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>

#define RESET_REGISTER                  0x10
#define RESET_REGISTER_FIFO_CLR         0x18
#define CONVERSION_REGISTER             0xE0
#define SETUP_REGISTER                  0x68
#define AVERAGING_REGISTER              0x3C
#define VREF_VOLTAGE                    2.50

int gpio_init_out(int n);
int spi_fd;
int delay;
int speed;

int spi_write_byte(uint8_t data)
{
	uint8_t buf[10];
	int sz;

	buf[0] = data;
	sz = write(spi_fd, buf, 1);
	if (sz > 0) {
        	printf("write successfull %d\n", sz);
	} else {
        	printf("write failed %d\n", sz);
	}
}

int spi_write_data(uint8_t *data, int len)
{
	int sz;
	sz = write(spi_fd, data, len);
	if (sz > 0) {
        	printf("write successfull %d\n", sz);
	} else {
        	printf("write failed %d\n", sz);
	}
}

static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.tx_nbits = 8,
		.rx_nbits = 8,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = 8,
	};
/*
	if (mode & SPI_TX_OCTAL)
		tr.tx_nbits = 8;
	else if (mode & SPI_TX_QUAD)
		tr.tx_nbits = 4;
	else if (mode & SPI_TX_DUAL)
		tr.tx_nbits = 2;
	if (mode & SPI_RX_OCTAL)
		tr.rx_nbits = 8;
	else if (mode & SPI_RX_QUAD)
		tr.rx_nbits = 4;
	else if (mode & SPI_RX_DUAL)
		tr.rx_nbits = 2;
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_OCTAL | SPI_TX_QUAD | SPI_TX_DUAL))
			tr.rx_buf = 0;
		else if (mode & (SPI_RX_OCTAL | SPI_RX_QUAD | SPI_RX_DUAL))
			tr.tx_buf = 0;
	}
*/
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		perror("can't send spi message");
	}
}

double bitstovoltageconverter(uint8_t in0, uint8_t in1)
{
        uint32_t data_channel;
        double value_voltage;
        data_channel = (in0 << 8) | in1 ;
//      data_channel = (*in1 << 8) | *in0 ;
        value_voltage = data_channel *(((float)VREF_VOLTAGE)/(float)(4096.0)) ;
        return value_voltage;
}

double read_adc_channel(unsigned int adc_channel)
{
        uint8_t conversion_register_buffer = (1<<7)|(adc_channel << 3)| (1<<2);
        spi_write_byte(conversion_register_buffer);
#if 0
        f.flag_adc_eoc_check = 1;
        while(ADC_EOC_N==1)
        {
        if(f.flag_adc_eoc_check == 0)
        {
                printf(COLOR_MAGENTA"\r\nEOC not low before 5sec"COLOR_RESET);
                error_number = SPI_Peripheral_ADC ;
                        break;
        }
        }
        f.flag_adc_eoc_check = 0;
                #if DEBUG2
                for(readspi=0;readspi<8;readspi++)
        {
                printf("\r\nsingle_channel_data_received[%d]: %x" , readspi,single_channel_data_received[readspi]);
        }
        #endif
                for(readspi=0;readspi<9;readspi++)
        {
                single_channel_data_received[readspi]=spi2_read_write(0x00);
        }
//      spi2_write(RESET_REGISTER_FIFO_CLR);

        #if DEBUG2
        printf("\r\n");
                for(readspi=0;readspi<9;readspi++)
        {
                printf("\r\nsingle_channel_data_received[%d]: %x" , readspi,single_channel_data_received[readspi]);
        }
        #endif
        single_channel_value = bitstovoltageconverter(&single_channel_data_received[7],&single_channel_data_received[8]) ;
        return single_channel_value ;
#endif
	return 0;
}


void external_adc_init()
{
	uint8_t data[10];
        int8_t reset_register_buffer = RESET_REGISTER ;
        int8_t reset_fifo_clr        = RESET_REGISTER_FIFO_CLR ;
        int8_t averaging_register_buffer = AVERAGING_REGISTER;
        int8_t setup_register_buffer = SETUP_REGISTER ;

	data[0] = RESET_REGISTER;
	data[1] = RESET_REGISTER_FIFO_CLR;
	data[2] = AVERAGING_REGISTER;
	data[3] = SETUP_REGISTER;

	//spi_write_data(data, 4);
	transfer(spi_fd, data, NULL, 4);
#if 0
	uint8_t tmp[10];
        spi_write_byte(reset_register_buffer);
	scanf("%s", tmp);

        spi_write_byte(reset_fifo_clr);
	scanf("%s", tmp);

        spi_write_byte(averaging_register_buffer);
	scanf("%s", tmp);

        spi_write_byte(setup_register_buffer);
	scanf("%s", tmp);
#endif
}

int main(void)
{
	int ret, i;
	uint8_t buf[10];
	speed = 1000000;
	delay = 100;
	spi_fd = open("/dev/spidev0.0", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (spi_fd < 0) {
        	perror("r1");
	}
	ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		perror("can't set max speed hz");
	}
	ret = ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		perror("can't get max speed hz");
	}
	//external_adc_init();
	//sleep(2);
	read_adc_channel(7);
	sleep(1);
	//ret = read(spi_fd, buf, 2);
	buf[0] = buf[1] = 0;
	memset(buf, 0 , 8);
	transfer(spi_fd, NULL, buf, 8);
	for(i=0; i<0; ++i) {
       		printf("%02x ", buf[i]);
	}
	float volt = bitstovoltageconverter(buf[0], buf[1]);
	printf("\nVolt=%f\n", volt);
	close(spi_fd);
}
