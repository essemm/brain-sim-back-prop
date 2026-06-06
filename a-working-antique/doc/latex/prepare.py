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
_cnt = {'part': -1, 'chap': -1, 'sec': 0, 'ssec': 0, 'sssec': 0, 'ssssec': 0,
        'app_ltr': '', 'sapp': 0, 'ssapp': 0}

def _reset_cnt():
    _cnt.update({'part': -1, 'chap': -1, 'sec': 0, 'ssec': 0, 'sssec': 0, 'ssssec': 0,
                 'app_ltr': '', 'sapp': 0, 'ssapp': 0})

def _sub_heading(m):
    r"""Single-pass handler for \ssssec / \sssec / \ssec / \sec / \chap in document order.

    Groups: 1=ssssec title, 2=sssec title, 3=ssec title, 4=sec title, 5=chap title.
    Using one re.sub call guarantees counters are updated in document order —
    separate passes would let e.g. all \sec fire before any \ssec, making
    _cnt['sec'] already at its final value when the first \ssec is reached.
    """
    if m.group(5) is not None:          # \chap
        _cnt['chap'] += 1
        _cnt['sec'] = _cnt['ssec'] = _cnt['sssec'] = _cnt['ssssec'] = 0
        return r'\chapter{Chapter ' + str(_cnt['chap']) + ': ' + m.group(5).strip() + '}'
    if m.group(4) is not None:          # \sec
        _cnt['sec'] += 1
        _cnt['ssec'] = _cnt['sssec'] = _cnt['ssssec'] = 0
        n = f'{_cnt["chap"]}.{_cnt["sec"]}'
        return r'\section{' + n + '. ' + m.group(4).strip() + '}'
    if m.group(3) is not None:          # \ssec
        _cnt['ssec'] += 1
        _cnt['sssec'] = _cnt['ssssec'] = 0
        n = f'{_cnt["chap"]}.{_cnt["sec"]}.{_cnt["ssec"]}'
        return r'\subsection{' + n + '. ' + m.group(3).strip() + '}'
    if m.group(2) is not None:          # \sssec
        _cnt['sssec'] += 1
        _cnt['ssssec'] = 0
        n = f'{_cnt["chap"]}.{_cnt["sec"]}.{_cnt["ssec"]}.{_cnt["sssec"]}'
        return r'\subsubsection{' + n + '. ' + m.group(2).strip() + '}'
    # \ssssec (group 1)
    _cnt['ssssec'] += 1
    n = f'{_cnt["chap"]}.{_cnt["sec"]}.{_cnt["ssec"]}.{_cnt["sssec"]}.{_cnt["ssssec"]}'
    return r'\paragraph{' + n + '. ' + m.group(1).strip() + '}'

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
    # Groups: 1=\ssssec, 2=\sssec, 3=\ssec, 4=\sec, 5=\chap  (most-specific first)
    text = re.sub(
        r'^\\ssssec\s+(.+)$|^\\sssec\s+(.+)$|^\\ssec\s+(.+)$|^\\sec\s+(.+)$|^\\chap\s+(.+)$',
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
    # Escaped special characters
    s = re.sub(r'\\%', '%', s)
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
    r"""Convert a \halign body to a Markdown pipe table or an HTML table.

    Two separator styles:
      vrule tables  — template contains \vrule; data cells delimited by &&
      simple tables — no \vrule; data cells delimited by single &

    When any cell carries a \multispan N (colspan > 1), emits an HTML table
    so that spanning headers render correctly with borders.  Plain tables with
    no spanning cells emit standard Markdown pipe syntax.

    Each cell is tracked as [content, colspan] where colspan=0 marks a cell
    that has been absorbed by the preceding span and should be skipped.
    """
    vrule_table = r'\vrule' in halign_body

    rows_raw = re.split(r'\\cr', halign_body)
    # grid_raw: list of rows; each row is a list of [content_str, colspan_int]
    grid_raw = []
    has_colspan = False
    first = True

    for row in rows_raw:
        row = row.strip()
        if not row:
            continue
        if first:
            first = False
            if re.match(r'#|\\strut#', row):
                continue
            if re.match(r'[#\\]', row) and '&' not in row.split('\n')[0]:
                continue
        if 'height2pt' in row and '\\omit' in row:
            continue
        if re.match(r'(\\hline|\\hrule)\s*$', row):
            continue
        row = re.sub(r'^(\\noalign\{[^{}]*\}|\\hline\b|\\hrule\b)\s*', '', row).strip()
        if not row:
            continue
        if re.match(r'\\noalign\b|\\hline\b|\\hrule\b', row):
            continue

        row_stripped = row.rstrip('&').strip()
        if vrule_table:
            parts = row_stripped.split('&&')
            if parts:
                parts[0] = parts[0].lstrip('&').strip()
            if parts and not parts[0]:
                parts = parts[1:]
        else:
            parts = row_stripped.split('&')

        # Build cell list, expanding \multispan N into colspan slots.
        # Strip each part first — TeX line-breaks leave leading \n on some parts.
        cells = []
        for p in parts:
            p = p.strip()
            ms = re.match(r'\\multispan\s*\{?(\d+)\}?\s*', p) if vrule_table else None
            if ms:
                n = int(ms.group(1))
                # In a vrule table \multispan N spans ceil(N/2) logical columns.
                colspan = (n + 1) // 2
                cells.append([clean_tex_text(p[ms.end():]), colspan])
                for _ in range(colspan - 1):
                    cells.append(['', 0])   # absorbed by the span
                if colspan > 1:
                    has_colspan = True
            else:
                cells.append([clean_tex_text(p), 1])

        # Drop trailing non-absorbed empty cells.
        while cells and cells[-1][0] == '' and cells[-1][1] != 0:
            cells.pop()
        if cells:
            grid_raw.append(cells)

    if not grid_raw:
        return ''

    # Normalise all rows to the same cell count.
    ncols = max(len(row) for row in grid_raw)
    for row in grid_raw:
        while len(row) < ncols:
            row.append(['', 1])

    # ------------------------------------------------------------------ HTML
    if has_colspan:
        # Rows before the first row whose lead cell is numeric are header rows.
        first_data = len(grid_raw)
        for i, row in enumerate(grid_raw):
            lead = next((c[0] for c in row if c[0] and c[1] != 0), '')
            if lead and re.match(r'^\d', lead):
                first_data = i
                break

        lines = ['<table border="1">']
        for i, row in enumerate(grid_raw):
            tag = 'th' if i < first_data else 'td'
            cells_html = []
            for content, colspan in row:
                if colspan == 0:
                    continue
                attrs = f' colspan="{colspan}"' if colspan > 1 else ''
                if tag == 'th':
                    attrs += ' align="center"'
                cells_html.append(f'<{tag}{attrs}>{content}</{tag}>')
            lines.append('<tr>' + ''.join(cells_html) + '</tr>')
        lines.append('</table>')
        return '\n'.join(lines)

    # --------------------------------------------------------------- Markdown
    grid = [[c[0] for c in row] for row in grid_raw]

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
    # GitHub requires a blank line before every opening $$ — add one where missing.
    out2 = []
    in_math2 = False
    for line in out:
        if line.rstrip() == '$$':
            if not in_math2 and out2 and out2[-1].strip() != '':
                out2.append('')
            in_math2 = not in_math2
        out2.append(line)
    return '\n'.join(out2)


def fix_inline_math_symbols(text: str) -> str:
    """Convert all remaining inline $...$ math to plain text / Markdown.

    GitHub's KaTeX rendering is unreliable for inline $...$ expressions, so
    this function converts every known pattern to a GitHub-friendly equivalent.
    Patterns are ordered from most-specific to least-specific to avoid partial
    matches.  Display-math $$...$$ blocks are left for the PDF renderer.
    """
    # ── Numbers ──────────────────────────────────────────────────────────────
    # $10\,000$ → 10,000  (TeX thin-space thousands separator)
    text = re.sub(r'\$(\d+)\\,(\d+)\$', r'\1,\2', text)
    # $0.5$  $3.$  $100$ → plain number
    text = re.sub(r'\$(\d+(?:\.\d*)?)\$', r'\1', text)
    # $(4)$ → (4)  (equation cross-references)
    text = re.sub(r'\$\((\d+)\)\$', r'(\1)', text)
    # $+0.3$  $-0.3$ → +0.3  -0.3  (signed numbers, before flag pattern)
    text = re.sub(r'\$([+-]\d+(?:\.\d+)?)\$', r'\1', text)

    # ── Greek letters ────────────────────────────────────────────────────────
    text = re.sub(r'\$\\varepsilon\s*=\s*(\d+(?:\.\d+)?)\$', r'ε = \1', text)
    text = re.sub(r'\$\\varepsilon\$', 'ε', text)
    text = re.sub(r'\$\\alpha\$', 'α', text)

    # ── Δw expressions (before single-letter $w$ rule) ───────────────────────
    text = re.sub(r'\$\\Delta\s+w\(t\)\$', r'$\\Delta w(t)$', text)
    text = re.sub(r'\$\\Delta w\(t-1\)\$', r'$\\Delta w(t-1)$', text)
    # Multiline: $\Delta w = {\partial E \over \partial w}.$ → inline LaTeX
    text = re.sub(r'\$\\Delta w =\s+\{\\partial E \\over \\partial w\}\.\$',
                  r'$\\Delta w = \\partial E/\\partial w.$', text, flags=re.DOTALL)
    # Standalone $\Delta w$ passes through as-is (no rule needed)

    # ── Partial derivatives ──────────────────────────────────────────────────
    text = re.sub(r'\$\\partial E \\over\s+\\partial (\w+)\$', r'$\\partial E/\\partial \1$', text)
    text = re.sub(r'\$\\sum \{?\\partial E \\over\s+\\partial (\w+)\}?\$', r'$\\sum \\partial E/\\partial \1$', text)

    # ── Command-line flags ───────────────────────────────────────────────────
    # Flag with brace-arg and angle-bracket arg: $-c\{{\rm path}\}\langle{\rm file name}\rangle$
    text = re.sub(
        r'\$(-[a-zA-Z])\\\{\{\\rm\s+([^}]+)\}\\}\{?\\langle\{?\\rm\s+([^}]+)\}?\\rangle\$',
        lambda m: f'`{m.group(1)}{{{m.group(2).strip()}}}<{m.group(3).strip()}>`',
        text,
    )
    # Flag with angle-bracket arg only: $-n\langle{\rm name}\rangle$
    text = re.sub(
        r'\$(-[a-zA-Z])\\langle\{?\\rm\s+([^}]+)\}?\\rangle\$',
        lambda m: f'`{m.group(1)}<{m.group(2).strip()}>`',
        text,
    )
    # Simple flags: $-v$  $-1$  $-?$ etc.
    text = re.sub(r'\$(-[a-zA-Z0-9?!]+)\$', lambda m: f'`{m.group(1)}`', text)

    # ── E_TOTAL (before single-letter $E$ rule) ──────────────────────────────
    text = re.sub(r'\$E_\{\\rm TOTAL\}\s*=\s*0\$', r'$E_{\\text{TOTAL}} = 0$', text)
    text = re.sub(r'\$E_\{\\rm TOTAL\}\$', r'$E_{\\text{TOTAL}}$', text)

    # ── Other specific complex expressions ───────────────────────────────────
    text = re.sub(r'\$n_\{\\rm layer\\ 1\}\s*=\s*15\$', r'$n_{\\text{layer 1}} = 15$', text)
    text = re.sub(r'\$y_\{\\text\{bias\}\} = 1,\$', r'$y_{\\text{bias}} = 1,$', text)
    text = re.sub(r'\$w_\{j,\\text\{bias\}\}\$', r'$w_{j,\\text{bias}}$', text)

    # ── Scientific notation and powers (before single-letter rules) ──────────
    text = re.sub(r'\$(\d+(?:\.\d+)?)\\times10\^\{?(\d+)\}?\$', r'\1×10^\2', text)
    text = re.sub(r'\$10\^\{?(\d+)\}?\$', r'10^\1', text)
    text = re.sub(r'\$(\d+)\\times\$', r'\1×', text)          # $1\times$ → 1×
    text = re.sub(r'\$(\d+)\\%\$', r'\1%', text)              # $95\%$ → 95%

    # ── fishNET inline command examples ──────────────────────────────────────
    # ${\rm fishNET\ } -x {\rm \thinspace} -v$ → `fishNET -x -v`
    text = re.sub(
        r'\$\{\\rm fishNET\\ \}\s*([^$]+?)\$',
        lambda m: '`fishNET ' + re.sub(
            r' +', ' ',
            re.sub(r'\{?\\rm\s*(?:\\thinspace\s*)?\}?', ' ', m.group(1)),
        ).strip() + '`',
        text,
    )

    # ── Trademark, standalone single-char math ───────────────────────────────
    text = re.sub(r'\$\^\{\\rm\s+TM\}\$', '™', text)
    text = re.sub(r'\$-\$', '-', text)
    text = re.sub(r'\$=\$', '=', text)

    # ── Approximation \sim ───────────────────────────────────────────────────
    text = re.sub(
        r'\$\\sim\s*([^$]+)\$',
        lambda m: '≈' + re.sub(r'\\times', '×', re.sub(r'\s+', ' ', m.group(1))).strip(),
        text,
    )

    # ── Greek letter assignments / comparisons ────────────────────────────────
    text = re.sub(r'\$\\alpha\s*=\s*([0-9.]+)\$', r'α = \1', text)
    text = re.sub(r'\$\\alpha e\^\{-t\}\s*\\Delta w\(t-1\)\$', r'$\\alpha e^{-t}\\Delta w(t-1)$', text)
    text = re.sub(r'\$\\varepsilon\s*([><=!]+)\s*([^$\n]+)\$', r'ε \1 \2', text)
    text = re.sub(
        r'\$\\lambda\s*=\s*\{(\d+)\\over([\d\\,]+)\}\$',
        lambda m: 'λ = ' + m.group(1) + '/' + m.group(2).replace(r'\,', ','),
        text,
    )
    text = re.sub(
        r'\$\\mu\s*=\s*\{(\d+)\\over(\d+)\}\$',
        lambda m: 'μ = ' + m.group(1) + '/' + m.group(2),
        text,
    )
    text = re.sub(r'\$\\theta_\{\\rm\s+([^}]+)\}\$',
                  lambda m: f'$\\theta_{{\\text{{{m.group(1).strip()}}}}}$', text)
    # θ_word: clean_tex_text() pre-converts \theta→θ and strips $...$ in table cells
    text = re.sub(r'θ_(\w+)', lambda m: f'$\\theta_{{\\text{{{m.group(1)}}}}}$', text)

    # ── Variable = value (possibly multiline) ────────────────────────────────
    text = re.sub(r'\$([a-zA-Z])\s*=\s*(\d+(?:\.\d+)?)\$', r'\1 = \2', text, flags=re.DOTALL)

    # ── Fractions with \over ─────────────────────────────────────────────────
    text = re.sub(r'\$\(\{?(\d+)\\over(\d+)\}?\)\$', r'(\1/\2)', text)
    text = re.sub(r'\$(\d+)\\over(\d+)\$', r'\1/\2', text)

    # ── Additional \times patterns ────────────────────────────────────────────
    text = re.sub(r'\$(\d+(?:\.\d+)?)\\times\s+10\^\{?(\d+)\}?\$', r'\1×10^\2', text)
    text = re.sub(r'\$(\d+)\\times\s+(\d+)\$', r'\1×\2', text)          # $5\times 7$
    text = re.sub(r'\$(\d+)\s+\\times\$', r'\1×', text)                 # $1 \times$

    # ── Hardware / measurement notation ──────────────────────────────────────
    text = re.sub(r'\$\{\\rm\s+([A-Za-z]+)\}(\d+)\$', r'\1\2', text)   # ${\rm V}20$ → V20
    text = re.sub(r'\$(\d+(?:\.\d+)?)\s*\{\\rm\s+([^}]+)\}\$', r'\1 \2', text)  # 4.77 {\rm MHz}
    text = re.sub(r'\$(\d+)\{\\rm\s+([^}]+)\}\$', r'\1\2', text)        # 640{\rm k}

    # ── Standalone subscripts ────────────────────────────────────────────────
    text = re.sub(r'\$_(\d)\$', lambda m: '₀₁₂₃₄₅₆₇₈₉'[int(m.group(1))], text)
    text = re.sub(r'\$_\{\\rm\s+([^}]+)\}\$', r'_\1', text)
    text = re.sub(r'\$_\{([^{}]+)\}\$', r'_\1', text)

    # ── Subscript with \rm (before generic subscript rules) ─────────────────
    text = re.sub(r'\$([a-zA-Z])_\{\\rm\s+([^}]+)\}\$',
                  lambda m: f'${m.group(1)}_{{\\text{{{m.group(2).strip()}}}}}$', text)

    # ── n:G notation ─────────────────────────────────────────────────────────
    text = re.sub(r'\$([a-zA-Z]):([a-zA-Z])\$', r'\1:\2', text)

    # ── Subscripted variables ────────────────────────────────────────────────
    # Subscript + trailing punctuation: $y_j,$ → $y_j$,
    text = re.sub(r'\$([a-zA-Z])_([a-zA-Z0-9])([,:])\$', r'$\1_\2$\3', text)
    # Braced subscript: $w_{ji}$ etc. (only simple alphanumeric content)
    text = re.sub(r'\$([a-zA-Z])_\{([a-zA-Z0-9,]+)\}\$', r'$\1_{\2}$', text)
    # Simple subscript: $x_j$ $y_i$
    text = re.sub(r'\$([a-zA-Z])_([a-zA-Z0-9])\$', r'$\1_\2$', text)

    # ── Single-letter variables ──────────────────────────────────────────────
    # With trailing comma: $i,$ → *i*,
    text = re.sub(r'\$([a-zA-Z]),\$', r'*\1*,', text)
    # Standalone: $i$  $j$  $t$  $E$ etc. → *i* *j* *t* *E*
    text = re.sub(r'\$([a-zA-Z])\$', r'*\1*', text)

    # ── Arithmetic and range expressions ─────────────────────────────────────
    text = re.sub(r'\$([a-zA-Z])-(\d+)\$', r'\1-\2', text)          # $n-1$ → n-1
    # $a_0$th → a₀th  (GitHub math span won't close before an unbroken letter run)
    _SUBS = '₀₁₂₃₄₅₆₇₈₉'
    text = re.sub(
        r'\$([a-zA-Z])_(\d)\$([a-zA-Z]+)',
        lambda m: f'{m.group(1)}{_SUBS[int(m.group(2))]}{m.group(3)}',
        text,
    )
    text = re.sub(
        r'\$([a-zA-Z])_(\d+)\s*\\times\s*([a-zA-Z])_(\d+)\$',
        lambda m: (f'{m.group(1)}{_SUBS[int(m.group(2))]} × '
                   f'{m.group(3)}{_SUBS[int(m.group(4))]}'),
        text,
    )                                                                # $a_0 \times a_1$ → a₀ × a₁
    # Ranges: $X\to Y$ — keep as inline LaTeX with normalised spacing
    text = re.sub(r'\$([^$]+?)\\to\s*([^$]+?)\$',
                  lambda m: f'${m.group(1).strip()} \\to {m.group(2).strip()}$',
                  text)

    # w_N,M weight subscripts in table cells (clean_tex_text strips $w_{N,M}$ → w_N,M)
    text = re.sub(r'\bw_(\d+),(\d+)\b', r'$w_{\1,\2}$', text)

    # ── Special bracket / brace placeholders ────────────────────────────────
    text = re.sub(r'\$\\langle\\rangle\$', '<>', text)
    text = re.sub(r'\$\\\{\\\}\$', '{}', text)

    return text


def fix_display_command_blocks(text: str) -> str:
    """Convert $$...$$ display blocks containing command-line text to centred HTML.

    In the original TeX, usage synopses and program messages are set in display
    math using \\hbox / \\text{} or {\\rm ...} with literal-brace markup (\\{...\\}).
    GitHub's KaTeX cannot render these; convert to a centred <code> block.
    Pure maths display blocks are left untouched.

    Discriminator:
      - Block starts with \\text{  → always a command/message display.
      - Block starts with {\\rm   → command only when it also contains \\{ (literal
        curly braces indicating flag syntax).  Blocks that use {\\rm} purely for
        subscript labels in equations do not contain \\{.
    """
    def _is_command_body(body: str) -> bool:
        s = body.lstrip()
        return (s.startswith(r'\text{') or s.startswith(r'\texttt{')
                or (s.startswith(r'{\rm ') and r'\{' in body))

    def _clean_body(body: str) -> str:
        body = body.strip()
        # Unwrap top-level \texttt{...} (function/command displays)
        m = re.fullmatch(r'\\texttt\{(.*)\}', body, re.DOTALL)
        if m:
            body = m.group(1).replace(r'\_', '_')
            body = re.sub(r'\\ ', ' ', body)
            return re.sub(r'  +', ' ', body).strip()
        # Unwrap top-level \text{...}
        m = re.fullmatch(r'\\text\{(.*)\}', body, re.DOTALL)
        if m:
            body = m.group(1)
        # {\rm text} or {\rm text\ } → text  (roman-font text nodes)
        body = re.sub(
            r'\{\\rm\s+((?:[^{}]|\{[^{}]*\})*?)\}',
            lambda mm: re.sub(r'\\[ \\]$', '', mm.group(1)).strip(),
            body,
        )
        # Inline $...$ → strip and clean content
        def _math(mm):
            c = mm.group(1)
            c = c.replace(r'\langle', '<').replace(r'\rangle', '>')
            c = re.sub(r'\{\\rm\s+(.*?)\}', r'\1', c)
            return c
        body = re.sub(r'\$(.*?)\$', _math, body)
        body = body.replace(r'\{', '{').replace(r'\}', '}')
        body = re.sub(r'_(\d)', lambda mm: '₀₁₂₃₄₅₆₇₈₉'[int(mm.group(1))], body)
        body = body.replace(r'\ldots', '…')
        body = re.sub(r'\\quad(\.)', r'\1', body)   # \quad. → . (sentence-end period)
        body = re.sub(r'\\(?:quad|thinspace|,|;)', ' ', body)
        body = re.sub(r'\\ ', ' ', body)
        body = body.replace('``', '“').replace("''", '”')
        body = re.sub(r'  +', ' ', body).strip()
        return body

    def _replace(m):
        body = m.group(1)
        if _is_command_body(body):
            cmd = _clean_body(body)
            return f'\n<div align="center"><code>{cmd}</code></div>\n'
        return m.group(0)

    return re.sub(r'\$\$\n(.*?)\n\$\$', _replace, text, flags=re.DOTALL)


def link_references(text: str) -> str:
    """Add HTML anchors to reference entries and hyperlink in-text citations.

    Reference entries: \\[N\\] at the start of a line in the References section →
        <a name="ref-N"></a>**[N]**
    In-text citations: \\[N\\] or \\[N,M,...\\] anywhere else →
        [[N]](#ref-N)  or  [[N]](#ref-N)[[M]](#ref-M) ...
    """
    ref_marker = '\n# References\n'
    if ref_marker not in text:
        return text

    pre, post = text.split(ref_marker, 1)

    # Anchor each reference entry (line-start \[N\] in the References section)
    post = re.sub(
        r'^\\\[(\d+)\\\]',
        lambda m: f'<a name="ref-{m.group(1)}"></a>**[{m.group(1)}]**',
        post,
        flags=re.MULTILINE,
    )

    # Link all remaining \[N\] and \[N,M,...\] citation markers
    def _link(m):
        nums = [n.strip() for n in m.group(1).split(',')]
        return ''.join(f'[[{n}]](#ref-{n})' for n in nums)

    pre  = re.sub(r'\\\[(\d+(?:,\s*\d+)*)\\\]', _link, pre)
    post = re.sub(r'\\\[(\d+(?:,\s*\d+)*)\\\]', _link, post)

    return pre + ref_marker + post


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
        m = re.match(r'^(#{1,5}) (.+)$', line)
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
    md = md.replace('\xa0', ' ')           # TeX ~ ties → plain spaces (GitHub math needs ASCII space before $)
    md = FRONT_MATTER_NOISE_RE.sub(FRONT_MATTER, md)
    md = fix_display_math(md)        # normalise $$ spacing first
    md = replace_vbox_tables(md)     # then convert \vbox tables
    md = strip_heading_attrs(md)     # remove {#id .unnumbered} noise
    md = fix_display_command_blocks(md)    # $${\rm fishNET...}$$ → centred <code> (before inline)
    md = fix_inline_math_symbols(md)       # unwrap bare-number $N$, Greek letters, etc.
    md = link_references(md)             # anchor ref entries; hyperlink \[N\] citations
    md = generate_toc(md)                 # insert TOC after front matter
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
