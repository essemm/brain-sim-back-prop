#!/usr/bin/env python3
r"""
Converts the 1988 plain-TeX thesis to a pandoc-readable LaTeX file.

The original uses custom macros (\chap, \sec, \ssec, \sssec, \app, \figure,
\item {tag}, \topinsert/\endinsert) and plain-TeX primitives that pandoc
can't parse. This script expands them to standard LaTeX equivalents and
writes a single combined file ready for pandoc.
"""

import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).parent

CONTENT_FILES = [
    "TFRONT.TEX",
    "THESIS00.TEX",
    "THESIS01.TEX",
    "THESIS02.TEX",
    "THESIS03.TEX",
    "THESIS04.TEX",
    "THESIS05.TEX",
    "THESIS06.TEX",
    "THESIS07.TEX",
    "THESIS08.TEX",
    "THESIS09.TEX",
    "THESISA.TEX",
    "THESISB.TEX",
    "REFS.TEX",
    "ACKNOWLE.TEX",
]

PREAMBLE = r"""\documentclass[12pt]{report}
\usepackage{amsmath}
\usepackage{verbatim}

\begin{document}
"""

POSTAMBLE = r"""
\end{document}
"""


# --- Heading numbering state (reset in main() before processing) ---
_cnt = {'part': -1, 'chap': -1, 'sec': 0, 'ssec': 0, 'sssec': 0,
        'app_ltr': '', 'sapp': 0, 'ssapp': 0}

def _reset_cnt():
    _cnt.update({'part': -1, 'chap': -1, 'sec': 0, 'ssec': 0, 'sssec': 0,
                 'app_ltr': '', 'sapp': 0, 'ssapp': 0})

def _sub_heading(m):
    r"""Single-pass handler for \sssec / \ssec / \sec / \chap in document order.

    Groups: 1=sssec title, 2=ssec title, 3=sec title, 4=chap title.
    Using one re.sub call guarantees counters are updated in document order —
    separate passes would let e.g. all \sec fire before any \ssec, making
    _cnt['sec'] already at its final value when the first \ssec is reached.
    """
    if m.group(4) is not None:          # \chap
        _cnt['chap'] += 1
        _cnt['sec'] = _cnt['ssec'] = _cnt['sssec'] = 0
        return r'\chapter{Chapter ' + str(_cnt['chap']) + ': ' + m.group(4).strip() + '}'
    if m.group(3) is not None:          # \sec
        _cnt['sec'] += 1
        _cnt['ssec'] = _cnt['sssec'] = 0
        n = f'{_cnt["chap"]}.{_cnt["sec"]}'
        return r'\section{' + n + '. ' + m.group(3).strip() + '}'
    if m.group(2) is not None:          # \ssec
        _cnt['ssec'] += 1
        _cnt['sssec'] = 0
        n = f'{_cnt["chap"]}.{_cnt["sec"]}.{_cnt["ssec"]}'
        return r'\subsection{' + n + '. ' + m.group(2).strip() + '}'
    # \sssec (group 1)
    _cnt['sssec'] += 1
    n = f'{_cnt["chap"]}.{_cnt["sec"]}.{_cnt["ssec"]}.{_cnt["sssec"]}'
    return r'\subsubsection{' + n + '. ' + m.group(1).strip() + '}'

def _sub_app(m):
    ltr = m.group(1).strip()
    _cnt['app_ltr'] = ltr
    _cnt['sapp'] = _cnt['ssapp'] = 0
    return r'\chapter*{Appendix ' + ltr + ': ' + m.group(2).strip() + '}'

def _sub_sapp(m):
    return r'\section*{' + m.group(1).strip() + '}'

def _sub_ssapp(m):
    return r'\subsection*{' + m.group(1).strip() + '}'


def skip_braced(text: str, i: int) -> int:
    """Advance i past a balanced {...} group starting at text[i]."""
    assert text[i] == '{'
    depth = 0
    while i < len(text):
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return i


def replace_parts(text: str) -> str:
    r"""Replace \part Title {quote} with \chapter{Title}, handling braced and
    unbraced titles and multi-line quotes."""
    result = []
    i = 0
    for m in re.finditer(r'\\part\b\s*', text):
        result.append(text[i:m.start()])
        i = m.end()
        # Skip spaces
        while i < len(text) and text[i] == ' ':
            i += 1
        # Read title — braced or single word
        if i < len(text) and text[i] == '{':
            j = skip_braced(text, i)
            title = text[i+1:j-1].strip()
            i = j
        else:
            word = re.match(r'\S+', text[i:])
            title = word.group() if word else ''
            i += len(title)
        # Read optional quote group
        quote = ''
        while i < len(text) and text[i] in ' \t\n':
            i += 1
        if i < len(text) and text[i] == '{':
            j = skip_braced(text, i)
            quote = text[i+1:j-1].strip()
            i = j
        _cnt['part'] += 1
        heading = r'\chapter*{Part ' + str(_cnt['part']) + ': ' + title + '}'
        if quote:
            # Clean up plain-TeX font commands in the quote
            quote = re.sub(r'\{\\bf\s+', r'\\textbf{', quote)
            quote = re.sub(r'\{\\sl\s+|\{\\it\s+', r'\\textit{', quote)
            quote = re.sub(r'\\cr\b', ' ', quote)
            quote = re.sub(r'\\hfill?\b', '', quote)
            result.append(heading + '\n\n\\begin{quote}\\textit{' + quote.strip() + '}\\end{quote}')
        else:
            result.append(heading)
    result.append(text[i:])
    return ''.join(result)


def _convert_eqalign(text: str) -> str:
    r"""Replace \eqalign{body} with \begin{aligned}body'\end{aligned}.

    Converts \cr row separators to \\ inside the body so KaTeX/MathJax
    can render the aligned equation block.  Uses brace counting so nested
    braces inside the body are handled correctly.
    """
    result = []
    i = 0
    for m in re.finditer(r'\\eqalign\{', text):
        result.append(text[i:m.start()])
        j = m.end()
        depth = 1
        while j < len(text) and depth > 0:
            if text[j] == '{':
                depth += 1
            elif text[j] == '}':
                depth -= 1
            j += 1
        body = text[m.end():j - 1]
        # If the body contains a table (vbox/halign), just unwrap the \eqalign{}
        # shell — leave the body and its \cr row separators intact so that
        # replace_vbox_tables / halign_to_code can process them later.
        if r'\vbox' in body or r'\halign' in body:
            result.append(body)
        else:
            body = re.sub(r'\\cr\b', r'\\\\', body)
            result.append(r'\begin{aligned}' + body + r'\end{aligned}')
        i = j
    result.append(text[i:])
    return ''.join(result)


def wrap_item_lists(text: str) -> str:
    r"""Wrap bare \item[tag] / \itemitem{tag} sequences in LaTeX list environments.

    Detects consecutive item lines (and their continuation lines) and inserts
    \begin{enumerate} / \begin{itemize} ... \end{...} around them so pandoc can
    render them as proper numbered or bullet lists.
    """
    lines = text.split('\n')
    result = []
    outer = None       # 'enumerate' or 'itemize' or None
    inner = None       # same for \itemitem nesting
    pending_blanks = []

    def close_inner():
        nonlocal inner
        if inner:
            result.append(r'\end{' + inner + '}')
            inner = None

    def close_outer():
        nonlocal outer
        close_inner()
        if outer:
            result.append(r'\end{' + outer + '}')
            outer = None

    for line in lines:
        m_item = re.match(r'\\item\[([^\]]*)\](.*)', line)
        m_sub  = re.match(r'\\itemitem\s*\{([^}]*)\}(.*)', line)

        if m_item:
            # Flush any buffered blank lines and close any inner list
            result.extend(pending_blanks)
            pending_blanks = []
            close_inner()
            tag  = m_item.group(1).strip()
            rest = m_item.group(2)
            # Numeric tag (1., 2., …) → enumerate; anything else → itemize
            ltype = 'enumerate' if re.match(r'\d', tag) else 'itemize'
            if outer is None:
                result.append(r'\begin{' + ltype + '}')
                outer = ltype
            elif outer != ltype:
                close_outer()
                result.append(r'\begin{' + ltype + '}')
                outer = ltype
            result.append(r'\item' + rest)

        elif m_sub:
            result.extend(pending_blanks)
            pending_blanks = []
            tag  = m_sub.group(1).strip()
            rest = m_sub.group(2)
            # Ensure outer list exists
            if outer is None:
                result.append(r'\begin{enumerate}')
                outer = 'enumerate'
            # Start inner list if needed
            if inner is None:
                result.append(r'\begin{enumerate}')
                inner = 'enumerate'
            result.append(r'\item' + rest)

        elif outer is not None:
            if line.strip() == '':
                pending_blanks.append(line)
            elif pending_blanks:
                # Non-blank content after blank lines → list has ended
                close_outer()
                result.extend(pending_blanks)
                pending_blanks = []
                result.append(line)
            else:
                # Continuation text within the current item
                result.append(line)

        else:
            result.extend(pending_blanks)
            pending_blanks = []
            result.append(line)

    result.extend(pending_blanks)
    close_outer()
    return '\n'.join(result)


def process(text: str) -> str:
    # Remove plain-TeX primitives that have no LaTeX equivalent we need
    text = re.sub(r'\\magnification\s*=\s*\\magstep\d', '', text)
    text = re.sub(r'\\pageno\s*=\s*\d+', '', text)
    text = re.sub(r'\\nopagenumbers\b', '', text)
    text = re.sub(r'\\vfill\b', '', text)
    text = re.sub(r'\\eject\b', r'\\newpage', text)
    text = re.sub(r'\\bye\b', '', text)
    text = re.sub(r'\\parskip\s*=.*', '', text)
    text = re.sub(r'\\parindent\s*=.*', '', text)
    text = re.sub(r'\\baselineskip\s*=.*', '', text)
    text = re.sub(r'\\hsize\s*=.*', '', text)
    text = re.sub(r'\\vsize\s*=.*', '', text)
    text = re.sub(r'\\hoffset\s*=.*', '', text)
    text = re.sub(r'\\voffset\s*=.*', '', text)
    text = re.sub(r'\\font\\[a-z]+=\S+.*', '', text)
    text = re.sub(r'\\newcount\\[a-z]+.*', '', text)
    text = re.sub(r'\\global\\[a-z]+=.*', '', text)
    text = re.sub(r'\\footline\s*=.*', '', text)
    text = re.sub(r'\\headline\s*=.*', '', text)
    text = re.sub(r'\\hfil\b', '', text)
    text = re.sub(r'\\hfill\b', '', text)
    text = re.sub(r'\\vskip\s*\S+', '', text)
    text = re.sub(r'\\hskip\s*\S+', '', text)
    text = re.sub(r'\\smallskip\b', '', text)
    text = re.sub(r'\\medskip\b', '', text)
    text = re.sub(r'\\bigskip\b', '', text)
    text = re.sub(r'\\noindent\b', '', text)
    text = re.sub(r'\\centerline\b', '', text)  # strip command, leave braced arg as a group
    text = re.sub(r'\\null\b', '', text)

    # Remove \def redefinitions of the macros we inline-expand below
    text = re.sub(r'\\def\\(nn|nns|Nn|NN|neu|neus|LAYER|NEURON|WEIGHT|struct|tablerule)\b.*', '', text)

    # Expand shorthand text macros inline (including trailing control-space \<macro>\ )
    # Order matters: longer names first to avoid partial matches.
    expansions = [
        (r'\\nns', 'neural networks'),
        (r'\\Nn',  'Neural network'),
        (r'\\NN',  'Neural Network'),
        (r'\\nn',  'neural network'),
        (r'\\neus','neurons'),
        (r'\\neu', 'neuron'),
        (r'\\LAYER',  r'\\texttt{LAYER}'),
        (r'\\NEURON', r'\\texttt{NEURON}'),
        (r'\\WEIGHT', r'\\texttt{WEIGHT}'),
        (r'\\struct', r'\\texttt{struct}'),
    ]
    for macro, expansion in expansions:
        # \macro\  (explicit control space) → expansion + space
        text = re.sub(macro + r'\\ ', expansion + ' ', text)
        # \macro followed by spaces: TeX consumes them — join expansion to next token
        text = re.sub(macro + r' +', expansion, text)
        # \macro at word boundary (followed by punctuation, end, etc.)
        text = re.sub(macro + r'\b', expansion, text)

    # Custom font uses — map to semantic equivalents
    text = re.sub(r'\{\\chapnumfont\s+', r'\\textbf{', text)
    text = re.sub(r'\{\\bigbf\s+', r'\\textbf{', text)
    text = re.sub(r'\{\\bigrm\s+', r'{', text)
    text = re.sub(r'\{\\bigit\s+', r'\\textit{', text)
    text = re.sub(r'\{\\medbf\s+', r'\\textbf{', text)
    text = re.sub(r'\{\\medrm\s+', r'{', text)
    text = re.sub(r'\{\\medit\s+', r'\\textit{', text)
    text = re.sub(r'\{\\tenrm\s+', r'{', text)
    text = re.sub(r'\{\\ninerm\s+', r'{', text)
    text = re.sub(r'\{\\tt\s+', r'\\texttt{', text)

    # Structure macros — single combined pass so counters update in document order.
    # Groups: 1=\sssec, 2=\ssec, 3=\sec, 4=\chap  (most-specific first)
    text = re.sub(
        r'^\\sssec\s+(.+)$|^\\ssec\s+(.+)$|^\\sec\s+(.+)$|^\\chap\s+(.+)$',
        _sub_heading, text, flags=re.MULTILINE,
    )

    # \part {Title} {quote}  →  \chapter*{Part N: Title}
    text = replace_parts(text)

    # \app {A} Title  or  \app A Title  →  \chapter*{Appendix A: Title}
    text = re.sub(r'^\\app\s+\{?([A-Za-z]+)\}?\s+(.+)$', _sub_app, text, flags=re.MULTILINE)
    # \sapp Title  →  \section*{A.N. Title}
    text = re.sub(r'^\\sapp\s+(.+)$',               _sub_sapp,  text, flags=re.MULTILINE)
    # \ssapp Title  →  \subsection*{A.N.M. Title}
    text = re.sub(r'^\\ssapp\s+(.+)$',              _sub_ssapp, text, flags=re.MULTILINE)

    # Figure inserts: convert to a simple caption comment so content isn't lost
    # \topinsert / \midinsert ... \endinsert
    text = re.sub(r'\\topinsert\b', r'% --- figure ---', text)
    text = re.sub(r'\\midinsert\b', r'% --- figure ---', text)
    text = re.sub(r'\\endinsert\b', r'% --- end figure ---', text)

    # \figure {N} {size} {Caption text}  →  Figure N: Caption text
    text = re.sub(r'\\figure\s*\{([^}]*)\}\s*\{([^}]*)\}\s*\{([^}]*)\}',
                  lambda m: '\\par\\textbf{Figure ' + m.group(1).strip() + ':} ' + m.group(3).strip(),
                  text)

    # \tablerule  →  \hline
    text = re.sub(r'\\tablerule\b', r'\\hline', text)

    # \eqno(N)  →  \qquad\text{(N)}  (plain TeX equation numbering → right-side label)
    # \tag{} requires equation/align environments; \qquad\text{} works in \[...\] too.
    text = re.sub(r'\\eqno\(([^)]+)\)', r'\\qquad\\text{(\1)}', text)

    # \lower.5ex\hbox{e}  →  e  (plain TeX typographic trick for "Intel" logo)
    text = re.sub(r'\\lower[^\\]*\\hbox\{([^}]*)\}', r'\1', text)

    # \hbox{...}  →  \text{...}  (plain TeX text-in-math → LaTeX amsmath)
    text = re.sub(r'\\hbox\{', r'\\text{', text)

    # \eqalign{body}  →  \begin{aligned}body\end{aligned}  (with \cr → \\)
    # Use brace-counting to find the matching closing brace.
    text = _convert_eqalign(text)

    # Commutative-diagram arrow macros (from THEADER.TEX)
    text = re.sub(r'\\mapright\{[^}]*\}', r'\\rightarrow', text)
    text = re.sub(r'\\mapdown\{[^}]*\}',  r'\\downarrow',  text)
    text = re.sub(r'\\mapup\{[^}]*\}',    r'\\uparrow',    text)

    # \item {tag} text  →  \item[tag] text  (used by wrap_item_lists below)
    text = re.sub(r'\\item\s*\{([^}]*)\}', lambda m: r'\item[' + m.group(1) + ']', text)

    # Wrap \item[...] and \itemitem{...} sequences in LaTeX list environments
    text = wrap_item_lists(text)

    # \input{name} lines inside content files — skip; we've already inlined everything
    text = re.sub(r'^\\input\{[^}]+\}.*$', '', text, flags=re.MULTILINE)

    # \listing filename  →  [Listing: filename]  (source file embed, not available here)
    text = re.sub(r'^\\listing\s+(\S+)$',
                  lambda m: r'\textit{[Listing: ' + m.group(1) + ']}',
                  text, flags=re.MULTILINE)

    # \ref Author, title...  (bibliography entries)
    # Number each [N] sequentially; insert \chapter*{References} before the first.
    if re.search(r'^\\ref\s+', text, flags=re.MULTILINE):
        _rn = [0]
        def _sub_ref(m):
            _rn[0] += 1
            prefix = '\\chapter*{References}\n\n' if _rn[0] == 1 else ''
            return f'{prefix}[{_rn[0]}] '
        text = re.sub(r'^\\ref\s+', _sub_ref, text, flags=re.MULTILINE)

    return text


FRONT_MATTER = """\
*Thesis*

---

**Brain Simulation: Computation in Back Propagation Neural Networks**

---

by **Scott MacGIBBON**

Supervisor: Dr Peter Nickolls

4 November, 1988

---

"""

# Matches the noisy TeX layout residue pandoc emits from TFRONT.TEX.
# Replaces everything from the first line (which starts "to 2cm ...") up to
# the first Markdown heading (# Part 0: ...).
FRONT_MATTER_NOISE_RE = re.compile(
    r'^to \d.*?(?=^# )',
    re.MULTILINE | re.DOTALL,
)


def clean_tex_text(s: str) -> str:
    """Strip TeX markup from a cell to get readable plain text."""
    # TeX commands end when a non-letter follows — use (?![a-zA-Z]) not \b
    # because \b treats _ as a word char, so \alpha\b fails on \alpha_0.
    s = re.sub(r'\\langle(?![a-zA-Z])', '<', s)
    s = re.sub(r'\\rangle(?![a-zA-Z])', '>', s)
    s = re.sub(r'\\rightarrow(?![a-zA-Z])', '→', s)
    s = re.sub(r'\\leftarrow(?![a-zA-Z])', '←', s)
    s = re.sub(r'\\downarrow(?![a-zA-Z])', '↓', s)
    s = re.sub(r'\\uparrow(?![a-zA-Z])', '↑', s)
    s = re.sub(r'\\alpha(?![a-zA-Z])', 'α', s)
    s = re.sub(r'\\beta(?![a-zA-Z])', 'β', s)
    s = re.sub(r'\\gamma(?![a-zA-Z])', 'γ', s)
    s = re.sub(r'\\delta(?![a-zA-Z])', 'δ', s)
    s = re.sub(r'\\epsilon(?![a-zA-Z])', 'ε', s)
    s = re.sub(r'\\varepsilon(?![a-zA-Z])', 'ε', s)
    s = re.sub(r'\\eta(?![a-zA-Z])', 'η', s)
    s = re.sub(r'\\theta(?![a-zA-Z])', 'θ', s)
    s = re.sub(r'\\lambda(?![a-zA-Z])', 'λ', s)
    s = re.sub(r'\\mu(?![a-zA-Z])', 'μ', s)
    s = re.sub(r'\\nu(?![a-zA-Z])', 'ν', s)
    s = re.sub(r'\\pi(?![a-zA-Z])', 'π', s)
    s = re.sub(r'\\sigma(?![a-zA-Z])', 'σ', s)
    s = re.sub(r'\\tau(?![a-zA-Z])', 'τ', s)
    s = re.sub(r'\\phi(?![a-zA-Z])', 'φ', s)
    s = re.sub(r'\\omega(?![a-zA-Z])', 'ω', s)
    s = re.sub(r'\\ldots(?![a-zA-Z])', '...', s)
    s = re.sub(r'\\vdots(?![a-zA-Z])', '⋮', s)
    s = re.sub(r'\\cdots(?![a-zA-Z])', '...', s)
    # Escaped braces → plain braces, then stripped below
    s = re.sub(r'\\[{}]', '', s)
    # TeX control space \ and spacing
    s = re.sub(r'\\ ', ' ', s)
    s = re.sub(r'\\[,;!]', '', s)
    # Strip inline math delimiters, keep content
    s = re.sub(r'\$(.*?)\$', lambda m: m.group(1), s)
    # TeX double quotes: ``word'' → "word"
    s = re.sub(r"``(.*?)''", r'"\1"', s)
    # \multispan{N} or \multispan N — strip the command+count, keep following text
    s = re.sub(r'\\multispan\s*\{[^}]*\}\s*', '', s)
    s = re.sub(r'\\multispan\s*\d+\s*', '', s)
    # Formatting-only commands — strip completely (no content to keep)
    s = re.sub(r'\\(?:noalign|omit|vrule|strut|hline|hrule)(?![a-zA-Z])[^&\n]*', '', s)
    # Remove remaining TeX commands (use (?![a-zA-Z]) to correctly end at _ or digits)
    s = re.sub(r'\\[a-zA-Z]+(?![a-zA-Z])\s*', '', s)
    # Strip grouping braces iteratively
    for _ in range(4):
        s = re.sub(r'\{([^{}]*)\}', r'\1', s)
    # Strip trailing & (column separator residue) and whitespace
    s = s.strip().rstrip('&').strip()
    s = re.sub(r'\s+', ' ', s).strip()
    return s


def halign_to_code(halign_body: str) -> str:
    r"""Convert a \halign body to readable text lines."""
    rows_raw = re.split(r'\\cr', halign_body)
    lines = []
    first = True
    for row in rows_raw:
        row = row.strip()
        if not row:
            continue
        # Skip template row (format spec: starts with # or \strut#)
        if first:
            first = False
            if re.match(r'[#\\]', row) and '&' not in row.split('\n')[0]:
                continue
            if re.match(r'#|\\strut#', row):
                continue
        # Skip spacer rows (height2pt&\omit&&\omit&)
        if 'height2pt' in row and '\\omit' in row:
            continue
        # \hline / \hrule separator → skip (blank lines add noise)
        if re.match(r'(\\hline|\\hrule)\s*$', row):
            continue
        # Strip any leading separators that may precede data in the same \cr segment
        row = re.sub(r'^(\\noalign\{[^{}]*\}|\\hline\b|\\hrule\b)\s*', '', row).strip()
        if not row:
            continue
        # If still starts with a TeX separator command, skip
        if re.match(r'\\noalign\b|\\hline\b|\\hrule\b', row):
            continue
        # Data row — handle both && and & column separators
        row_stripped = row.rstrip('&').strip()
        if '&&' in row_stripped:
            parts = row_stripped.split('&&')
            # Strip leading & from first part (single-& row prefix)
            if parts[0].startswith('&'):
                parts[0] = parts[0][1:]
            # Drop empty leading part
            if not parts[0].strip():
                parts = parts[1:]
            cols = [clean_tex_text(p) for p in parts]
        else:
            cols = [clean_tex_text(c) for c in row_stripped.split('&')]
        cols = [c for c in cols if c]
        if cols:
            lines.append('  '.join(cols))
    # Collapse multiple consecutive blank lines to one
    result, prev_blank = [], False
    for ln in lines:
        blank = (ln == '')
        if blank and prev_blank:
            continue
        result.append(ln)
        prev_blank = blank
    return '\n'.join(result).strip()


def halign_to_table(halign_body: str) -> str:
    r"""Convert a \halign body to a Markdown pipe table.

    Two separator styles:
      vrule tables  — template contains \vrule; data cells delimited by &&
      simple tables — no \vrule; data cells delimited by single &
    Interior empty cells are preserved for column alignment; only trailing
    empty cells are trimmed.
    """
    # Detect separator style from the template
    vrule_table = r'\vrule' in halign_body

    rows_raw = re.split(r'\\cr', halign_body)
    grid = []
    first = True

    for row in rows_raw:
        row = row.strip()
        if not row:
            continue
        # Skip template row (format spec line — has # placeholders)
        if first:
            first = False
            if re.match(r'#|\\strut#', row):
                continue
            if re.match(r'[#\\]', row) and '&' not in row.split('\n')[0]:
                continue
        # Skip spacer rows
        if 'height2pt' in row and '\\omit' in row:
            continue
        # Pure separator rows
        if re.match(r'(\\hline|\\hrule)\s*$', row):
            continue
        # Strip leading \noalign / \hline / \hrule prefix
        row = re.sub(r'^(\\noalign\{[^{}]*\}|\\hline\b|\\hrule\b)\s*', '', row).strip()
        if not row:
            continue
        if re.match(r'\\noalign\b|\\hline\b|\\hrule\b', row):
            continue

        row_stripped = row.rstrip('&').strip()
        if vrule_table:
            # Split by && (vrule columns sit between data columns)
            parts = row_stripped.split('&&')
            # Strip any leading bare & that comes from a template starting with &\vrule#
            if parts:
                parts[0] = parts[0].lstrip('&').strip()
            # Drop now-empty leading part
            if parts and not parts[0]:
                parts = parts[1:]
        else:
            # Simple table: each & is a column separator
            parts = row_stripped.split('&')

        cells = [clean_tex_text(p) for p in parts]
        # Drop trailing empty cells but preserve interior ones for alignment
        while cells and not cells[-1]:
            cells.pop()
        if cells:
            grid.append(cells)

    if not grid:
        return ''

    # Normalize all rows to the same column count
    ncols = max(len(row) for row in grid)
    for row in grid:
        while len(row) < ncols:
            row.append('')

    def fmt_row(cells):
        return '| ' + ' | '.join(cells) + ' |'

    header = fmt_row(grid[0])
    sep    = '| ' + ' | '.join(['---'] * ncols) + ' |'
    body   = '\n'.join(fmt_row(r) for r in grid[1:])
    return header + '\n' + sep + ('\n' + body if body else '')


def _extract_halign_body(text: str, start: int) -> tuple[str, int]:
    r"""Return (body, end) for a \halign{body} starting after the opening {.

    Advances past the matching closing brace.
    """
    depth = 1
    i = start
    while i < len(text) and depth > 0:
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
        i += 1
    return text[start:i - 1], i


def replace_vbox_tables(text: str) -> str:
    r"""Replace all $$...$$  blocks that contain \vbox with fenced code blocks.

    Handles both simple \vbox{\halign{#&\ #\cr}} and complex forms with
    \offinterlineskip, \tabskip, \eqalign side-by-side tables, etc.
    Must be called after fix_display_math.
    """
    # Match display math blocks: $$ ... $$
    # \n?  — optional newline after $$ (handles $$\vbox{ on same line after fix_display_math
    #         leaves some $$ adjacent to \vbox{ when blocks were originally separated by space)
    # (?!\s*\$\$) — negative lookahead: don't match when closing $$ of one block immediately
    #              precedes opening $$ of another (produces $$\n\n$$\vbox{ after fix_display_math)
    pattern = re.compile(r'\$\$[ \t]*\n?(?!\s*\$\$)(.*?)\n\$\$', re.DOTALL)

    def replace(m):
        content = m.group(1)

        # Handle \matrix{...} (plain TeX commutative-diagram / table)
        if '\\matrix' in content and '\\halign' not in content:
            mm = re.search(r'\\matrix\{', content)
            if mm:
                body, _ = _extract_halign_body(content, mm.end())
                code = halign_to_code(body)
                return '```\n' + code + '\n```' if code else m.group(0)

        if '\\vbox' not in content and '\\halign' not in content:
            return m.group(0)

        # Find all \halign{ bodies using brace-aware extraction
        tables = []
        for hm in re.finditer(r'\\halign\{', content):
            body, _ = _extract_halign_body(content, hm.end())
            tbl = halign_to_table(body)
            if tbl:
                tables.append(tbl)

        return '\n\n'.join(tables) if tables else m.group(0)

    return pattern.sub(replace, text)


def fix_display_math(text: str) -> str:
    """Ensure every $$ ... $$ display block is on its own line with no inner blank lines.

    Pandoc sometimes emits  $$expr$$ prose  all on one line.  KaTeX chokes
    on this because the second $ in the closing $$ looks like the start of a
    new inline-math span.  Split so the closing $$ and any trailing prose each
    get their own line, separated by a blank line.

    Pandoc also emits blank lines inside $$ ... $$ blocks.  When thesis.md is
    fed back to pandoc for PDF generation, inner blank lines break the display-math
    block into separate paragraphs, causing Markdown italic processing to corrupt
    underscores (e.g. y_i becomes y followed by italic "i w").
    """
    # Match a closing $$ that is followed by non-whitespace on the same line.
    text = re.sub(r'\$\$( *)(\S)', r'$$\n\n\2', text)
    # Match an opening $$ that is preceded by non-whitespace on the same line.
    text = re.sub(r'(\S)( *)\$\$', r'\1\n\n$$', text)
    # Remove blank lines inside $$ ... $$ blocks using a line-scanning approach.
    # The regex approach fails because the non-greedy match stops at the wrong \n$$.
    lines = text.split('\n')
    out = []
    in_math = False
    for line in lines:
        if line.rstrip() == '$$':
            in_math = not in_math
            out.append(line)
        elif in_math and line.strip() == '':
            pass  # drop blank lines inside $$...$$
        else:
            out.append(line)
    return '\n'.join(out)


def _gfm_anchor(text: str) -> str:
    """Compute the GitHub-Flavored Markdown heading anchor for a heading string."""
    s = text.lower()
    s = re.sub(r'[^a-z0-9 -]', '', s)
    s = s.strip()
    s = re.sub(r'\s+', '-', s)
    return s


def generate_toc(text: str) -> str:
    """Insert a Table of Contents after the front-matter block.

    Scans all headings (levels 1–4) and builds a nested link list placed
    between the last '---' rule of the front matter and the first heading.
    Display text preserves the heading exactly (including section numbers
    with dots); anchors follow the GFM algorithm (lowercase, strip
    non-alphanumeric except hyphens, spaces → hyphens).
    """
    lines = text.split('\n')

    # Find the last '---' before the first '#' heading — end of front matter
    last_rule = 0
    for i, line in enumerate(lines):
        if re.match(r'^#{1,6} ', line):
            break
        if line.strip() == '---':
            last_rule = i

    entries = []
    for line in lines:
        m = re.match(r'^(#{1,4}) (.+)$', line)
        if m:
            entries.append((len(m.group(1)), m.group(2).strip()))

    if not entries:
        return text

    toc = ['## Table of Contents', '']
    for level, title in entries:
        indent = '  ' * (level - 1)
        anchor = _gfm_anchor(title)
        toc.append(f'{indent}- [{title}](#{anchor})')
    toc.append('')

    before = lines[:last_rule + 1]
    after  = lines[last_rule + 1:]
    return '\n'.join(before + [''] + toc + after)


def strip_heading_attrs(text: str) -> str:
    r"""Remove pandoc's {#id .class} attribute blocks from heading lines.

    Starred LaTeX commands (\chapter*, \section*, etc.) produce e.g.
    '# References {#references .unnumbered}' in pandoc's Markdown output.
    The attributes are noise for plain-Markdown rendering.
    """
    return re.sub(r'^(#{1,6} .*?) \{[^{}]+\}$', r'\1', text, flags=re.MULTILINE)


def main():
    out_path = HERE.parent / "thesis_pandoc.tex"
    md_out  = HERE.parent / "thesis.md"
    _reset_cnt()          # initialise heading counters before processing
    parts = [PREAMBLE]

    for fname in CONTENT_FILES:
        fpath = HERE / fname
        if not fpath.exists():
            print(f"Warning: {fname} not found, skipping", file=sys.stderr)
            continue
        raw = fpath.read_text(encoding="latin-1", errors="replace")
        if fname == "ACKNOWLE.TEX":
            raw = r"\chapter*{Acknowledgements}" + "\n\n" + raw
        parts.append(f"\n% ---- {fname} ----\n")
        parts.append(process(raw))

    parts.append(POSTAMBLE)
    out_path.write_text("".join(parts), encoding="utf-8")
    print(f"Written: {out_path}")

    # Run pandoc
    result = subprocess.run(
        ["pandoc", "-f", "latex", "-t", "markdown",
         str(out_path), "-o", str(md_out)],
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        print(result.stderr, file=sys.stderr)
        sys.exit(result.returncode)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    print(f"Pandoc wrote: {md_out}")

    # Post-process the markdown
    md = md_out.read_text(encoding="utf-8")
    md = FRONT_MATTER_NOISE_RE.sub(FRONT_MATTER, md)
    md = fix_display_math(md)        # normalise $$ spacing first
    md = replace_vbox_tables(md)     # then convert \vbox tables
    md = strip_heading_attrs(md)     # remove {#id .unnumbered} noise
    md = generate_toc(md)            # insert TOC after front matter
    md_out.write_text(md, encoding="utf-8")
    print(f"Post-processed: {md_out}")

    # Generate PDF — strip the manually-inserted Markdown TOC first; pandoc --toc
    # generates its own correct TOC internally, and the GFM anchor links in our
    # manual TOC produce only "undefined reference" warnings in the LaTeX pass.
    pdf_out = HERE.parent / "thesis.pdf"
    md_for_pdf = re.sub(
        r'^## Table of Contents\n.*?(?=^# )',
        '', md, flags=re.MULTILINE | re.DOTALL,
    )
    result = subprocess.run(
        ["pandoc", "-", "-o", str(pdf_out),
         "--pdf-engine=/usr/local/texlive/2026basic/bin/universal-darwin/xelatex",
         "-V", "mainfont=Palatino",
         "--toc"],
        input=md_for_pdf,
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        print(result.stderr, file=sys.stderr)
        sys.exit(result.returncode)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    print(f"PDF written: {pdf_out}")


if __name__ == "__main__":
    main()
