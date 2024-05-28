# Informe

## Presentacion

<!-- Presentar muy brevemente los objetivos del proyecto y el proyecto en si, ... presentar brevemente -->

### Objetivos del proyecto

● Leer, comprender y generar modelos de red en Omnet++.
● Analizar tráfico de red bajo diferentes estrategias de enrutamiento.
● Diseñar y proponer soluciones de enrutamiento bajo diferentes topologías.

## Resumen

<!-- Presentar brevemente la estructura del informe. Dar algunos detalles mas del proyecto.
Introducir que vamos a dividir el informe en dos partes y que luego las compararemos. 
-->

## Introduccion
<!--
Describir el estado del arte. (trabajos previos) ..."
-->

<!--
- Metodología de trabajo.
-->

### Generalidades

<!-- Agregar definiciones generales, teoria para ya dar por sentada en ambas partes. -->

#### Topologia de red

La red consta de 8 nodos conectados en forma de anillo, cada uno con dos interfaces de comunicación full-duplex con dos posibles vecinos.

#### Estructura interna de los nodos

Internamente, cada nodo cuenta con dos capas de enlace (link o lnk, una con cada vecino),
una capa de red (net) y una capa de aplicación (app). La capa de aplicación y la capa de
enlace implementan generadores de tráfico y buffers respectivamente.

### Problematicas
<!--
- Definir el problema y contextualizar al lector con definiciones básicas.
  + "Nosotros en las redes vamos a encontrar tal y tal problema ..."
-->

### Casos de estudio

<!--
- Presentación de nuestros casos de estudio.
   + Explicar caso 1: su ventaja, problemas, que esperamos ver, etc.
   + Explicar caso 2: su ventaja, problemas, que esperamos ver, etc.
-->

1. Caso 1) Se deberá correr el modelo con las fuentes de tráfico configuradas (node[0] y
node[2] transmitiendo datos a node[5]) y estudiar las métricas tomadas. ¿Qué
métricas se obtienen? ¿Cómo es el uso de los recursos de la red? ¿Se puede mejorar?
1. Caso 2) Asuma ahora que todos los nodos (0,1,2,3,4,6,7) generan tráfico hacia el
node[5] con packetByteSize e interArrivalTime idénticos entre todos los nodos.
Explore y determine a partir de qué valor de interArrivalTime se puede garantizar un
equilibrio o estabilidad en la red. Justifique.

## Parte 1

### Metodos

<!--
Una sección que describir nuestra propuesta de solución:
- Describimos el algoritmo.
- Como llegamos a esa idea.
- Una pequeña hipótesis de porque creemos que va a funcionar.
-->
Cada paquete que ésta recibe es evaluado para determinar si el nodo local es el destino final del mismo. En caso de que lo sea, el paquete es enviado a la capa de aplicación local. En caso de que el paquete esté destinado a otro nodo se elige una interface para re-transmitirlo. La capa de red del kickstarter elige siempre la interface número 0 (toLn [0]) que es la que envía el tráfico en sentido de las manecillas del reloj a lo largo del anillo hasta llegar al destino.

### Resultados

## Parte 2

### Metodos

<!--
Una sección que describir nuestra propuesta de solución:
- Describimos el algoritmo.
- Como llegamos a esa idea.
- Una pequeña hipótesis de porque creemos que va a funcionar.
-->

### Resultados

## Comparacion de resultados

<!-- Comparar graficos de ambas partes. solo poner los graficos si hay algo que comparar o recordar. -->

## Discusiones

<!-- cualquier cosa que no entre en la comparacion de resultados va aca, ej: posibles mejoras, obs,conclusiones, ... -->

## Referencias
<!--
- Todas las referencias que usamos en el trabajo. LIBROS, PAPERS, WEB, ETC.
(Nosotros usamos el manual de Omnet++ y quizás algo más ...).

Si agregamos imágenes de Tanembaun para explicar algo, también se debe referenciar.
-->

- Andrew S. Tanenbaum, David J. Wetherall, Redes de Computadoras (5ta edición 2011), Pearson.
- Omnet++ Simulation Manual, (OMNeT++ version 6.0.3, 2020).
