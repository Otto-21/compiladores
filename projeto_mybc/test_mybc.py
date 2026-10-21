#!/usr/bin/env python3
r"""
Suite de testes automatizados para o analisador lexico/parser `mybc`.

Cobre:
  - Tokens ID e numeros decimais
  - skipcomments(): comentarios simples, multilinha, escapados ({\{ \}}),
    consecutivos e nao-fechados
  - Gramatica LL(1) (E, T, F): soma, subtracao, multiplicacao, divisao,
    parenteses, sinal unario
  - Tratamento de erros: EOF prematuro, mismatch de token, comentario
    nao fechado, caractere invalido
  - Dicionario de erros e recuperacao por linha/separador

Uso:
    python3 test_mybc.py                  # usa ./mybc no diretorio atual
    python3 test_mybc.py --bin /caminho/mybc
    python3 test_mybc.py -v                # mostra stdout/stderr de cada teste
"""

import argparse
import subprocess
import sys
from dataclasses import dataclass, field
from typing import Optional

# O parser retorna 1 quando uma expressao falha; o lexer retorna 253 para
# erros lexicos fatais.
EXIT_OK = 0
EXIT_ERROR = 1
EXIT_LEXER_ERROR = 253


@dataclass
class TestCase:
    name: str
    input: str
    expected_exit: int
    stderr_contains: Optional[str] = None
    expected_stdout: Optional[str] = None
    category: str = ""
    note: str = ""


TESTS: list[TestCase] = []


def add(
    name,
    input_,
    expected_exit,
    stderr_contains=None,
    category="",
    note="",
    expected_stdout=None,
):
    TESTS.append(
        TestCase(
            name,
            input_,
            expected_exit,
            stderr_contains,
            expected_stdout,
            category,
            note,
        )
    )


# ---------------------------------------------------------------------------
# 1. TOKEN: ID  ->  [A-Za-z][A-Za-z0-9]*
# ---------------------------------------------------------------------------
add("ID_letra_unica", "a:=1\na", EXIT_OK, category="ID", expected_stdout="1\n1\n")
add("ID_alfanumerico", "abc123:=2\nabc123", EXIT_OK, category="ID", expected_stdout="2\n2\n")
add("ID_maiusculas", "XYZ:=3\nXYZ", EXIT_OK, category="ID", expected_stdout="3\n3\n")
add("ID_mixed_case", "aB3cD4:=4\naB3cD4", EXIT_OK, category="ID", expected_stdout="4\n4\n")

# ---------------------------------------------------------------------------
# 2. TOKEN: DEC  ->  '0' | [1-9][0-9]*
# ---------------------------------------------------------------------------
add("DEC_zero", "0", EXIT_OK, category="DEC")
add("DEC_digito_unico", "5", EXIT_OK, category="DEC")
add("DEC_multiplos_digitos", "12345", EXIT_OK, category="DEC")

# ---------------------------------------------------------------------------
# 3. TOKEN: DECIMAIS
# ---------------------------------------------------------------------------
add("DEC_decimal_ponto", ".5", EXIT_OK, category="DEC", expected_stdout="0.5\n")
add("DEC_notacao_cientifica", "1e2", EXIT_OK, category="DEC", expected_stdout="100\n")

# ---------------------------------------------------------------------------
# 5. skipcomments(): comentarios estilo Pascal { ... }
# ---------------------------------------------------------------------------
add("COMENTARIO_simples_antes", "{ comentario } 5", EXIT_OK, category="COMMENT")
add("COMENTARIO_simples_depois", "5 { comentario }", EXIT_OK, category="COMMENT")
add(
    "COMENTARIO_multilinha",
    "{ linha1\nlinha2\nlinha3 } 7",
    EXIT_OK,
    category="COMMENT",
    note="Comentario cruzando varias linhas nao deve quebrar o parser.",
)
add(
    "COMENTARIO_chaves_escapadas",
    r"{ \{ aninhado \} } 9",
    EXIT_OK,
    category="COMMENT",
    note=r"'\{' e '\}' devem ser tratados como texto comum dentro do comentario.",
)
add(
    "COMENTARIO_consecutivos",
    "{a}{b}5",
    EXIT_OK,
    category="COMMENT",
    note="skipcomments() usa goto _skpspc para consumir varios comentarios seguidos.",
)
add(
    "COMENTARIO_backslash_seguido_de_letra",
    r"{ a \n b } 3",
    EXIT_OK,
    category="COMMENT",
    note=r"'\' seguido de algo que nao e '{' ou '}' nao tem tratamento especial.",
)
add(
    "COMENTARIO_nao_fechado",
    "{ abc",
    EXIT_LEXER_ERROR,
    stderr_contains="unclosed comment",
    category="COMMENT",
)
add(
    "COMENTARIO_nao_fechado_multilinha_conta_linha",
    "{ linha1\nlinha2\nlinha3",
    EXIT_LEXER_ERROR,
    stderr_contains="line 3",
    category="COMMENT",
    note="Confere se a variavel global `line` e incrementada corretamente dentro do comentario.",
)
add(
    "COMENTARIO_sem_expressao_depois",
    "{ so um comentario }",
    EXIT_OK,
    category="COMMENT",
    note="Comentario fechado corretamente, mas nao ha token/expressao apos ele.",
)

# ---------------------------------------------------------------------------
# 6. Gramatica: E -> ['+'|'-'] T R ; T -> F Q ; F -> ID|NUM|'('E')'
# ---------------------------------------------------------------------------
add("EXPR_soma", "1+2", EXIT_OK, category="EXPR")
add("EXPR_subtracao", "5-3", EXIT_OK, category="EXPR")
add("EXPR_multiplicacao", "2*3", EXIT_OK, category="EXPR")
add("EXPR_divisao", "6/2", EXIT_OK, category="EXPR")
add("EXPR_precedencia_mista", "2+3*4", EXIT_OK, category="EXPR")
add(
    "EXPR_resultado_soma",
    "1+2",
    EXIT_OK,
    category="EXPR",
    expected_stdout="3\n",
)
add(
    "EXPR_resultado_subtracao",
    "5-3",
    EXIT_OK,
    category="EXPR",
    expected_stdout="2\n",
)
add(
    "EXPR_resultado_multiplicacao",
    "2*3",
    EXIT_OK,
    category="EXPR",
    expected_stdout="6\n",
)
add(
    "EXPR_resultado_divisao",
    "6/2",
    EXIT_OK,
    category="EXPR",
    expected_stdout="3\n",
)
add(
    "EXPR_resultado_precedencia",
    "2+3*4",
    EXIT_OK,
    category="EXPR",
    expected_stdout="14\n",
)
add(
    "EXPR_resultado_parenteses",
    "(2+3)*4",
    EXIT_OK,
    category="EXPR",
    expected_stdout="20\n",
)
add("EXPR_parenteses", "(1+2)*3", EXIT_OK, category="EXPR")
add("EXPR_parenteses_aninhados", "((1+2)*(3-4))/5", EXIT_OK, category="EXPR")
add(
    "EXPR_id_entre_parenteses",
    "abc:=7\n(abc)",
    EXIT_OK,
    category="EXPR",
    expected_stdout="7\n7\n",
)
add("EXPR_sinal_unario_mais", "+5", EXIT_OK, category="EXPR")
add("EXPR_sinal_unario_menos", "-5", EXIT_OK, category="EXPR")
add("EXPR_unario_seguido_de_binario", "-5+3", EXIT_OK, category="EXPR")
add(
    "EXPR_todos_tipos_de_token_somados",
    "1.5+2e1+123",
    EXIT_OK,
    category="EXPR",
    expected_stdout="144.5\n",
)
add(
    "EXPR_comentario_entre_operandos",
    "1 {soma} + {com} 2",
    EXIT_OK,
    category="EXPR",
    note="Comentarios podem aparecer entre quaisquer tokens, inclusive entre operador e operando.",
)

# ---------------------------------------------------------------------------
# 7. Atribuicao e tabela de simbolos
# ---------------------------------------------------------------------------
add(
    "ASGN_atribuicao",
    "a:=5",
    EXIT_OK,
    category="ASGN",
    expected_stdout="5\n",
)
add(
    "ASGN_atribuicao_com_expressao",
    "resultado:=2+3*4",
    EXIT_OK,
    category="ASGN",
    expected_stdout="14\n",
)
add(
    "ASGN_atribuicao_direita_associativa",
    "a:=b:=5",
    EXIT_OK,
    category="ASGN",
    expected_stdout="5\n",
)


# ---------------------------------------------------------------------------
# 8. Tratamento de erros
# ---------------------------------------------------------------------------
add(
    "ERRO_entrada_vazia",
    "",
    EXIT_OK,
    category="ERROR",
)
add(
    "ERRO_parenteses_sem_fechar",
    "(1+2",
    EXIT_ERROR,
    stderr_contains="received EOF, expected ')'",
    category="ERROR",
)
add(
    "ERRO_operador_duplicado",
    "1+*2",
    EXIT_ERROR,
    stderr_contains="expected '('",
    category="ERROR",
)
add(
    "ERRO_dois_sinais_unarios",
    "-+5",
    EXIT_ERROR,
    stderr_contains="expected '('",
    category="ERROR",
    note="E() so aceita UM sinal de prefixo; o segundo '+' cai no default de F() e falha.",
)
add(
    "ERRO_caractere_invalido",
    "@",
    EXIT_ERROR,
    stderr_contains="invalid token at start of expression",
    category="ERROR",
)
add(
    "ERRO_operador_no_final",
    "1+",
    EXIT_ERROR,
    stderr_contains="received EOF, expected '('",
    category="ERROR",
)
add(
    "ERRO_variavel_nao_declarada",
    "variavel",
    EXIT_ERROR,
    stderr_contains="use of undeclared variable",
    category="ERROR",
)
add(
    "ERRO_lixo_apos_expressao_valida",
    "1+2)",
    EXIT_ERROR,
    stderr_contains="expected expression separator",
    category="ERROR",
)
add(
    "ERRO_divisao_por_zero",
    "1/0",
    EXIT_ERROR,
    stderr_contains="division by zero",
    category="ERROR",
)
add(
    "ERRO_numero_malformado",
    "1e",
    EXIT_LEXER_ERROR,
    stderr_contains="malformed numeric literal",
    category="ERROR",
)
add(
    "ERRO_identificador_longo",
    "a" * 257,
    EXIT_LEXER_ERROR,
    stderr_contains="identifier too long",
    category="ERROR",
)
add(
    "ERRO_recuperacao_apos_linha_invalida",
    "x+1\n2+3\n",
    EXIT_ERROR,
    stderr_contains="use of undeclared variable",
    category="ERROR",
    expected_stdout="5\n",
)
add(
    "ERRO_separador_obrigatorio",
    "1 2",
    EXIT_ERROR,
    stderr_contains="expected expression separator",
    category="ERROR",
)


# ---------------------------------------------------------------------------
# Execucao
# ---------------------------------------------------------------------------

GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
RESET = "\033[0m"


def run_one(binary: str, tc: TestCase, verbose: bool) -> bool:
    try:
        proc = subprocess.run(
            [binary],
            input=tc.input.encode("utf-8"),
            capture_output=True,
            timeout=5,
        )
    except subprocess.TimeoutExpired:
        print(f"{RED}FALHOU{RESET} [{tc.category}] {tc.name} -- timeout (possivel loop infinito)")
        return False
    except FileNotFoundError:
        print(f"Binario '{binary}' nao encontrado. Use --bin /caminho/para/mybc")
        sys.exit(1)

    ok = proc.returncode == tc.expected_exit
    stdout_txt = proc.stdout.decode("utf-8", errors="replace")
    stderr_txt = proc.stderr.decode("utf-8", errors="replace")
    if ok and tc.stderr_contains:
        ok = tc.stderr_contains in stderr_txt
    if ok and tc.expected_stdout is not None:
        ok = stdout_txt == tc.expected_stdout

    status = f"{GREEN}OK{RESET}" if ok else f"{RED}FALHOU{RESET}"
    print(f"{status}  [{tc.category:6s}] {tc.name}")
    if not ok or verbose:
        shown_input = tc.input if len(tc.input) <= 50 else tc.input[:50] + "..."
        print(f"       entrada       : {shown_input!r}")
        print(f"       exit esperado : {tc.expected_exit}   obtido: {proc.returncode}")
        if tc.stderr_contains:
            print(f"       stderr esperado contendo: {tc.stderr_contains!r}")
        if tc.expected_stdout is not None:
            print(f"       stdout esperado: {tc.expected_stdout!r}")
        print(f"       stdout: {stdout_txt!r}")
        print(f"       stderr: {stderr_txt!r}")
        if tc.note:
            print(f"       {YELLOW}nota: {tc.note}{RESET}")
        print()
    return ok


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--bin", default="./mybc", help="caminho para o executavel mybc")
    ap.add_argument("-v", "--verbose", action="store_true", help="mostra detalhes de todos os testes, mesmo os que passaram")
    ap.add_argument("--only", default=None, help="roda apenas testes cujo nome contem esta substring")
    args = ap.parse_args()

    selected = TESTS
    if args.only:
        selected = [t for t in TESTS if args.only.lower() in t.name.lower()]
        if not selected:
            print(f"Nenhum teste encontrado contendo '{args.only}'")
            sys.exit(1)

    print(f"Executando {len(selected)} casos de teste contra '{args.bin}'\n")

    passed = 0
    failed = 0
    for tc in selected:
        if run_one(args.bin, tc, args.verbose):
            passed += 1
        else:
            failed += 1

    total = passed + failed
    print("-" * 60)
    color = GREEN if failed == 0 else RED
    print(f"{color}{passed}/{total} testes passaram{RESET}")
    if failed:
        sys.exit(1)


if __name__ == "__main__":
    main()
