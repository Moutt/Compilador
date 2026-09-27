/*
Grupo:
Vitor Tibaes Santos  RA: 10418976
Luis Felipe Cunha    RA: 10419514

Para compilar:
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador
./compilador arquivo.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Átomos do Portugol
typedef enum {
    ERRO, FIM_ARQUIVO,
    ALGORITMO, CARACTERE, DIV, E, ENQUANTO, ENTAO, ESCREVA, FACA, FALSO, 
    FIM, FUNCAO, INICIO, INTEIRO, LEIA, LOGICO, MOD, OU, PROCEDIMENTO, 
    SE, SENAO, VAR, VERDADEIRO,
    IDENTIFICADOR, CONSTINT, CONSTCHAR, COMENTARIO,
    PONTO_VIRGULA, PONTO, DOIS_PONTOS, VIRGULA, ATRIBUICAO,
    ABRE_PAR, FECHA_PAR, ABRE_COL, FECHA_COL,
    OP_MAIOR, OP_MENOR, OP_MAIOR_IGUAL, OP_MENOR_IGUAL,
    OP_IGUAL, OP_DIFERENTE, OP_MAIS, OP_MENOS, OP_MULT
} TAtomo;

typedef struct {
    TAtomo atomo;
    int linha;
    union {
        int numero;       
        char id[16];      
        char ch;          
    } atributo; 
} TInfoAtomo;

// Variáveis globais.
char *buffer;       
int nLinha;         
int linhas_analisadas;
TInfoAtomo lookahead; 

// Funções Léxico.
TInfoAtomo obter_atomo();
void reconhece_numero(TInfoAtomo *infoAtomo);
void reconhece_id(TInfoAtomo *infoAtomo);
void reconhece_constchar(TInfoAtomo *infoAtomo);
void reconhece_simbolos(TInfoAtomo *infoAtomo);   
void reconhece_comentario(TInfoAtomo *infoAtomo);
const char* nome_atomo(TAtomo a);

// Funções Sintático.
void consome(TAtomo esperado);
void programa();
void bloco();
void declaracao_variaveis();
void lista_variaveis();
void tipo();
void declaracao_de_rotinas();
void declaracao_de_funcao();
void declaracao_de_procedimento();
void parametros_formais();
void parametro_formal();
void comando_composto();
void comando();
void comando_atribuicao();
void comando_entrada();
void comando_saida();
void comando_condicional();
void comando_repeticao();
void lista_expressao();
void expressao();
void expressao_simples();
void termo();
void fator();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <arquivo_fonte>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long tamanho = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = (char*)malloc(tamanho + 1);
    if(buffer == NULL){
        printf("Erro ao alocar memoria.\n");
        fclose(f);
        return 1;
    }

    char *ponteiro_original_para_liberar = buffer;
    fread(buffer, 1, tamanho, f);
    buffer[tamanho] = '\0';
    fclose(f);
    
    nLinha = 1;
    linhas_analisadas = 0;

    lookahead = obter_atomo();
    
    while (lookahead.atomo == COMENTARIO) {
        lookahead = obter_atomo();
    }
    
    programa();

    if (lookahead.atomo != FIM_ARQUIVO) {
         printf("# %2d:erro sintatico, codigo apos o fim do programa [%s]\n", lookahead.linha, nome_atomo(lookahead.atomo));
         exit(1);
    }
    
    printf("\n%d linhas analisadas, programa sintaticamente correto\n", linhas_analisadas);
    
    free(ponteiro_original_para_liberar);
    return 0;
}


// ------ ANALISADOR LÉXICO ------


const char* nome_atomo(TAtomo a) {
    switch(a) {
        case ALGORITMO: return "algoritmo"; case CARACTERE: return "caractere";
        case DIV: return "div"; case E: return "e"; case ENQUANTO: return "enquanto";
        case ENTAO: return "entao"; case ESCREVA: return "escreva"; case FACA: return "faca";
        case FALSO: return "falso"; case FIM: return "fim"; case FUNCAO: return "funcao";
        case INICIO: return "inicio"; case INTEIRO: return "inteiro"; case LEIA: return "leia";
        case LOGICO: return "logico"; case MOD: return "mod"; case OU: return "ou";
        case PROCEDIMENTO: return "procedimento"; case SE: return "se"; case SENAO: return "senao";
        case VAR: return "var"; case VERDADEIRO: return "verdadeiro";
        case IDENTIFICADOR: return "identificador"; case CONSTINT: return "constint";
        case CONSTCHAR: return "constchar"; case COMENTARIO: return "comentario";
        case PONTO_VIRGULA: return "ponto_virgula"; case PONTO: return "ponto"; 
        case DOIS_PONTOS: return "dois_pontos"; case VIRGULA: return "virgula"; 
        case ATRIBUICAO: return "atribuicao"; case ABRE_PAR: return "abre_par"; 
        case FECHA_PAR: return "fecha_par"; case OP_MAIOR: return "maior"; 
        case OP_MENOR: return "menor"; case OP_MAIOR_IGUAL: return "maior_igual"; 
        case OP_MENOR_IGUAL: return "menor_igual"; case OP_IGUAL: return "igual"; 
        case OP_DIFERENTE: return "diferente"; case OP_MAIS: return "mais"; 
        case OP_MENOS: return "menos"; case OP_MULT: return "mult";
        case FIM_ARQUIVO: return "EOF"; default: return "erro";
    }
}

TInfoAtomo obter_atomo() {
    TInfoAtomo infoAtomo;
    infoAtomo.atomo = ERRO;

    while (1) {
        while (*buffer == ' ' || *buffer == '\t' || *buffer == '\r') buffer++;

        if (*buffer == '\n') {
            nLinha++;
            buffer++;
            continue;
        }
        break; 
    }

    infoAtomo.linha = nLinha;
    linhas_analisadas = nLinha;

    if (*buffer == '\0') {
        infoAtomo.atomo = FIM_ARQUIVO;
        return infoAtomo;
    } else if (*buffer == '{' && *(buffer + 1) == '-') {
        reconhece_comentario(&infoAtomo);
    } else if (isdigit(*buffer)) {
        reconhece_numero(&infoAtomo);
    } else if (isalpha(*buffer) || *buffer == '_') {
        reconhece_id(&infoAtomo);
    // Verifica se é a aspa padrão simples (') OU a aspa inteligente inicial (‘) em UTF-8 (E2 80 98)
    } else if (*buffer == '\'' || ((unsigned char)buffer[0] == 0xE2 && (unsigned char)buffer[1] == 0x80 && (unsigned char)buffer[2] == 0x98)) {
        reconhece_constchar(&infoAtomo);
    } else {
        reconhece_simbolos(&infoAtomo);
    }
    
    if (infoAtomo.atomo == IDENTIFICADOR) {
        printf("# %2d:identificador: %s\n", infoAtomo.linha, infoAtomo.atributo.id);
    } else if (infoAtomo.atomo == CONSTINT) {
        printf("# %2d:constint: %d\n", infoAtomo.linha, infoAtomo.atributo.numero);
    } else if (infoAtomo.atomo == CONSTCHAR) {
        printf("# %2d:constchar: '%c'\n", infoAtomo.linha, infoAtomo.atributo.ch);
    } else {
        printf("# %2d:%s\n", infoAtomo.linha, nome_atomo(infoAtomo.atomo));
    }

    return infoAtomo;
}

void reconhece_comentario(TInfoAtomo *infoAtomo) {
    int linha_inicio = nLinha;
    buffer += 2; 
    while (*buffer != '\0') {
        if (*buffer == '\n') nLinha++;
        if (*buffer == '-' && *(buffer + 1) == '}') {
            buffer += 2; 
            infoAtomo->atomo = COMENTARIO;
            return;
        }
        buffer++;
    }
    printf("# %2d: erro lexico, comentario nao fechado.\n", linha_inicio);
    exit(1);
}

void reconhece_numero(TInfoAtomo *infoAtomo) {
    char *ini_lexema = buffer;
    while(isdigit(*buffer)) buffer++;
    
    if (tolower(*buffer) == 'e') {
        buffer++;
        if (*buffer == '+' || *buffer == '-') buffer++;
        if (!isdigit(*buffer)) {
            printf("# %2d: erro lexico, notacao exponencial mal formada.\n", infoAtomo->linha);
            exit(1);
        }
        while(isdigit(*buffer)) buffer++;
    }
    
    char num_str[50];
    int len = buffer - ini_lexema;
    strncpy(num_str, ini_lexema, len);
    num_str[len] = '\0';
    
    infoAtomo->atributo.numero = (int)strtod(num_str, NULL);
    infoAtomo->atomo = CONSTINT;
}

void reconhece_id(TInfoAtomo *infoAtomo){
    char *ini_lexema = buffer;
    while(isalnum(*buffer) || *buffer == '_') buffer++;
    int tamanho = buffer - ini_lexema;
    
    if (tamanho > 15) {
        char lexema_erro[50];
        strncpy(lexema_erro, ini_lexema, tamanho);
        lexema_erro[tamanho] = '\0';
        printf("# %2d:erro lexico, identificador excedeu 15 caracteres [%s]\n", infoAtomo->linha, lexema_erro);
        exit(1); 
    }

    strncpy(infoAtomo->atributo.id, ini_lexema, tamanho);
    infoAtomo->atributo.id[tamanho] = '\0';

    char temp[16];
    for(int i = 0; i < tamanho; i++) temp[i] = tolower(infoAtomo->atributo.id[i]);
    temp[tamanho] = '\0';

    if (strcmp(temp, "algoritmo") == 0) infoAtomo->atomo = ALGORITMO;
    else if (strcmp(temp, "caractere") == 0) infoAtomo->atomo = CARACTERE;
    else if (strcmp(temp, "div") == 0) infoAtomo->atomo = DIV;
    else if (strcmp(temp, "e") == 0) infoAtomo->atomo = E;
    else if (strcmp(temp, "enquanto") == 0) infoAtomo->atomo = ENQUANTO;
    else if (strcmp(temp, "entao") == 0) infoAtomo->atomo = ENTAO;
    else if (strcmp(temp, "escreva") == 0) infoAtomo->atomo = ESCREVA;
    else if (strcmp(temp, "faca") == 0) infoAtomo->atomo = FACA;
    else if (strcmp(temp, "falso") == 0) infoAtomo->atomo = FALSO;
    else if (strcmp(temp, "fim") == 0) infoAtomo->atomo = FIM;
    else if (strcmp(temp, "funcao") == 0) infoAtomo->atomo = FUNCAO;
    else if (strcmp(temp, "inicio") == 0) infoAtomo->atomo = INICIO;
    else if (strcmp(temp, "inteiro") == 0) infoAtomo->atomo = INTEIRO;
    else if (strcmp(temp, "leia") == 0) infoAtomo->atomo = LEIA;
    else if (strcmp(temp, "logico") == 0) infoAtomo->atomo = LOGICO;
    else if (strcmp(temp, "mod") == 0) infoAtomo->atomo = MOD;
    else if (strcmp(temp, "ou") == 0) infoAtomo->atomo = OU;
    else if (strcmp(temp, "procedimento") == 0) infoAtomo->atomo = PROCEDIMENTO;
    else if (strcmp(temp, "se") == 0) infoAtomo->atomo = SE;
    else if (strcmp(temp, "senao") == 0) infoAtomo->atomo = SENAO;
    else if (strcmp(temp, "var") == 0) infoAtomo->atomo = VAR;
    else if (strcmp(temp, "verdadeiro") == 0) infoAtomo->atomo = VERDADEIRO;
    else infoAtomo->atomo = IDENTIFICADOR;
}

void reconhece_constchar(TInfoAtomo *infoAtomo){
    int is_smart_quote = 0;
    
    if (*buffer == '\'') {
        buffer++; 
    } else {
        buffer += 3;
        is_smart_quote = 1;
    }

    if (*buffer != '\0') {
        infoAtomo->atributo.ch = *buffer;
        buffer++; 
        
        if (!is_smart_quote && *buffer == '\'') {
            buffer++; 
            infoAtomo->atomo = CONSTCHAR;
            return;
        } else if (is_smart_quote && ((unsigned char)buffer[0] == 0xE2 && (unsigned char)buffer[1] == 0x80 && (unsigned char)buffer[2] == 0x99)) {
            buffer += 3;
            infoAtomo->atomo = CONSTCHAR;
            return;
        }
    }
    
    printf("# %2d: erro lexico, constante char mal formada.\n", infoAtomo->linha);
    exit(1);
}

void reconhece_simbolos(TInfoAtomo *infoAtomo){
    switch(*buffer) {
        case '+': infoAtomo->atomo = OP_MAIS; buffer++; break;
        case '-': infoAtomo->atomo = OP_MENOS; buffer++; break;
        case '*': infoAtomo->atomo = OP_MULT; buffer++; break;
        case ';': infoAtomo->atomo = PONTO_VIRGULA; buffer++; break;
        case ',': infoAtomo->atomo = VIRGULA; buffer++; break;
        case '.': infoAtomo->atomo = PONTO; buffer++; break;
        case '(': infoAtomo->atomo = ABRE_PAR; buffer++; break;
        case ')': infoAtomo->atomo = FECHA_PAR; buffer++; break;
        case '=': infoAtomo->atomo = OP_IGUAL; buffer++; break;
        case ':':
            if (*(buffer + 1) == '=') { infoAtomo->atomo = ATRIBUICAO; buffer += 2; }
            else { infoAtomo->atomo = DOIS_PONTOS; buffer++; }
            break;
        case '<':
            if (*(buffer + 1) == '=') { infoAtomo->atomo = OP_MENOR_IGUAL; buffer += 2; }
            else if (*(buffer + 1) == '>') { infoAtomo->atomo = OP_DIFERENTE; buffer += 2; }
            else { infoAtomo->atomo = OP_MENOR; buffer++; }
            break;
        case '>':
            if (*(buffer + 1) == '=') { infoAtomo->atomo = OP_MAIOR_IGUAL; buffer += 2; }
            else { infoAtomo->atomo = OP_MAIOR; buffer++; }
            break;
        default:
            printf("# %2d: erro lexico, caractere invalido [%c]\n", infoAtomo->linha, *buffer);
            exit(1);
    }
}


// ------ ANALISADOR SINTÁTICO ------


void consome(TAtomo esperado) {
    if (lookahead.atomo == esperado) {
        if (lookahead.atomo != FIM_ARQUIVO) {
            lookahead = obter_atomo();
            
            while (lookahead.atomo == COMENTARIO) {
                lookahead = obter_atomo();
            }
        }
    } else {
        printf("# %2d:erro sintatico, esperado [%s] encontrado [%s]\n", lookahead.linha, nome_atomo(esperado), nome_atomo(lookahead.atomo));
        exit(1);
    }
}

void programa() {
    consome(ALGORITMO); consome(IDENTIFICADOR); consome(PONTO_VIRGULA);
    bloco(); consome(PONTO);
}

void bloco() {
    declaracao_variaveis(); declaracao_de_rotinas(); comando_composto();
}

void declaracao_variaveis() {
    if (lookahead.atomo == VAR) {
        consome(VAR);
        lista_variaveis(); consome(PONTO_VIRGULA);
        while (lookahead.atomo == IDENTIFICADOR) {
            lista_variaveis(); consome(PONTO_VIRGULA);
        }
    }
}

void lista_variaveis() {
    consome(IDENTIFICADOR);
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA); consome(IDENTIFICADOR);
    }
    consome(DOIS_PONTOS); tipo();
}

void tipo() {
    if (lookahead.atomo == CARACTERE) consome(CARACTERE);
    else if (lookahead.atomo == INTEIRO) consome(INTEIRO);
    else if (lookahead.atomo == LOGICO) consome(LOGICO);
    else {
        printf("# %2d:erro sintatico, esperado tipo valido encontrado [%s]\n", lookahead.linha, nome_atomo(lookahead.atomo));
        exit(1);
    }
}

void declaracao_de_rotinas() {
    while (lookahead.atomo == FUNCAO || lookahead.atomo == PROCEDIMENTO) {
        if (lookahead.atomo == FUNCAO) declaracao_de_funcao();
        else declaracao_de_procedimento();
    }
}

void declaracao_de_funcao() {
    consome(FUNCAO); tipo(); consome(IDENTIFICADOR);
    parametros_formais(); declaracao_variaveis(); comando_composto();
}

void declaracao_de_procedimento() {
    consome(PROCEDIMENTO); consome(IDENTIFICADOR);
    parametros_formais(); declaracao_variaveis(); comando_composto();
}

void parametros_formais() {
    if (lookahead.atomo == ABRE_PAR) {
        consome(ABRE_PAR);
        if (lookahead.atomo == FECHA_PAR) {
            consome(FECHA_PAR);
        } else {
            parametro_formal();
            while (lookahead.atomo == PONTO_VIRGULA) {
                consome(PONTO_VIRGULA); parametro_formal();
            }
            consome(FECHA_PAR);
        }
    }
}

void parametro_formal() {
    if (lookahead.atomo == VAR) consome(VAR);
    lista_variaveis();
}

void comando_composto() {
    consome(INICIO);
    comando();
    while (lookahead.atomo == PONTO_VIRGULA) {
        consome(PONTO_VIRGULA); comando();
    }
    consome(FIM);
}

void comando() {
    if (lookahead.atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
        if (lookahead.atomo == ATRIBUICAO) {
            consome(ATRIBUICAO); expressao();
        } else if (lookahead.atomo == ABRE_PAR) {
            consome(ABRE_PAR); lista_expressao(); consome(FECHA_PAR);
        }
    } 
    else if (lookahead.atomo == LEIA) comando_entrada();
    else if (lookahead.atomo == ESCREVA) comando_saida();
    else if (lookahead.atomo == SE) comando_condicional();
    else if (lookahead.atomo == ENQUANTO) comando_repeticao();
    else if (lookahead.atomo == INICIO) comando_composto();
}

void comando_entrada() {
    consome(LEIA); consome(ABRE_PAR); consome(IDENTIFICADOR);
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA); consome(IDENTIFICADOR);
    }
    consome(FECHA_PAR);
}

void comando_saida() {
    consome(ESCREVA); consome(ABRE_PAR);
    lista_expressao(); consome(FECHA_PAR);
}

void comando_condicional() {
    consome(SE); expressao(); consome(ENTAO); comando();
    if (lookahead.atomo == SENAO) {
        consome(SENAO); comando();
    }
}

void comando_repeticao() {
    consome(ENQUANTO); expressao(); consome(FACA); comando();
}

void lista_expressao() {
    expressao();
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA); expressao();
    }
}

void expressao() {
    expressao_simples();
    if (lookahead.atomo >= OP_MAIOR && lookahead.atomo <= OP_DIFERENTE) {
        consome(lookahead.atomo); expressao_simples();
    }
}

void expressao_simples() {
    if(lookahead.atomo == OP_MAIS || lookahead.atomo == OP_MENOS) consome(lookahead.atomo);
    termo();
    while (lookahead.atomo == OP_MAIS || lookahead.atomo == OP_MENOS || lookahead.atomo == MOD || lookahead.atomo == OU) {
        consome(lookahead.atomo); termo();
    }
}

void termo() {
    fator();
    while (lookahead.atomo == OP_MULT || lookahead.atomo == DIV || lookahead.atomo == E) {
        consome(lookahead.atomo); fator();
    }
}

void fator() {
    if (lookahead.atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
        if (lookahead.atomo == ABRE_PAR) {
            consome(ABRE_PAR); lista_expressao(); consome(FECHA_PAR);
        }
    } else if (lookahead.atomo == CONSTINT) consome(CONSTINT);
    else if (lookahead.atomo == CONSTCHAR) consome(CONSTCHAR);
    else if (lookahead.atomo == ABRE_PAR) {
        consome(ABRE_PAR); expressao(); consome(FECHA_PAR);
    } else if (lookahead.atomo == VERDADEIRO) consome(VERDADEIRO);
    else if (lookahead.atomo == FALSO) consome(FALSO);
    else {
        printf("# %2d:erro sintatico, fator mal formado, token: %s\n", lookahead.linha, nome_atomo(lookahead.atomo));
        exit(1);
    }
}