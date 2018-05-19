ADC_Random：为随机数产生API原型，ADC初始化配置沿用cube mx，在main跟msp里面；
DRV_Spi：为SPI驱动灯组API原型，SPI初始化配置沿用cube mx，在main跟msp里面；




灯效移植MCU选型要求：
1、ROM：128M RAM 16K；
2、硬件SPI；
3、需要有DMA；
4、ADC随机数产生；
5、需要支持C标准库；
6、主频最少在40MHZ以上。
