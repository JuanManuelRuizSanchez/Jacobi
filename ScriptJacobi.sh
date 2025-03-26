#!/bin/bash

# --- Configuración ---
dimensions=(10000 100000 1000000 2500000 5000000)  # Tamaños de la malla (n)
nSweeps=15000                                      # Número de barridos en Jacobi
iterations=10                                      # Número de ejecuciones por configuración
thread_counts=(2 4 8 16 32)                        # Cantidades de hilos a evaluar
process_counts=(2 4 8 16 32)                       # Cantidades de procesos a evaluar

# --- Ejecución Secuencial ---
output_file="JacobiSec.csv"
echo "Dimension,Iteration,Time" > "$output_file"

for dim in "${dimensions[@]}"; do
    for (( iter=1; iter<=iterations; iter++ )); do  # Se ejecuta 10 veces cada configuración
        time_result=$(./JacobiSec "$dim" "$nSweeps")
        echo "$dim,$iter,$time_result" >> "$output_file"
    done
done

# --- Ejecución con Hilos ---
for num_hilos in "${thread_counts[@]}"; do
    output_file="JacobiHilos_${num_hilos}hilos.csv"
    echo "Dimension,Iteration,Threads,Time" > "$output_file"

    for dim in "${dimensions[@]}"; do
        for (( iter=1; iter<=iterations; iter++ )); do
            time_result=$(./JacobiHilos "$dim" "$nSweeps" "$num_hilos")
            echo "$dim,$iter,$num_hilos,$time_result" >> "$output_file"
        done
    done
done

# --- Ejecución con Procesos ---
for num_procesos in "${process_counts[@]}"; do
    output_file="JacobiProc_${num_procesos}procesos.csv"
    echo "Dimension,Iteration,Processes,Time" > "$output_file"

    for dim in "${dimensions[@]}"; do
        for (( iter=1; iter<=iterations; iter++ )); do
            time_result=$(./JacobiProc "$dim" "$nSweeps" "$num_procesos")
            echo "$dim,$iter,$num_procesos,$time_result" >> "$output_file"
        done
    done
done

echo "✅ Todas las pruebas han finalizado. Los archivos CSV han sido generados."
