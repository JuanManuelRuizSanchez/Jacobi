# **Jacobi**  

Este repositorio contiene implementaciones en C del método de **Jacobi 1D** para la resolución de la ecuación de Poisson en tres versiones:  
- **Secuencial** (`JacobiSec.c`)  
- **Paralelo con hilos (pthreads)** (`JacobiHilos.c`)  
- **Paralelo con procesos (fork y memoria compartida)** (`JacobiProc.c`)  

## **Autores**  
- **Juan Manuel Ruiz Sánchez**  
- **Esteban Gómez**  

## **Uso**  
Para compilar y ejecutar cada versión, usa los siguientes comandos:  

### **Versión secuencial**  
```sh
gcc JacobiSec.c -o JacobiSec
./JacobiSec [Tamaño de malla] [Número de iteraciones]
```

### **Versión con hilos**  
```sh
gcc JacobiHilos.c -o JacobiHilos -pthread
./JacobiHilos [Tamaño de malla] [Número de iteraciones] [Número de hilos]
```

### **Versión con procesos**  
```sh
gcc JacobiProc.c -o JacobiProc
./JacobiProc [Tamaño de malla] [Número de iteraciones] [Número de procesos]
```

## **Reportes de rendimiento**  
El repositorio incluye:  
- Reportes de análisis de rendimiento generados con **gprof** (`reporte_Jacobi.txt`).  
- Un script en Bash (`ScriptJacobi.sh`) que automatiza las pruebas con distintas configuraciones de paralelización.