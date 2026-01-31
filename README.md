# Component Tester – Proyecto de residencia profesional

Este repositorio contiene mi **proyecto de residencia profesional: Dispositivo para el diagnóstico y medición de componentes electrónicos**, desarrollado en el **Instituto Tecnológico de Tijuana** para el **CETis No. 58**.

El proyecto consiste de un probador de componentes electrónicos capaz de evaluar hasta **20 familias de componentes**, proporcionando información del estado de cada componente. El sistema está diseñado para apoyar el trabajo educativo y práctico en los estudiantes de electrónica en el CETis No. 58, permitiendo a los usuarios verificar el correcto funcionamiento de sus componentes electrónicos utilizados en sus prácticas de laboratorio.


## Familias de componentes soportados

- **Compuertas lógicas:** 74LS00 (NAND), 74LS02 (NOR), 74LS04 (NOT), 74LS08 (AND), 74LS32 (OR) y 74LS86 (XOR)
- **Reguladores de voltaje lineales:** LM317, LM337, LM7805, LM7812 y LM7912  
- **Diodos:** diodos rectificadores (familia 1N400x), LEDs y diodos Schottky 
- **Temporizador y OpAmp:** NE555 y LM741  
- **Motores:** servomotores SG90 / MOT1XX  y motor a pasos unipolar 28BYJ-48 
- **Otros componentes:** resistencias y capacitores

## Estructura del repositorio

Este repositorio está organizado en carpetas individuales que contienen todos los recursos relacionados con el proyecto. Estos incluyen el código fuente del proyecto, el esquema eléctrico, los archivos de diseño 3D de la PCB y la carcasa, y los recursos gráficos utilizados para el menú de la interfaz de usuario.

### Para abrir estos archivos se necesita el siguiente software:
- **Arduino IDE 2.6.3** recomendable
- **KiCad 9**
- **SolidWorks 2022**, o bien, cualquier software que pueda abrir archivos con extensión **.STL**
- **Photopea**, o bien, **Photoshop**

## Advertencias importantes:
- Debido a un bug con los botones para navegar en el menú, se tiene que hacer un puente entre el pin 17 y pin 2 del ATmega2560. Esto se ve reflejado en el codigo fuente del proyecto pero no en el diseño de la PCB actual.
- El conector JST XH2.54 macho que se utiliza para el motor a pasos 28BYJ-48 está al revés en el diseño de la PCB.



**Ambos errores se pueden corregir modificando directamente la PCB en KiCad y generar otro diseño.** Si se quisiera fabricar la PCB tal y como está entonces se tienen que hacer esas dos modificaciones.

<img width="1280" height="959" alt="20251230_010616-Photoroom" src="https://github.com/user-attachments/assets/cffd68cd-df20-4588-87f6-601db9976273" />


