#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Definição dos tokens (átomos)
typedef enum {
    ERRO = 0,
    NUMERO = 1,
    IDENTIFICADOR = 2,
    OP_SOMA = 3,
    OP_MULT = 4,
    ABRE_PAR = 5,
    FECHA_PAR = 6,
    FIM_ARQUIVO = 7
} TAtomo;

// Estrutura para informações do átomo
typedef struct {
    TAtomo atomo;
    char lexema[50];
    int valor;  // para números
} TInfoAtomo;

// Variáveis globais
char *buffer;           // ponteiro para o código fonte
TInfoAtomo lookahead;   // token atual sendo analisado
int pos_buffer = 0;     // posição atual no buffer

// Protótipos das funções do analisador léxico
TInfoAtomo obter_atomo();
void pular_espacos();
int eh_digito(char c);
int eh_letra(char c);
int eh_alfanumerico(char c);

// Protótipos das funções do analisador sintático
void consome(TAtomo atomo_esperado);
void erro_sintatico(const char* mensagem);
void E();    // Expressão
void E_linha(); // E'
void T();    // Termo
void T_linha(); // T'
void F();    // Fator

// =============================================================================
// ANALISADOR LÉXICO
// =============================================================================

void pular_espacos() {
    while (buffer[pos_buffer] == ' ' || buffer[pos_buffer] == '\t' || 
           buffer[pos_buffer] == '\n' || buffer[pos_buffer] == '\r') {
        pos_buffer++;
    }
}

int eh_digito(char c) {
    return (c >= '0' && c <= '9');
}

int eh_letra(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

int eh_alfanumerico(char c) {
    return eh_letra(c) || eh_digito(c);
}

TInfoAtomo obter_atomo() {
    TInfoAtomo info;
    int inicio_lexema;
    
    pular_espacos();
    
    // Verifica fim do arquivo
    if (buffer[pos_buffer] == '\0') {
        info.atomo = FIM_ARQUIVO;
        strcpy(info.lexema, "EOF");
        return info;
    }
    
    char c = buffer[pos_buffer];
    
    // Reconhece números
    if (eh_digito(c)) {
        inicio_lexema = pos_buffer;
        int valor = 0;
        
        while (eh_digito(buffer[pos_buffer])) {
            valor = valor * 10 + (buffer[pos_buffer] - '0');
            pos_buffer++;
        }
        
        info.atomo = NUMERO;
        info.valor = valor;
        strncpy(info.lexema, &buffer[inicio_lexema], pos_buffer - inicio_lexema);
        info.lexema[pos_buffer - inicio_lexema] = '\0';
        
        return info;
    }
    
    // Reconhece identificadores
    if (eh_letra(c)) {
        inicio_lexema = pos_buffer;
        
        while (eh_alfanumerico(buffer[pos_buffer])) {
            pos_buffer++;
        }
        
        info.atomo = IDENTIFICADOR;
        strncpy(info.lexema, &buffer[inicio_lexema], pos_buffer - inicio_lexema);
        info.lexema[pos_buffer - inicio_lexema] = '\0';
        
        return info;
    }
    
    // Reconhece operadores e símbolos
    switch (c) {
        case '+':
            info.atomo = OP_SOMA;
            strcpy(info.lexema, "+");
            pos_buffer++;
            break;
        case '*':
            info.atomo = OP_MULT;
            strcpy(info.lexema, "*");
            pos_buffer++;
            break;
        case '(':
            info.atomo = ABRE_PAR;
            strcpy(info.lexema, "(");
            pos_buffer++;
            break;
        case ')':
            info.atomo = FECHA_PAR;
            strcpy(info.lexema, ")");
            pos_buffer++;
            break;
        default:
            info.atomo = ERRO;
            sprintf(info.lexema, "ERRO: char '%c'", c);
            pos_buffer++;
    }
    
    return info;
}

// =============================================================================
// ANALISADOR SINTÁTICO
// =============================================================================

void consome(TAtomo atomo_esperado) {
    if (lookahead.atomo == atomo_esperado) {
        printf("Consumindo: %s\n", lookahead.lexema);
        lookahead = obter_atomo();  // Obtém próximo token
    } else {
        printf("ERRO SINTÁTICO: Esperado token %d, encontrado %s\n", 
               atomo_esperado, lookahead.lexema);
        exit(1);
    }
}

void erro_sintatico(const char* mensagem) {
    printf("ERRO SINTÁTICO: %s. Token atual: %s\n", mensagem, lookahead.lexema);
    exit(1);
}

// E -> T E'
void E() {
    printf("Entrando em E()\n");
    T();        // Chama T
    E_linha();  // Chama E'
    printf("Saindo de E()\n");
}

// E' -> + T E' | ε
void E_linha() {
    printf("Entrando em E'()\n");
    
    if (lookahead.atomo == OP_SOMA) {
        consome(OP_SOMA);  // Consome '+'
        T();               // Chama T
        E_linha();         // Chama E' recursivamente
    }
    // Caso contrário: produção epsilon (não faz nada)
    
    printf("Saindo de E'()\n");
}

// T -> F T'
void T() {
    printf("Entrando em T()\n");
    F();        // Chama F
    T_linha();  // Chama T'
    printf("Saindo de T()\n");
}

// T' -> * F T' | ε
void T_linha() {
    printf("Entrando em T'()\n");
    
    if (lookahead.atomo == OP_MULT) {
        consome(OP_MULT);  // Consome '*'
        F();               // Chama F
        T_linha();         // Chama T' recursivamente
    }
    // Caso contrário: produção epsilon (não faz nada)
    
    printf("Saindo de T'()\n");
}

// F -> ( E ) | numero | identificador
void F() {
    printf("Entrando em F()\n");
    
    if (lookahead.atomo == ABRE_PAR) {
        consome(ABRE_PAR);  // Consome '('
        E();                // Chama E
        consome(FECHA_PAR); // Consome ')'
    } else if (lookahead.atomo == NUMERO) {
        consome(NUMERO);    // Consome número
    } else if (lookahead.atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR); // Consome identificador
    } else {
        erro_sintatico("Esperado '(', número ou identificador");
    }
    
    printf("Saindo de F()\n");
}

// =============================================================================
// FUNÇÃO PRINCIPAL E TESTES
// =============================================================================

void analisar_expressao(char* codigo) {
    printf("\n=== ANALISANDO: %s ===\n", codigo);
    
    // Inicializa o analisador
    buffer = codigo;
    pos_buffer = 0;
    
    // Inicializa lookahead com primeiro token
    lookahead = obter_atomo();
    
    // Inicia análise sintática
    E();  // Chama símbolo inicial da gramática
    
    // Verifica se chegou ao fim
    if (lookahead.atomo == FIM_ARQUIVO) {
        printf("✓ ANÁLISE CONCLUÍDA COM SUCESSO!\n");
    } else {
        printf("✗ ERRO: Não chegou ao fim da entrada. Token restante: %s\n", 
               lookahead.lexema);
    }
}

int main() {
    printf("=== ANALISADOR SINTÁTICO TOP-DOWN RECURSIVO PREDITIVO ===\n");
    printf("Gramática:\n");
    printf("E  -> T E'\n");
    printf("E' -> + T E' | ε\n");
    printf("T  -> F T'\n");
    printf("T' -> * F T' | ε\n");
    printf("F  -> ( E ) | numero | identificador\n\n");
    
    // Casos de teste
    char* testes[] = {
        "123",                    // número simples
        "abc",                    // identificador simples
        "123 + 456",             // soma simples
        "abc * 123",             // multiplicação simples
        "123 + 456 * 789",       // precedência (mult antes soma)
        "(123 + 456) * 789",     // parênteses alterando precedência
        "a + b * c + d",         // expressão complexa
        "(a + b) * (c + d)",     // parênteses aninhados
        "123 +",                 // erro: operador sem operando
        "123 + + 456",           // erro: operadores consecutivos
        "",                      // erro: entrada vazia
    };
    
    int num_testes = sizeof(testes) / sizeof(testes[0]);
    
    for (int i = 0; i < num_testes; i++) {
        analisar_expressao(testes[i]);
        printf("\n" "----------------------------------------\n");
    }
    
    return 0;
}

// =============================================================================
// EXEMPLO DE USO INTERATIVO
// =============================================================================

/*
Para usar interativamente, substitua o main() por:

int main() {
    char codigo[1000];
    
    printf("=== ANALISADOR SINTÁTICO TOP-DOWN ===\n");
    printf("Digite uma expressão (ou 'quit' para sair):\n");
    
    while (1) {
        printf("\n> ");
        if (!fgets(codigo, sizeof(codigo), stdin)) break;
        
        // Remove quebra de linha
        codigo[strcspn(codigo, "\n")] = 0;
        
        if (strcmp(codigo, "quit") == 0) break;
        if (strlen(codigo) == 0) continue;
        
        analisar_expressao(codigo);
    }
    
    return 0;
}
*/