# **Viscosimetro Digital**

![Versión](https://img.shields.io/badge/version-1.0.0-green)
![Python](https://img.shields.io/badge/Python-3.8.10-blue)
![Arduino](https://img.shields.io/badge/Arduino-IDE_2.1-blue?logo=arduino&logoColor=white)
![Build In Progress](https://img.shields.io/badge/build-in%20progress-blue)


**A través de este proyecto de hacer un viscosimetro por el metodo de caida libre, nos permite aplicar de forma práctica conocimientos de electromagnetismo, electronica de circuitos, desarrollo de HMI's tanto fisicas como en software, aplicacion de logica de control inteligente fuzzy y diseño mecanico; este proyecto mecatronico de aula nos ha permtido aplicar y expander conocimientos de la carrera**

---

## **Índice**

1. [Introducción](#introducción)  
2. [Características](#características)  
3. [Instalación](#instalación)  
4. [Uso](#uso)  
5. [Configuración](#configuración)  
6. [Contribuciones](#contribuciones)  
7. [Roadmap](#roadmap)  
8. [Licencia](#licencia)  
9. [Contacto](#contacto)  

---

## **Introducción**

- **Motivación**: La medición de la viscosidad de los fluidos es fundamental en diversos procesos industriales y científicos. Sin embargo, muchos laboratorios educativos no cuentan con equipos accesibles, versátiles o actualizados para este tipo de análisis. Esta situación motivó el desarrollo de un sistema experimental propio que no solo permita estudiar este parámetro, sino también explorar cómo varía frente a condiciones como la temperatura. Al diseñar y construir un viscosímetro digital, se fomenta además la integración de conocimientos teóricos con habilidades prácticas en un contexto de aprendizaje activo.

- **Propósito**: Desarrollar un banco experimental funcional y didáctico que permita realizar pruebas de viscosidad en el laboratorio de mecánica de fluidos de la Universidad Tecnológica de Bolívar. Este dispositivo busca ser una herramienta de apoyo a la docencia y la experimentación, facilitando el estudio del comportamiento reológico de los líquidos bajo distintas condiciones térmicas.

- **Audiencia objetivo**: Este proyecto está dirigido principalmente a los estudiantes de la Universidad Tecnológica de Bolívar, en particular a quienes cursan asignaturas relacionadas con mecánica de fluidos. Está diseñado para brindar una herramienta accesible y precisa para medir la viscosidad de distintos líquidos y analizar cómo esta propiedad varía con los cambios de temperatura, permitiendo una comprensión más profunda de los fenómenos físicos involucrados.

---

## **Características**

- Lista detallada de las características principales.  
  - ✨ HMI para modificar los parametros del experimento.  
  - 📊 Algoritmo de control inteligente para el control de temperatura
  - 🚀 diseño electronico y mecanico del banco experimental.

---

## **Instalación**

### Requisitos previos
- Lista de herramientas o tecnologías necesarias 
    - Python 3.8.10   
    - Git
    - QTDesign
    - Arduino IDE

### Pasos

```bash
# Clona el repositorio
git clone https://github.com/iCruzDaniel/VISCO-DI-S-A.git

#### Para correr el codigo de la HMI. Vete a la carpeta ""
cd /HMI/Visualizer/

#### Crear un entorno virtual
python -m venv mi_entorno

#### Activar el entorno virtual
mi_entorno\Scripts\activate  # En Windows
source mi_entorno/bin/activate  # En Linux/macOS

#### Instalar las dependencias
pip install -r requirements.txt
```

-''''''''''''''''''''''''''''
## **Uso**

1. El proyecto, como interfaz HMI, se ejecuta desde el script _App.py_.  

```bash 

```


---

## **Configuración**


```env
NO CONFIGS
```

---
<!-- 
## **Contribuciones**

¡Las contribuciones son bienvenidas! Sigue estos pasos para contribuir:  

1. Haz un fork del repositorio.  
2. Crea una nueva rama: `git checkout -b feature/nueva-funcionalidad`.  
3. Realiza tus cambios y haz un commit: `git commit -m 'Añadir nueva funcionalidad'`.  
4. Envía un pull request.

Consulta las [guías de contribución](CONTRIBUTING.md) para más detalles. -->

---
## **Roadmap**

- **Versión actual**:  
  - ✔️ Implementación de algoritmos .  
  - ✔️ Diseño de algoritmo.  
  - ✔️ Desarrollo I.  

- **Futuras versiones**:  
  - 🛠️ En desarrollo: Optimización 
  - 🕒 Planeado: Mejora  

---

<!-- ## **Licencia**

Este proyecto está licenciado bajo la Licencia `  ` . Consulta el archivo [LICENSE](LICENSE) para más detalles. -->

---

## **Contacto**

- **Autores**: [Daniel Cruz](https://github.com/iCruzDaniel), [Jesús Miranda](https://github.com/jesuMiranda), [Juan Herrera](), [Jesús Polo](), [Jhonny Stevenson](), [Antonio De Leon]().   
- **Correo electrónico**: sp_dicruz@hotmail.com
- **Sitio web**: [DanielCruzPortfolio](https://icruzdaniel.github.io/portfolio/)  
