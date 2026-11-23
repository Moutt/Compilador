# Compilador Portugol para MEPA

Projeto desenvolvido para a disciplina de Compiladores, com o objetivo de implementar um compilador para uma linguagem baseada em Portugol, utilizando a linguagem C.

O compilador realiza análise léxica, sintática e semântica, gerando código intermediário na linguagem MEPA.

## Funcionalidades implementadas

- Reconhecimento de tokens e identificadores
- Análise sintática de programas em Portugol
- Tabela de símbolos e controle de escopos
- Declaração de variáveis inteiras
- Operações aritméticas e relacionais
- Estruturas condicionais (`se`, `senao`)
- Estruturas de repetição (`enquanto`)
- Comandos de entrada e saída (`leia`, `escreva`)
- Procedimentos com parâmetros
- Funções com parâmetros e retorno
- Variáveis locais e globais
- Verificações semânticas de declarações e chamadas
- Geração de instruções MEPA

## Compilação

No ambiente Linux ou WSL, utilize:

```bash
gcc -Wall -Wextra -Wno-unused-result -g -Og compilador.c hash_mack.o -o compilador
```

**Observação:** a compilação depende do arquivo `hash_mack.o`, que deve estar disponível localmente.

## Execução

Para compilar um programa Portugol:

```bash
./compilador teste_integrado.txt
```

O compilador gera o arquivo `saida.mepa`.

Para executar o código gerado, utilize um interpretador MEPA compatível:

```bash
/caminho/para/mepa -f saida.mepa
```

No interpretador utilizado durante o desenvolvimento, o comando `e` inicia a execução.

## Exemplo de programa

```text
algoritmo teste;
var contador: inteiro;

funcao inteiro dobro(a: inteiro);
inicio
    dobro := a * 2
fim;

procedimento mostrar(valor: inteiro);
inicio
    escreva(valor)
fim;

inicio
    contador := 1;
    enquanto contador <= 3 faca
    inicio
        se contador > 1 entao
            mostrar(dobro(contador))
        senao
            mostrar(contador);
        contador := contador + 1
    fim
fim.
```

Saída esperada:

```text
1
4
6
```

## Testes

O repositório contém programas de teste para validar operações aritméticas, expressões, condicionais, repetições, funções, procedimentos, parâmetros e variáveis locais.

Também contém testes de erros semânticos, incluindo identificadores não declarados, declarações duplicadas, quantidade incorreta de argumentos e utilização de procedimentos como funções.

## Validação

O projeto foi testado com GCC, interpretador MEPA e Valgrind.

No teste integrado, o Valgrind reportou zero erros e nenhum bloco de memória pendente de liberação.
