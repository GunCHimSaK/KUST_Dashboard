# ☀️ KUST 2027 WSC - Dashboard

### Project info

2027 World Solar Challenge (27WSC)를 위한 태양광 자동차 대시보드 펌웨어입니다.

차량 내 핵심 데이터(Driving/Power, Battery BMS)들을 CAN 통신을 통해 데이터를 주고받으며, 주행 정보 및 에러 상태를 실시간으로 계기판 LCD에 띄우고 핸들에서 입력되는 다양한 버튼 데이터들을 각 부분에 전송합니다.

### **System Block Diagram**
<img width="922" height="293" alt="image" src="https://github.com/user-attachments/assets/05a70434-7c00-4ddf-8185-0e943a41ad37" />


### **Dashboard Screen**

![](file://C:\Users\Han%20Gyujin\AppData\Roaming\marktext\images\2026-05-11-21-05-10-image.png?msec=1778501110466)

### Hardware

- **MCU:** Nucleo U535CE
  
- **Clock:** 160MHz
  
- **IDE:** STM32CubeIDE
  
- **Tool:** LVGL, Square Line Studio
  
- **Communication:** FDCAN (Classic CAN Mode), LPUART1 (115200 bps, Debug)
  

###

### CAN Matrix

|     | CAN ID | Group | Byte0 | Byte1 | Byte2 | Byte3 | Byte4 | Byte5 | Byte6 | Byte7 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Receive | 0x010 | Fault/Err | Fault Num | Fault Code | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 |
|     | 0x100 | Driving/Power | Speed | Recommend Speed | MPPT Power | Motor Power | 0x00 | 0x00 | 0x00 | 0x00 |
|     | 0x200 | Status/Information1 | Battery Soc(%) | Battery Signal Bits | Time(Hour) | Time(Min) | 0x00 | 0x00 | 0x00 | 0x00 |
|     | 0x201 | Status/Information2 | Info1 Value | Info2 Value | Info3 Value L | Info3 Value H | Info4 Value | 0x00 | 0x00 | 0x00 |
| Transmit | 0x011 | Fault/Err | Fault Num | Fault Code | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 |
|     | 0x050 | Control Data | Control Bits | ACC Speed | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 |

|     | Bit0 | Bit1 | Bit2 | Bit3 | Bit4 | Bit5 | Bit6 | Bit7 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Battery Signal Bits | Signal State | Battery State | 0   | 0   | 0   | 0   | 0   | 0   |
| Control Bits | ACC ON/OFF | Test Value1 ON/OFF | Test Value2 ON/OFF | Test Value3 ON/OFF | 0   | 0   | 0   | 0   |

###

### Dashboard CAN Data

| Internal data | Notes | Receive data | Notes | Transmit data | Notes |
| --- | --- | --- | --- | --- | --- |
| Left signal ON/OFF | 0&1 | Speed | 1byte | ACC ON/OFF | 0&1 |
| Right signal ON/OFF | 0&1 | Recommend Speed | 1byte | Test Value1 ON/OFF | 0&1 |
| Hazard ON/OFF | 0&1 | MPPT Power | 1byte(scaling1/10) | Test Value2 ON/OFF | 0&1 |
| ACC ON/OFF | 0&1 | Motor Power | 1byte | Test Value3 ON/OFF | 0&1 |
| Test Value1 ON/OFF | 0&1 | Signal State | 0&1 | Fault Num | 1byte |
| Test Value2 ON/OFF | 0&1 | Battery Soc(%) | 1byte | Fault Code | 1byte |
| Test Value3 ON/OFF | 0&1 | Battery State | 0&1 |     |     |
| Fault Num | 1byte | Info1 Value | 1byte |     |     |
| Fault Code | 1byte | Info2 Value | 1byte |     |     |
| ACC Speed | 1byte | Info3 Value | 2byte |     |     |
|     |     | Info4 Value | 1byte |     |     |
|     |     | Time | 2byte |     |     |
|     |     | Fault Num | 1byte |     |     |
|     |     | Fault Code | 1byte |     |     |

###

### PCB Artwork (26.04.13. Update)

![](file:///C:/Users/Han%20Gyujin/AppData/Roaming/marktext/images/2026-05-11-21-10-33-image.png?msec=1778501437729)![](file:///C:/Users/Han%20Gyujin/AppData/Roaming/marktext/images/2026-05-11-21-10-50-image.png?msec=1778501459337)

### LED

| **Status Bits** | **LED 1** | **LED 0** | **Status Description** |
| --- | --- | --- | --- |
| 00  | Off | Off | **Normal Operation** |
| 01  | On  | Off | **Error A** |
| 10  | Off | On  | **Error B** |
| 11  | On  | On  | **Fatal Error** |

### Error Code

| **Section** | **Classification** | Remarks |
| --- | --- | --- |
| **00** |     |     |
| **0x** |     |     |
| **1x** |     |     |
| **2x** |     |     |
| **3x** |     |     |
| **4x** |     |     |
| **5x** |     |     |
| **6x ~ 7x** |     |     |
| **8x** |     |     |
| **9x** |     |     |

### How to run

1. ST-LINK를 통해 보드를 PC와 연결합니다.
  
2. Tera Term 또는 PuTTY 등 시리얼 모니터 프로그램을 실행합니다.
  
  **Baud Rate:** `115200`
  
3. 보드에 전원을 인가하거나 리셋 버튼을 누르면 초기화가 진행됩니다.
  
4. 1초마다 터미널 화면에 송신 완료 메시지가 출력되며, 특정 ID(`0x050`, `0x011`) 수신 시 즉시 해당 데이터가 화면에 표시됩니다.
