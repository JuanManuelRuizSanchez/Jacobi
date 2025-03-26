#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_THREADS 32

// Variables globales compartidas entre hilos
int n, nsweeps, num_threads;
double *u, *utmp, *f, h2;
pthread_barrier_t barrier;

// Función para medir tiempo (en segundos)
double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1e6;
}

// Función que ejecuta cada hilo: realiza los pasos de la iteración de Jacobi en un bloque de índices
void* jacobi_thread(void* arg) {
    int tid = *(int*)arg;
    // Dividir el rango de índices [1, n-1] entre los hilos
    int total_indices = n - 1; // índices 1 hasta n-1 (los extremos son condiciones de frontera fijas)
    int chunk = total_indices / num_threads;
    int start = 1 + tid * chunk;
    int end = (tid == num_threads - 1) ? n : (start + chunk);

    for (int sweep = 0; sweep < nsweeps; sweep += 2) {
        // Actualización: utmp[i] = (u[i-1] + u[i+1] + h2 * f[i]) / 2
        for (int i = start; i < end; i++) {
            utmp[i] = (u[i-1] + u[i+1] + h2 * f[i]) / 2.0;
        }
        pthread_barrier_wait(&barrier);
        // Actualización: u[i] = (utmp[i-1] + utmp[i+1] + h2 * f[i]) / 2
        for (int i = start; i < end; i++) {
            u[i] = (utmp[i-1] + utmp[i+1] + h2 * f[i]) / 2.0;
        }
        pthread_barrier_wait(&barrier);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <n> <nsweeps> <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    n = atoi(argv[1]);
    nsweeps = atoi(argv[2]);
    num_threads = atoi(argv[3]);

    if (n <= 0 || nsweeps <= 0 || num_threads <= 0 || num_threads > MAX_THREADS) {
        fprintf(stderr, "Error: Los parámetros deben ser positivos y num_threads <= %d\n", MAX_THREADS);
        return EXIT_FAILURE;
    }

    // Reservar memoria para las mallas: se usan n+1 puntos (índices 0 a n)
    u = (double*) malloc((n+1) * sizeof(double));
    utmp = (double*) malloc((n+1) * sizeof(double));
    f = (double*) malloc((n+1) * sizeof(double));
    if (!u || !utmp || !f) {
        perror("Error en la asignación de memoria");
        return EXIT_FAILURE;
    }

    // Inicialización:
    // Establecer condiciones de frontera (por ejemplo, u[0] = u[n] = 0)
    u[0] = 0.0; u[n] = 0.0;
    utmp[0] = 0.0; utmp[n] = 0.0;
    // Inicializar los valores interiores de u (por ejemplo, todos 0)
    for (int i = 1; i < n; i++) {
        u[i] = 0.0;
    }
    // Definir la función forzada f; aquí se puede usar una función simple, por ejemplo, f[i] = i*h
    double h = 1.0 / n;
    for (int i = 0; i <= n; i++) {
        f[i] = i * h;
    }
    h2 = h * h;

    // Inicializar el barrier para num_threads hilos.
    pthread_barrier_init(&barrier, NULL, num_threads);

    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    double t0 = get_time();

    // Crear los hilos
    for (int t = 0; t < num_threads; t++) {
        thread_ids[t] = t;
        pthread_create(&threads[t], NULL, jacobi_thread, &thread_ids[t]);
    }
    // Esperar a que todos los hilos finalicen
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }

    double t1 = get_time();

    // Imprimir el tiempo de ejecución (tiempo del kernel) en formato numérico limpio.
    printf("%.6f\n", t1 - t0);

    pthread_barrier_destroy(&barrier);
    free(u);
    free(utmp);
    free(f);

    return EXIT_SUCCESS;
}
