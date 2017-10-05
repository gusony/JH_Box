/******************** LoRa  mode**************************/
//Error Coding rate (CR)setting
#define CR_4_5 0
#define CR  0x01  // 4/5
//CRC Enable
#define CRC_EN
#define CRC 0x01  //CRC Enable
//RFM98 Internal registers Address
/********************Lroa mode***************************/
// Common settings
#define LR_RegOpMode  0x01
#define LR_RegFrMsb 0x06
// Tx settings
#define LR_RegPaConfig  0x09
#define LR_RegOcp 0x0B
// Rx settings
#define LR_RegLna 0x0C
#define LR_RegFifoAddrPtr 0x0D
#define LR_RegFifoTxBaseAddr  0x0E
#define LR_RegFifoRxBaseAddr  0x0F
#define LR_RegFifoRxCurrentaddr 0x10
#define LR_RegIrqFlagsMask  0x11
#define LR_RegIrqFlags  0x12
#define LR_RegRxNbBytes 0x13
#define LR_RegModemStat 0x18
#define LR_RegModemConfig1  0x1D
#define LR_RegModemConfig2  0x1E
#define LR_RegSymbTimeoutLsb  0x1F
#define LR_RegPreambleMsb 0x20
#define LR_RegPreambleLsb 0x21
#define LR_RegPayloadLength 0x22
#define LR_RegHopPeriod 0x24
// I/O settings
#define REG_LR_DIOMAPPING1  0x40
#define REG_LR_DIOMAPPING2  0x41
// Additional settings
#define REG_LR_PADAC  0x4D

unsigned char mode;//lora--1/FSK--0
unsigned char Freq_Sel;
unsigned char Power_Sel;
unsigned char Lora_Rate_Sel;
unsigned char BandWide_Sel;
unsigned char Fsk_Rate_Sel;

int reset = 32;
int led  = 13;
int sck  = 12;
int mosi = 11;
int miso = 10;
int nsel = 9;
int dio0 = 8;

#include "DHT.h"
DHT dht(13,DHT22);
/*********Parameter table define**************************/
unsigned char sx1276_7_8FreqTbl[1][3] = {
  {0x6C, 0x80, 0x00}, //434MHz 
};
unsigned char sx1276_7_8PowerTbl[4] = {
  0xFF, 0xFC, 0xF9, 0xF6, //20dbm,17dbm,14dbm,11dbm
};
unsigned char sx1276_7_8SpreadFactorTbl[7] = {
  6, 7, 8, 9, 10, 11, 12
};
unsigned char sx1276_7_8LoRaBwTbl[10] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9 //7.8,10.4,15.6,20.8,31.2,41.7,62.5,125,250,500KHz
};
unsigned char sx1276_7_8Data[19] = {0x60,0x45,0x28,0x90,0xc0,0xb1,0x72,0xff};
unsigned char RxData[64];
int i;
char temp[6];
char humi[6];

/**********************************************************
**Name:  SPICmd8bit
**Function: SPI Write one byte
**Input:  WrPara
**Output: none
**note: use for burst mode
**********************************************************/
void SPICmd8bit(unsigned char WrPara) {
  unsigned char bitcnt;
  digitalWrite(nsel, LOW);//nSEL_L();
  digitalWrite(sck, LOW);//SCK_L();
  for (bitcnt = 8; bitcnt != 0; bitcnt--)
  {
    digitalWrite(sck, LOW);//SCK_L();
    if (WrPara & 0x80)//最高位元是否為1
      digitalWrite(mosi, HIGH);//SDI_H();
    else
      digitalWrite(mosi, LOW);//SDI_L();

    digitalWrite(sck, HIGH);//SCK_H();
    WrPara <<= 1;
  }
  digitalWrite(sck, LOW);//SCK_L();
  digitalWrite(mosi, HIGH);//SDI_H();
}
/**********************************************************
**Name: SPIRead8bit
**Function: SPI Read one byte
**Input:  None
**Output: result byte
**Note: use for burst mode
**********************************************************/
unsigned char SPIRead8bit(void) {
  unsigned char RdPara = 0;
  unsigned char bitcnt;

  digitalWrite(nsel, LOW);//nSEL_L();
  digitalWrite(mosi, HIGH);//SDI_H(); //Read one byte data from FIFO, MOSI hold to High
  for (bitcnt = 8; bitcnt != 0; bitcnt--)
  {
    digitalWrite(sck, LOW);//SCK_L();
    RdPara <<= 1;
    digitalWrite(sck, HIGH); //SCK_H();

    if (digitalRead(miso)) //if(Get_SDO())
      RdPara |= 0x01;
    else
      RdPara |= 0x00;
  }
  digitalWrite(sck, LOW);//SCK_L();
  return (RdPara);
}
/**********************************************************
**Name: SPIRead
**Function: SPI Read CMD
**Input:  adr -> address for read
**Output: None
**********************************************************/
unsigned char SPIRead(unsigned char adr) {
  unsigned char tmp;

  SPICmd8bit(adr);  //Send address first
  tmp = SPIRead8bit();
  digitalWrite(nsel, HIGH);//nSEL_H();
  return (tmp);
}
/**********************************************************
**Name: SPIWrite
**Function: SPI Write CMD
**Input:  u8 address & u8 data
**Output: None
**********************************************************/
void SPIWrite(unsigned char adr, unsigned char WrPara) {
  digitalWrite(nsel, LOW);//nSEL_L();
  SPICmd8bit(adr | 0x80); //寫入地址，一定大於0x80
  SPICmd8bit(WrPara);//寫入數據
  digitalWrite(sck, LOW);//SCK_L();
  digitalWrite(mosi, HIGH);//SDI_H();
  digitalWrite(nsel, HIGH);//nSEL_H();
}
/**********************************************************
**Name: SPIBurstRead
**Function: SPI burst read mode
**Input:  adr-----address for read
**        ptr-----data buffer point for read
**        length--how many bytes for read
**Output: None
**********************************************************/
void SPIBurstRead(unsigned char adr, unsigned char *ptr, unsigned char leng) {
  unsigned char i;

  if (leng <= 1) //length must more than  one
    return;
  else
  {
    digitalWrite(sck, LOW); //SCK_L();
    digitalWrite(nsel, LOW);//nSEL_L();

    SPICmd8bit(adr);
    for (i = 0; i < leng; i++)
      ptr[i] = SPIRead8bit();
    digitalWrite(nsel, HIGH);//nSEL_H();
  }
}
/**********************************************************
**Name: SPIBurstWrite
**Function: SPI burst write mode
**Input:  adr-----address for write
**        ptr-----data buffer point for write
**        length--how many bytes for write
**Output: none
**********************************************************/
void BurstWrite(unsigned char adr, unsigned char *ptr, unsigned char leng) {
  unsigned char i;

  if (leng <= 1) //length must more than  one
    return;
  else
  {
    digitalWrite(sck, LOW);//SCK_L();
    digitalWrite(nsel, LOW);//nSEL_L();
    SPICmd8bit(adr | 0x80);
    for (i = 0; i < leng; i++)
      SPICmd8bit(ptr[i]);
    digitalWrite(nsel, HIGH);//nSEL_H();
  }
}

/***********************LoRa mode**************************/
/**********************************************************
**Name: sx1276_7_8_Standby
**Function: Entry standby mode
**Input:  None
**Output: None
**********************************************************/
void sx1276_7_8_Standby(void) {
  /*   7  :0->FSK/OOK Mode，1->LoRa mode
       6-5:00->FSK，01->OOK，10->11->reserved
       4  :0->reserved
       3  :0->High 1->Low frequency mode
       2-0:000->sleep，001->standby，....*/
  SPIWrite(LR_RegOpMode, 0x09);//0000 1001
  //SPIWrite(LR_RegOpMode,0x01);//0000 0001
}
/**********************************************************
**Name: sx1276_7_8_Sleep
**Function: Entry sleep mode
**Input:  None
**Output: None
**********************************************************/
void sx1276_7_8_Sleep(void) {
  SPIWrite(LR_RegOpMode, 0x08);//0000 1000
  //SPIWrite(LR_RegOpMode,0x00);
}
/**********************************************************
**Name: sx1276_7_8_EntryLoRa
**Function: Set RFM69 entry LoRa(LongRange) mode
**Input:  None
**Output: None
**********************************************************/
void sx1276_7_8_EntryLoRa(void) {
  SPIWrite(LR_RegOpMode, 0x88); //Low Frequency Mode//1000 1000
  //SPIWrite(LR_RegOpMode,0x80);//High Frequency Mode
}
/**********************************************************
**Name: sx1276_7_8_LoRaClearIrq
**Function: Clear all irq
**Input:  None
**Output: None
**********************************************************/
void sx1276_7_8_LoRaClearIrq(void) {
  SPIWrite(LR_RegIrqFlags, 0xFF);
}
/**********************************************************
**Name: sx1276_7_8_LoRaEntryRx
**Function: Entry Rx mode
**Input:  None
**Output: None
**********************************************************/
unsigned char sx1276_7_8_LoRaEntryRx(void) {
  unsigned char addr;

  sx1276_7_8_Config();  //setting base parameter

  SPIWrite(REG_LR_PADAC, 0x84); //Normal and Rx //Higher power settings of the PA
  SPIWrite(LR_RegHopPeriod, 0xFF); //RegHopPeriod NO FHSS
  SPIWrite(REG_LR_DIOMAPPING1, 0x01);  //DIO0=00, DIO1=00, DIO2 = 00, DIO3 = 01

  SPIWrite(LR_RegIrqFlagsMask, 0x3F); //Open RxDone interrupt & Timeout 0011 1111
  sx1276_7_8_LoRaClearIrq();

  SPIWrite(LR_RegPayloadLength, 21); //RegPayloadLength 21byte(this register must difine when the data long of one byte in SF is 6)

  addr = SPIRead(LR_RegFifoRxBaseAddr); //Read RxBaseAddr
  SPIWrite(LR_RegFifoAddrPtr, addr); //RxBaseAddr -> FiFoAddrPtr
  SPIWrite(LR_RegOpMode, 0x8d); //Continuous  Rx Mode//Low Frequency Mode
  //SPIWrite(LR_RegOpMode,0x05);  //Continuous  Rx Mode//High Frequency Mode
  //SysTime = 0;
  while (1)
  {
    if ((SPIRead(LR_RegModemStat) & 0x04) == 0x04) //Rx-on going RegModemStat
      break;    
  }
}
/**********************************************************
**Name: sx1276_7_8_LoRaRxPacket
**Function: Receive data in LoRa mode
**Input:  None
**Output: 1- Success
          0- Fail
**********************************************************/
unsigned char sx1276_7_8_LoRaRxPacket(void) {
  unsigned char i;
  unsigned char addr;
  unsigned char packet_size;

  if (digitalRead(dio0)) //if(Get_NIRQ())
  {
    for (i = 0; i < 32; i++)
      RxData[i] = 0x00;

    addr = SPIRead(LR_RegFifoRxCurrentaddr);//last packet addr
    SPIWrite(LR_RegFifoAddrPtr, addr);//RxBaseAddr ->   FiFoAddrPtr
    if (sx1276_7_8SpreadFactorTbl[Lora_Rate_Sel] == 6) //When SpreadFactor is six，will used Implicit Header mode(Excluding internal packet length)
      packet_size = 21;
    else
      packet_size = SPIRead(LR_RegRxNbBytes); //Number for received bytes

    SPIBurstRead(0x00, RxData, packet_size);
    Serial.print("RECV: ");
    for(i=0;i<packet_size;i++){
      Serial.print((char)RxData[i]);
    }
    Serial.println();

    sx1276_7_8_LoRaClearIrq(); 
    return (1);
  }
  else
    return (0);
}
/**********************************************************
**Name: sx1276_7_8_LoRaEntryTx
**Function: Entry Tx mode
**Input:  None
**Output: None
**********************************************************/
unsigned char sx1276_7_8_LoRaEntryTx(void) {
  unsigned char addr, temp;

  sx1276_7_8_Config();  //setting base parameter

  SPIWrite(REG_LR_PADAC, 0x87); //Tx for 20dBm
  SPIWrite(LR_RegHopPeriod, 0x00); //RegHopPeriod NO FHSS
  SPIWrite(REG_LR_DIOMAPPING1, 0x41); //DIO0=01, DIO1=00, DIO2 = 00, DIO3 = 01

  sx1276_7_8_LoRaClearIrq();
  SPIWrite(LR_RegIrqFlagsMask, 0xF7); //Open TxDone interrupt
  SPIWrite(LR_RegPayloadLength, 21); //RegPayloadLength 21byte

  addr = SPIRead(LR_RegFifoTxBaseAddr); //RegFiFoTxBaseAddr
  SPIWrite(LR_RegFifoAddrPtr, addr);  //RegFifoAddrPtr

  //SysTime = 0;
  while (1)
  {
    temp = SPIRead(LR_RegPayloadLength);
    if (temp == 21)
    {
      break;
    }
  }
}
/**********************************************************
**Name: sx1276_7_8_LoRaTxPacket
**Function: Send data in LoRa mode
**Input:  None
**Output: 1- Send over
**********************************************************/
unsigned char sx1276_7_8_LoRaTxPacket(void) {
  unsigned char TxFlag = 0;
  unsigned char addr;
  
  BurstWrite(0x00, (unsigned char *)sx1276_7_8Data, sizeof(sx1276_7_8Data));
  
  Serial.print("SEND: ");
  for(i=0;i<sizeof(sx1276_7_8Data);i++){
    Serial.print((char)sx1276_7_8Data[i]);
    Serial.print(" ");
  }
  Serial.println();
 
  SPIWrite(LR_RegOpMode, 0x8b); //Tx Mode
  while (1)
  {
    if (digitalRead(dio0)) //if(Get_NIRQ())     //Packet send over
    {
      SPIRead(LR_RegIrqFlags);
      sx1276_7_8_LoRaClearIrq();  //Clear irq
      sx1276_7_8_Standby(); //Entry Standby mode
      break;
    }
  }
}
/**********************************************************
**Name: sx1276_7_8_Config
**Function: sx1276_7_8 base config
**Input:  mode
**Output: None
**********************************************************/
void sx1276_7_8_Config(void) {
  unsigned char i;

  sx1276_7_8_Sleep(); //Change modem mode Must in Sleep mode
  for (i = 250; i != 0; i--); //Delay
  delay(15);

  //lora mode
  sx1276_7_8_EntryLoRa();
  //SPIWrite(0x5904);   //Change digital regulator form 1.6V to 1.47V: see errata note

  //setting frequency parameter
  BurstWrite(LR_RegFrMsb, sx1276_7_8FreqTbl[Freq_Sel], 3);

  //setting base parameter
  SPIWrite(LR_RegPaConfig, sx1276_7_8PowerTbl[Power_Sel]);    //Setting output power parameter

  SPIWrite(LR_RegOcp, 0x0B); //RegOcp,Close Ocp，0000 1011
  SPIWrite(LR_RegLna, 0x23);   //RegLNA,High & LNA Enable

  if (sx1276_7_8SpreadFactorTbl[Lora_Rate_Sel] == 6) //SFactor=6
  {
    unsigned char tmp;

    SPIWrite(LR_RegModemConfig1, ((sx1276_7_8LoRaBwTbl[BandWide_Sel] << 4) + (CR << 1) + 0x01));//Implicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)
    SPIWrite(LR_RegModemConfig2, ((sx1276_7_8SpreadFactorTbl[Lora_Rate_Sel] << 4) + (CRC << 2) + 0x03));

    tmp = SPIRead(0x31);
    tmp &= 0xF8;//1111 1000
    tmp |= 0x05;//0000 0101
    SPIWrite(0x31, tmp);//0011 0001，
    SPIWrite(0x37, 0x0C);//0011 0111，0000 1100
  }
  else
  {
    SPIWrite(LR_RegModemConfig1, ((sx1276_7_8LoRaBwTbl[BandWide_Sel] << 4) + (CR << 1) + 0x00));//Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)
    SPIWrite(LR_RegModemConfig2, ((sx1276_7_8SpreadFactorTbl[Lora_Rate_Sel] << 4) + (CRC << 2) + 0x03)); //SFactor & LNA gain set by the internal AGC loop
  }
  SPIWrite(LR_RegSymbTimeoutLsb, 0xFF); //RegSymbTimeoutLsb  Timeout = 0x3FF(Max)
  SPIWrite(LR_RegPreambleMsb, 0x00); //RegPreambleMsb
  SPIWrite(LR_RegPreambleLsb, 12);   //RegPreambleLsb  8 + 4 = 12byte Preamble
  SPIWrite(REG_LR_DIOMAPPING2, 0x01); //RegDioMapping2 DIO5=00, DIO4=01
  
  sx1276_7_8_Standby(); //Entry standby mode
}
void setup() {
  pinMode(led,  OUTPUT);
  pinMode(nsel, OUTPUT);
  pinMode(sck,  OUTPUT);
  pinMode(mosi, OUTPUT);
  pinMode(miso, INPUT);
  pinMode(reset, OUTPUT);
  Serial.begin(9600);
}
void loop() {
  mode = 0x01;
  Freq_Sel = 0x00;
  Power_Sel = 0x00;
  Lora_Rate_Sel = 0x06;
  BandWide_Sel = 0x07;
  Fsk_Rate_Sel = 0x00;

  sx1276_7_8_Config();
  sx1276_7_8_LoRaEntryRx();
  String arr_temp=(String)dht.readTemperature();
  String arr_humi=(String)dht.readHumidity();
  
  while (1)
  {
    //slaver
    if (sx1276_7_8_LoRaRxPacket())//return 1 == RX success
    {
      sx1276_7_8_LoRaEntryRx();

      arr_temp=(String)dht.readTemperature();
      arr_humi=(String)dht.readHumidity();
    
      arr_temp.toCharArray(temp,6);
      arr_humi.toCharArray(humi,6);
    
      for(i=0;i<5;i++)
        sx1276_7_8Data[8+i]=temp[i];
        
      sx1276_7_8Data[13]=',';
      for(i=0;i<5;i++)
        sx1276_7_8Data[14+i]=humi[i];
      /*
      Serial.print("Loop: ");
      for(i=0;i<sizeof(sx1276_7_8Data);i++)
       Serial.print(sx1276_7_8Data[i]);
      Serial.println();
      */
      
      sx1276_7_8_LoRaEntryTx();
      sx1276_7_8_LoRaTxPacket();
      
      sx1276_7_8_LoRaEntryRx();
    }
  }
}
