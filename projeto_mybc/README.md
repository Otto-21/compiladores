# mybc

Interpretador de expressões aritméticas implementado em C como exercício de
compiladores. O programa lê expressões da entrada padrão, avalia cada
expressão e escreve o resultado na saída padrão.

## Funcionalidades implementadas

- Análise léxica de:
  - identificadores iniciados por letra e seguidos por letras ou dígitos;
  - números inteiros, decimais e em notação científica;
  - operadores `+`, `-`, `*`, `/` e `:=`;
  - parênteses;
  - comentários no estilo Pascal, delimitados por `{` e `}`.
- Análise sintática LL(1) com parser recursivo.
- Precedência usual de operadores:
  - multiplicação e divisão antes de soma e subtração;
  - expressões entre parênteses;
  - sinais unários `+` e `-`.
- Atribuição de valores a variáveis, incluindo atribuições encadeadas:

  ```text
  a := b := 5
  ```

- Tabela de símbolos para armazenar variáveis durante a execução.
- Máquina virtual simples com acumulador e pilha de operandos.
- Recuperação de erros ao encontrar uma nova linha, `;` ou o fim da entrada.
- Mensagens de erro com indicação da linha e do tipo de erro.

## Sintaxe

Uma expressão pode ser formada por números, variáveis, operadores e
parênteses:

```text
2 + 3 * 4
(2 + 3) * 4
-10 / 2
```

Variáveis são atribuídas com `:=` e podem ser utilizadas em expressões
posteriores:

```text
raio := 5
area := 3.14 * raio * raio
area
```

Os identificadores seguem o formato `[A-Za-z][A-Za-z0-9]*`. Os resultados são
impressos com uma linha por expressão. Expressões podem ser separadas por
quebra de linha ou ponto e vírgula:

```text
a := 2; b := 3
a + b
```

Comentários são ignorados pelo analisador léxico e podem ocupar várias linhas:

```text
{ este comentário é ignorado }
1 + { comentário entre operandos } 2
```

Dentro de comentários, `\{` e `\}` representam chaves literais. Um comentário
sem fechamento é reportado como erro léxico.

## Compilação

Requisitos:

- compilador C compatível com `make` (o `Makefile` usa `cc`);
- Python 3, apenas para executar a suíte de testes.

Para compilar:

```bash
make
```

O executável gerado é `mybc`. Para remover os arquivos objeto:

```bash
make clean
```

## Uso

O programa não recebe o nome de um arquivo como argumento: a implementação
atual usa a entrada padrão (`stdin`) e a saída padrão (`stdout`).

Execução interativa:

```bash
./mybc
```

Execução com entrada redirecionada:

```bash
printf 'x := 10\nx * 2\n' | ./mybc
```

Saída:

```text
10
20
```

Também é possível usar um arquivo:

```bash
./mybc < entrada.txt
```

## Erros

O parser reporta, entre outros casos:

- token inesperado ou expressão inválida;
- separador ausente após uma expressão;
- divisão por zero;
- uso de variável não declarada;
- estouro da tabela de símbolos;
- estouro ou esvaziamento da pilha de operandos.

Erros léxicos incluem números malformados, identificadores longos, caracteres
inválidos e comentários não fechados. Erros são escritos em `stderr`.

O programa termina com código `0` quando a entrada é processada sem erros.
Falhas reportadas pelo parser resultam em código `1`; erros léxicos fatais
resultam em código `-3` no programa (normalmente observado como `253` pelo
shell).

## Testes

A suíte automatizada está em [`test_mybc.py`](./test_mybc.py) e verifica tokens,
comentários, precedência e associatividade, atribuições, recuperação de erros
e mensagens de diagnóstico.

Depois de compilar, execute:

```bash
python3 test_mybc.py
```

Para testar outro executável ou mostrar os detalhes de cada caso:

```bash
python3 test_mybc.py --bin /caminho/para/mybc
python3 test_mybc.py -v
```

## Organização dos arquivos

- [`main.c`](./main.c): inicialização do programa e conexão de `stdin`/`stdout`.
- [`lexer.c`](./lexer.c) e [`lexer.h`](./lexer.h): análise léxica e
  armazenamento do lexema atual.
- [`tokens.h`](./tokens.h): códigos dos tokens especiais.
- [`parser.c`](./parser.c) e [`parser.h`](./parser.h): gramática LL(1),
  avaliação das expressões e tratamento de erros.
- [`symtab.c`](./symtab.c) e [`symtab.h`](./symtab.h): tabela de símbolos e
  memória das variáveis.
- [`vmachine.c`](./vmachine.c) e [`vmachine.h`](./vmachine.h): acumulador,
  pilha de operandos e operações de armazenamento/recuperação.
- [`Makefile`](./Makefile): regras de compilação e limpeza.
- [`test_mybc.py`](./test_mybc.py): testes automatizados.
