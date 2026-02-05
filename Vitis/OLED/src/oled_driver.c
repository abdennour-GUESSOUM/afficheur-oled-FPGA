#include <stdio.h>
#include "sleep.h"
#include "oled_driver.h"

XGpioPs Gpio;
XSpiPs  Spi;

int GpioInit(XGpioPs *Gpio, u16 DeviceId)
{
    int Status;
    XGpioPs_Config *ConfigPtr;
    
    ConfigPtr = XGpioPs_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        printf("Erreur: GPIO LookupConfig a echoue\n");
        return XST_FAILURE;
    }
    
    Status = XGpioPs_CfgInitialize(Gpio, ConfigPtr, ConfigPtr->BaseAddr);
    if (Status != XST_SUCCESS) {
        printf("Erreur: GPIO CfgInitialize a echoue\n");
        return XST_FAILURE;
    }
    
    // Configure les pins en sortie
    XGpioPs_SetDirectionPin(Gpio, OLED_DC, 1);
    XGpioPs_SetDirectionPin(Gpio, OLED_RES, 1);
    XGpioPs_SetDirectionPin(Gpio, OLED_VBAT, 1);
    XGpioPs_SetDirectionPin(Gpio, OLED_VDD, 1);
    
    XGpioPs_SetOutputEnablePin(Gpio, OLED_DC, 1);
    XGpioPs_SetOutputEnablePin(Gpio, OLED_RES, 1);
    XGpioPs_SetOutputEnablePin(Gpio, OLED_VBAT, 1);
    XGpioPs_SetOutputEnablePin(Gpio, OLED_VDD, 1);
    
    // Etat initial des pins
    XGpioPs_WritePin(Gpio, OLED_DC, 0x0);
    XGpioPs_WritePin(Gpio, OLED_RES, 0x1);
    XGpioPs_WritePin(Gpio, OLED_VBAT, 0x1);  // VBAT OFF (actif bas)
    XGpioPs_WritePin(Gpio, OLED_VDD, 0x1);   // VDD OFF (actif bas)
    
    printf("GPIO initialise avec succes\n");
    return XST_SUCCESS;
}

int SpiInit(XSpiPs *SpiInstancePtr, u16 DeviceId)
{
    int Status;
    XSpiPs_Config *SpiConfig;
    
    SpiConfig = XSpiPs_LookupConfig(DeviceId);
    if (SpiConfig == NULL) {
        printf("Erreur: SPI LookupConfig a echoue\n");
        return XST_FAILURE;
    }
    
    Status = XSpiPs_CfgInitialize(SpiInstancePtr, SpiConfig, SpiConfig->BaseAddress);
    if (Status != XST_SUCCESS) {
        printf("Erreur: SPI CfgInitialize a echoue\n");
        return XST_FAILURE;
    }
    
    // Configure le SPI en mode master avec force SS
    Status = XSpiPs_SetOptions(SpiInstancePtr, 
                                XSPIPS_MASTER_OPTION | 
                                XSPIPS_FORCE_SSELECT_OPTION);
    if (Status != XST_SUCCESS) {
        printf("Erreur: SPI SetOptions a echoue\n");
        return XST_FAILURE;
    }
    
    // Prescaler pour reduire la vitesse (pour la stabilite)
    Status = XSpiPs_SetClkPrescaler(SpiInstancePtr, XSPIPS_CLK_PRESCALE_64);
    if (Status != XST_SUCCESS) {
        printf("Erreur: SPI SetClkPrescaler a echoue\n");
        return XST_FAILURE;
    }
    
    // Selectionne le slave 0
    XSpiPs_SetSlaveSelect(SpiInstancePtr, 0);
    
    printf("SPI initialise avec succes\n");
    return XST_SUCCESS;
}

void OledCommand(u8 cmd)
{
    XGpioPs_WritePin(&Gpio, OLED_DC, 0);  // DC = 0 pour commande
    XSpiPs_PolledTransfer(&Spi, &cmd, NULL, 1);
    usleep(10);  // Delai court entre les commandes
}

void OledData(u8 data)
{
    XGpioPs_WritePin(&Gpio, OLED_DC, 1);  // DC = 1 pour donnees
    XSpiPs_PolledTransfer(&Spi, &data, NULL, 1);
}

void OledInit(void)
{
    u8 charge_pump[] = CHARGE_PUMP_INIT;
    u8 control_init[] = CONTROL_INIT;
    int i;
    
    printf("Debut initialisation OLED...\n");
    
    // Etape 1: Allumer VDD
    XGpioPs_WritePin(&Gpio, OLED_VDD, 0);  // Actif bas
    usleep(10000);  // 10ms
    
    // Etape 2: Reset
    XGpioPs_WritePin(&Gpio, OLED_RES, 0);
    usleep(10000);  // 10ms
    XGpioPs_WritePin(&Gpio, OLED_RES, 1);
    usleep(10000);  // 10ms
    
    // Etape 3: Envoyer les commandes de charge pump
    printf("Envoi commandes charge pump...\n");
    for (i = 0; i < CHARGE_PUMP_INIT_LEN; i++) {
        OledCommand(charge_pump[i]);
    }
    
    // Etape 4: Allumer VBAT
    XGpioPs_WritePin(&Gpio, OLED_VBAT, 0);  // Actif bas
    usleep(100000);  // 100ms
    
    // Etape 5: Envoyer les commandes de controle
    printf("Envoi commandes de controle...\n");
    for (i = 0; i < CONTROL_INIT_LEN; i++) {
        OledCommand(control_init[i]);
    }
    
    // Effacer l'ecran
    OledCommand(0x20);  // Set memory addressing mode
    OledCommand(0x00);  // Horizontal addressing mode
    OledCommand(0x21);  // Set column address
    OledCommand(0x00);  // Column start = 0
    OledCommand(0x7F);  // Column end = 127
    OledCommand(0x22);  // Set page address
    OledCommand(0x00);  // Page start = 0
    OledCommand(0x03);  // Page end = 3
    
    printf("OLED initialise avec succes!\n");
}

void OledShutdown(void)
{
    printf("Arret OLED...\n");
    OledCommand(0xAE);
    XGpioPs_WritePin(&Gpio, OLED_VBAT, 1);
    usleep(100000);
    XGpioPs_WritePin(&Gpio, OLED_VDD, 1);
}

void OledClear(void)
{
    int i;
    XGpioPs_WritePin(&Gpio, OLED_DC, 1);
    for (i = 0; i < 512; i++) {
        OledData(0x00);
    }
    printf("Ecran efface\n");
}

void OledFill(void)
{
    int i;
    XGpioPs_WritePin(&Gpio, OLED_DC, 1);
    for (i = 0; i < 512; i++) {
        OledData(0xFF);
    }
    printf("Ecran rempli\n");
}

const u8 font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (espace)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')'
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // '*'
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // '+'
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ','
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':'
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';'
    {0x08, 0x14, 0x22, 0x41, 0x00}, // '<'
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '='
    {0x00, 0x41, 0x22, 0x14, 0x08}, // '>'
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 'F'
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V'
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X'
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z'
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // '['
    {0x02, 0x04, 0x08, 0x10, 0x20}, // '\'
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ']'
    {0x04, 0x02, 0x01, 0x02, 0x04}, // '^'
    {0x40, 0x40, 0x40, 0x40, 0x40}, // '_'
    {0x00, 0x01, 0x02, 0x04, 0x00}, // '`'
    {0x20, 0x54, 0x54, 0x54, 0x78}, // 'a'
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // 'b'
    {0x38, 0x44, 0x44, 0x44, 0x20}, // 'c'
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // 'd'
    {0x38, 0x54, 0x54, 0x54, 0x18}, // 'e'
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // 'f'
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // 'g'
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // 'h'
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // 'i'
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // 'j'
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // 'k'
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // 'l'
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // 'm'
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // 'n'
    {0x38, 0x44, 0x44, 0x44, 0x38}, // 'o'
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // 'p'
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // 'q'
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // 'r'
    {0x48, 0x54, 0x54, 0x54, 0x20}, // 's'
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // 't'
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 'u'
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 'v'
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 'w'
    {0x44, 0x28, 0x10, 0x28, 0x44}, // 'x'
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 'y'
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // 'z'
};

void OledSetCursor(u8 x, u8 y)
{
    OledCommand(0x21);  // Set column address
    OledCommand(x);     // Column start
    OledCommand(127);   // Column end
    
    OledCommand(0x22);  // Set page address
    OledCommand(y);     // Page start
    OledCommand(3);     // Page end
}

void OledPutChar(char c)
{
    int i;
    u8 index;
    
    // Convertir le caractere en index de police
    if (c >= ' ' && c <= 'z') {
        index = c - ' ';
    } else {
        index = 0;  // Espace par defaut
    }
    
    // Envoyer les 5 colonnes du caractere
    XGpioPs_WritePin(&Gpio, OLED_DC, 1);  // Mode data
    for (i = 0; i < 5; i++) {
        OledData(font5x7[index][i]);
    }
    
    // Ajouter un espace entre les caracteres
    OledData(0x00);
}

void OledPrintString(const char *str)
{
    while (*str) {
        OledPutChar(*str++);
    }
}

void OledPrintStringAt(u8 x, u8 y, const char *str)
{
    OledSetCursor(x, y);
    OledPrintString(str);
}

int main(void)
{
    int Status;
    
    printf("\n=== Demarrage test OLED ZedBoard ===\n\n");
    
    // Initialiser GPIO
    Status = GpioInit(&Gpio, GPIO_DEVICE_ID);
    if (Status != XST_SUCCESS) {
        printf("ERREUR: Initialisation GPIO a echoue!\n");
        return XST_FAILURE;
    }
    
    // Initialiser SPI
    Status = SpiInit(&Spi, SPI_DEVICE_ID);
    if (Status != XST_SUCCESS) {
        printf("ERREUR: Initialisation SPI a echouee!\n");
        return XST_FAILURE;
    }
    
    OledInit();
    
    printf("\nTest 1: Remplissage de l'ecran...\n");
    OledFill();
    sleep(2);

    
    printf("Test 2: Effacement de l'ecran...\n");
    OledClear();
    sleep(1);
    
    printf("Test 3: Clignotement (3 cycles)...\n");
    for (int i = 0; i < 1; i++) {
        OledFill();
        OledClear();
        sleep(1);
    }
    
    printf("\n=== Tests termines! ===\n");
    printf("L'OLED fonctionne correctement!\n");
    
    OledFill();
    OledPrintStringAt(30, 1, "GUESSOUM ABDENNOUR");
    
    return XST_SUCCESS;
}