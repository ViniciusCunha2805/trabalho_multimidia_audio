/*
 * gera_binarios.c
 *
 * Gera três arquivos binários de exatos 10 MB cada, compostos por
 * unsigned char (0–255), de três formas distintas:
 *
 *  1. Modo completamente aleatório   → arquivo1_aleatorio_completo.bin
 *  2. Modo aleatório restrito (0–25) → arquivo2_aleatorio_restrito.bin
 *  3. Modo não aleatório (padrões)   → arquivo3_nao_aleatorio.bin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAMANHO_MB   10
#define BYTES_POR_MB (1024 * 1024)
#define TOTAL_BYTES  (TAMANHO_MB * BYTES_POR_MB)   /* 10 485 760 bytes */
#define BUF_SIZE     (64 * 1024)                    /* buffer de 64 KB  */

/* ------------------------------------------------------------------ */
/* Utilitário: verifica se o arquivo foi criado com o tamanho correto  */
/* ------------------------------------------------------------------ */
void verifica_tamanho(const char *nome)
{
    FILE *f = fopen(nome, "rb");
    if (!f) { perror("fopen verificacao"); return; }
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fclose(f);
    printf("  [OK] %-42s  %ld bytes (%d MB)\n",
           nome, tam, (int)(tam / BYTES_POR_MB));
}

/* ================================================================== */
/* 1. MODO COMPLETAMENTE ALEATÓRIO                                     */
/*    Faixa: 0–255  (toda a faixa de unsigned char)                   */
/* ================================================================== */
void gera_aleatorio_completo(const char *nome, unsigned int semente)
{
    FILE *f = fopen(nome, "wb");
    if (!f) { perror("fopen modo 1"); return; }

    srand(semente);

    unsigned char buf[BUF_SIZE];
    long restante = TOTAL_BYTES;

    while (restante > 0) {
        long bloco = (restante < BUF_SIZE) ? restante : BUF_SIZE;
        for (long i = 0; i < bloco; i++)
            buf[i] = (unsigned char)(rand() % 256);
        fwrite(buf, 1, bloco, f);
        restante -= bloco;
    }

    fclose(f);
    printf("Modo 1 (aleatorio completo) concluido.\n");
    verifica_tamanho(nome);
}

/* ================================================================== */
/* 2. MODO ALEATÓRIO RESTRITO                                          */
/*    Faixa: 0–25  (≈10 % de 256)                                     */
/* ================================================================== */
void gera_aleatorio_restrito(const char *nome, unsigned int semente)
{
    FILE *f = fopen(nome, "wb");
    if (!f) { perror("fopen modo 2"); return; }

    srand(semente);

    unsigned char buf[BUF_SIZE];
    long restante = TOTAL_BYTES;

    while (restante > 0) {
        long bloco = (restante < BUF_SIZE) ? restante : BUF_SIZE;
        for (long i = 0; i < bloco; i++)
            buf[i] = (unsigned char)(rand() % 26);   /* 0–25 inclusive */
        fwrite(buf, 1, bloco, f);
        restante -= bloco;
    }

    fclose(f);
    printf("Modo 2 (aleatorio restrito 0-25) concluido.\n");
    verifica_tamanho(nome);
}

/* ================================================================== */
/* 3. MODO NÃO ALEATÓRIO                                               */
/*    Estratégias combinadas em blocos alternados:                     */
/*                                                                     */
/*   A) Sequência crescente 0→255 repetida                            */
/*   B) Sequência decrescente 255→0 repetida                          */
/*   C) Cadeia de valor constante (ex.: 42 repetido 1 024 vezes)      */
/*   D) Padrão de onda quadrada (0 por 512 bytes, 200 por 512 bytes)  */
/*   E) Padrão de dente-de-serra de 16 em 16 (0,16,32,...,240,0,...)  */
/*   F) Alternância simples par/ímpar (0, 1, 0, 1, ...)               */
/*                                                                     */
/*   Cada bloco tem tamanho fixo de BLOCO_NAO_RAND bytes e os         */
/*   padrões se alternam ciclicamente até completar 10 MB.            */
/* ================================================================== */
#define BLOCO_NAO_RAND (256 * 4)   /* 1 024 bytes por segmento        */
#define NUM_PADROES    6

void preenche_padrao(unsigned char *buf, long tamanho, int padrao)
{
    switch (padrao) {
        case 0:  /* Crescente 0→255 */
            for (long i = 0; i < tamanho; i++)
                buf[i] = (unsigned char)(i % 256);
            break;

        case 1:  /* Decrescente 255→0 */
            for (long i = 0; i < tamanho; i++)
                buf[i] = (unsigned char)(255 - (i % 256));
            break;

        case 2:  /* Constante 42 */
            memset(buf, 42, tamanho);
            break;

        case 3:  /* Onda quadrada: metade 0, metade 200 */
            for (long i = 0; i < tamanho; i++)
                buf[i] = (i < tamanho / 2) ? 0 : 200;
            break;

        case 4:  /* Dente-de-serra de 16 em 16: 0,16,32,...,240,0,... */
            for (long i = 0; i < tamanho; i++)
                buf[i] = (unsigned char)((i % 16) * 16);
            break;

        case 5:  /* Alternância par/ímpar: 0,1,0,1,... */
            for (long i = 0; i < tamanho; i++)
                buf[i] = (unsigned char)(i % 2);
            break;
    }
}

void gera_nao_aleatorio(const char *nome)
{
    FILE *f = fopen(nome, "wb");
    if (!f) { perror("fopen modo 3"); return; }

    unsigned char buf[BLOCO_NAO_RAND];
    long escrito  = 0;
    int  padrao   = 0;

    while (escrito < TOTAL_BYTES) {
        long restante = TOTAL_BYTES - escrito;
        long bloco    = (restante < BLOCO_NAO_RAND) ? restante : BLOCO_NAO_RAND;

        preenche_padrao(buf, bloco, padrao % NUM_PADROES);
        fwrite(buf, 1, bloco, f);

        escrito += bloco;
        padrao++;
    }

    fclose(f);
    printf("Modo 3 (nao aleatorio / padroes) concluido.\n");
    verifica_tamanho(nome);
}

/* ================================================================== */
/* MAIN                                                                */
/* ================================================================== */
int main(void)
{
    unsigned int semente = (unsigned int)time(NULL);
    printf("Semente usada: %u\n\n", semente);

    printf("=== Gerando arquivos de %d MB ===\n\n", TAMANHO_MB);

    gera_aleatorio_completo ("arquivo1_aleatorio_completo.bin",  semente);
    gera_aleatorio_restrito ("arquivo2_aleatorio_restrito.bin",  semente + 1);
    gera_nao_aleatorio      ("arquivo3_nao_aleatorio.bin");

    printf("\nTodos os arquivos gerados com sucesso!\n");
    return 0;
}
