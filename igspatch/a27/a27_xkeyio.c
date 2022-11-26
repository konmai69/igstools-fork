#define _GNU_SOURCE

// A27 IO X11 Keyboard Emulator for PercussionMaster

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>

// X11 includes
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include "a27_utils.h"


static struct iostate {
    unsigned char hidden_sw[2];
    unsigned char coin;
    unsigned char sw_test;
    unsigned char sw_service;
    unsigned char p1_drum[6];
    unsigned char p2_drum[6]; 
}keyst,keyst_last;

static unsigned char state_read = 0;
static unsigned int switch_state;
static unsigned short coin_counter;
static unsigned char coin_state;
static pthread_t hthread;



void update_iostate(void){

    // Player 1
    (keyst.p1_drum[0] && !keyst_last.p1_drum[0]) ? (switch_state |= (1 << INP_P1_DRUM_1))  : (switch_state &= ~(1 << INP_P1_DRUM_1));
    (keyst.p1_drum[1] && !keyst_last.p1_drum[1]) ? (switch_state |= (1 << INP_P1_DRUM_2))  : (switch_state &= ~(1 << INP_P1_DRUM_2));
    (keyst.p1_drum[2] && !keyst_last.p1_drum[2]) ? (switch_state |= (1 << INP_P1_DRUM_3))  : (switch_state &= ~(1 << INP_P1_DRUM_3));
    (keyst.p1_drum[3] && !keyst_last.p1_drum[3]) ? (switch_state |= (1 << INP_P1_DRUM_4))  : (switch_state &= ~(1 << INP_P1_DRUM_4));
    (keyst.p1_drum[4] && !keyst_last.p1_drum[4]) ? (switch_state |= (1 << INP_P1_DRUM_5))  : (switch_state &= ~(1 << INP_P1_DRUM_5));
    (keyst.p1_drum[5] && !keyst_last.p1_drum[5]) ? (switch_state |= (1 << INP_P1_DRUM_6))  : (switch_state &= ~(1 << INP_P1_DRUM_6));
    
    // Player 2
    (keyst.p2_drum[0] && !keyst_last.p2_drum[0])  ? (switch_state |= (1 << INP_P2_DRUM_1)) : (switch_state &= ~(1 << INP_P2_DRUM_1));
    (keyst.p2_drum[1] && !keyst_last.p2_drum[1])  ? (switch_state |= (1 << INP_P2_DRUM_2)) : (switch_state &= ~(1 << INP_P2_DRUM_2));
    (keyst.p2_drum[2] && !keyst_last.p2_drum[2])  ? (switch_state |= (1 << INP_P2_DRUM_3)) : (switch_state &= ~(1 << INP_P2_DRUM_3));
    (keyst.p2_drum[3] && !keyst_last.p2_drum[3])  ? (switch_state |= (1 << INP_P2_DRUM_4)) : (switch_state &= ~(1 << INP_P2_DRUM_4));
    (keyst.p2_drum[4] && !keyst_last.p2_drum[4])  ? (switch_state |= (1 << INP_P2_DRUM_5)) : (switch_state &= ~(1 << INP_P2_DRUM_5));
    (keyst.p2_drum[5] && !keyst_last.p2_drum[5])  ? (switch_state |= (1 << INP_P2_DRUM_6)) : (switch_state &= ~(1 << INP_P2_DRUM_6));
    
    // Hidden
    keyst.hidden_sw[0] ? (switch_state |= (1 << INP_HIDDEN_3))    : (switch_state &= ~(1 << INP_HIDDEN_3));
    keyst.hidden_sw[1] ? (switch_state |= (1 << INP_HIDDEN_4))    : (switch_state &= ~(1 << INP_HIDDEN_4));

    // Switches
    (keyst.sw_service && !keyst_last.sw_service) ? (switch_state |= (1 << INP_SW_SERVICE)) : (switch_state &= ~(1 << INP_SW_SERVICE));        
    keyst.sw_test ? (switch_state |= (1 << INP_SW_TEST))    : (switch_state &= ~(1 << INP_SW_TEST));
    
    // Coin
    coin_state = (keyst.coin && !keyst_last.coin) ? 1 : 0;
    if(coin_state){
        coin_counter++;
        if(coin_counter >= 0xFFFF){
            coin_counter = 0;
        }
    }
    state_read = 0;
}


static void *input_thread(void *arg){
    int ksym;
    int kstate;
    Display *display;
    XEvent event;

    display = XOpenDisplay(NULL);
    if (display == NULL){
        printf("Cannot open display\n");
        exit(-1);
    }
    XGrabKeyboard(display, DefaultRootWindow(display),
                 True, GrabModeAsync, GrabModeAsync, CurrentTime);
    while (1){
        XNextEvent(display, &event);

        /* keyboard events */
        if (event.type == KeyPress || event.type == KeyRelease){
            memcpy(&keyst_last,&keyst,sizeof(struct iostate));
            ksym = XLookupKeysym(&event.xkey, 0);         
            switch(ksym){
                // For Fun
                case XK_Escape:
                    printf("[IGSTools::KeyIO] User Exited.\n");                
                    exit(0);
                    break;
                // System Switches
                case XK_1:
                    keyst.sw_test = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_2:
                    keyst.sw_service = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_5:
                    keyst.coin = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_6:
                    keyst.hidden_sw[0] = (event.type == KeyPress) ? 1:0;
                    break;
                case XK_7:
                    keyst.hidden_sw[1] = (event.type == KeyPress) ? 1:0;
                    break;                    
                // Player 1 
                case XK_z:
                    keyst.p1_drum[0] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_x:
                    keyst.p1_drum[1] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_c:
                    keyst.p1_drum[2] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_v:
                    keyst.p1_drum[3] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_b:
                    keyst.p1_drum[4] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_n:
                    keyst.p1_drum[5] = (event.type == KeyPress) ? 1 : 0;
                    break;
                // Player 2 
                case XK_a:
                    keyst.p2_drum[0] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_s:
                    keyst.p2_drum[1] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_d:
                    keyst.p2_drum[2] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_f:
                    keyst.p2_drum[3] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_g:
                    keyst.p2_drum[4] = (event.type == KeyPress) ? 1 : 0;
                    break;
                case XK_h:
                    keyst.p2_drum[5] = (event.type == KeyPress) ? 1 : 0;
                    break;                                                                                               
                default:
                break;
            }            
        }
        update_iostate();
    }
}


// -- Exports --

unsigned short A27KeyIO_GetCoinCounter(void){return coin_counter;}

void A27KeyIO_GetTestBuffer(unsigned char* io_test_buffer){
    // This is the status code.
    *(unsigned short*)io_test_buffer = 0;
    
    io_test_buffer[9]  = ((switch_state >> INP_P1_DRUM_1)  & 0x01); 
    io_test_buffer[10] = ((switch_state >> INP_P1_DRUM_2)  & 0x01);
    io_test_buffer[11] = ((switch_state >> INP_P1_DRUM_3)  & 0x01);   
    io_test_buffer[4]  = ((switch_state >> INP_P1_DRUM_4)  & 0x01);
    io_test_buffer[5]  = ((switch_state >> INP_P1_DRUM_5)  & 0x01); 
    io_test_buffer[6]  = ((switch_state >> INP_P1_DRUM_6)  & 0x01); 

    io_test_buffer[12] = ((switch_state >> INP_P2_DRUM_1)  & 0x01); 
    io_test_buffer[13] = ((switch_state >> INP_P2_DRUM_2)  & 0x01); 
    io_test_buffer[14] = ((switch_state >> INP_P2_DRUM_3) & 0x01); 
    io_test_buffer[15] = ((switch_state >> INP_P2_DRUM_4) & 0x01); 
    io_test_buffer[16] = ((switch_state >> INP_P2_DRUM_5) & 0x01); 
    io_test_buffer[17] = ((switch_state >> INP_P2_DRUM_6) & 0x01); 

    io_test_buffer[33] = coin_state;
    io_test_buffer[34] = ((switch_state >> INP_SW_SERVICE) & 0x01);
    io_test_buffer[35] = ((switch_state >> INP_SW_TEST) & 0x01);
    
    // Copies to deal with counts. Likely "Last State"
    io_test_buffer[36] = io_test_buffer[4];
    io_test_buffer[37] = io_test_buffer[5];
    io_test_buffer[38] = io_test_buffer[6];
    io_test_buffer[41] = io_test_buffer[9];
    io_test_buffer[42] = io_test_buffer[10];
    io_test_buffer[43] = io_test_buffer[11];

    io_test_buffer[44] = io_test_buffer[12];
    io_test_buffer[45] = io_test_buffer[13];
    io_test_buffer[46] = io_test_buffer[14];
    io_test_buffer[47] = io_test_buffer[15];
    io_test_buffer[48] = io_test_buffer[16];
    io_test_buffer[49] = io_test_buffer[17];

    io_test_buffer[65] = io_test_buffer[33];
    io_test_buffer[66] = io_test_buffer[34];
    io_test_buffer[67] = io_test_buffer[35];
}

void A27KeyIO_Inject(unsigned char* buf){
    struct A27_Read_Message* msg = (struct A27_Read_Message*)buf;
    
    if(state_read){        
        msg->coin_inserted = 0;
        msg->num_io_channels = 6;
        msg->button_io[2] = 0;
        msg->button_io[3] = 0;
        msg->button_io[4] = 0;
        msg->button_io[5] = 0;
        
    }else{
    msg->coin_inserted = coin_state;
    msg->num_io_channels = 6;
    
    msg->button_io[2] = switch_state;
    msg->button_io[3] = switch_state;
    msg->button_io[4] = switch_state;
    msg->button_io[5] = switch_state;
    state_read = 1;
    }


    // Because we've modified the packet, we now have to reset the checksum.
    A27SetChecksum(msg);
    
}

void A27KeyIO_Init(void){
    memset(&keyst,0,sizeof(struct iostate));
    memset(&keyst_last,0,sizeof(struct iostate));    
    pthread_create(&hthread, 0, input_thread, 0);
}

