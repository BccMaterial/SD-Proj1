#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define REPETICOES 10
typedef struct {
  int inicio;
  int fim;
  int n;
  float *A;
  float *B;
  float *C;
} DadosThread;

double tempo_atual() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec + t.tv_nsec / 1e9;
}

void inicializar_matriz(float *M, int n) {
  for (long long i = 0; i < (long long)n * n; i++) {
    M[i] = (float)(rand() % 10);
  }
}

void zerar_matriz(float *M, int n) {
  memset(M, 0, sizeof(float) * (long long)n * n);
}

void multiplicar_sequencial(float *A, float *B, float *C, int n) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      float soma = 0.0f;

      for (int k = 0; k < n; k++) {
        soma += A[i * n + k] * B[k * n + j];
      }

      C[i * n + j] = soma;
    }
  }
}

void *multiplicar_parcial(void *arg) {
  DadosThread *dados = (DadosThread *)arg;

  int inicio = dados->inicio;
  int fim = dados->fim;
  int n = dados->n;

  float *A = dados->A;
  float *B = dados->B;
  float *C = dados->C;

  for (int i = inicio; i < fim; i++) {
    for (int j = 0; j < n; j++) {
      float soma = 0.0f;

      for (int k = 0; k < n; k++) {
        soma += A[i * n + k] * B[k * n + j];
      }

      C[i * n + j] = soma;
    }
  }

  pthread_exit(NULL);
}

void multiplicar_paralelo(float *A, float *B, float *C, int n,
                          int num_threads) {
  pthread_t threads[num_threads];
  DadosThread dados[num_threads];

  int linhas_por_thread = n / num_threads;
  int resto = n % num_threads;

  int linha_atual = 0;

  for (int i = 0; i < num_threads; i++) {
    int linhas = linhas_por_thread;

    if (i < resto) {
      linhas++;
    }

    dados[i].inicio = linha_atual;
    dados[i].fim = linha_atual + linhas;
    dados[i].n = n;
    dados[i].A = A;
    dados[i].B = B;
    dados[i].C = C;

    pthread_create(&threads[i], NULL, multiplicar_parcial, &dados[i]);

    linha_atual += linhas;
  }

  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }
}

int verificar_resultado(float *C1, float *C2, int n) {
  for (long long i = 0; i < (long long)n * n; i++) {
    if (C1[i] != C2[i]) {
      return 0;
    }
  }

  return 1;
}

int main() {
  srand(time(NULL));

  int tamanhos[] = {1000, 2000, 4000, 8000, 10000};
  int qtd_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

  int threads[] = {2, 4, 8, 16};
  int qtd_threads = sizeof(threads) / sizeof(threads[0]);

  mkdir("output", 0755);
  FILE *arquivo = fopen("output/resultados.csv", "w");

  if (arquivo == NULL) {
    printf("Erro ao criar o arquivo output/resultados.csv\n");
    return 1;
  }

  fprintf(arquivo, "Tamanho,Threads,TempoSequencialMedio,TempoParaleloMedio,"
                   "Speedup,Eficiencia\n");

  printf("O processamento pode demorar alguns minutos...\n\n");

  for (int t = 0; t < qtd_tamanhos; t++) {
    int n = tamanhos[t];

    printf("Testando matrizes %dx%d...\n", n, n);

    long long total_elementos = (long long)n * n;

    float *A = malloc(sizeof(float) * total_elementos);
    float *B = malloc(sizeof(float) * total_elementos);
    float *C_seq = malloc(sizeof(float) * total_elementos);
    float *C_par = malloc(sizeof(float) * total_elementos);

    if (A == NULL || B == NULL || C_seq == NULL || C_par == NULL) {
      printf("Erro de alocacao de memoria para matriz %dx%d.\n", n, n);
      printf("Tente remover tamanhos grandes como 8000 ou 10000.\n");

      free(A);
      free(B);
      free(C_seq);
      free(C_par);
      continue;
    }

    inicializar_matriz(A, n);
    inicializar_matriz(B, n);
    double soma_tempo_seq = 0.0;

    for (int r = 0; r < REPETICOES; r++) {
      zerar_matriz(C_seq, n);

      double inicio = tempo_atual();
      multiplicar_sequencial(A, B, C_seq, n);
      double fim = tempo_atual();

      double tempo = fim - inicio;
      soma_tempo_seq += tempo;

      printf("Sequencial | n=%d | repeticao=%d | tempo=%.4f segundos\n", n,
             r + 1, tempo);
    }

    double media_seq = soma_tempo_seq / REPETICOES;

    for (int th = 0; th < qtd_threads; th++) {
      int num_threads = threads[th];

      double soma_tempo_par = 0.0;

      for (int r = 0; r < REPETICOES; r++) {
        zerar_matriz(C_par, n);

        double inicio = tempo_atual();
        multiplicar_paralelo(A, B, C_par, n, num_threads);
        double fim = tempo_atual();

        double tempo = fim - inicio;
        soma_tempo_par += tempo;

        printf("Paralelo    | n=%d | threads=%d | repeticao=%d | tempo=%.4f "
               "segundos\n",
               n, num_threads, r + 1, tempo);
      }

      double media_par = soma_tempo_par / REPETICOES;
      double speedup = media_seq / media_par;
      double eficiencia = speedup / num_threads;

      fprintf(arquivo, "%d,%d,%.6f,%.6f,%.6f,%.6f\n", n, num_threads, media_seq,
              media_par, speedup, eficiencia);

      printf("\nResumo n=%d threads=%d\n", n, num_threads);
      printf("Tempo medio sequencial: %.4f segundos\n", media_seq);
      printf("Tempo medio paralelo:   %.4f segundos\n", media_par);
      printf("Speed-up:               %.4f\n", speedup);
      printf("Eficiencia:             %.4f\n\n", eficiencia);
    }

    free(A);
    free(B);
    free(C_seq);
    free(C_par);

    printf("Finalizado tamanho %dx%d.\n\n", n, n);
  }

  fclose(arquivo);

  printf("Experimentos finalizados.\n");
  printf("Resultados salvos em resultados.csv\n");

  return 0;
}
