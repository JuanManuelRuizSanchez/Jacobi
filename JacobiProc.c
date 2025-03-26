#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_PROCS 32

// Función para medir tiempo (en segundos)
double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1e6;
}

// Función de Jacobi con paralelización mediante fork y memoria compartida
void jacobi(int nsweeps, int n, int num_procs, double* u, double* utmp, double* f, double h2) {
    int total_indices = n - 1; // Se excluyen los bordes
    int base = 1;
    int chunk = total_indices / num_procs;
    int rem   = total_indices % num_procs;

    for (int sweep = 0; sweep < nsweeps; sweep += 2) {
        // Primer barrido: actualizar utmp[i] = (u[i-1] + u[i+1] + h2 * f[i]) / 2
        for (int p = 0; p < num_procs; p++) {
            int start = base + p * chunk;
            int end = start + chunk + (p == num_procs - 1 ? rem : 0);

            pid_t pid = fork();
            if (pid < 0) {
                perror("Error en fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {  // Proceso hijo
                for (int i = start; i < end; i++) {
                    utmp[i] = (u[i-1] + u[i+1] + h2 * f[i]) / 2.0;
                }
                exit(EXIT_SUCCESS);
            }
        }
        for (int p = 0; p < num_procs; p++) wait(NULL); // Sincronizar procesos

        // Segundo barrido: actualizar u[i] = (utmp[i-1] + utmp[i+1] + h2 * f[i]) / 2
        for (int p = 0; p < num_procs; p++) {
            int start = base + p * chunk;
            int end = start + chunk + (p == num_procs - 1 ? rem : 0);

            pid_t pid = fork();
            if (pid < 0) {
                perror("Error en fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {  // Proceso hijo
                for (int i = start; i < end; i++) {
                    u[i] = (utmp[i-1] + utmp[i+1] + h2 * f[i]) / 2.0;
                }
                exit(EXIT_SUCCESS);
            }
        }
        for (int p = 0; p < num_procs; p++) wait(NULL); // Sincronizar procesos
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <n> <nsweeps> <num_procs>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);         // Número de subintervalos (malla: n+1 puntos)
    int nsweeps = atoi(argv[2]);   // Número de barridos (iteraciones)
    int num_procs = atoi(argv[3]); // Número de procesos

    if (n <= 0 || nsweeps <= 0 || num_procs <= 0 || num_procs > MAX_PROCS) {
        fprintf(stderr, "Error: Parámetros inválidos.\n");
        return EXIT_FAILURE;
    }

    double h = 1.0 / n;
    double h2 = h * h;
    int size = (n + 1) * sizeof(double);

    // Crear memoria compartida para u, utmp y f
    int shm_id_u    = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    int shm_id_utmp = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    int shm_id_f    = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);

    if (shm_id_u == -1 || shm_id_utmp == -1 || shm_id_f == -1) {
        perror("Error en shmget");
        return EXIT_FAILURE;
    }

    double *u    = (double *) shmat(shm_id_u, NULL, 0);
    double *utmp = (double *) shmat(shm_id_utmp, NULL, 0);
    double *f    = (double *) shmat(shm_id_f, NULL, 0);

    if (u == (void*) -1 || utmp == (void*) -1 || f == (void*) -1) {
        perror("Error en shmat");
        return EXIT_FAILURE;
    }

    // Inicializar u, utmp y f
    u[0] = 0.0; u[n] = 0.0;
    utmp[0] = 0.0; utmp[n] = 0.0;
    for (int i = 1; i < n; i++) u[i] = 0.0;
    for (int i = 1; i < n; i++) utmp[i] = 0.0;
    for (int i = 0; i <= n; i++) f[i] = i * h;

    double t0 = get_time();

    // Llamar a la función Jacobi (ahora sí aparece en gprof)
    jacobi(nsweeps, n, num_procs, u, utmp, f, h2);

    double t1 = get_time();

    // Imprimir solo el tiempo de ejecución del kernel en segundos
    printf("%.6f\n", t1 - t0);

    // Liberar memoria compartida
    shmdt(u); shmdt(utmp); shmdt(f);
    shmctl(shm_id_u, IPC_RMID, NULL);
    shmctl(shm_id_utmp, IPC_RMID, NULL);
    shmctl(shm_id_f, IPC_RMID, NULL);

    return EXIT_SUCCESS;
}
