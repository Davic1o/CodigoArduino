
# Carrito ESP32 y Wi-Fi

Este proyecto consiste en un carrito  que se controla a través de un servidor web utilizando el módulo Wi-Fi de ESP32. El carrito tiene dos motores controlados por un L298N, dos sensores ultrasónicos para detectar obstáculos y un buzzer para alertar al usuario. También tiene un sensor de luz para detectar la iluminación del entorno y encender o apagar los led de los lados correspondientes.

## Componentes

-	1 ESP32
-	1 Base para ESP32
-	2 Sensor ultrasónico HC-SR04 (frontal y trasero)
-	1 Sensor LDR + resistencia
-	1 Buzzer
-	1 Módulo L298N (Driver)
-	2 Motores DC (delantero y trasero)
-	1Carrito Hijo David
-	2 Leds
-	2 resistencias 220 Ohms
-	1 Caja para 4 Pilas
-	4 Pilas
-	1 Powerbank de 5v a 1700mAh
-	Cables  

## Funcionamiento

El carrito se controla a través de un servidor web que se inicia en el Arduino. El usuario puede enviar comandos al carrito a través de una petición HTTP POST. Los comandos posibles son:

* adelante
* atras
* izquierda
* derecha

El carrito se detendrá automáticamente si detecta un obstáculo a menos de 7 cm de distancia. También se detendrá si no recibe comandos durante 5 segundos.

El buzzer se activará si el carrito se encuentra a una distancia crítica de un obstáculo.

## Configuración

Es necesario configurar el ssid y el password del Wi-Fi en el archivo Carrito.ino. También se puede configurar el umbral de distancia para detener el carrito y el umbral de luz para encender o apagar los LEDs.

## BD

El carrito envía eventos a una base de datos remota cuando se detiene o cambia de estado. Los eventos se envían a través de una petición HTTP POST. Es necesario configurar la URL de la base de datos en el archivo Carrito.ino.