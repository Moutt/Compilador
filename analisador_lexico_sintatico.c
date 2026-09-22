#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    FILE *codigo;
    char *buffer;
    long tamanho_codigo;

    // Verificação de quantidade de argumentos
    if (argc != 2) {
        printf("Uso correto: %s <nome_do_codigo.txt>\n", argv[0]);
        return 1;
    }

    // Abre o arquivo
    codigo = fopen(argv[1], "r");

    // Verifica se o arquivo existe e pode ser lido
    if (codigo == NULL) {
        printf("Erro: Não foi possível abrir o arquivo '%s'.\n", argv[1]);
        return 1; 
    }

    // 1. Descobrir o tamanho_codigo do arquivo
    fseek(codigo, 0, SEEK_END); // Pula para o final do arquivo
    tamanho_codigo = ftell(codigo);    // Pega a posição atual (o tamanho total em bytes)

    // 2. Voltar ao início
    rewind(codigo);             // Retorna o cursor de leitura para o começo

    // 3. Preparar o buffer (alocando tamanho_codigo + 1 byte para o terminador '\0')
    buffer = (char *)malloc((tamanho_codigo + 1) * sizeof(char));
    
    // Boa prática: verificar se o sistema conseguiu alocar a memória
    if (buffer == NULL) {
        printf("Erro: Não foi possível alocar memória suficiente.\n");
        fclose(codigo);
        return 1;
    }

    // 4. Ler tudo de uma vez
    // fread(destino, tamanho_de_cada_item, quantidade_de_itens, arquivo)
    fread(buffer, sizeof(char), tamanho_codigo, codigo);
    
    // Adiciona o caractere nulo no final para transformar em uma string válida em C
    buffer[tamanho_codigo] = '\0';

    // Imprime o conteúdo inteiro do arquivo de uma vez
    printf("%c \n", buffer[1]);
    printf("%s", buffer);

    fclose(codigo);
    free(buffer); // Essencial para evitar vazamento de memória (memory leak)

    return 0;
}