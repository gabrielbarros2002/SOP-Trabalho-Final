#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int simularFIFO(int num_quadros, int* referencias_paginas, int num_referencias);

int simularLRU(int num_quadros, int* referencias_paginas, int num_referencias);

int simularOPT(int num_quadros, int* referencias_paginas, int num_referencias);
int encontrarPaginaParaSubstituir_OPT(int* quadros, int num_quadros, int* referencias_paginas, int num_referencias, int indice_atual_ref);

int main(int argc, char *argv[]) {

    int num_quadros = atoi(argv[1]);
    if (num_quadros <= 0) {
        fprintf(stderr, "Numero de quadros deve ser um inteiro positivo.\n");
        return 1;
    }

    int* referencias_paginas = NULL;
    int num_referencias = 0;
    int capacidade = 10;

    referencias_paginas = (int*)malloc(capacidade * sizeof(int));
    if (referencias_paginas == NULL) {
        perror("Erro ao alocar memoria para referencias de pagina");
        return 1;
    }

    int referencia_pagina;
    while (scanf("%d", &referencia_pagina) == 1) {
        if (num_referencias == capacidade) {
            capacidade *= 2;
            int* temp = (int*)realloc(referencias_paginas, capacidade * sizeof(int));
            if (temp == NULL) {
                perror("Erro ao realocar memoria para referencias de pagina");
                free(referencias_paginas);
                return 1;
            }
            referencias_paginas = temp;
        }
        referencias_paginas[num_referencias++] = referencia_pagina;
    }
    
    int faltas_fifo = simularFIFO(num_quadros, referencias_paginas, num_referencias);
    int faltas_lru = simularLRU(num_quadros, referencias_paginas, num_referencias); 
    int faltas_opt = simularOPT(num_quadros, referencias_paginas, num_referencias);

    printf("%5d quadros, %7d refs: FIFO: %5d PFs, LRU: %5d PFs, OPT: %5d PFs\n",
           num_quadros, num_referencias, faltas_fifo, faltas_lru, faltas_opt);
    
    free(referencias_paginas);
    return 0;
}

// --- Algoritmo FIFO ---
int simularFIFO(int num_quadros, int* referencias_paginas, int num_referencias) {
    int faltas_pagina = 0;
    int* quadros = (int*)malloc(num_quadros * sizeof(int));

    for (int i = 0; i < num_quadros; i++) {
        quadros[i] = -1;
    }

    int indice_proximo_a_sair = 0; // Ponteiro para o elemento mais antigo (o próximo a ser removido)

    for (int i = 0; i < num_referencias; i++) {
        int pagina_atual = referencias_paginas[i];
        bool pagina_encontrada = false;

        // Verifica se a página já está na memória (nos quadros)
        for (int j = 0; j < num_quadros; j++) {
            if (quadros[j] == pagina_atual) {
                pagina_encontrada = true;
                break;
            }
        }

        if (!pagina_encontrada) {
            faltas_pagina++;

            quadros[indice_proximo_a_sair] = pagina_atual; // Coloca a nova página no lugar da página mais antiga
            indice_proximo_a_sair = (indice_proximo_a_sair + 1) % num_quadros; // Move o ponteiro para o próximo quadro (circular)
        }
    }

    free(quadros);
    return faltas_pagina;
}

// --- Algoritmo LRU ---
int simularLRU(int num_quadros, int* referencias_paginas, int num_referencias) {
    int faltas_pagina = 0;
    int* quadros = (int*)malloc(num_quadros * sizeof(int));

    for (int i = 0; i < num_quadros; i++) {
        quadros[i] = -1;
    }

    // Array para armazenar o "tempo" da última vez que cada página foi usada.
    int* tempos_ultimo_uso = (int*)malloc(num_quadros * sizeof(int));
    for (int i = 0; i < num_quadros; i++) {
        tempos_ultimo_uso[i] = 0; // Inicializa com 0 (ou qualquer valor que indique "nunca usado")
    }

    for (int i = 0; i < num_referencias; i++) {
        int pagina_atual = referencias_paginas[i];
        bool pagina_encontrada = false;
        int indice_quadro_encontrado = -1;

        // Verifica se a página já está na memória (nos quadros)
        for (int j = 0; j < num_quadros; j++) {
            if (quadros[j] == pagina_atual) {
                pagina_encontrada = true;
                indice_quadro_encontrado = j;
                break;
            }
        }

        if (pagina_encontrada) {
            // Se a página foi encontrada, atualiza seu tempo de último uso para o tempo atual
            tempos_ultimo_uso[indice_quadro_encontrado] = i; // 'i' é o índice da referência atual, servindo como "tempo"
        } else {
            // Falta de Página (Page Fault)!
            faltas_pagina++;

            // Primeiro, verifica se há quadros vazios
            bool quadro_vazio_encontrado = false;
            for (int k = 0; k < num_quadros; k++) {
                if (quadros[k] == -1) {
                    quadros[k] = pagina_atual; // Coloca a página no quadro vazio
                    tempos_ultimo_uso[k] = i;  // Atualiza seu tempo de último uso
                    quadro_vazio_encontrado = true;
                    break;
                }
            }

            if (!quadro_vazio_encontrado) {
                // Se não há quadros vazios, precisamos substituir a página menos recentemente usada
                int indice_para_substituir = 0;
                int menor_tempo_uso = tempos_ultimo_uso[0];

                // Encontra o quadro com o menor tempo de último uso
                for (int k = 1; k < num_quadros; k++) {
                    if (tempos_ultimo_uso[k] < menor_tempo_uso) {
                        menor_tempo_uso = tempos_ultimo_uso[k];
                        indice_para_substituir = k;
                    }
                }

                quadros[indice_para_substituir] = pagina_atual; // Substitui a página
                tempos_ultimo_uso[indice_para_substituir] = i;  // Atualiza o tempo de uso da nova página
            }
        }
    }

    free(quadros); 
    free(tempos_ultimo_uso);
    return faltas_pagina;
}

// --- Algoritmo OPT ---
int simularOPT(int num_quadros, int* referencias_paginas, int num_referencias) {
    int faltas_pagina = 0;
    int* quadros = (int*)malloc(num_quadros * sizeof(int));

    for (int i = 0; i < num_quadros; i++) {
        quadros[i] = -1;
    }

    for (int i = 0; i < num_referencias; i++) {
        int pagina_atual = referencias_paginas[i];
        bool pagina_encontrada = false;

        // Verifica se a página já está na memória (nos quadros)
        for (int j = 0; j < num_quadros; j++) {
            if (quadros[j] == pagina_atual) {
                pagina_encontrada = true;
                break;
            }
        }

        if (!pagina_encontrada) {
            // Falta de Página
            faltas_pagina++;

            // Primeiro, verifica se há quadros vazios
            bool quadro_vazio_encontrado = false;
            for (int k = 0; k < num_quadros; k++) {
                if (quadros[k] == -1) {
                    quadros[k] = pagina_atual; // Coloca a página no quadro vazio
                    quadro_vazio_encontrado = true;
                    break;
                }
            }

            if (!quadro_vazio_encontrado) {
                // Se não há quadros vazios, precisamos substituir.
                // Usa a função auxiliar para encontrar a página a ser substituída.
                int indice_para_substituir = encontrarPaginaParaSubstituir_OPT(quadros, num_quadros, referencias_paginas, num_referencias, i);
                quadros[indice_para_substituir] = pagina_atual;
            }
        }
    }

    free(quadros);
    return faltas_pagina;
}

// Função auxiliar para encontrar qual página substituir no OPT
// Retorna o índice do quadro que deve ser substituído
int encontrarPaginaParaSubstituir_OPT(int* quadros, int num_quadros, int* referencias_paginas, int num_referencias, int indice_atual_ref) {
    int indice_substituir = -1;
    int max_distancia_futura = -1; // Armazena a maior distância para o próximo uso

    for (int i = 0; i < num_quadros; i++) {
        int pagina_no_quadro = quadros[i];
        int distancia_futura = 0; // Distância até a próxima ocorrência da página

        // Procura a próxima ocorrência da página_no_quadro na sequência futura
        bool encontrada_no_futuro = false;
        for (int k = indice_atual_ref + 1; k < num_referencias; k++) {
            if (referencias_paginas[k] == pagina_no_quadro) {
                distancia_futura = k - indice_atual_ref;
                encontrada_no_futuro = true;
                break;
            }
        }

        if (!encontrada_no_futuro) {
            // Se a página não será mais usada no futuro, ela é a melhor candidata
            return i;
        }

        // Atualiza o candidato se esta página for usada mais tarde que a atual `max_distancia_futura`
        if (distancia_futura > max_distancia_futura) {
            max_distancia_futura = distancia_futura;
            indice_substituir = i;
        }
    }
    return indice_substituir;
}