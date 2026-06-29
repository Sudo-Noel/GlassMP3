# GlassMP3 - Reproductor de Audio Avanzado para PSP

GlassMP3 es un reproductor de audio moderno, ligero y altamente optimizado para la PlayStation Portable (PSP). Soporta múltiples formatos de audio, cuenta con una interfaz totalmente personalizable (skins) y está pensado para exprimir al máximo el hardware de la consola gracias a su bajo consumo de memoria y uso eficiente de la CPU.

## Características Principales

*   **Soporte Multiformato:** Reproducción nativa de FLAC, MP3, OGG, WMA, AAC y más.
*   **Decodificación FLAC Optimizada:** Implementación ajustada con un gran búfer (2 segundos) y operaciones de memoria (`memmove`) ultrarrápidas para evitar tirones y saltos, incluso con archivos FLAC de 24-bit y 192kHz.
*   **Gestión Dinámica de CPU:** El reproductor es capaz de subir la frecuencia de la CPU a 333 MHz para formatos exigentes (como FLAC) y reducirla en formatos comprimidos para ahorrar batería. Total compatibilidad con los límites de firmware modernos como ARK-4 y ME/PRO.
*   **Totalmente a Prueba de Cuelgues:** Prevención exhaustiva de bloqueos mutuos (Deadlocks) y vulnerabilidades de memoria (Double-Free) al cambiar de canción rápidamente.
*   **Interfaz Glass Personalizable:** Dos temas por defecto (`LightGlass` y `DarkGlass`) con menús superpuestos, rediseño de iconos, barras de navegación elegantes y corrección de márgenes y solapamientos (`POS_FILE_BROWSER` ajustado).
*   **Bajo Consumo de Batería:** Posibilidad de apagar la pantalla mientras se reproduce audio.

## Instalación

1.  Conecta tu PSP al ordenador mediante el cable USB.
2.  Copia la carpeta principal del reproductor (`GlassMP3` que contiene el archivo `EBOOT.PBP`) a la ruta `X:/PSP/GAME/` de tu tarjeta de memoria (donde `X` es la letra de tu PSP).
3.  Desconecta la consola y ejecuta la aplicación desde el menú `Juego -> Memory Stick`.

## Controles (Por Defecto)

*   **Pad Direccional / Stick Analógico:** Navegar por las carpetas y listas.
*   **X:** Entrar en carpeta / Reproducir canción / Seleccionar.
*   **O (Círculo):** Atrás / Subir un nivel en el explorador de archivos.
*   **Triángulo:** Abrir/Cerrar menú de ajustes de la aplicación.
*   **Gatillo L / R:** Cambiar entre pestañas (Explorador, Lista de reproducción, Ajustes).
*   **Select:** Apagar la pantalla para ahorrar batería (pulsa cualquier botón para volver a encenderla).
*   **Start:** Pausar/Reanudar canción.

## Historial de Actualizaciones (Changelog Reciente)

*   **Fix Crítico FLAC:** Resuelto el problema de reproducción lenta/cámara lenta (x0.25) en archivos FLAC pesados incrementando enormemente el búfer de decodificación y optimizando el desplazamiento en memoria (`memmove` vs bucles ineficientes).
*   **Fix Bloqueo al Saltar:** Arreglado un *Deadlock* que congelaba la consola al cambiar o retroceder de canción rápidamente.
*   **Fix Cuelgue (Bus Error / Double-Free):** Arreglada una corrupción crítica en el *Heap* de la memoria RAM producida por intentar liberar un decodificador FLAC de la memoria dos veces seguidas desde hilos distintos.
*   **Parche Frecuencia CPU:** Corregida la detección de hardware para permitir correctamente Overclock a 333 MHz bajo Custom Firmwares modernos como ARK-4/ARK-5 sin que el reproductor revierta los límites a 222 MHz.
*   **Diseño de la Interfaz:** Rediseñado el Logo/Icono principal (`ICON0.PNG`), pulido estético de la barra superior e inferior y reajustadas las listas de las carpetas para evitar que tapen o se superpongan a los botones de navegación superiores.

## Licencia

Este proyecto y sus modificaciones están sujetos a licencia de código abierto. ¡Disfruta de la mejor calidad de audio en tu querida PSP!
