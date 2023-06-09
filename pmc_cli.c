#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "arpa/inet.h"
#include <signal.h>
#include <stdbool.h>

int sd;
char *fmt = "{\"method\":\"%s\"}";
char *fmss = "{\"method\":\"%s\", \"params\": [\"%s\",\"%s\"]}";
char *fmts = "{\"method\":\"%s\", \"params\": [%d]}";
char *fmtsf = "{\"method\":\"%s\", \"params\": [%f]}";
char *fmtss = "{\"method\":\"%s\", \"params\": [%d,%d]}";
char *fmtsss = "{\"method\":\"%s\", \"params\": [%d,%d,%d]}";
char *fmtf = "{\"method\":\"%s\", \"params\": [\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d]}";

int mqtt_init();
int mqtt_subscribe(char *topic);
int mqtt_unsubscribe(char *topic);

int show_sys_params()
{
	char in[10];
	mqtt_subscribe("sys/tmp");
	do {
		scanf("%c", in);
		if('q' == in[0]) {
			mqtt_unsubscribe("sys/tmp");
		} else {
			printf("Enter q to stop monitoring!\n");
		}
	} while ('q' != in[0]);
	return 0;
}

typedef enum
{
	ACTION_SUBMENU,
	ACTION_EXEC,
	ACTION_BACK,
	ACTION_HOME
} action_t;
typedef struct menu_item_s
{
	char cmd[4];
	char *label;
	action_t type;
	void *action;
	void *arg;
} menu_t;

menu_t main_menu[];
menu_t debug_menu[];

// head on
menu_t monitor_menu[] =
{
	{"1", " system parameters", ACTION_EXEC, show_sys_params, NULL},
	{"2", " solar parameters", ACTION_EXEC, NULL, NULL},
	{"3", " Battery parameters", ACTION_EXEC, NULL, NULL},
	{"4", " dg parameters", ACTION_EXEC, NULL, NULL},
	{"5", " fcs parameters", ACTION_EXEC, NULL, NULL},
	{"6", " help", ACTION_EXEC, NULL, NULL},
	{"00", "BACK", ACTION_BACK, main_menu, NULL},
	{"", "", 0, NULL, NULL}};
// head off
menu_t start_menu[] =
{
	{"1", " Auto", ACTION_EXEC, NULL, NULL},
	{"2", " Manual", ACTION_EXEC, NULL, NULL},
	{"3", " Help", ACTION_EXEC, NULL, NULL},
	{"4", " Save", ACTION_EXEC, NULL, NULL},
	{"00", "BACK", ACTION_BACK, start_menu, NULL},
	{"99", "Exit", ACTION_HOME, main_menu, NULL},
	{"", "", 0, NULL, NULL}};
menu_t priority_menu[] =
{
	{"1", " Solar-Grid-Battery", ACTION_EXEC, NULL, NULL},
	{"2", " Solar-Battery-Grid", ACTION_EXEC, NULL, NULL},
	{"3", " Grid-Solar-Battery", ACTION_EXEC, NULL, NULL},
	{"4", " Grid-Battery-Solar", ACTION_EXEC, NULL, NULL},
	{"5", " Battery-Solar-Grid", ACTION_EXEC, NULL, NULL},
	{"6", " Battery-Grid-Solar", ACTION_EXEC, NULL, NULL},
	{"7", " Help", ACTION_EXEC, NULL, NULL},
	{"8", " Save", ACTION_EXEC, NULL, NULL},
	{"00", "BACK", ACTION_BACK, priority_menu, NULL},
	{"99", "Exit", ACTION_HOME, main_menu, NULL},
	{"", "", 0, NULL, NULL}};

menu_t battery_menu[] =
{

	{"1", " battery parameter", ACTION_EXEC, NULL, NULL},
	{"2", " Help", ACTION_EXEC, NULL, NULL},
	{"3", " Save", ACTION_EXEC, NULL, NULL},
	{"00", "BACK", ACTION_BACK, battery_menu, NULL},
	{"99", "Exit", ACTION_HOME, main_menu, NULL},
	{"", "", 0, NULL, NULL}};
menu_t dg_menu[] =
{

	{"1", " aa", ACTION_EXEC, NULL, NULL},
	{"2", " bb", ACTION_EXEC, NULL, NULL},
	{"3", " Help", ACTION_EXEC, NULL, NULL},
	{"4", " Save", ACTION_EXEC, NULL, NULL},
	{"00", "BACK", ACTION_BACK, dg_menu, NULL},
	{"99", "Exit", ACTION_HOME, main_menu, NULL},
	{"", "", 0, NULL, NULL}};
menu_t solar_menu[] =
{

	{"1", " PV under voltage", ACTION_EXEC, NULL, NULL},
	{"2", " Battery DOD", ACTION_EXEC, NULL, NULL},
	{"3", " help", ACTION_EXEC, NULL, NULL},
	{"4", " Save", ACTION_EXEC, NULL, NULL},
	{"00", "BACK", ACTION_BACK, solar_menu, NULL},
	{"99", "Exit", ACTION_HOME, main_menu, NULL},
	{"", "", 0, NULL, NULL}};
menu_t fc_menu[] =
{

	{"1", " H2 Storage tank Volume", ACTION_EXEC, NULL, NULL},
	{"2", " No of H2 Tanks", ACTION_EXEC, NULL, NULL},
	{"3", " h2 Tank Filling Pressure", ACTION_EXEC, NULL, NULL},
	{"4", " FCS Low Voltage Start Enabled", ACTION_EXEC, NULL, NULL},
	{"5", " FCS Low Voltage Start disabled", ACTION_EXEC, NULL, NULL},
	{"6", " FCS Manage Mode Enabled", ACTION_EXEC, NULL, NULL},
	{"7", " FCS Manage Mode disabled", ACTION_EXEC, NULL, NULL},
	{"8", " FCS Float Mode Enabled", ACTION_EXEC, NULL, NULL},
	{"9", " FCS Float Mode disabled", ACTION_EXEC, NULL, NULL},
	{"10", "Help", ACTION_EXEC, NULL, NULL},
	{"11", "save", ACTION_EXEC, NULL, NULL},
	{"00", "BACK", ACTION_BACK, fc_menu, NULL},
	{"99", "Exit", ACTION_HOME, main_menu, NULL},
	{"", "", 0, NULL, NULL}};

menu_t configure_menu[] =
{
	{"1", " Start-Up Mode", ACTION_SUBMENU, start_menu, NULL},
	{"2", " Priority Modes", ACTION_SUBMENU, priority_menu, NULL},
	{"3", " Solar System", ACTION_SUBMENU, solar_menu, NULL},
	{"4", " Battery System", ACTION_SUBMENU, battery_menu, NULL},
	{"5", " Diesel Generator", ACTION_SUBMENU, dg_menu, NULL},
	{"6", " Fule Cell System", ACTION_SUBMENU, fc_menu, NULL},
	{"7", " Date & Time", ACTION_EXEC, NULL, NULL},
	{"8", " Location", ACTION_EXEC, NULL, NULL},
	{"9", " Help", ACTION_EXEC, NULL, NULL},
	{"00", "BACK", ACTION_BACK, configure_menu, NULL},
	{"99", "Exit", ACTION_HOME, main_menu, NULL},
	{"", "", 0, NULL, NULL}};

int gpio_set() 
{
	int ch, ch1, ch2;
	char buf[1024];
	char *match;

	printf("Please enter the gpio group \n");
	scanf("%d",&ch);
	printf("Please enter the gpio number \n");
	scanf("%d",&ch1);
	printf("Please enter the gpio value to be set \n");
	scanf("%d",&ch2);
	sprintf(buf, fmtsss, "gpio_set",ch,ch1,ch2);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int gpio_get() 
{
	int ch, ch1, ch2;
	char buf[1024];
	char *match;

	printf("Please enter the gpio group \n");
	scanf("%d",&ch);
	printf("Please enter the gpio number \n");
	scanf("%d",&ch1);
	sprintf(buf, fmtss, "gpio_get",ch,ch1);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("value is :  %s:\n",buf);
	}
	return 0;
}

int compressor_set_freq() 
{
	char buf[1024];
	char *match;
	float ch;

	printf("Please enter the frequency\n");
	scanf("%f",&ch);
	sprintf(buf, fmtsf, "modbus_set_freq",ch);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	printf("Frequency %.2f\n",ch);
	match = strstr(buf,"failed");
	if(match){
		//printf("\e[1;1H\e[2J");
	} else {
		printf("Frequency %.2f\n",ch);
		printf("Successfully Updated..\n");
	}
	return 0;
}
int air_circulator_set_speed()
{
	char buf[1024];
	char *match;
	char ch[100];
	long value;

	printf("Please enter the speed in hex {01 to 0A}\n");
	scanf("%s",&ch);
	value = strtol(ch, NULL, 16);
	sprintf(buf, fmts, "air_set",value);
	printf("setting spped %s\n",ch);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Speed set successfully %s:\n",buf);
	}
	return 0;
} 

int air_circulator_off()
{
	char buf[1024];
	char *match;

	sprintf(buf, fmts, "air_off");
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Success %s:\n",buf);
	}
	return 0;
} 

int air_circulator_on() 
{
	char buf[1024];
	char *match;

	sprintf(buf, fmts, "air_on");
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Success %s:\n",buf);
	} 
	return 0;
} 

int compressor_on() 
{
	char buf[1024];
	char *match;

	sprintf(buf, fmts, "modbus_on");
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Voltage %s:\n",buf);
	}
	return 0;
}

int compressor_off() 
{
	char buf[1024];
	char *match;

	sprintf(buf, fmts, "modbus_off");
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Voltage %s:\n",buf);
	}
	return 0;
}

int adc_read_all() 
{
	char buf[1024];
	char *match;

	sprintf(buf, fmts, "adc_read_all");
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Voltage %s:\n",buf);
	}
	return 0;
}

int adc_read() 
{
	int ch;
	int sub_ch = 0;
	char buf[1024];
	char *match;

	printf("Please enter the channel number\n");
	scanf("%d",&ch);
	if (ch == 9) {
		printf("Please enter the sub channel number[1 to 8]\n");
		scanf("%d",&sub_ch);
		if (sub_ch <=0 && sub_ch >= 9) {
			printf("wrong sub channel\n");
			return -1;
		}
	}
	if (ch == 10) {
		printf("Please enter the sub channel number[1 to 8]\n");
		scanf("%d",&sub_ch);
		if (sub_ch <=0 && sub_ch >= 9) {
			printf("wrong sub channel\n");
			return -1;
		}
	}
	sprintf(buf, fmtss, "adc_read",ch,sub_ch);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Voltage %s:\n",buf);
	}
	return 0;
}

int air_flow() 
{
	int ch;
	char buf[1024];
	char *match;

	sprintf(buf, fmts, "air_flow",7);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("%s:\n",buf);
	}
	return 0;
}

int can_monitor() 
{
	int ch;
	char buf[1024];
	char *match;

	sprintf(buf, fmts, "can_monitor",7);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("%s:\n",buf);
	}
	return 0;
}

int i2c_get() 
{
	char ch[100],ch1[100];
	char buf[1024];
	char *match;
	long addr,reg;

	printf("Please enter the address\n");
	scanf("%s",&ch);
	printf("Please enter the register\n");
	scanf("%s",&ch1);
	addr = strtol(ch, NULL, 16);
	reg = strtol(ch1, NULL, 16);
	sprintf(buf, fmtss, "i2c_get",addr,reg);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("%s\n",buf);
	} else {
		printf("Success %s:\n",buf);
	}
	return 0;
}

int i2c_detect() 
{
	char buf[1024];
	char *match;
	int ch,ch1;

	printf("Please enter the interface id [2, 3]\n");
	scanf("%d",&ch);
	sprintf(buf,"i2cdetect -y -r %d",ch);
	system(buf);
	printf("Press q to quit\n");
	scanf("%d",&ch1);
	printf("\e[1;1H\e[2J");
	return 0;
}

int i2c_set() 
{
	char ch[100], ch1[100], ch2[100];
	char buf[1024];
	char bufs[1024];
	char *match;
	long addr, reg, value;

	printf("Please enter the address\n");
	scanf("%s",&ch);
	printf("Please enter the register\n");
	scanf("%s",&ch1);
	printf("Please enter the value\n");
	scanf("%s",&ch2);
	addr = strtol(ch, NULL, 16);
	reg = strtol(ch1, NULL, 16);
	value = strtol(ch2, NULL, 16);
	sprintf(buf, fmtsss, "i2c_set",addr,reg,value);
	write(sd, buf, strlen(buf));
	read(sd, bufs, 1024);
	match = strstr(bufs,"failed");
	if(match){
		printf("%s\n",bufs);
	} else {
		printf("Success %s:\n",bufs);
	}
	return 0;
}

int drain_valve() 
{
	int value;
	char buf[1024];
	char *match;

	printf("\n1. ON\n0. OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",2,3,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int purge_valve()
{
	int value;
	char buf[1024];
	char *match;

	printf("\n1. ON\n0.OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",2,2,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int inlet_valve()
{
	int value;
	char buf[1024];
	char *match;

	printf("\n1. ON\n0.OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",2,1,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int thermostatic_valve()
{
	int value;
	char buf[1024];
	char *match;

	printf("\n1. ON\n0.OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",2,0,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int load_contactor()
{
	int value;
	char buf[1024];
	char *match;

	printf("\n1. ON\n0.OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",3,23,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int stack_contactor()
{
	int value;
	char buf[1024];
	char *match;

	printf("\n1. ON\n0.OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",3,24,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int bleed_relay()
{
	int value;
	char buf[1024];
	char *match;

	printf("\n1. ON\n0.OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",4,28,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int system_ok_led()
{
	char ch2[100];
	char buf[1024];
	char bufs[1024];
	char *match;
	char *addr, *reg, *value;
	char *b_addr, *b_reg, *b_value;
	char *c_addr, *c_reg, *c_value;
	int val,bus;

	printf("\n1. ON\n0. OFF\n");
	scanf("%d",&val);
	bus = 1;
	b_addr= "0x71";
	b_reg ="0x00";
	b_value = "0x01";
	c_addr = "0x21";
	c_reg = "0x03";
	c_value = "0x00";
	addr = "0x21";
	reg = "0x01";
	if (val == 1) {
		value = "0x01";
	} else {
		value = "0x00";
	}
	printf("\nBUF2:%s\n",b_reg);
	sprintf(buf, fmtf, "i2c_set",addr,reg,value,b_addr,b_reg,b_value,c_addr, c_reg, c_value,bus);
	printf("\nBUF3 %s\n",buf);
	write(sd, buf, strlen(buf));
	read(sd, bufs, 1024);
	match = strstr(bufs,"failed");
	if(match){
		printf("%s\n",bufs);
	} else {
		printf("Success %s:\n",bufs);
	}
	return 0;
}

int minor_alarm_led()
{
	char ch2[100];
	char buf[1024];
	char bufs[1024];
	char *match;
	char *addr, *reg, *value;
	char *b_addr, *b_reg, *b_value;
	char *c_addr, *c_reg, *c_value;
	int val,bus;

	printf("\n1. ON\n0. OFF\n");
	scanf("%d",&val);
	bus = 1;
	b_addr= "0x71";
	b_reg ="0x00";
	b_value = "0x01";
	c_addr = "0x21";
	c_reg = "0x03";
	c_value = "0x00";
	addr = "0x21";
	reg = "0x01";
	if (val == 1) {
		value = "0x02";
	} else {
		value = "0x00";
	}
	printf("\nBUF2:%s\n",b_reg);
	sprintf(buf, fmtf, "i2c_set",addr,reg,value,b_addr,b_reg,b_value,c_addr, c_reg, c_value,bus);
	printf("\nBUF3 %s\n",buf);
	write(sd, buf, strlen(buf));
	read(sd, bufs, 1024);
	match = strstr(bufs,"failed");
	if(match){
		printf("%s\n",bufs);
	} else {
		printf("Success %s:\n",bufs);
	}
	return 0;
}

int major_alarm_led()
{
	char ch2[100];
	char buf[1024];
	char bufs[1024];
	char *match;
	char *addr, *reg, *value;
	char *b_addr, *b_reg, *b_value;
	char *c_addr, *c_reg, *c_value;
	int val,bus;

	printf("\n1. ON\n0. OFF\n");
	scanf("%d",&val);
	bus = 1;
	b_addr= "0x71";
	b_reg ="0x00";
	b_value = "0x01";
	c_addr = "0x21";
	c_reg = "0x03";
	c_value = "0x00";
	addr = "0x21";
	reg = "0x01";
	if (val == 1) {
		value = "0x04";
	} else {
		value = "0x00";
	}
	printf("\nBUF2:%s\n",b_reg);
	sprintf(buf, fmtf, "i2c_set",addr,reg,value,b_addr,b_reg,b_value,c_addr, c_reg, c_value,bus);
	printf("\nBUF3 %s\n",buf);
	write(sd, buf, strlen(buf));
	read(sd, bufs, 1024);
	match = strstr(bufs,"failed");
	if(match){
		printf("%s\n",bufs);
	} else {
		printf("Success %s:\n",bufs);
	}
	return 0;
}

int buzzer_enable()
{
	char ch2[100];
	char buf[1024];
	char bufs[1024];
	char *match;
	char *addr, *reg, *value;
	char *b_addr, *b_reg, *b_value;
	char *c_addr, *c_reg, *c_value;
	int val,bus;

	printf("\n1. ON\n0. OFF\n");
	scanf("%d",&val);
	bus = 1;
	b_addr= "0x71";
	b_reg ="0x00";
	b_value = "0x01";
	c_addr = "0x21";
	c_reg = "0x03";
	c_value = "0x00";
	addr = "0x21";
	reg = "0x01";
	if (val == 1) {
		value = "0x08";
	} else {
		value = "0x00";
	}
	printf("\nBUF2:%s\n",b_reg);
	sprintf(buf, fmtf, "i2c_set",addr,reg,value,b_addr,b_reg,b_value,c_addr, c_reg, c_value,bus);
	printf("\nBUF3 %s\n",buf);
	write(sd, buf, strlen(buf));
	read(sd, bufs, 1024);
	match = strstr(bufs,"failed");
	if(match){
		printf("%s\n",bufs);
	} else {
		printf("Success %s:\n",bufs);
	}
	return 0;
}

int sensor_power()
{
	int value;
	char buf[1024];
	char *match;

	printf("\n1. ON\n0.OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",1,0,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int rs485_enable()
{
	int value;
	char buf[1024];
	char *match;

	printf("\n1. ON\n0.OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",2,5,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}

int level_translator()
{
	int value;
	char buf[1024];
	char *match;

	printf("\n0. ON\n1.OFF\n");
	scanf("%d",&value);
	sprintf(buf, fmtsss, "gpio_set",4,27,value);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"failed");
	if(match){
		printf("\e[1;1H\e[2J");
		printf("%s\n",buf);
	} else {
		printf("\e[1;1H\e[2J");
		printf("Set successfull %s:\n",buf);
	}
	return 0;
}


menu_t i2c_set_menu[] =
{
	{"1", "MONITOR", ACTION_SUBMENU, monitor_menu, NULL},
	{"2", "CONFIGURE", ACTION_SUBMENU, configure_menu, NULL},
	{"3", "DEBUG", ACTION_SUBMENU, debug_menu, NULL},
	{"4", "HELP", ACTION_SUBMENU, NULL, NULL},
	{"00", "PREVIOUS MENU", ACTION_BACK, NULL, NULL},
	{"", "", 0, NULL, NULL}};

menu_t i2c_get_menu[] =
{
	{"1", "MONITOR", ACTION_SUBMENU, monitor_menu, NULL},
	{"2", "CONFIGURE", ACTION_SUBMENU, configure_menu, NULL},
	{"3", "DEBUG", ACTION_SUBMENU, debug_menu, NULL},
	{"4", "HELP", ACTION_SUBMENU, NULL, NULL},
	{"00", "PREVIOUS MENU", ACTION_BACK, NULL, NULL},
	{"", "", 0, NULL, NULL}};

menu_t i2c_detect_menu[] =
{
	{"1", "MONITOR", ACTION_SUBMENU, monitor_menu, NULL},
	{"2", "CONFIGURE", ACTION_SUBMENU, configure_menu, NULL},
	{"3", "DEBUG", ACTION_SUBMENU, debug_menu, NULL},
	{"4", "HELP", ACTION_SUBMENU, NULL, NULL},
	{"00", "PREVIOUS MENU", ACTION_BACK, NULL, NULL},
	{"", "", 0, NULL, NULL}};

menu_t debug_menu[] =
{
	{"1", " adc_read", ACTION_EXEC, adc_read, NULL},
	{"2", " adc_read_all", ACTION_EXEC, adc_read_all, NULL},
	{"3", " air_compressor_on", ACTION_EXEC, compressor_on, NULL},
	{"4", " air_compressor_off", ACTION_EXEC, compressor_off, NULL},
	{"5", " air_compressor_set_freq", ACTION_EXEC, compressor_set_freq, NULL},
	{"6", "air_flow", ACTION_EXEC, air_flow, NULL},
	{"7", "bleed_relay", ACTION_EXEC, bleed_relay, NULL},
	{"8", "buzzer_enable", ACTION_EXEC, buzzer_enable, NULL},
	{"9", "can_monitor", ACTION_EXEC, can_monitor, NULL},
	{"10", "drain_valve", ACTION_EXEC, drain_valve, NULL},
	{"11", " gpioset", ACTION_EXEC, gpio_set, NULL},
	{"12", " gpioget", ACTION_EXEC, gpio_get, NULL},
	{"13", " i2c_detect", ACTION_EXEC, i2c_detect, NULL},
	{"14", " i2c_get", ACTION_EXEC, i2c_get, NULL},
	{"15", " i2c_set", ACTION_EXEC, i2c_set, NULL},
	{"16", "inlet_valve", ACTION_EXEC, inlet_valve, NULL},
	{"17", "level_translator", ACTION_EXEC, level_translator, NULL},
	{"18", "load_contactor", ACTION_EXEC, load_contactor, NULL},
	{"19", "major_alarm_led", ACTION_EXEC, major_alarm_led, NULL},
	{"20", "minor_alarm_led", ACTION_EXEC, minor_alarm_led, NULL},
	{"21", "purge_valve", ACTION_EXEC, purge_valve, NULL},
	{"22", "re_circulation_pump_on", ACTION_EXEC, air_circulator_on, NULL},
	{"23", "re_circulation_pump_off", ACTION_EXEC, air_circulator_off, NULL},
	{"24", "re_circulation_pump_set_speed", ACTION_EXEC, air_circulator_set_speed, NULL},
	{"25", "rs485_enable", ACTION_EXEC, rs485_enable, NULL},
	{"26", "sensor_power", ACTION_EXEC, sensor_power, NULL},
	{"27", "stack_contactor", ACTION_EXEC, stack_contactor, NULL},
	{"28", "system_ok_led", ACTION_EXEC, system_ok_led, NULL},
	{"29", "thermostatic_valve", ACTION_EXEC, thermostatic_valve, NULL},
	{"30", "Help", ACTION_EXEC, NULL, NULL},
	{"31", "Save", ACTION_EXEC, NULL, NULL},
	{"00", "Back", ACTION_BACK, debug_menu, NULL},
	{"99", "Exit", ACTION_HOME, main_menu, NULL},
	{"", "", 0, NULL, NULL}};

menu_t main_menu[] =
{
	{"1", " MONITOR", ACTION_SUBMENU, monitor_menu, NULL},
	{"2", " CONFIGURE", ACTION_SUBMENU, configure_menu, NULL},
	{"3", " DEBUG", ACTION_SUBMENU, debug_menu, NULL},
	{"4", " HELP", ACTION_SUBMENU, NULL, NULL},
	{"00", "EXIT", ACTION_BACK, NULL, NULL},
	{"", "", 0, NULL, NULL}};

#if 0
void show_menu(menu_t *m)
{
	char ch[15];
	int i;
	void (*cb)(char *);
	do
	{
		i = 0; // MENU BASE;
		do
		{
			printf("%s. %s\n", m[i].cmd, m[i].label);
			++i;
		} while (m[i].cmd[0]);
		scanf("%s", ch);
		i = 0;
		do
		{
			if (!strcmp(m[i].cmd, ch))
			{
				switch (m[i].type)
				{
					case ACTION_SUBMENU:
						printf("\e[1;1H\e[2J");
						show_menu((menu_t *)m[i].action);
						break;
					case ACTION_EXEC:
						printf("\e[1;1H\e[2J");
						cb = (void (*)(char *))m[i].action;
						(*cb)((char *)m[i].action);
						break;
					case ACTION_HOME:
						printf("\e[1;1H\e[2J");
						printf("********** WELCOME TO PMC CLI **********\n");
						show_menu((menu_t *)m[i].action);
						break;
					case ACTION_BACK:
						printf("\e[1;1H\e[2J");
						return;
				}
			}
			++i;
		} while (m[i].cmd[0]);
	} while (1);
}
#endif
void show_menu(menu_t *m)
{
  int i;
  void (*cb)(char *);
  menu_t *cm;
  int match;
  char cmd[3];
  int cmd_idx;
	int is_enter;
	char c;

  do  {
    i = 0;
    cmd_idx = 0;
    match = -1;
		is_enter = 0;
    printf("********** WELCOME TO FCS CLI **********\r\n");
    do {
      printf("   %s.  %s   \r\n", m[i].cmd, m[i].label);
      ++i;
    } while (m[i].cmd[0]);
    i = 0;
    c = getch();
		printf("\nc: %d\n",c);
		if('\n' == c) {
			printf("\nis_enter\n");
			is_enter = 1;
		} else {
    	cmd[cmd_idx++] = getch();
		}
    do{
      if (!strncmp(m[i].cmd, cmd, cmd_idx)) {
        if(match >= 0) {
          cmd[cmd_idx++] = getch();
          i = 0;
          match = -1;
          continue;
        } else {
          match = i;
					if(is_enter) {
						printf("\nis_entear: %d\n",match);
						break;
					}
        }
      }
      ++i;
    }while (m[i].cmd[0]);

    if(match >= 0) {
      cm = &m[match];
      switch (cm->type){
        case ACTION_SUBMENU:
          show_menu((menu_t *)cm->action);
          break;
        case ACTION_EXEC:
          cb = (void (*)(char *))cm->action;
          (*cb)((char *)cm->action);
          break;
        case ACTION_HOME:
          show_menu((menu_t *)cm->action);
          break;
        case ACTION_BACK:

          return;
        }
    } else {
              printf("NOT FOR CHOICE MENU %d \r\n", c);
    }
  } while (1);
  refresh();
  endwin();
}

void *mqtt_th(void *vargp)
{
	mqtt_init();
}

int cli_password_check(char *username, char *password) 
{
	char buf[1024];
	char *match;

	sprintf(buf, fmss, "password_check",username,password);
	write(sd, buf, strlen(buf));
	read(sd, buf, 1024);
	match = strstr(buf,"Success");
	if(match){
		return 2;
	} else {
		return 1;
	}

	return 1;
}

int user_login()
{
	int ret = 1;
	char username[15]; 
	char password[12]; 

	printf("********** WELCOME TO PMC CLI **********\r\n");\
	printf("Username  :"); 
	scanf("%s",&username); 

	printf("Password  :"); 
	scanf("%s",&password); 

	ret = cli_password_check(username, password);

	if(ret == 2){ 
		printf("\e[1;1H\e[2J");
		printf("********** WELCOME TO PMC CLI **********\r\n");\
			show_menu(main_menu);
	} else { 
		printf("\e[1;1H\e[2J");
		printf("********** WELCOME TO PMC CLI LOGIN **********\r\n");
		printf("\nWrong username or password\n"); 
		user_login();
	} 

	return 0;
}

int main()
{
	struct sockaddr_in addr;
	int ret;
	pthread_t thread_id;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd < 0){
		printf("Failed to create socket\n");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = 0;
	addr.sin_port = htons(5555);

	ret = connect(sd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0){
		perror("Connect failed: ");
		close(sd);
		sd = -1;
	}

	pthread_create(&thread_id, NULL, mqtt_th, NULL);

	printf("\ninit screen!!!!\n");
  initscr();
	printf("\n1init screen!!!!\n");
  refresh();
	user_login();
	pthread_join(thread_id, NULL);
}
