# 🗣️ HRI & Listener — Interfaz de Voz ROS 2

Este paquete implementa una **interfaz de interacción humano–robot (HRI)** a través de voz.  
Consta de dos componentes principales:

1. **Nodo C++ `HRI`** — Provee métodos de alto nivel (`say()` y `listen()`) para que otros nodos interactúen con el sistema de voz.  
2. **Nodo Python `listener`** — Implementa el servicio de reconocimiento de voz mediante **Vosk** y **PyAudio**, ofreciendo el servicio `/listen`.

---

## 📁 Estructura general

  hri_tools/
  
  ├── include/hri_tools/HRI.h # Cabecera del wrapper HRI (C++)
  
  ├── src/HRI.cpp # Implementación del wrapper HRI (C++)
  
  listener/
  
  ├── listener.py # Nodo principal del servicio de voz
  
  ├── grammar/ # Carpeta con archivos JSON de gramática
  
  │ └── <gramática>.json
  
  listener_interfaces/
  
  ├── srv/Listen.srv # Definición del servicio de reconocimiento


---

## 🚀 Nodo `HRI` (C++)

### Descripción

`HRI` actúa como una capa de comunicación entre el resto del sistema ROS 2 y los nodos de voz.  
Expone métodos sincrónicos:

- `say(text)` → Publica un mensaje de voz y espera hasta que termine de hablar.  
- `listen(grammar)` → Solicita reconocimiento de voz mediante el servicio `/listen`.

---

### Inicialización

```cpp
auto node = rclcpp::Node::make_shared("my_node");
HRI hri(node);
```

Métodos principales
🗣️ void say(const std::string& text)

Publica una cadena de texto para ser hablada por el sistema TTS (text-to-speech).
El método bloquea la ejecución hasta que se reciba confirmación de que la reproducción terminó.

Publica en:

Tópico: voice/speech

Tipo: std_msgs/msg/String

Depende de:

Tópico: voice/speech_status

Tipo: std_msgs/msg/Bool

true → indica que la locución finalizó.

👂 std::string listen(const std::string& grammar)

Llama al servicio de reconocimiento de voz.
El método bloquea hasta recibir respuesta o hasta 10 s de espera.

Servicio:

Nombre: /listen

Tipo: listener_interfaces/srv/Listen

Flujo:

Se envía una solicitud con el nombre del archivo de gramática (JSON) a usar.

El nodo listener procesa el audio, reconoce la voz y devuelve el texto reconocido.

Ejemplo:

```cpp
std::string result = hri.listen("commands.json");
RCLCPP_INFO(node->get_logger(), "Dijo: %s", result.c_str());
```
🧠 Nodo listener (Python)
Descripción

Nodo responsable de procesar audio en tiempo real con Vosk y PyAudio, exponiendo el servicio /listen.
Puede usar gramáticas JSON opcionales para restringir las frases reconocidas.

| Parámetro          | Tipo   | Valor por defecto | Descripción                  |
| ------------------ | ------ | ----------------- | ---------------------------- |
| `language`         | string | `"en"`            | Idioma del modelo            |
| `model_path`       | string | `"model"`         | Ruta relativa al modelo Vosk |
| `sample_rate`      | int    | `48000`           | Frecuencia de muestreo       |
| `grammar_path`     | string | `""`              | Ruta base para gramáticas    |
| `listener_service` | string | `"listen"`        | Nombre del servicio ROS 2    |


🔧 Servicio /listen

Tipo: listener_interfaces/srv/Listen

Estructura del servicio

```
# Request
bool enable          # Si se activa o desactiva la escucha
string grammar       # Nombre del archivo de gramática (opcional)

---
# Response
bool success
string recognized_speech
```
Comportamiento

Si enable = true, el nodo comienza a escuchar mediante el micrófono.

Si grammar está definido, se cargará el archivo JSON desde share/listener/grammar/<grammar>.

El sistema devolverá la frase reconocida cuando:

La confianza de las palabras sea > 0.8.

Se haya capturado una frase completa.

Ejemplo de llamada manual:
```
ros2 service call /listen listener_interfaces/srv/Listen "{enable: true, grammar: 'commands.json'}"
```
| Tópico                | Tipo                             | Dirección                  | Descripción                                     |
| --------------------- | -------------------------------- | -------------------------- | ----------------------------------------------- |
| `voice/speech`        | `std_msgs/msg/String`            | ⬅️ Publicación desde `HRI` | Texto que se enviará al nodo de síntesis de voz |
| `voice/speech_status` | `std_msgs/msg/Bool`              | ⬅️ Suscripción en `HRI`    | Indica si la locución terminó (`true`)          |
| `/listen`             | `listener_interfaces/srv/Listen` | 🔁 Servicio                | Comunicación entre `HRI` y `listener`           |

🧩 Integración con otras herramientas
Desde C++

El módulo HRI puede incluirse fácilmente:
```
#include "hri_tools/HRI.h"

HRI hri(node);

// Decir algo
hri.say("Hola, soy el robot");

// Escuchar comando
auto command = hri.listen("commands.json");
```


🧰 Dependencias principales
En C++:

rclcpp

std_msgs

listener_interfaces

En Python:

rclpy

vosk

pyaudio

ament_index_python



