// Various Binary Patches for PercussionMaster
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../utils/utils.h"
#include "patches.h"

// Hardcoded Addresses for Patches
#define ADDR_SDL_VIDEOFLAGS     0x08056BC8
#define ADDR_MENU_QCTEST        0x08075088
#define ADDR_MENU_DEVTEST       0x08076880
#define ADDR_MENU_TRACKBALL     0x08069768
#define ADDR_CALL_OPMENU        0x08056A78
#define ADDR_SKIP_WARNING       0x080A7C84
#define ADDR_SKIP_WARNING2      0x080A7D3D
#define ADDR_ENABLE_AUTOPLAY    0x080A7D47
#define ADDR_MENU_LANGUAGE      0x080739E8
#define ADDR_OSS_SOUNDFIX       0x0807C34B
#define ADDR_REAL_SOUNDPLAY     0x08065524
#define ADDR_SOUND_CH_TABLE     0x083EF020

// MTV Localization Stuff
static char* noyes[] = {
    "No",
    "Yes"
};

static char* yesno[] = {
    "Yes",
    "No"
};

static char* speed_options[] = {
    "Normal",
    "S1x",
    "S2x",
    "S3x",
    "S4x"
};

static char* cloak_options[] = {
    "Normal",
    "1x",
    "2x",
    "3x"
};

static char* record_options[] = {
    "None",
    "Record",
    "Play"
};

static char* judgement_options[] = {
    "Easy",
    "Normal",
    "Hard",
    "Expert"
};

static char* difficulty_options[] = {
    "Training",
    "Elementary",
    "Intermediate",
    "Advanced",
    "Super Advanced",
    "Challenge",
    "Drum King",
    "END",
    "Battle"
};

static StrReplaceEntry test_menu_english[] = {
    {0x0807B1B9,"[%d:%2d] Notes:%d Tempo:%d Moves:%d"},
    {0x0807B1E6,"Stars: %02d SecID: %d Difficulty: %s"},
    {0x0807B1FD,"Select Song"},
    {0x0807B227,(char*)difficulty_options},
    {0x0807B22B,"Mode: %s"},
    {0x0807B252,(char*)noyes},
    {0x0807B261,"%dP Enabled: %s"},
    {0x0807B2A9,"%dP Autoplay: %s"},
    {0x0807B2CF,(char*)speed_options},
    {0x0807B2DB,"%dP Speed Mod: %s"},
    {0x0807B304,(char*)cloak_options},
    {0x0807B309,"%dP Cloak Mod: %s"},
    {0x0807B32F,"%dP Noteskin: %d"},
    {0x0807B352,(char*)record_options},
    {0x0807B359,"KeyRecord: %s"},
    {0x0807B381,"File Index: %d"},
    {0x0807B3A8,(char*)yesno},
    {0x0807B3AF,"Keysounds: %s"},
    {0x0807B3DB,"Player Vol: %d"},
    {0x0807B407,"Song Vol: %d"},
    {0x0807B433,"BG Vol: %d"},
    {0x0807B45A,(char*)judgement_options},
    {0x0807B462,"Judgement: %s"},
};

// Linux Path Replacements
static StrReplaceEntry linux_path_entries[] = {
    {0x08171F40,"./band1/rom/chainl.rom"},
    {0x08172040,"./band1/rom/effect3l.rom"},
    {0x08172140,"./band1/rom/churchl.rom"},
    {0x08172240,"./band1/rom/effect3l.rom"},
    {0x08172340,"./band1/rom/starnightl.rom"},
    {0x08172440,"./band1/rom/effect2l.rom"},
    {0x08172540,"./band1/rom/herol.rom"},
    {0x08172640,"./band1/rom/effect1l.rom"},
    {0x08172740,"./band1/rom/punkl.rom"},
    {0x08172840,"./band1/rom/effect1l.rom"},
    {0x08172940,"./band1/rom/taiwangol.rom"},
    {0x08172A40,"./band1/rom/effect3l.rom"},
    {0x08172B40,"./band1/rom/lovewordl.rom"},
    {0x08172C40,"./band1/rom/effect2l.rom"},
    {0x08172D40,"./band1/rom/ancientl.rom"},
    {0x08172E40,"./band1/rom/effect1l.rom"},
    {0x08172F40,"./band1/rom/ninjal.rom"},
    {0x08173040,"./band1/rom/effect2l.rom"},
    {0x08173140,"./band1/rom/spacel.rom"},
    {0x08173240,"./band1/rom/effect1l.rom"},
    {0x08173340,"./band1/rom/baseballl.rom"},
    {0x08173440,"./band1/rom/effect3l.rom"},
    {0x08173540,"./band1/rom/summerl.rom"},
    {0x08173640,"./band1/rom/effect3l.rom"},
    {0x08173740,"./band1/rom/bigcityl.rom"},
    {0x08173840,"./band1/rom/effect3l.rom"},
    {0x08173940,"./band1/rom/wildwestl.rom"},
    {0x08173A40,"./band1/rom/effect2l.rom"},
    {0x08120037,"./main/selmodel.rom"},
    {0x08120052,"./main/rolel.rom"},
    {0x0812006A,"./main/role1l.rom"},
    {0x08120083,"./main/role2l.rom"},
    {0x0812009C,"./main/role3l.rom"},
    {0x081200B5,"./main/role4l.rom"},
    {0x0812016F,"./main/coinpagel.rom"},
    {0x081202A5,"./main/photol.rom"},
    {0x081206BC,"./main/selsongl.rom"},
    {0x08120745,"./main/optionl.rom"},
    {0x081228E4,"./main/selrolel.rom"},
    {0x0812295F,"./main/cardinfol.rom"},
    {0x08122F17,"./main/signl.rom"},
    {0x08122FC5,"./main/resultl.rom"},
    {0x081234B7,"./main/irl.rom"},
    {0x081234F5,"./main/rankl.rom"},
    {0x081235CD,"./main/stylel.rom"},
    {0x0816E6E0,"write-avi.so"},
    {0x0816E720,"snd-oss.so"}
};

// Localize the Song Test Menu 
void Localize_MTVMenu(void){
    int num_str_entries;
    int curr_entry;
    void** cur_addr;
    num_str_entries = sizeof(test_menu_english) / sizeof(StrReplaceEntry);
    for(curr_entry=0; curr_entry < num_str_entries; curr_entry++){
        UnprotectPage(test_menu_english[curr_entry].offset);
        cur_addr = (void*)test_menu_english[curr_entry].offset;
        *cur_addr = test_menu_english[curr_entry].str;
    }    
}

// Replace call to operator menu with the menu given.
void Replace_OperatorMenu(int replacement_menu){
    UnprotectPage(ADDR_CALL_OPMENU);
    *(int*)(ADDR_CALL_OPMENU+1) = (replacement_menu - (int)(ADDR_CALL_OPMENU + 5));
}


// -- Exports --

void Patch_SetWindowedMode(void){
    printf("[Patches::Windowed] Setting Game to Windowed Mode.\n");
    UnprotectPage(ADDR_SDL_VIDEOFLAGS);
    *(char*)ADDR_SDL_VIDEOFLAGS = 0;
}

void Patch_QCTest(void){
    printf("[Patches::QCTest] Enabling QCTest Menu.\n");
    Replace_OperatorMenu(ADDR_MENU_QCTEST);
}

void Patch_TrackballMenu(void){
    printf("[Patches::Autoplay] Enabling Trackball Menu.\n");
    Replace_OperatorMenu(ADDR_MENU_TRACKBALL);
}

void Patch_DevTest(void){
    printf("[Patches::DevTest] Enabling DevTest Menu.\n");
    Replace_OperatorMenu(ADDR_MENU_DEVTEST);
    Localize_MTVMenu();
}

void Patch_Language(void){
    printf("[Patches::DevTest] Enabling Language Menu.\n");
    Replace_OperatorMenu(ADDR_MENU_LANGUAGE);    
}

void Patch_Autoplay(void){
    printf("[Patches::Autoplay] Enabling Autoplay.\n");
    UnprotectPage(ADDR_ENABLE_AUTOPLAY);
    *(char*)ADDR_ENABLE_AUTOPLAY = 1;
}

void Patch_SkipWarning(void){
        printf("[Patches::Warning] Skip Warning.\n");
        UnprotectPage(ADDR_SKIP_WARNING);
        UnprotectPage(ADDR_SKIP_WARNING2);
        *(unsigned int*)ADDR_SKIP_WARNING = 10;
        *(unsigned int*)ADDR_SKIP_WARNING2 = 10;
}


int (*real_soundplay)(int channel, int volume) = (void*)ADDR_REAL_SOUNDPLAY;
int fake_soundplay(int channel, int volume){
    
    // The songs get obnoxiously loud, probably due to the OSS emulation layer.
    if(volume > 5){volume = 2;}
    // Avoid calling a channel that doesn't exist - the staff page has an issue that crashes the game otherwise.
    char channel_state = *(unsigned char*)ADDR_SOUND_CH_TABLE+(6 * channel);
    if(!channel_state){return 1;}
    return real_soundplay(channel,volume);
}
void Patch_OSSSoundFix(void){
    printf("[Patches::StaffAudio] Patching Staff Audio Crash and SongVolume.\n");
    PatchCall((void*)ADDR_OSS_SOUNDFIX,(void*)fake_soundplay);
}

void Patch_FilesystemPaths(void){
    unsigned int num_entries_in_table = sizeof(linux_path_entries) / sizeof(StrReplaceEntry);
    for(int i=0;i<num_entries_in_table;i++){
        UnprotectPage(linux_path_entries->offset);
        strcpy((char*)linux_path_entries->offset,linux_path_entries->str);
    }
}


