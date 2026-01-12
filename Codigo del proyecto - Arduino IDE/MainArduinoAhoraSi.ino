#include <Arduino.h>
#include <stdio.h>
#include <U8g2lib.h>
#include "menu.h"
#include "iconos.h"
#include "Servo.h"

//SOLO PINES--------------------------------

//Reservados para la pantalla OLED 2.42 I2C

//#define D20 20 //SDA
//#define D21 21 //SCL

//SALIDAS DE VOLTAJE MOSFET
#define POSITIVE_15V_MOSFET_REG 23
#define NEGATIVE_15V_MOSFET_REG 24
#define POSITIVE_5V_REG 25
#define POSITIVE_5V_MEGA2560_MOSFET 26

//SWITCHES CHERRY MX
#define BOTON_UP_PIN 18     // INT4
#define BOTON_SELECT_PIN 2  // INT0
#define BOTON_DOWN_PIN 19   // INT5

//CONMUTADOR
#define RLV200 31
#define RLV201 32
#define RLV202 33
#define RLV203 34
#define RLV204 35

//PROBADOR 555
#define D41 41

//PROBADOR 741
#define D42 42
#define D43 43
//#define A6 A6

//PROBADOR SERVO MOTOR
#define D45 45

//PROBADOR DIODOS/LED
#define D47 47
#define D48 48
//#define A7 A7
//#define A8 A8

//PROBADOR RESISTENCIAS
#define D49 49
#define D50 50
#define D51 51
#define D52 52
#define D53 53
//#define A9 A9

//PROBADOR MOTOR A PASOS
#define D36 36
#define D37 37
#define D38 38
#define D39 39

//PROBADOR CAPACITORES
#define D28 28
#define D29 29
//#define A5 A5

//PROBADOR DE COMPUERTAS LOGICAS

#define Pin1Compuerta 4
#define Pin2Compuerta 5
#define Pin3Compuerta 6
#define Pin4Compuerta 7
#define Pin5Compuerta 8
#define Pin6Compuerta 9
#define Pin8Compuerta 10
#define Pin9Compuerta 11
#define Pin10Compuerta 12
#define Pin11Compuerta 13
#define Pin12Compuerta 14
#define Pin13Compuerta 15

//PINES PARA REGULADORES DE VOLTAJE LINEALES
#define pinLM7805 A0
#define pinLM7812 A1
#define pinLM317 A2
#define pinLM7912 A3
#define pinLM337 A4

//SOLO PINES---------------------------------

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

Servo motor;  // Objeto Servo global

const unsigned char* bitmap_icons[19] = {
    epd_bitmap__icono_555,
    epd_bitmap__icono_741,
    epd_bitmap__icono_74LS00,
    epd_bitmap__icono_74LS02,
    epd_bitmap__icono_74LS04,
    epd_bitmap__icono_74LS08,
    epd_bitmap__icono_74LS32,
    epd_bitmap__icono_74LS86,
    epd_bitmap__icono_LM317,
    epd_bitmap__icono_LM337,
    epd_bitmap__icono_LM7805,
    epd_bitmap__icono_LM7812,
    epd_bitmap__icono_LM7912,
    epd_bitmap__icono_capacitor,
    epd_bitmap__icono_diodo,
    epd_bitmap__icono_resistencia,
    epd_bitmap__icono_motor,  //servo
    epd_bitmap__icono_motor,  //stepper
    epd_bitmap__icono_signo_interrogacion,
};

const int NUM_ITEMS = 19;
const int CARACTERES_MAXIMOS = 20;

char menu_items[NUM_ITEMS][CARACTERES_MAXIMOS] = {
    {"NE555"},
    {"LM741"},
    {"74LS00 NAND"},
    {"74LS02 NOR"},
    {"74LS04 NOT"},
    {"74LS08 AND"},
    {"74LS32 OR"},
    {"74LS86 XOR"},
    {"LM317"},
    {"LM337"},
    {"LM7805"},
    {"LM7812"},
    {"LM7912"},
    {"Capacitor"},
    {"Diodo/LED"},
    {"Resistencia"},
    {"Servomotor"},
    {"Motor a pasos"},
    {"Acerca de"},
};

volatile bool flag_up = false;
volatile bool flag_down = false;
volatile bool flag_select = false;

int pantalla_actual = 0;

int item_seleccionado = 0;
int item_sel_previo;
int item_sel_siguiente;

void prueba_555();
void prueba_741();
void prueba_compuertas_logicas();
void prueba_LED();
void prueba_reguladores();
void prueba_capacitor();
void prueba_diodo();
void prueba_resistencia();
void prueba_servomotor();
void prueba_stepper();
void prueba_creditos();

void (*pruebas_electricas[19])() = {
    prueba_555,
    prueba_741,
    prueba_compuertas_logicas,
    prueba_compuertas_logicas,
    prueba_compuertas_logicas,
    prueba_compuertas_logicas,
    prueba_compuertas_logicas,
    prueba_compuertas_logicas,
    prueba_reguladores,
    prueba_reguladores,
    prueba_reguladores,
    prueba_reguladores,
    prueba_reguladores,
    prueba_capacitor,
    prueba_diodo,
    prueba_resistencia,
    prueba_servomotor,
    prueba_stepper,
    prueba_creditos
};

//---------------- ISR ----------------

void ISR_up() {
    flag_up = true;
}

void ISR_down() {
    flag_down = true;
}

void ISR_select() {
    flag_select = true;
}

void setup() {
    pinMode(BOTON_UP_PIN, INPUT);
    pinMode(BOTON_SELECT_PIN, INPUT);  // Pin 17 (pero conectado físicamente a 2)
    pinMode(BOTON_DOWN_PIN, INPUT);

    // Activar interrupciones en RISING (flanco de subida)
    attachInterrupt(digitalPinToInterrupt(BOTON_UP_PIN), ISR_up, RISING);
    attachInterrupt(digitalPinToInterrupt(BOTON_DOWN_PIN), ISR_down, RISING);
    attachInterrupt(digitalPinToInterrupt(BOTON_SELECT_PIN), ISR_select, RISING);

    pinMode(POSITIVE_15V_MOSFET_REG, OUTPUT);
    pinMode(NEGATIVE_15V_MOSFET_REG, OUTPUT);
    pinMode(POSITIVE_5V_REG, OUTPUT);
    pinMode(POSITIVE_5V_MEGA2560_MOSFET, OUTPUT);
    digitalWrite(POSITIVE_15V_MOSFET_REG, LOW);
    digitalWrite(NEGATIVE_15V_MOSFET_REG, LOW);
    digitalWrite(POSITIVE_5V_REG, LOW);
    digitalWrite(POSITIVE_5V_MEGA2560_MOSFET, LOW);

    pinMode(RLV200, OUTPUT);
    pinMode(RLV201, OUTPUT);
    pinMode(RLV202, OUTPUT);
    pinMode(RLV203, OUTPUT);
    pinMode(RLV204, OUTPUT);

    pinMode(Pin1Compuerta, OUTPUT);
    pinMode(Pin2Compuerta, OUTPUT);
    pinMode(Pin3Compuerta, INPUT_PULLUP);
    pinMode(Pin4Compuerta, OUTPUT);
    pinMode(Pin5Compuerta, OUTPUT);
    pinMode(Pin6Compuerta, INPUT_PULLUP);
    pinMode(Pin8Compuerta, INPUT_PULLUP);
    pinMode(Pin9Compuerta, OUTPUT);
    pinMode(Pin10Compuerta, OUTPUT);
    pinMode(Pin11Compuerta, INPUT_PULLUP);
    pinMode(Pin12Compuerta, OUTPUT);
    pinMode(Pin13Compuerta, OUTPUT);

    pinMode(D28, INPUT);
    pinMode(D29, OUTPUT);

    pinMode(D36, OUTPUT);
    pinMode(D37, OUTPUT);
    pinMode(D38, OUTPUT);
    pinMode(D39, OUTPUT);

    pinMode(D41, INPUT);

    pinMode(D42, OUTPUT);
    pinMode(D43, OUTPUT);

    pinMode(D45, OUTPUT);

    pinMode(D47, OUTPUT);
    pinMode(D48, OUTPUT);
    digitalWrite(D47, LOW);
    digitalWrite(D48, LOW);

    pinMode(D49, OUTPUT);
    pinMode(D50, OUTPUT);
    pinMode(D51, OUTPUT);
    pinMode(D52, OUTPUT);
    pinMode(D53, OUTPUT);

    motor.attach(D45); //servo

    inicializarOLED();
}

void loop() {
    // ---- PROCESAR BOTÓN UP ----
    if (flag_up && digitalRead(BOTON_UP_PIN) == LOW) {
        flag_up = false;
        if (pantalla_actual == 0) {
            item_seleccionado--;
            if (item_seleccionado < 0) item_seleccionado = NUM_ITEMS - 1;
        }
    }

    // ---- PROCESAR BOTÓN DOWN ----
    if (flag_down && digitalRead(BOTON_DOWN_PIN) == LOW) {
        flag_down = false;
        if (pantalla_actual == 0) {
            item_seleccionado++;
            if (item_seleccionado >= NUM_ITEMS) item_seleccionado = 0;
        }
    }

    // ---- PROCESAR BOTÓN SELECT ----
    if (flag_select && digitalRead(BOTON_SELECT_PIN) == LOW) {
        flag_select = false;
        pantalla_actual ^= 1;   // cambia entre menú y prueba
    }

    if (pantalla_actual == 0) {
    motor.detach();
    desactivar_MOSFETS();
    desactivar_conmutador();
    }

    // ---- ÍNDICES DEL MENÚ ----
    item_sel_previo = item_seleccionado - 1;
    if (item_sel_previo < 0) item_sel_previo = NUM_ITEMS - 1;

    item_sel_siguiente = item_seleccionado + 1;
    if (item_sel_siguiente >= NUM_ITEMS) item_sel_siguiente = 0;

    u8g2.firstPage();
    do {
        if (pantalla_actual == 0) {
            u8g2.drawXBMP(0,0,5,5,epd_bitmap__contorno_esq_sup_izq);
            u8g2.drawXBMP(123,0,5,5,epd_bitmap__contorno_esq_sup_der);
            u8g2.drawXBMP(0,59,5,5,epd_bitmap__contorno_esq_inf_izq);
            u8g2.drawXBMP(123,59,5,5,epd_bitmap__contorno_esq_inf_der);
            u8g2.drawXBMP(5,0,118,1,epd_bitmap__contorno_linea_sup);
            u8g2.drawXBMP(5,63,118,1,epd_bitmap__contorno_linea_inf);
            u8g2.drawXBMP(0,5,1,54,epd_bitmap__contorno_linea_izq);
            u8g2.drawXBMP(127,5,1,54,epd_bitmap__contorno_linea_der);
            
            u8g2.drawXBMP(122,5,3,53,epd_bitmap__scroll_background);
            u8g2.drawXBMP(2,22,119,21,epd_bitmap__seleccion_de_item_background);
            u8g2.drawBox(122, (53/(NUM_ITEMS-2))*item_seleccionado + 4, 3, 53/NUM_ITEMS);

            u8g2.setFont(u8g2_font_7x14_tf); 
            u8g2.drawStr(25, 16, menu_items[item_sel_previo]);
            u8g2.drawStr(25, 37, menu_items[item_seleccionado]);
            u8g2.drawStr(25, 58, menu_items[item_sel_siguiente]);

            u8g2.drawXBMP(5,3,16,16,bitmap_icons[item_sel_previo]); 
            u8g2.drawXBMP(5,24,16,16,bitmap_icons[item_seleccionado]);   
            u8g2.drawXBMP(5,45,16,16,bitmap_icons[item_sel_siguiente]);   
        } else {
            pruebas_electricas[item_seleccionado]();
        }
    } while (u8g2.nextPage());
}



void prueba_reguladores() {
    static float inputVoltage = 0.0;
    static float inputCurrent = 0.0;
    float R_reg = 0.00001;
    static unsigned long ultimo_muestreo = 0;
    const unsigned long intervalo_muestreo = 500;
    const float Vmin = -15.0;
    const float Vmax = 15.0;
    int analogPin = -1;
    const char* nombre_componente = "";
    const char* mensaje_medicion = "";
    bool es_variable = false;

    switch (item_seleccionado) {
        case 8:
            analogPin = pinLM317;
            nombre_componente = "LM317";
            mensaje_medicion = "OBJETIVO: 1.2V a 13V";
            es_variable = true;
            R_reg = 200.0;
            digitalWrite(POSITIVE_15V_MOSFET_REG, HIGH);
            break;
        case 9:
            analogPin = pinLM337;
            nombre_componente = "LM337";
            mensaje_medicion = "OBJETIVO: -1.2V a -13V";
            es_variable = true;
            R_reg = 200.0;
            digitalWrite(NEGATIVE_15V_MOSFET_REG, HIGH);
            break;
        case 10:
            analogPin = pinLM7805;
            nombre_componente = "LM7805";
            mensaje_medicion = "OBJETIVO: 5V";
            es_variable = false;
            R_reg = 97.0;
            digitalWrite(POSITIVE_15V_MOSFET_REG, HIGH);
            break;
        case 11:
            analogPin = pinLM7812;
            nombre_componente = "LM7812";
            mensaje_medicion = "OBJETIVO: 12V";
            es_variable = false;
            R_reg = 200.0;
            digitalWrite(POSITIVE_15V_MOSFET_REG, HIGH);
            break;
        case 12:
            analogPin = pinLM7912;
            nombre_componente = "LM7912";
            mensaje_medicion = "OBJETIVO: -12V";
            es_variable = false;
            R_reg = 200.0;
            digitalWrite(NEGATIVE_15V_MOSFET_REG, HIGH);
            break;
        default:
            return;
    }

    unsigned long tiempo_actual = millis();
    if (tiempo_actual - ultimo_muestreo >= intervalo_muestreo) {
        int sensorValue = analogRead(analogPin);
        float voltageADC = sensorValue * (5.0 / 1023.0);
        inputVoltage = mapFloat(voltageADC, 0.17, 4.17, Vmin, Vmax);
        inputCurrent = inputVoltage / R_reg;
        ultimo_muestreo = tiempo_actual;
    }

    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(44, 13, nombre_componente);

    u8g2.setFont(u8g2_font_5x7_tf);
    if (es_variable) {
        u8g2.drawStr(85, 30, "MUEVA EL");
        u8g2.drawStr(96, 40, "POT");
    }

    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(0, 32, "V:");
    u8g2.setCursor(15, 32);
    u8g2.print(inputVoltage, 3);
    u8g2.drawStr(65, 32, "V");

    u8g2.drawStr(0, 47, "I:");
    u8g2.setCursor(15, 47);
    u8g2.print(inputCurrent, 3);
    u8g2.drawStr(60, 47, "A");

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0, 64, mensaje_medicion);
}

void defaultt() {
  pinMode(Pin1Compuerta, OUTPUT);
  pinMode(Pin2Compuerta, OUTPUT);
  pinMode(Pin3Compuerta, INPUT_PULLUP);
  pinMode(Pin4Compuerta, OUTPUT);
  pinMode(Pin5Compuerta, OUTPUT);
  pinMode(Pin6Compuerta, INPUT_PULLUP);
  pinMode(Pin8Compuerta, INPUT_PULLUP);
  pinMode(Pin9Compuerta, OUTPUT);
  pinMode(Pin10Compuerta, OUTPUT);
  pinMode(Pin11Compuerta, INPUT_PULLUP);
  pinMode(Pin12Compuerta, OUTPUT);
  pinMode(Pin13Compuerta, OUTPUT);
}

void CasoEspecialNOT() { // Configura pines para el 74LS04 (6 compuertas NOT)
  pinMode(Pin1Compuerta, OUTPUT);
  pinMode(Pin2Compuerta, INPUT_PULLUP);
  pinMode(Pin3Compuerta, OUTPUT);
  pinMode(Pin4Compuerta, INPUT_PULLUP);
  pinMode(Pin5Compuerta, OUTPUT);
  pinMode(Pin6Compuerta, INPUT_PULLUP);
  pinMode(Pin8Compuerta, INPUT_PULLUP);
  pinMode(Pin9Compuerta, OUTPUT);
  pinMode(Pin10Compuerta, INPUT_PULLUP);
  pinMode(Pin11Compuerta, OUTPUT);
  pinMode(Pin12Compuerta, INPUT_PULLUP);
  pinMode(Pin13Compuerta, OUTPUT);
}

void CasoEspecialNOR() { // Configura pines para el 74LS02 (Compuerta NOR estan voltadas las entradas y salidas)
  pinMode(Pin1Compuerta, INPUT_PULLUP);
  pinMode(Pin2Compuerta, OUTPUT);
  pinMode(Pin3Compuerta, OUTPUT);
  pinMode(Pin4Compuerta, INPUT_PULLUP);
  pinMode(Pin5Compuerta, OUTPUT);
  pinMode(Pin6Compuerta, OUTPUT);
  pinMode(Pin8Compuerta, OUTPUT);
  pinMode(Pin9Compuerta, OUTPUT);
  pinMode(Pin10Compuerta, INPUT_PULLUP);
  pinMode(Pin11Compuerta, OUTPUT);
  pinMode(Pin12Compuerta, OUTPUT);
  pinMode(Pin13Compuerta, INPUT_PULLUP);
}

// Función principal para compuertas lógicas
void prueba_compuertas_logicas() {
    const char* nombre_componente = "";

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(10, 60, "Espere 4-5s por prueba");

    switch (item_seleccionado) {
        case 2: 
        nombre_componente = "74LS00 NAND"; 
        digitalWrite(POSITIVE_5V_MEGA2560_MOSFET, HIGH); 
        break;
        case 3: 
        nombre_componente = "74LS02 NOR"; 
        digitalWrite(POSITIVE_5V_MEGA2560_MOSFET, HIGH); 
        CasoEspecialNOR(); 
        break;
        case 4: nombre_componente = "74LS04 NOT"; 
        digitalWrite(POSITIVE_5V_MEGA2560_MOSFET, HIGH); 
        CasoEspecialNOT(); 
        break;
        case 5: 
        nombre_componente = "74LS08 AND"; 
        digitalWrite(POSITIVE_5V_MEGA2560_MOSFET, HIGH); 
        break;
        case 6: 
        nombre_componente = "74LS32 OR"; 
        digitalWrite(POSITIVE_5V_MEGA2560_MOSFET, HIGH); 
        break;
        case 7: 
        nombre_componente = "74LS86 XOR"; 
        digitalWrite(POSITIVE_5V_MEGA2560_MOSFET, HIGH); 
        break;
        default: 
        return;
    }

    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(30, 13, nombre_componente);



    if (item_seleccionado == 2) {
        u8g2.setFont(u8g2_font_5x7_tf);
        u8g2.drawStr(0, 25, "Gate 1:");
        u8g2.drawStr(64, 25, "Gate 4:");
        u8g2.drawStr(0, 35, "Gate 2:");
        u8g2.drawStr(64, 35, "Gate 3:");
        pruebaNAND();
    } else if (item_seleccionado == 3) {
        u8g2.setFont(u8g2_font_5x7_tf);
        u8g2.drawStr(0, 25, "Gate 1:");
        u8g2.drawStr(64, 25, "Gate 4:");
        u8g2.drawStr(0, 35, "Gate 2:");
        u8g2.drawStr(64, 35, "Gate 3:");
        pruebaNOR();
    } else if (item_seleccionado == 4) {
        u8g2.setFont(u8g2_font_5x7_tf);
        u8g2.drawStr(0, 25, "Gate 1:");
        u8g2.drawStr(64, 25, "Gate 6:");
        u8g2.drawStr(0, 35, "Gate 2:");
        u8g2.drawStr(64, 35, "Gate 5:");
        u8g2.drawStr(0, 45, "Gate 3:");
        u8g2.drawStr(64, 45, "Gate 4:");
        pruebaNOT();
    } else if (item_seleccionado == 5) {
        u8g2.setFont(u8g2_font_5x7_tf);
        u8g2.drawStr(0, 25, "Gate 1:");
        u8g2.drawStr(64, 25, "Gate 4:");
        u8g2.drawStr(0, 35, "Gate 2:");
        u8g2.drawStr(64, 35, "Gate 3:");
        pruebaAND();
    } else if (item_seleccionado == 6) {
        u8g2.setFont(u8g2_font_5x7_tf);
        u8g2.drawStr(0, 25, "Gate 1:");
        u8g2.drawStr(64, 25, "Gate 4:");
        u8g2.drawStr(0, 35, "Gate 2:");
        u8g2.drawStr(64, 35, "Gate 3:");
        pruebaOR();
    } else if (item_seleccionado == 7) {
        u8g2.setFont(u8g2_font_5x7_tf);
        u8g2.drawStr(0, 25, "Gate 1:");
        u8g2.drawStr(64, 25, "Gate 4:");
        u8g2.drawStr(0, 35, "Gate 2:");
        u8g2.drawStr(64, 35, "Gate 3:");
        pruebaXOR();
    }
}

void pruebaNAND() {
    static bool gateOK[4] = {false, false, false, false};
    static unsigned long ultimo_muestreo = 0;

    static int iter = 0;
    static int combo = 0;
    static int rep1 = 0, rep2 = 0, rep3 = 0, rep4 = 0;
    static bool consistente1 = true, consistente2 = true, consistente3 = true, consistente4 = true;
    static int PruebaSalida1 = 0, PruebaSalida2 = 0, PruebaSalida3 = 0, PruebaSalida4 = 0;

    // Para mostrar "TEST" y pausa de 1.5s
    static bool mostrando_resultado = false;
    static unsigned long tiempo_resultado = 0;
    const unsigned long duracion_resultado = 1500;  // 1.5 segundos

    unsigned long tiempo_actual = millis();

    if (mostrando_resultado) {
        // Durante 1.5s mostrar resultado final
        if (tiempo_actual - tiempo_resultado >= duracion_resultado) {
            mostrando_resultado = false;
            iter = 0;
            consistente1 = consistente2 = consistente3 = consistente4 = true;
        }
    } else {
        // Prueba normal
        if (tiempo_actual - ultimo_muestreo >= 1) {
            if (combo == 0) {
                digitalWrite(Pin1Compuerta, LOW); digitalWrite(Pin2Compuerta, LOW);
                digitalWrite(Pin4Compuerta, LOW); digitalWrite(Pin5Compuerta, LOW);
                digitalWrite(Pin9Compuerta, LOW); digitalWrite(Pin10Compuerta, LOW);
                digitalWrite(Pin12Compuerta, LOW); digitalWrite(Pin13Compuerta, LOW);
            } else if (combo == 1) {
                digitalWrite(Pin1Compuerta, HIGH); digitalWrite(Pin2Compuerta, LOW);
                digitalWrite(Pin4Compuerta, HIGH); digitalWrite(Pin5Compuerta, LOW);
                digitalWrite(Pin9Compuerta, LOW); digitalWrite(Pin10Compuerta, HIGH);
                digitalWrite(Pin12Compuerta, LOW); digitalWrite(Pin13Compuerta, HIGH);
            } else if (combo == 2) {
                digitalWrite(Pin1Compuerta, LOW); digitalWrite(Pin2Compuerta, HIGH);
                digitalWrite(Pin4Compuerta, LOW); digitalWrite(Pin5Compuerta, HIGH);
                digitalWrite(Pin9Compuerta, HIGH); digitalWrite(Pin10Compuerta, LOW);
                digitalWrite(Pin12Compuerta, HIGH); digitalWrite(Pin13Compuerta, LOW);
            } else if (combo == 3) {
                digitalWrite(Pin1Compuerta, HIGH); digitalWrite(Pin2Compuerta, HIGH);
                digitalWrite(Pin4Compuerta, HIGH); digitalWrite(Pin5Compuerta, HIGH);
                digitalWrite(Pin9Compuerta, HIGH); digitalWrite(Pin10Compuerta, HIGH);
                digitalWrite(Pin12Compuerta, HIGH); digitalWrite(Pin13Compuerta, HIGH);
            }

            if (combo >= 0 && combo <= 3) {
                if (digitalRead(Pin3Compuerta) == (combo == 3 ? LOW : HIGH)) PruebaSalida1++;
                if (digitalRead(Pin6Compuerta) == (combo == 3 ? LOW : HIGH)) PruebaSalida2++;
                if (digitalRead(Pin8Compuerta) == (combo == 3 ? LOW : HIGH)) PruebaSalida3++;
                if (digitalRead(Pin11Compuerta) == (combo == 3 ? LOW : HIGH)) PruebaSalida4++;
            }

            combo++;

            if (combo > 3) {
                if (iter == 0) {
                    rep1 = PruebaSalida1; rep2 = PruebaSalida2; rep3 = PruebaSalida3; rep4 = PruebaSalida4;
                } else {
                    if (PruebaSalida1 != rep1) consistente1 = false;
                    if (PruebaSalida2 != rep2) consistente2 = false;
                    if (PruebaSalida3 != rep3) consistente3 = false;
                    if (PruebaSalida4 != rep4) consistente4 = false;
                }

                iter++;
                combo = 0;
                PruebaSalida1 = 0; PruebaSalida2 = 0; PruebaSalida3 = 0; PruebaSalida4 = 0;

                if (iter >= 40) {
                    gateOK[0] = consistente1 && rep1 == 4;
                    gateOK[1] = consistente2 && rep2 == 4;
                    gateOK[2] = consistente3 && rep3 == 4;
                    gateOK[3] = consistente4 && rep4 == 4;

                    mostrando_resultado = true;
                    tiempo_resultado = tiempo_actual;
                }
            }

            ultimo_muestreo = tiempo_actual;
        }
    }

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(35, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[0] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[1] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(35, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[2] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[3] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    defaultt();
}

void pruebaNOR() {
    static bool gateOK[4] = {false, false, false, false};
    static unsigned long ultimo_muestreo = 0;

    static int iter = 0;
    static int combo = 0;
    static int rep1 = 0, rep2 = 0, rep3 = 0, rep4 = 0;
    static bool consistente1 = true, consistente2 = true, consistente3 = true, consistente4 = true;
    static int PruebaSalida1 = 0, PruebaSalida2 = 0, PruebaSalida3 = 0, PruebaSalida4 = 0;

    // Para mostrar "TEST" y pausa de 1.5s
    static bool mostrando_resultado = false;
    static unsigned long tiempo_resultado = 0;
    const unsigned long duracion_resultado = 1500;  // 1.5 segundos

    unsigned long tiempo_actual = millis();

    if (mostrando_resultado) {
        // Durante 1.5s mostrar resultado final
        if (tiempo_actual - tiempo_resultado >= duracion_resultado) {
            mostrando_resultado = false;
            iter = 0;
            consistente1 = consistente2 = consistente3 = consistente4 = true;
        }
    } else {
        // Prueba normal
        if (tiempo_actual - ultimo_muestreo >= 1) {
            if (combo == 0) {
                digitalWrite(Pin2Compuerta, LOW); digitalWrite(Pin3Compuerta, LOW);
                digitalWrite(Pin5Compuerta, LOW); digitalWrite(Pin6Compuerta, LOW);
                digitalWrite(Pin8Compuerta, LOW); digitalWrite(Pin9Compuerta, LOW);
                digitalWrite(Pin11Compuerta, LOW); digitalWrite(Pin12Compuerta, LOW);
            } else if (combo == 1) {
                digitalWrite(Pin2Compuerta, HIGH); digitalWrite(Pin3Compuerta, LOW);
                digitalWrite(Pin5Compuerta, HIGH); digitalWrite(Pin6Compuerta, LOW);
                digitalWrite(Pin8Compuerta, LOW); digitalWrite(Pin9Compuerta, HIGH);
                digitalWrite(Pin11Compuerta, LOW); digitalWrite(Pin12Compuerta, HIGH);
            } else if (combo == 2) {
                digitalWrite(Pin2Compuerta, LOW); digitalWrite(Pin3Compuerta, HIGH);
                digitalWrite(Pin5Compuerta, LOW); digitalWrite(Pin6Compuerta, HIGH);
                digitalWrite(Pin8Compuerta, HIGH); digitalWrite(Pin9Compuerta, LOW);
                digitalWrite(Pin11Compuerta, HIGH); digitalWrite(Pin12Compuerta, LOW);
            } else if (combo == 3) {
                digitalWrite(Pin2Compuerta, HIGH); digitalWrite(Pin3Compuerta, HIGH);
                digitalWrite(Pin5Compuerta, HIGH); digitalWrite(Pin6Compuerta, HIGH);
                digitalWrite(Pin8Compuerta, HIGH); digitalWrite(Pin9Compuerta, HIGH);
                digitalWrite(Pin11Compuerta, HIGH); digitalWrite(Pin12Compuerta, HIGH);
            }

            if (combo >= 0 && combo <= 3) {
                if (digitalRead(Pin1Compuerta) == (combo == 0 ? HIGH : LOW)) PruebaSalida1++;
                if (digitalRead(Pin4Compuerta) == (combo == 0 ? HIGH : LOW)) PruebaSalida2++;
                if (digitalRead(Pin10Compuerta) == (combo == 0 ? HIGH : LOW)) PruebaSalida3++;
                if (digitalRead(Pin13Compuerta) == (combo == 0 ? HIGH : LOW)) PruebaSalida4++;
            }

            combo++;

            if (combo > 3) {
                if (iter == 0) {
                    rep1 = PruebaSalida1; rep2 = PruebaSalida2; rep3 = PruebaSalida3; rep4 = PruebaSalida4;
                } else {
                    if (PruebaSalida1 != rep1) consistente1 = false;
                    if (PruebaSalida2 != rep2) consistente2 = false;
                    if (PruebaSalida3 != rep3) consistente3 = false;
                    if (PruebaSalida4 != rep4) consistente4 = false;
                }

                iter++;
                combo = 0;
                PruebaSalida1 = 0; PruebaSalida2 = 0; PruebaSalida3 = 0; PruebaSalida4 = 0;

                if (iter >= 40) {
                    gateOK[0] = consistente1 && rep1 == 4;
                    gateOK[1] = consistente2 && rep2 == 4;
                    gateOK[2] = consistente3 && rep3 == 4;
                    gateOK[3] = consistente4 && rep4 == 4;

                    mostrando_resultado = true;
                    tiempo_resultado = tiempo_actual;
                }
            }

            ultimo_muestreo = tiempo_actual;
        }
    }

    // Dibujo (tus setCursor originales intactos)
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(35, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[0] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[1] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(35, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[2] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[3] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    defaultt();
}

void pruebaNOT() {
    static bool gateOK[6] = {false, false, false, false, false, false};
    static unsigned long ultimo_muestreo = 0;

    static int iter = 0;
    static int combo = 0;
    static int rep1 = 0, rep2 = 0, rep3 = 0, rep4 = 0, rep5 = 0, rep6 = 0;
    static bool consistente1 = true, consistente2 = true, consistente3 = true, consistente4 = true, consistente5 = true, consistente6 = true;
    static int PruebaSalida1 = 0, PruebaSalida2 = 0, PruebaSalida3 = 0, PruebaSalida4 = 0, PruebaSalida5 = 0, PruebaSalida6 = 0;

    // Para mostrar "TEST" y pausa de 1.5s
    static bool mostrando_resultado = false;
    static unsigned long tiempo_resultado = 0;
    const unsigned long duracion_resultado = 1500;  // 1.5 segundos

    unsigned long tiempo_actual = millis();

    if (mostrando_resultado) {
        // Durante 1.5s mostrar resultado final
        if (tiempo_actual - tiempo_resultado >= duracion_resultado) {
            mostrando_resultado = false;
            iter = 0;
            consistente1 = consistente2 = consistente3 = consistente4 = consistente5 = consistente6 = true;
        }
    } else {
        // Prueba normal
        if (tiempo_actual - ultimo_muestreo >= 1) {
            // Entradas para NOT (solo HIGH o LOW)
            bool input = (combo == 0) ? LOW : HIGH;

            digitalWrite(Pin1Compuerta, input);  
            digitalWrite(Pin3Compuerta, input);  
            digitalWrite(Pin5Compuerta, input);  
            digitalWrite(Pin9Compuerta, input);  
            digitalWrite(Pin11Compuerta, input); 
            digitalWrite(Pin13Compuerta, input); 

            // Lectura (espera salida opuesta)
            bool expected = (combo == 0) ? HIGH : LOW;

            if (digitalRead(Pin2Compuerta) == expected) PruebaSalida1++;
            if (digitalRead(Pin4Compuerta) == expected) PruebaSalida2++;
            if (digitalRead(Pin6Compuerta) == expected) PruebaSalida3++;
            if (digitalRead(Pin8Compuerta) == expected) PruebaSalida4++;
            if (digitalRead(Pin10Compuerta) == expected) PruebaSalida5++;
            if (digitalRead(Pin12Compuerta) == expected) PruebaSalida6++;

            combo++;

            if (combo > 1) {
                if (iter == 0) {
                    rep1 = PruebaSalida1; rep2 = PruebaSalida2; rep3 = PruebaSalida3; 
                    rep4 = PruebaSalida4; rep5 = PruebaSalida5; rep6 = PruebaSalida6;
                } else {
                    if (PruebaSalida1 != rep1) consistente1 = false;
                    if (PruebaSalida2 != rep2) consistente2 = false;
                    if (PruebaSalida3 != rep3) consistente3 = false;
                    if (PruebaSalida4 != rep4) consistente4 = false;
                    if (PruebaSalida5 != rep5) consistente5 = false;
                    if (PruebaSalida6 != rep6) consistente6 = false;
                }

                iter++;
                combo = 0;
                PruebaSalida1 = 0; PruebaSalida2 = 0; PruebaSalida3 = 0; PruebaSalida4 = 0; PruebaSalida5 = 0; PruebaSalida6 = 0;

                if (iter >= 40) {
                    gateOK[0] = consistente1 && rep1 == 2;
                    gateOK[1] = consistente2 && rep2 == 2;
                    gateOK[2] = consistente3 && rep3 == 2;
                    gateOK[3] = consistente4 && rep4 == 2;
                    gateOK[4] = consistente5 && rep5 == 2;
                    gateOK[5] = consistente6 && rep6 == 2;

                    mostrando_resultado = true;
                    tiempo_resultado = tiempo_actual;
                }
            }

            ultimo_muestreo = tiempo_actual;
        }
    }

    // Dibujo (tus setCursor originales intactos)
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(35, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[0] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(35, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[1] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(35, 45);
    if (mostrando_resultado) {
        u8g2.print(gateOK[2] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 45);
    if (mostrando_resultado) {
        u8g2.print(gateOK[3] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[4] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[5] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    defaultt();
}


void pruebaAND() {
    static bool gateOK[4] = {false, false, false, false};
    static unsigned long ultimo_muestreo = 0;

    static int iter = 0;
    static int combo = 0;
    static int rep1 = 0, rep2 = 0, rep3 = 0, rep4 = 0;
    static bool consistente1 = true, consistente2 = true, consistente3 = true, consistente4 = true;
    static int PruebaSalida1 = 0, PruebaSalida2 = 0, PruebaSalida3 = 0, PruebaSalida4 = 0;

    // Para mostrar "TEST" y pausa de 1.5s
    static bool mostrando_resultado = false;
    static unsigned long tiempo_resultado = 0;
    const unsigned long duracion_resultado = 1500;  // 1.5 segundos

    unsigned long tiempo_actual = millis();

    if (mostrando_resultado) {
        // Durante 1.5s mostrar resultado final
        if (tiempo_actual - tiempo_resultado >= duracion_resultado) {
            mostrando_resultado = false;
            iter = 0;
            consistente1 = consistente2 = consistente3 = consistente4 = true;
        }
    } else {
        // Prueba normal
        if (tiempo_actual - ultimo_muestreo >= 1) {
            if (combo == 0) {
                digitalWrite(Pin1Compuerta, LOW); digitalWrite(Pin2Compuerta, LOW);
                digitalWrite(Pin4Compuerta, LOW); digitalWrite(Pin5Compuerta, LOW);
                digitalWrite(Pin9Compuerta, LOW); digitalWrite(Pin10Compuerta, LOW);
                digitalWrite(Pin12Compuerta, LOW); digitalWrite(Pin13Compuerta, LOW);
            } else if (combo == 1) {
                digitalWrite(Pin1Compuerta, HIGH); digitalWrite(Pin2Compuerta, LOW);
                digitalWrite(Pin4Compuerta, HIGH); digitalWrite(Pin5Compuerta, LOW);
                digitalWrite(Pin9Compuerta, LOW); digitalWrite(Pin10Compuerta, HIGH);
                digitalWrite(Pin12Compuerta, LOW); digitalWrite(Pin13Compuerta, HIGH);
            } else if (combo == 2) {
                digitalWrite(Pin1Compuerta, LOW); digitalWrite(Pin2Compuerta, HIGH);
                digitalWrite(Pin4Compuerta, LOW); digitalWrite(Pin5Compuerta, HIGH);
                digitalWrite(Pin9Compuerta, HIGH); digitalWrite(Pin10Compuerta, LOW);
                digitalWrite(Pin12Compuerta, HIGH); digitalWrite(Pin13Compuerta, LOW);
            } else if (combo == 3) {
                digitalWrite(Pin1Compuerta, HIGH); digitalWrite(Pin2Compuerta, HIGH);
                digitalWrite(Pin4Compuerta, HIGH); digitalWrite(Pin5Compuerta, HIGH);
                digitalWrite(Pin9Compuerta, HIGH); digitalWrite(Pin10Compuerta, HIGH);
                digitalWrite(Pin12Compuerta, HIGH); digitalWrite(Pin13Compuerta, HIGH);
            }

            if (combo >= 0 && combo <= 3) {
                if (digitalRead(Pin3Compuerta) == (combo == 3 ? HIGH : LOW)) PruebaSalida1++;
                if (digitalRead(Pin6Compuerta) == (combo == 3 ? HIGH : LOW)) PruebaSalida2++;
                if (digitalRead(Pin8Compuerta) == (combo == 3 ? HIGH : LOW)) PruebaSalida3++;
                if (digitalRead(Pin11Compuerta) == (combo == 3 ? HIGH : LOW)) PruebaSalida4++;
            }

            combo++;

            if (combo > 3) {
                if (iter == 0) {
                    rep1 = PruebaSalida1; rep2 = PruebaSalida2; rep3 = PruebaSalida3; rep4 = PruebaSalida4;
                } else {
                    if (PruebaSalida1 != rep1) consistente1 = false;
                    if (PruebaSalida2 != rep2) consistente2 = false;
                    if (PruebaSalida3 != rep3) consistente3 = false;
                    if (PruebaSalida4 != rep4) consistente4 = false;
                }

                iter++;
                combo = 0;
                PruebaSalida1 = 0; PruebaSalida2 = 0; PruebaSalida3 = 0; PruebaSalida4 = 0;

                if (iter >= 40) {
                    gateOK[0] = consistente1 && rep1 == 4;
                    gateOK[1] = consistente2 && rep2 == 4;
                    gateOK[2] = consistente3 && rep3 == 4;
                    gateOK[3] = consistente4 && rep4 == 4;

                    mostrando_resultado = true;
                    tiempo_resultado = tiempo_actual;
                }
            }

            ultimo_muestreo = tiempo_actual;
        }
    }

    // Dibuja resultados continuamente (tus setCursor originales intactos)
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(35, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[0] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(35, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[1] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[2] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[3] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    defaultt();
}

void pruebaOR() {
    static bool gateOK[4] = {false, false, false, false};
    static unsigned long ultimo_muestreo = 0;

    static int iter = 0;
    static int combo = 0;
    static int rep1 = 0, rep2 = 0, rep3 = 0, rep4 = 0;
    static bool consistente1 = true, consistente2 = true, consistente3 = true, consistente4 = true;
    static int PruebaSalida1 = 0, PruebaSalida2 = 0, PruebaSalida3 = 0, PruebaSalida4 = 0;

    // Para mostrar "TEST" y pausa de 1.5s
    static bool mostrando_resultado = false;
    static unsigned long tiempo_resultado = 0;
    const unsigned long duracion_resultado = 1500;  // 1.5 segundos

    unsigned long tiempo_actual = millis();

    if (mostrando_resultado) {
        // Durante 1.5s mostrar resultado final
        if (tiempo_actual - tiempo_resultado >= duracion_resultado) {
            mostrando_resultado = false;
            iter = 0;
            consistente1 = consistente2 = consistente3 = consistente4 = true;
        }
    } else {
        // Prueba normal
        if (tiempo_actual - ultimo_muestreo >= 1) {
            if (combo == 0) {
                digitalWrite(Pin1Compuerta, LOW); digitalWrite(Pin2Compuerta, LOW);
                digitalWrite(Pin4Compuerta, LOW); digitalWrite(Pin5Compuerta, LOW);
                digitalWrite(Pin9Compuerta, LOW); digitalWrite(Pin10Compuerta, LOW);
                digitalWrite(Pin12Compuerta, LOW); digitalWrite(Pin13Compuerta, LOW);
            } else if (combo == 1) {
                digitalWrite(Pin1Compuerta, HIGH); digitalWrite(Pin2Compuerta, LOW);
                digitalWrite(Pin4Compuerta, HIGH); digitalWrite(Pin5Compuerta, LOW);
                digitalWrite(Pin9Compuerta, LOW); digitalWrite(Pin10Compuerta, HIGH);
                digitalWrite(Pin12Compuerta, LOW); digitalWrite(Pin13Compuerta, HIGH);
            } else if (combo == 2) {
                digitalWrite(Pin1Compuerta, LOW); digitalWrite(Pin2Compuerta, HIGH);
                digitalWrite(Pin4Compuerta, LOW); digitalWrite(Pin5Compuerta, HIGH);
                digitalWrite(Pin9Compuerta, HIGH); digitalWrite(Pin10Compuerta, LOW);
                digitalWrite(Pin12Compuerta, HIGH); digitalWrite(Pin13Compuerta, LOW);
            } else if (combo == 3) {
                digitalWrite(Pin1Compuerta, HIGH); digitalWrite(Pin2Compuerta, HIGH);
                digitalWrite(Pin4Compuerta, HIGH); digitalWrite(Pin5Compuerta, HIGH);
                digitalWrite(Pin9Compuerta, HIGH); digitalWrite(Pin10Compuerta, HIGH);
                digitalWrite(Pin12Compuerta, HIGH); digitalWrite(Pin13Compuerta, HIGH);
            }

            if (combo >= 0 && combo <= 3) {
                if (digitalRead(Pin3Compuerta) == (combo == 0 ? LOW : HIGH)) PruebaSalida1++;
                if (digitalRead(Pin6Compuerta) == (combo == 0 ? LOW : HIGH)) PruebaSalida2++;
                if (digitalRead(Pin8Compuerta) == (combo == 0 ? LOW : HIGH)) PruebaSalida3++;
                if (digitalRead(Pin11Compuerta) == (combo == 0 ? LOW : HIGH)) PruebaSalida4++;
            }

            combo++;

            if (combo > 3) {
                if (iter == 0) {
                    rep1 = PruebaSalida1; rep2 = PruebaSalida2; rep3 = PruebaSalida3; rep4 = PruebaSalida4;
                } else {
                    if (PruebaSalida1 != rep1) consistente1 = false;
                    if (PruebaSalida2 != rep2) consistente2 = false;
                    if (PruebaSalida3 != rep3) consistente3 = false;
                    if (PruebaSalida4 != rep4) consistente4 = false;
                }

                iter++;
                combo = 0;
                PruebaSalida1 = 0; PruebaSalida2 = 0; PruebaSalida3 = 0; PruebaSalida4 = 0;

                if (iter >= 40) {
                    gateOK[0] = consistente1 && rep1 == 4;
                    gateOK[1] = consistente2 && rep2 == 4;
                    gateOK[2] = consistente3 && rep3 == 4;
                    gateOK[3] = consistente4 && rep4 == 4;

                    mostrando_resultado = true;
                    tiempo_resultado = tiempo_actual;
                }
            }

            ultimo_muestreo = tiempo_actual;
        }
    }

    // Dibujo (tus setCursor originales intactos)
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(35, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[0] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[1] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(35, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[2] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[3] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    defaultt();
}

void pruebaXOR() {
    static bool gateOK[4] = {false, false, false, false};
    static unsigned long ultimo_muestreo = 0;

    static int iter = 0;
    static int combo = 0;
    static int rep1 = 0, rep2 = 0, rep3 = 0, rep4 = 0;
    static bool consistente1 = true, consistente2 = true, consistente3 = true, consistente4 = true;
    static int PruebaSalida1 = 0, PruebaSalida2 = 0, PruebaSalida3 = 0, PruebaSalida4 = 0;

    // Para mostrar "TEST" y pausa de 1.5s
    static bool mostrando_resultado = false;
    static unsigned long tiempo_resultado = 0;
    const unsigned long duracion_resultado = 1500;  // 1.5 segundos

    unsigned long tiempo_actual = millis();

    if (mostrando_resultado) {
        // Durante 1.5s mostrar resultado final
        if (tiempo_actual - tiempo_resultado >= duracion_resultado) {
            mostrando_resultado = false;
            iter = 0;
            consistente1 = consistente2 = consistente3 = consistente4 = true;
        }
    } else {
        // Prueba normal
        if (tiempo_actual - ultimo_muestreo >= 1) {
            if (combo == 0) {
                digitalWrite(Pin1Compuerta, LOW); digitalWrite(Pin2Compuerta, LOW);
                digitalWrite(Pin4Compuerta, LOW); digitalWrite(Pin5Compuerta, LOW);
                digitalWrite(Pin9Compuerta, LOW); digitalWrite(Pin10Compuerta, LOW);
                digitalWrite(Pin12Compuerta, LOW); digitalWrite(Pin13Compuerta, LOW);
            } else if (combo == 1) {
                digitalWrite(Pin1Compuerta, HIGH); digitalWrite(Pin2Compuerta, LOW);
                digitalWrite(Pin4Compuerta, HIGH); digitalWrite(Pin5Compuerta, LOW);
                digitalWrite(Pin9Compuerta, LOW); digitalWrite(Pin10Compuerta, HIGH);
                digitalWrite(Pin12Compuerta, LOW); digitalWrite(Pin13Compuerta, HIGH);
            } else if (combo == 2) {
                digitalWrite(Pin1Compuerta, LOW); digitalWrite(Pin2Compuerta, HIGH);
                digitalWrite(Pin4Compuerta, LOW); digitalWrite(Pin5Compuerta, HIGH);
                digitalWrite(Pin9Compuerta, HIGH); digitalWrite(Pin10Compuerta, LOW);
                digitalWrite(Pin12Compuerta, HIGH); digitalWrite(Pin13Compuerta, LOW);
            } else if (combo == 3) {
                digitalWrite(Pin1Compuerta, HIGH); digitalWrite(Pin2Compuerta, HIGH);
                digitalWrite(Pin4Compuerta, HIGH); digitalWrite(Pin5Compuerta, HIGH);
                digitalWrite(Pin9Compuerta, HIGH); digitalWrite(Pin10Compuerta, HIGH);
                digitalWrite(Pin12Compuerta, HIGH); digitalWrite(Pin13Compuerta, HIGH);
            }

            if (combo >= 0 && combo <= 3) {
                if (digitalRead(Pin3Compuerta) == ((combo == 0 || combo == 3) ? LOW : HIGH)) PruebaSalida1++;
                if (digitalRead(Pin6Compuerta) == ((combo == 0 || combo == 3) ? LOW : HIGH)) PruebaSalida2++;
                if (digitalRead(Pin8Compuerta) == ((combo == 0 || combo == 3) ? LOW : HIGH)) PruebaSalida3++;
                if (digitalRead(Pin11Compuerta) == ((combo == 0 || combo == 3) ? LOW : HIGH)) PruebaSalida4++;
            }

            combo++;

            if (combo > 3) {
                if (iter == 0) {
                    rep1 = PruebaSalida1; rep2 = PruebaSalida2; rep3 = PruebaSalida3; rep4 = PruebaSalida4;
                } else {
                    if (PruebaSalida1 != rep1) consistente1 = false;
                    if (PruebaSalida2 != rep2) consistente2 = false;
                    if (PruebaSalida3 != rep3) consistente3 = false;
                    if (PruebaSalida4 != rep4) consistente4 = false;
                }

                iter++;
                combo = 0;
                PruebaSalida1 = 0; PruebaSalida2 = 0; PruebaSalida3 = 0; PruebaSalida4 = 0;

                if (iter >= 40) {
                    gateOK[0] = consistente1 && rep1 == 4;
                    gateOK[1] = consistente2 && rep2 == 4;
                    gateOK[2] = consistente3 && rep3 == 4;
                    gateOK[3] = consistente4 && rep4 == 4;

                    mostrando_resultado = true;
                    tiempo_resultado = tiempo_actual;
                }
            }

            ultimo_muestreo = tiempo_actual;
        }
    }

    // Dibujo (tus setCursor originales intactos)
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(35, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[0] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[1] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(35, 35);
    if (mostrando_resultado) {
        u8g2.print(gateOK[2] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    u8g2.setCursor(100, 25);
    if (mostrando_resultado) {
        u8g2.print(gateOK[3] ? "OK" : "FALLA");
    } else {
        u8g2.print("TEST");
    }

    defaultt();
}

void prueba_servomotor() {
    motor.attach(D45); //servo
    static int angulo = 0;
    static int direccion = 1;  // 1 = subiendo, -1 = bajando
    static unsigned long ultimo_muestreo = 0;
    const unsigned long intervalo_muestreo = 20;  // Velocidad suave (15ms por grado)

    unsigned long tiempo_actual = millis();

    // Activar alimentación del servo
    digitalWrite(POSITIVE_5V_REG, HIGH);

    if (tiempo_actual - ultimo_muestreo >= intervalo_muestreo) {
        motor.write(angulo);

        angulo += direccion;

        if (angulo >= 180) {
            angulo = 180;
            direccion = -1;
        } else if (angulo <= 0) {
            angulo = 0;
            direccion = 1;
        }

        ultimo_muestreo = tiempo_actual;
    }

        // Mostrar en pantalla
        u8g2.setFont(u8g2_font_7x13_tf);
        u8g2.drawStr(30, 13, "Servomotor");

        u8g2.setFont(u8g2_font_7x13_tf);
        u8g2.drawStr(10, 35, "Angulo:");
        u8g2.setCursor(70, 35);
        u8g2.print(angulo);


        u8g2.print((char)176);

        u8g2.setFont(u8g2_font_5x7_tf);
        u8g2.drawStr(20, 55, "-CONECTE SU SERVO-");

}

void prueba_stepper() {
    digitalWrite(POSITIVE_5V_REG, HIGH);  // Activar alimentación del driver 

    static int paso = 0;
    static int direccion = 1;  // 1 = un sentido, -1 = el otro
    static unsigned long ultimo_paso = 0;
    const unsigned long intervalo_paso = 4;  

    static unsigned long tiempo_cambio_dir = 0;
    const unsigned long duracion_giro = 25000;  // 25 segundos por dirección

    unsigned long tiempo_actual = millis();

    // Cambiar dirección cada 10 segundos
    if (tiempo_actual - tiempo_cambio_dir >= duracion_giro) {
        direccion = -direccion;
        tiempo_cambio_dir = tiempo_actual;
    }

    // Avanzar paso
    if (tiempo_actual - ultimo_paso >= intervalo_paso) {
        // Secuencia de paso completo (4 pasos)
        if (paso == 0) {
            digitalWrite(D36, HIGH);  // Bobina 1
            digitalWrite(D37, HIGH);  // Bobina 2
            digitalWrite(D38, LOW);
            digitalWrite(D39, LOW);
        } else if (paso == 1) {
            digitalWrite(D36, LOW);
            digitalWrite(D37, HIGH);  // Bobina 2
            digitalWrite(D38, HIGH);  // Bobina 3
            digitalWrite(D39, LOW);
        } else if (paso == 2) {
            digitalWrite(D36, LOW);
            digitalWrite(D37, LOW);
            digitalWrite(D38, HIGH);  // Bobina 3
            digitalWrite(D39, HIGH);  // Bobina 4
        } else if (paso == 3) {
            digitalWrite(D36, HIGH);  // Bobina 1
            digitalWrite(D37, LOW);
            digitalWrite(D38, LOW);
            digitalWrite(D39, HIGH);  // Bobina 4
        }

        paso += direccion;
        if (paso > 3) paso = 0;
        if (paso < 0) paso = 3;

        ultimo_paso = tiempo_actual;
    }

    // Mostrar en pantalla
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(20, 13, "Motor a pasos");

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(10, 30, "Direccion:");
    u8g2.drawStr(70, 30, direccion == 1 ? "Izquierda" : "Derecha");

    long segundos_restantes = (duracion_giro - (tiempo_actual - tiempo_cambio_dir)) / 1000;
    u8g2.drawStr(10, 45, "Tiempo cambio:");
    u8g2.setCursor(90, 45);
    u8g2.print(segundos_restantes);
    u8g2.drawStr(110, 45, "s");

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(20, 60, "-CONECTE SU MOTOR-");
}

void prueba_555() {

    digitalWrite(POSITIVE_5V_REG, HIGH);  // Activar alimentación del 555
    static float frecuencia = 0.0;
    static const char* estado = "Sin señal";
    static unsigned long ultimo_muestreo = 0;
    const unsigned long intervalo_muestreo = 500;  // Intenta medir cada 500ms

    static int fase_medicion = 0;  // 0 = esperar, 1 = medir HIGH, 2 = medir LOW
    static unsigned long duracionAlta = 0;
    static unsigned long duracionBaja = 0;

    unsigned long tiempo_actual = millis();



    if (tiempo_actual - ultimo_muestreo >= intervalo_muestreo) {
        fase_medicion = 1;  // Inicia medición
        ultimo_muestreo = tiempo_actual;
    }

    if (fase_medicion == 1) {
        duracionAlta = pulseIn(D41, HIGH, 50000);  // Timeout corto 50ms
        if (duracionAlta == 0) {
            // No llegó pulso HIGH → sin señal
            frecuencia = 0.0;
            estado = "Sin senal";
            fase_medicion = 0;
        } else {
            fase_medicion = 2;
        }
    } else if (fase_medicion == 2) {
        duracionBaja = pulseIn(D41, LOW, 50000);  // Timeout corto 50ms
        if (duracionBaja == 0) {
            frecuencia = 0.0;
            estado = "Sin senal";
        } else {
            unsigned long periodo = duracionAlta + duracionBaja;
            frecuencia = 1000000.0 / periodo;

            if (frecuencia >= 95 && frecuencia <= 105) {
                estado = "OK";
            } else {
                estado = "NG";
            }
        }
        fase_medicion = 0;  // Listo, espera siguiente ciclo
    }

    // Mostrar en pantalla
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(50, 13, "NE555");

    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(10, 35, "Frec:");
    u8g2.setCursor(50, 35);
    if (frecuencia > 0) {
        u8g2.print(frecuencia, 1);
        u8g2.drawStr(90, 35, "Hz");
    } else {
        u8g2.print("---");
    }

    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(10, 50, "Estado:");
    u8g2.drawStr(60, 50, estado);

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(10, 62, "Rango ok: 95-105 Hz");
}

void prueba_741() {
    static int prueba_actual = 1;
    static float salida_medida = 0.0;
    static float salida_corregida = 0.0;

    static const char* estado_prueba1 = "Pendiente";
    static const char* estado_prueba2 = "Pendiente";
    static bool prueba1_ejecutada = false;
    static bool prueba2_ejecutada = false;

    static unsigned long ultimo_cambio = 0;
    static unsigned long tiempo_estabilizacion = 0;
    static bool en_estabilizacion = true;

    const unsigned long tiempo_por_prueba = 1000;
    const unsigned long tiempo_settling = 150;  // ms de estabilización

    unsigned long tiempo_actual = millis();

    digitalWrite(POSITIVE_15V_MOSFET_REG, HIGH);
    digitalWrite(NEGATIVE_15V_MOSFET_REG, HIGH);

    // Cambio de prueba
    if (tiempo_actual - ultimo_cambio >= tiempo_por_prueba) {
        if (prueba_actual == 1) {
            digitalWrite(D42, HIGH);
            digitalWrite(D43, LOW);
            prueba_actual = 2;
        } else {
            digitalWrite(D42, LOW);
            digitalWrite(D43, HIGH);
            prueba_actual = 1;
        }

        ultimo_cambio = tiempo_actual;
        tiempo_estabilizacion = tiempo_actual;
        en_estabilizacion = true;
    }

    // Medición
    int sensorValue = analogRead(A6);
    float voltageADC = sensorValue * (5.0 / 1023.0);
    salida_medida = mapFloat(voltageADC, 0.17, 4.17, -15.0, 15.0);

    if (salida_medida >= 2.0 && salida_medida <= 3.0) {
        salida_corregida = 0.00;
    } else {
        salida_corregida = salida_medida;
    }

    // Esperar estabilización antes de evaluar
    if (en_estabilizacion && (tiempo_actual - tiempo_estabilizacion >= tiempo_settling)) {
        en_estabilizacion = false;
    }

    // Evaluación SOLO cuando ya está estable
    if (!en_estabilizacion) {
        if (prueba_actual == 1) {
            estado_prueba1 = (salida_corregida >= 11.0 && salida_corregida <= 15.0) ? "OK" : "FALLA";
            prueba1_ejecutada = true;
        } else {
            estado_prueba2 = (salida_corregida >= -15.0 && salida_corregida <= -11.0) ? "OK" : "FALLA";
            prueba2_ejecutada = true;
        }
    }

    // Pantalla
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(50, 13, "LM741");

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(10, 25, "Salida741:");
    u8g2.setCursor(70, 25);
    u8g2.print(salida_corregida, 2);
    u8g2.drawStr(105, 25, "V");

    u8g2.drawStr(10, 35, "Prueba 1:");
    u8g2.drawStr(70, 35, estado_prueba1);

    u8g2.drawStr(10, 45, "Prueba 2:");
    u8g2.drawStr(70, 45, estado_prueba2);

    if (prueba1_ejecutada && prueba2_ejecutada) {
        u8g2.setFont(u8g2_font_7x13_tf);
        if (strcmp(estado_prueba1, "OK") == 0 && strcmp(estado_prueba2, "OK") == 0) {
            u8g2.drawStr(20, 60, "Estado: OK");
        } else {
            u8g2.drawStr(20, 60, "Estado: NG");
        }
    } else {
        u8g2.setFont(u8g2_font_7x13_tf);
        u8g2.drawStr(30, 60, "Probando...");
    }
}




void prueba_capacitor() {
  digitalWrite(RLV200, HIGH);
  digitalWrite(RLV201, HIGH);

  static unsigned long time0, time1, time2;
  static float c, null0;
  static byte kn, mk, i;

  // ================= CONFIGURACIÓN DE CARGA =================

  if (mk == 0) {
    pinMode(D29, OUTPUT);  // 1M
    pinMode(D28, INPUT);   // 1k
    digitalWrite(D29, HIGH);
  }

  if (mk == 1) {
    pinMode(D28, OUTPUT);
    pinMode(D29, INPUT);
    digitalWrite(D28, HIGH);
  }

  time0 = micros();
  while (analogRead(A5) < 644) {
    time2 = micros() - time0;
    if (time2 >= 1000000 && mk == 0) {
      mk = 1;
      time0 = 100000000;
      break;
    }
  }

  time1 = micros() - time0;

  // ================= DESCARGA =================

  while (analogRead(A5) > 0) {
    pinMode(D28, OUTPUT);
    pinMode(D29, OUTPUT);
    digitalWrite(D28, LOW);
    digitalWrite(D29, LOW);
  }

  if (mk == 1 && time1 < 1000) {
    mk = 0;
  }

  // ================= CÁLCULO =================

  c = time1;
  c = c / 1000.0 - null0;
  c = abs(c);
  c = c * 1.00; // para arreglar las tolerancias

  if (i == 0) {
    i++;
    null0 = c + 0.004;
  }

  // Si es menor a 0.2 nF → mostrar 0.00
  float display_c = c;
  const char* display_unidad = (mk == 0) ? "nF" : "uF";

  if (mk == 0 && c < 0.2) {  // Solo en modo nF
    display_c = 0.0;
  }

  // ================= DIBUJO OLED =================

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_5x7_tf);

    u8g2.drawStr(28, 48, "**ADVERTENCIA**");   
    u8g2.drawStr(10, 56, "DESCARGUE SU CAPACITOR");   
    u8g2.drawStr(8, 64, "Min 0.2nF - Max 10000uF");   

    u8g2.setFont(u8g2_font_7x13_tf);

    u8g2.drawStr(35, 10, "Capacitor");
    u8g2.drawStr(20, 30, "C:");
    u8g2.setCursor(55, 30);

    if (time1 >= 10000000) {
      u8g2.print("TEST uF");
    } else {
      u8g2.print(display_c, 2);
      u8g2.print(" ");
      u8g2.print(display_unidad);
    }

  } while (u8g2.nextPage());

  delay(100);
}


void prueba_diodo() {
    digitalWrite(RLV202, HIGH);
    digitalWrite(RLV203, HIGH);
    static float Vd = 0.0;
    static float I_mA = 0.0;
    static const char* tipo = "Ninguno";
    static const char* estado = "Esperando";

    static unsigned long lastMeasure = 0;
    const unsigned long MEASURE_INTERVAL = 500; // ms

    const float Rserie = 240.0;
    const float VD_MIN = 0.15;
    const float VD_MIN_SCHOT = 0.20;    
    const float VD_MAX_SCHOT = 0.4;
    const float VD_MIN_RECT = 0.55;    
    const float VD_MAX_RECT = 0.9;
    const float VD_MIN_LED = 1.5;
    const float VD_MAX_LED = 3.3;
    const float VD_MAX = 4.9;
    // ---------- MEDICIÓN (solo cada 500 ms) ----------
    if (millis() - lastMeasure >= MEASURE_INTERVAL) {
        lastMeasure = millis();


        // Polaridad normal
        digitalWrite(D47, HIGH);
        digitalWrite(D48, LOW);
        delayMicroseconds(500);
        float VA_n = analogRead(A7) * (5.0 / 1023.0);
        float VB_n = analogRead(A8) * (5.0 / 1023.0);
        float Vd_n = fabs(VA_n - VB_n);
        digitalWrite(D47, LOW);
        digitalWrite(D48, LOW);

        // Polaridad invertida
        digitalWrite(D47, LOW);
        digitalWrite(D48, HIGH);
        delayMicroseconds(500);
        float VA_i = analogRead(A7) * (5.0 / 1023.0);
        float VB_i = analogRead(A8) * (5.0 / 1023.0);
        float Vd_i = fabs(VA_i - VB_i);
        digitalWrite(D47, LOW);
        digitalWrite(D48, LOW);

        bool normalConduce = (Vd_n >= VD_MIN);
        bool invertConduce = (Vd_i >= VD_MIN);

        if (!normalConduce && !invertConduce) {
            Vd = 0.0;
            I_mA = 0.0;
            tipo = "Corto";
            estado = "CortoCircuito";
        } else {
            bool usarNormal = normalConduce && (!invertConduce || Vd_n < Vd_i);

            if (usarNormal) {
                digitalWrite(D47, HIGH);
                digitalWrite(D48, LOW);
                Vd = Vd_n;
            } else {
                digitalWrite(D47, LOW);
                digitalWrite(D48, HIGH);
                Vd = Vd_i;
            }

            float VA = analogRead(A7) * (5.0 / 1023.0);
            float VB = analogRead(A8) * (5.0 / 1023.0);
            Vd = fabs(VA - VB);
            I_mA = (5.0 - Vd) / Rserie * 1000.0;

             if (Vd >= VD_MIN_RECT && Vd <= VD_MAX_RECT) {
                tipo = "Rectificador";
                estado = "OK";
            } else if (Vd >= VD_MIN_SCHOT && Vd <= VD_MAX_SCHOT) {
                tipo = "Schottky";
                estado = "OK";
            }
              else if (Vd >= VD_MIN_LED && Vd <= VD_MAX_LED) {
                tipo = "LED";
                estado = "OK";
            }
              else if (Vd > VD_MAX) {
                tipo = "Desconocido";
                estado = "Abierto/NC";
            } else {
                tipo = "Desconocido";
                estado = "Desconocido";
            }
        }
    }


    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(28, 13, "Diodo / LED");

    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(10, 25, "Tipo:");
    u8g2.drawStr(50, 25, tipo);

    u8g2.drawStr(10, 35, "Vd:");
    u8g2.setCursor(40, 35);
    u8g2.print(Vd, 3);
    u8g2.drawStr(80, 35, "V");

    u8g2.drawStr(10, 45, "I:");
    u8g2.setCursor(30, 45);
    u8g2.print(I_mA, 2);
    u8g2.drawStr(70, 45, "mA");

    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(8, 60, "Estado:");    
    u8g2.drawStr(51, 60, estado);
}



////////AQUI INICIA LA PRUEBA DE RESISTENCIA

byte ch_number;
uint32_t res;
const uint32_t res_table[5] = {75, 1000, 10000, 100000, 470000};
char _buffer[12];

// ----------- FILTRO TIPO MULTÍMETRO -------------
const byte N_SAMPLES = 20;           // Ventana de promedio
float sampleBuffer[N_SAMPLES];
byte sampleIndex = 0;
bool bufferFilled = false;

float lastStable = 0;                // Valor estable mostrado
unsigned long lastUpdate = 0;
const unsigned long UPDATE_RATE = 250;  // 0.25 segundos

// Umbral de estabilidad (porcentaje)
const float STABLE_THRESHOLD = 0.015;   // 1.5% variación máxima

// --------------------------------------------------

float addSample(float v) {
  sampleBuffer[sampleIndex] = v;
  sampleIndex++;

  if (sampleIndex >= N_SAMPLES) {
    sampleIndex = 0;
    bufferFilled = true;
  }

  float sum = 0;
  byte count = bufferFilled ? N_SAMPLES : sampleIndex;
  for (byte i = 0; i < count; i++)
    sum += sampleBuffer[i];

  return sum / count;
}

bool isStable(float a, float b) {
  if (a == 0) return false;
  float diff = abs(a - b) / a;
  return (diff < STABLE_THRESHOLD);
}


void prueba_resistencia() {
    digitalWrite(RLV204, HIGH);
    digitalWrite(RLV201, HIGH);

static bool initDone = false;

if (!initDone) {
  ch_number = 4;
  ch_select(ch_number);
  initDone = true;
}




  uint16_t volt_image = analogRead(A9) + 1;

  if(volt_image >= 550 && ch_number < 4) {
    ch_number++;
    ch_select(ch_number);
    delay(50);
  }

  if(volt_image <= 90 && ch_number > 0) {
    ch_number--;
    ch_select(ch_number);
    delay(50);
  }

  float value = 0;

  if(volt_image < 900) {

    value = (float)volt_image * res / (1023 - volt_image);

    // ---- PROMEDIO RÁPIDO ----
    float avg = addSample(value);

    // ---- DETECCIÓN DE ESTABILIDAD ----
    if (isStable(lastStable, avg)) {
      // Se mantiene el valor estable
    } else {
      // El valor cambió → actualizar valor estable
      lastStable = avg;
    }

  } else {
    sprintf(_buffer, "Over Load ");
  }

  // ----------- ACTUALIZACIÓN CADA 250ms -------------
  if (millis() - lastUpdate >= UPDATE_RATE) {
    lastUpdate = millis();

    // ---- Formateo final para pantalla ----
    float displayValue = lastStable;

    if(volt_image >= 900)
      sprintf(_buffer, "Over Load ");
    else if(displayValue < 1000.0)
      sprintf(_buffer, "%03u.%1u Ohm ", (uint16_t)displayValue, (uint16_t)(displayValue*10)%10);
    else if(displayValue < 10000.0)
      sprintf(_buffer, "%1u.%03u kOhm", (uint16_t)(displayValue/1000), (uint16_t)displayValue%1000);
    else if(displayValue < 100000.0)
      sprintf(_buffer, "%02u.%02u kOhm", (uint16_t)(displayValue/1000), (uint16_t)(displayValue/10)%100);
    else if(displayValue < 1000000.0)
      sprintf(_buffer, "%03u.%1u kOhm", (uint16_t)(displayValue/1000), (uint16_t)(displayValue/100)%10);
    else
      sprintf(_buffer, "%1u.%03u MOhm", (uint16_t)(displayValue/1000000), (uint16_t)(displayValue/1000)%1000);


  }
     // Actualizar OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(13, 64, "Min 10Ohm - Max 2MOhm");  
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(25, 13, "Resistencia");
    u8g2.drawStr(10, 40, "R:");
    u8g2.drawStr(25, 40, _buffer);
    u8g2.sendBuffer();

    Serial.println(_buffer);

  delay(5); // Muy pequeño para obtener muchas muestras rápidamente
}

void ch_select(byte n) {
  switch(n) {
    case 0:
      digitalWrite(D53, LOW);
      digitalWrite(D52, HIGH);
      digitalWrite(D51, HIGH);
      digitalWrite(D50, HIGH);
      digitalWrite(D49, HIGH);
      break;
    case 1:
      digitalWrite(D53, HIGH);
      digitalWrite(D52, LOW);
      digitalWrite(D51, HIGH);
      digitalWrite(D50, HIGH);
      digitalWrite(D49, HIGH);
      break;
    case 2:
      digitalWrite(D53, HIGH);
      digitalWrite(D52, HIGH);
      digitalWrite(D51, LOW);
      digitalWrite(D50, HIGH);
      digitalWrite(D49, HIGH);
      break;
    case 3:
      digitalWrite(D53, HIGH);
      digitalWrite(D52, HIGH);
      digitalWrite(D51, HIGH);
      digitalWrite(D50, LOW);
      digitalWrite(D49, HIGH);
      break;
    case 4:
      digitalWrite(D53, HIGH);
      digitalWrite(D52, HIGH);
      digitalWrite(D51, HIGH);
      digitalWrite(D50, HIGH);
      digitalWrite(D49, LOW);
  }
  res = res_table[n];
}




////AQUI TERMINA LA PRUEBA DE RESISTENCIA.




//---------------------
void prueba_creditos() {
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0, 10, "Component Tester V1.00");
    u8g2.drawStr(0, 20, "PROYECTO DE RESIDENCIA"); 
    u8g2.drawStr(0, 30, "ING. OLIVER CUIRIZ");        
    u8g2.drawStr(0, 40, "L20211585");
    u8g2.drawStr(0, 50, "@TECTIJUANA.EDU.MX");
    u8g2.drawStr(0, 60, "MCU ATmega2560");

}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void desactivar_MOSFETS(){
    digitalWrite(POSITIVE_15V_MOSFET_REG, LOW);
    digitalWrite(NEGATIVE_15V_MOSFET_REG, LOW);
    digitalWrite(POSITIVE_5V_REG, LOW);
    digitalWrite(POSITIVE_5V_MEGA2560_MOSFET, LOW);
}

void desactivar_conmutador(){
digitalWrite(RLV200, LOW);
digitalWrite(RLV201, LOW);
digitalWrite(RLV202, LOW);
digitalWrite(RLV203, LOW);
digitalWrite(RLV204, LOW);
}

void inicializarOLED() {
    for (int i = 0; i < 10; i++) {
        u8g2.begin();

        // Reset lógico del SSD1306
        u8g2.sendF("c", 0xAE); // Display OFF
        u8g2.sendF("c", 0xA8); // Set multiplex ratio
        u8g2.sendF("c", 0x3F); // 1/64 duty
        u8g2.sendF("c", 0xD3); // Display offset
        u8g2.sendF("c", 0x00); // Offset = 0
        u8g2.sendF("c", 0x40); // Start line = 0
        u8g2.sendF("c", 0xA1); // Segment remap
        u8g2.sendF("c", 0xC8); // COM scan direction
        u8g2.sendF("c", 0xDA); // COM pins hardware config
        u8g2.sendF("c", 0x12);
        u8g2.sendF("c", 0xAF); // Display ON

        u8g2.clearBuffer();
        u8g2.sendBuffer();
        delay(150);
    }
}
