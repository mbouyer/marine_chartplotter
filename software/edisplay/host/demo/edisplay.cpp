#include <err.h>
#include <iostream>
#include <libusb-1.0/libusb.h>

#define INV0 	0xA6   
#define INV1	0xA7   
#define LSB0	0x00 
#define CMDLCD 	1
#define CMDPIC 	0
#define MSB0	0x10   
#define COL0	0x60
#define LCDEN	0xAf
#define LCDDIS	0xAE
#define DEBUG   0    // for stdout debug 
#define TIMEOUT 1000
using namespace std;

unsigned char zero [64] { 0 }; // used for clearing the lcd 

unsigned char sesi [1440] { // sesi logo bitmap
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x000,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,   
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,   
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};



void clearDsiplay(libusb_device_handle *dev_handle)
{
	unsigned char *VecClear = new unsigned char[4];
	int read;   //used to find out how many bytes were actually written
	VecClear[0]=CMDLCD; // to send a cmd for the lcd not the pic
        VecClear[1]=COL0;  
	VecClear[2]=LSB0 ;
	VecClear[3]=MSB0 ;
	int r = libusb_interrupt_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),VecClear,4, &read, TIMEOUT); 
#if DEBUG
		if(r==0){
		cout<<"writing CMD Successful! the writing lengh "<< read<<endl;
		}
#endif

   printf("zero\n");
   for(int adr=0; adr<160*104/4; adr+= 64) {
	int len = 160*100/4 - adr;
	if (len > 64) len = 64;
         r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),zero,64, &read, TIMEOUT); // writing 0 on all the lcd memory
#if DEBUG
		if(r==0){
		cout<<"writing clear bulk Successful! the writing lengh "<< read<<endl;
		}
#endif
   } 
   printf("zero done\n");
	r = libusb_interrupt_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),VecClear,4, &read, TIMEOUT); // reposition the cursor to the start after cleaning 
#if DEBUG
		if(r==0){
		cout<<"repositionning cursor after clear Successfully! the cmd lengh written "<< read<<endl;
		}
#endif
}

void DrawBitMap(libusb_device_handle *dev_handle)
{
#define BULKL 64
	int read;
   printf("bitmap\n");
	for (int l=0; l < 1440;l += BULKL){
	    int len = 1440 - l;
	    if (len > BULKL)
		len = BULKL;
	    int r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),(sesi+l),len, &read, TIMEOUT); 
#if DEBUG
		if(r==0){
		    cout<<"writing bulk Successful!, the writing lengh "<< read<<endl;
		}
#endif
	}
   printf("bitmap done\n");

}

void HelloBitMap(libusb_device_handle *dev_handle)
{
	int read;
	unsigned char hello [] ={0x7F,0x08,0x08,0x7F,0x00,0x7E,0x4A,0x4A,0x00,0x7E,0x40,0x40,
		                 0x00,0x7E,0x40,0x40,0x00,0x3C,0x42,0x42,0x3C,0x00,0x8f}; // hello! bitMap

	int r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),hello,23, &read, TIMEOUT); 
#if DEBUG
	if(r==0){
		cout<<"writing hello bulk Successful! the writing lengh "<< read<<endl;
	}
#endif

}
int ReadButton(libusb_device_handle *dev_handle,char *recieve)
{
    int read = 0;
    int r = libusb_interrupt_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_IN), (unsigned char *)recieve, 64, &read, TIMEOUT);
#if DEBUG	
	if(r==0){
			    cout<<"reading Successful the reading lengh : "<< read<<endl;
        }
#endif
    return read;
}

int LedCMD(libusb_device_handle *dev_handle,int cmd)
{
    unsigned char *VecCMD = new unsigned char[2];
    int read;
    VecCMD[0]=CMDPIC; // send cmd to the pic not the lcd 
    
    if      (cmd==0) // 0 to turn the Led OFF
        VecCMD[1]=0;
    else if (cmd==1) // 1 to turn the led ON
        VecCMD[1]=1;
    else {
    delete[] VecCMD;
        return 1; // unkown CMD clear exit 
    }
    int r = libusb_interrupt_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),VecCMD,2, &read, TIMEOUT); 
#if DEBUG
    if(r==0){
	    cout<<"writing LED cmd interrupt Successful! the writing lengh "<< read<<endl;
	    }
#endif
    delete[] VecCMD; // clearing memory 
    return 0 ;

}

void SendCMD(libusb_device_handle *dev_handle,unsigned char *command,int N)
{
    int read;
    int r = libusb_interrupt_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),command,N, &read, TIMEOUT); 
#if DEBUG
    if(r==0){
	    cout<<"writing interrupt Successful! the writing lengh "<< read<<endl;
    }
#endif

}
int SetPosition(libusb_device_handle *dev_handle,int page, int msb,int lsb)
{
    unsigned char *VecCMD = new unsigned char[4];
    VecCMD[0]=CMDLCD;
    if(page<32)
        VecCMD[1] = 0X60 + page;
    else 
        return -1;
    if(msb<=8)
        VecCMD[2] = 0X10 + msb;
    else 
        return -1;
    if(lsb<16)
        VecCMD[3] = 0X00 + lsb;
    else 
        return -1;
    int read;
    int r = libusb_interrupt_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),VecCMD,4, &read, TIMEOUT); 
#if DEBUG
    if(r==0){
	    cout<<"writing interrupt Successful! the writing lengh "<< read<<endl;
	    }
#endif
    return 0;

}


int main() {
	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	libusb_device_handle *dev_handle; //a device handle
	libusb_context *ctx = NULL; //a libusb session
	int r; //for return values
	ssize_t cnt; //holding number of devices in list
	r = libusb_init(&ctx); //initialize the library for the session we just declared
	if(r < 0) {
		cout<<"Init Error "<<r<<endl; //there was an error
		return 1;
	}
	libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, 3); //set verbosity level to 3, as suggested in the documentation

	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
	if(cnt < 0) {
		cout<<"Get Device Error"<<endl; //there was an error
		return 1;
	}
	cout<<cnt<<" Devices in list."<<endl;

	dev_handle = libusb_open_device_with_vid_pid(ctx, 0x04d8,0x4541); //these are vendorID and productID we gave our Pic
	if(dev_handle == NULL)
		errx(1, "Cannot open device");
	else
		cout<<"Device Opened"<<endl;
	libusb_free_device_list(devs, 1); //free the list, unref the devices in it

	if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
		cout<<"Kernel Driver Active"<<endl;
		if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
			cout<<"Kernel Driver Detached!"<<endl;
	}
	r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
	if(r < 0) {
		cout<<"Cannot Claim Interface"<<endl;
		return 1;
	}
	cout<<"Claimed Interface"<<endl;
	

unsigned char *command = new unsigned char[64];

clearDsiplay(dev_handle); 
SetPosition(dev_handle,10,2,0);
for(int temp=0;temp<15000;temp++)
__asm("nop");
DrawBitMap(dev_handle); // draw bitmap logo 

//--------------- CMD VECTOR -------------------------------------------
command[0]=CMDLCD;
command[1]=COL0;   //column 0
command[2]=MSB0;   //MSB  0
command[3]=LSB0;   // LSB 0
command[4]=INV0;   // inv0 = non inv   inv1 = inv 
command[5]=LCDEN;  // LCDEN = EN   LCDDIS = EN_OFF
//----------------- ------------  --------- -- - -- ---------------- 

SendCMD(dev_handle,command,6);       // send command 
//clearDsiplay(dev_handle);            // clear Display reposition cursor  
SetPosition(dev_handle,25,4,0);          // MSB between 0 - 8 // lsb between 0-15// page between 0 - 25  // the visual part of the lcd beyond that will not be visible but still there 
HelloBitMap(dev_handle);             // hello bitMap
if(LedCMD(dev_handle,1)==1){         // CMD LED
    cout<<"unknown command"<<endl;;  //error handling 
    return 1;}
int pos=10; // position of the rows
int ind = 2;// position of columns 
bool invEtat = 0;// invert state 
bool dispEtat = 0;// display state 
char *recieve = new char[64];// reading the interrupt value sent from pic
int rot_pos = 0;
while(1){ 
    

	int read = ReadButton(dev_handle,recieve);   // reading buffer 
	if (read == 2){
	     rot_pos += recieve[1];
             printf("received 0x%x %d pos %d\n", recieve[0], recieve[1], rot_pos);
	    if (recieve[1] !=0)  // if rotatory change row
		    {pos=(pos+recieve[1])%32;
		    clearDsiplay(dev_handle);
		    SetPosition(dev_handle,pos,ind,0);
		    DrawBitMap(dev_handle);
	    }
	    if (recieve[0] & 0x4) // if right black button pressed   increase column
		{ind=(ind+1)%9;
		clearDsiplay(dev_handle);
		SetPosition(dev_handle,pos,ind,0);
		DrawBitMap(dev_handle);
	    }
	    if (recieve[0] & 0x2)                     // if left black button pressed     decrease column
		{ind=(ind-1);
		if(ind<0)
		    ind = 8;
		clearDsiplay(dev_handle);
		SetPosition(dev_handle,pos,ind,0);
		DrawBitMap(dev_handle);
	    }
	    if (recieve[0] & 0x8)                  // if yellow button pressed      inv display mode
		{
		if(invEtat==0){
		    command[1]=INV1;
		    invEtat=1;
		}
		else 
		{
		    command[1]=INV0;
		    invEtat=0;
		}
		SendCMD(dev_handle,command,2);
	    }

	    if (recieve[0] & 0x1) // if red button pressed      display ON/OFF
		{
		if(dispEtat==0){
		    command[1]=LCDEN;
		    dispEtat=1;
		}
		else 
		{
		    command[1]=LCDDIS;
		    dispEtat=0;
		}
		SendCMD(dev_handle,command,2);
	    }
	} // end if
   
	} // end while 
	
	r = libusb_release_interface(dev_handle, 0); //release the claimed interface
	if(r!=0) {
		cout<<"Cannot Release Interface"<<endl;
		return 1;
	}
	cout<<"Released Interface"<<endl;

	libusb_close(dev_handle); //close the device we opened
	libusb_exit(ctx); //needs to be called to end the

	delete[] recieve,delete[] command ; ; //delete the allocated memory for data
	return 0;
}
