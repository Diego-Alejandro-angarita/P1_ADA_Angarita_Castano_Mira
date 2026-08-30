# Genera la figura "tiempo vs. tamano de entrada" (y nodos vs. n) del informe BT
# a partir de results/BTExperimento.csv.
#
#   python tests/graficaBT.py
#
# Salida: report/figuras/tiempo_vs_n.png

import csv
import math
import os
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import LogLocator, NullFormatter, FuncFormatter

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CSV = os.path.join(RAIZ, "results", "BTExperimento.csv")
SALIDA = os.path.join(RAIZ, "report", "figuras", "tiempo_vs_n.png")

# --- paleta categorica (validada: CVD dE 24.7 / vision normal dE 33.6 sobre #fcfcfb)
SIN_PODA = "#eb6834"   # slot 2, naranja
CON_PODA = "#2a78d6"   # slot 1, azul
SURFACE = "#fcfcfb"
INK = "#0b0b0b"        # texto primario
INK_2 = "#52514e"      # texto secundario
INK_3 = "#8a8983"      # texto terciario / notas
GRID = "#e6e5e1"
BANDA = "#f0efec"      # zona infactible

for familia in ("Calibri", "Carlito", "DejaVu Sans"):
    if any(f.name == familia for f in matplotlib.font_manager.fontManager.ttflist):
        plt.rcParams["font.family"] = familia
        break

SUPER = str.maketrans("0123456789-", "⁰¹²³⁴⁵⁶⁷⁸⁹⁻")


def fmt_tiempo(v, _pos=None):
    """Etiquetas legibles en vez de 10^-7: 100 ns, 10 us, 1 ms, 10 s."""
    if v <= 0:
        return ""
    for umbral, factor, unidad in ((1e-6, 1e9, "ns"), (1e-3, 1e6, "µs"),
                                   (1.0, 1e3, "ms"), (float("inf"), 1.0, "s")):
        if v < umbral:
            return f"{v * factor:,.0f} {unidad}".replace(",", " ")
    return ""


def fmt_nodos(v, _pos=None):
    """1, 10^2, 10^4, ... con superindices Unicode (evita mathtext)."""
    if v <= 0:
        return ""
    exp = round(math.log10(v))
    return "1" if exp == 0 else "10" + str(exp).translate(SUPER)


# ---------------------------------------------------------------- datos
datos = defaultdict(dict)
with open(CSV, newline="", encoding="utf-8") as fh:
    for fila in csv.DictReader(fh):
        datos[fila["variante"]][int(fila["n"])] = {
            "nodos": int(fila["nodos"]),
            "soluciones": int(fila["soluciones"]),
            "segundos": float(fila["segundos"]),
        }

ns = sorted(datos["con_poda"])
serie = lambda v, k: [datos[v][n][k] for n in ns]

SUMA_MIN = 4          # politica (1,1,1,1): ninguna n < 4 admite solucion
FRONTERA = SUMA_MIN - 0.5

fig, (ax_t, ax_n) = plt.subplots(1, 2, figsize=(8.6, 4.35))
fig.patch.set_facecolor(SURFACE)

X0, X1 = min(ns) - 0.42, max(ns) + 1.02


def base(ax, ylabel, titulo, subtitulo, formateador):
    ax.set_facecolor(SURFACE)
    ax.set_yscale("log")
    ax.set_axisbelow(True)
    ax.grid(True, which="major", axis="y", color=GRID, linewidth=0.9)
    ax.grid(True, which="minor", axis="y", color=GRID, linewidth=0.5, alpha=0.55)
    ax.yaxis.set_major_locator(LogLocator(base=100.0, numticks=40))
    ax.yaxis.set_major_formatter(FuncFormatter(formateador))
    ax.yaxis.set_minor_locator(LogLocator(base=10.0, numticks=40))
    ax.yaxis.set_minor_formatter(NullFormatter())

    for lado in ("top", "right", "left"):
        ax.spines[lado].set_visible(False)
    ax.spines["bottom"].set_color(GRID)
    ax.spines["bottom"].set_linewidth(0.9)

    ax.tick_params(axis="y", colors=INK_2, labelsize=8, length=0, pad=4)
    ax.tick_params(axis="x", colors=INK_2, labelsize=9.5, length=0, pad=6)
    ax.set_xlabel("longitud de la contraseña  n", fontsize=9, color=INK_3, labelpad=8)
    ax.set_ylabel(ylabel, fontsize=9, color=INK_3, labelpad=8)
    ax.set_xticks(ns)
    ax.set_xlim(X0, X1)

    # titulo + subtitulo, en coordenadas de eje para un espaciado estable
    ax.set_title("")
    ax.text(0, 1.135, titulo, transform=ax.transAxes, fontsize=11.5,
            color=INK, fontweight="bold", va="baseline")
    ax.text(0, 1.045, subtitulo, transform=ax.transAxes, fontsize=8.5,
            color=INK_3, va="baseline")

    # zona infactible: tinte suave + frontera punteada
    ax.axvspan(X0, FRONTERA, color=BANDA, zorder=0, lw=0)
    ax.axvline(FRONTERA, color="#c9c8c3", lw=0.9, ls=(0, (3, 3)), zorder=1)
    ax.annotate("n < Σmín = 4\ninfactible", xy=(FRONTERA, 1.0),
                xycoords=("data", "axes fraction"), xytext=(-8, -8),
                textcoords="offset points", ha="right", va="top",
                fontsize=7.5, color=INK_3, linespacing=1.45)



def dibujar(ax, ys, color, etiqueta, dy=0):
    ax.plot(ns, ys, color=color, linewidth=2.2, zorder=3, solid_capstyle="round")
    ax.plot(ns, ys, "o", color=color, markersize=6.4, zorder=4,
            markeredgecolor=SURFACE, markeredgewidth=2.0)
    # etiqueta directa en pastilla: el tinte da identidad, el texto va en tinta
    ax.annotate(etiqueta, xy=(ns[-1], ys[-1]), xytext=(12, dy),
                textcoords="offset points", va="center", ha="left",
                fontsize=8.5, color=INK, fontweight="bold", zorder=5,
                bbox=dict(boxstyle="round,pad=0.32", facecolor=color,
                          alpha=0.16, edgecolor="none"))


# ---------------------------------------------------------------- panel A: tiempo
t_sin, t_con = serie("sin_poda", "segundos"), serie("con_poda", "segundos")
ax_t.fill_between(ns, t_con, t_sin, color=CON_PODA, alpha=0.09, lw=0, zorder=1)
dibujar(ax_t, t_sin, SIN_PODA, "sin poda", dy=9)
dibujar(ax_t, t_con, CON_PODA, "con poda", dy=-9)
base(ax_t, "tiempo por corrida", "Tiempo vs. tamaño de entrada",
     "escala logarítmica · más abajo es mejor", fmt_tiempo)

# factor de crecimiento por nivel, sobre la curva sin poda
for i in range(1, len(ns)):
    ax_t.annotate(f"×{t_sin[i] / t_sin[i - 1]:.0f}",
                  xy=((ns[i] + ns[i - 1]) / 2, (t_sin[i] * t_sin[i - 1]) ** 0.5),
                  xytext=(0, 11), textcoords="offset points", ha="center",
                  fontsize=8.5, color=INK_2)

ax_t.annotate("1 nodo · ≈53 ns", xy=(2.5, t_con[0]), xytext=(0, 12),
              textcoords="offset points", ha="center", fontsize=8, color=INK_2)

# ---------------------------------------------------------------- panel B: nodos
n_sin, n_con = serie("sin_poda", "nodos"), serie("con_poda", "nodos")
ax_n.fill_between(ns, n_con, n_sin, color=CON_PODA, alpha=0.09, lw=0, zorder=1)
dibujar(ax_n, n_sin, SIN_PODA, "sin poda", dy=9)
dibujar(ax_n, n_con, CON_PODA, "con poda", dy=-9)
base(ax_n, "nodos visitados", "Nodos recorridos vs. tamaño de entrada",
     "escala logarítmica · más abajo es mejor", fmt_nodos)

ax_n.annotate("1 nodo", xy=(2.5, 1), xytext=(0, 12), textcoords="offset points",
              ha="center", fontsize=8, color=INK_2)
ax_n.annotate("−73 % de nodos", xy=(4, n_con[2]), xytext=(10, -9),
              textcoords="offset points", ha="left", va="center",
              fontsize=8, color=INK_2)

# ---------------------------------------------------------------- leyenda y nota
manejadores = [
    plt.Line2D([], [], color=SIN_PODA, lw=2.2, marker="o", markersize=6.4,
               markeredgecolor=SURFACE, markeredgewidth=1.6,
               label="sin poda — valida solo en la hoja"),
    plt.Line2D([], [], color=CON_PODA, lw=2.2, marker="o", markersize=6.4,
               markeredgecolor=SURFACE, markeredgewidth=1.6,
               label="con poda — corta si faltan > n − pos"),
]
fig.legend(handles=manejadores, loc="lower center", bbox_to_anchor=(0.5, 0.112),
           ncol=2, frameon=False, fontsize=9, handlelength=1.9,
           columnspacing=3.0, labelcolor=INK_2)

# filete fino que separa la nota al pie
fig.add_artist(plt.Line2D([0.09, 0.91], [0.086, 0.086], color=GRID,
                          linewidth=0.9, transform=fig.transFigure))

fig.text(0.5, 0.022,
         "Enumeración completa sin tope · política (1,1,1,1) con no-consecutivos · alfabeto de 67 símbolos · g++ 13.2.0 −O2\n"
         "El área sombreada entre curvas es lo que ahorra la poda. Ambas variantes reportan el mismo número de soluciones en cada n.",
         fontsize=7.5, color=INK_3, va="bottom", ha="center", linespacing=1.6)

fig.subplots_adjust(left=0.095, right=0.985, top=0.845, bottom=0.325, wspace=0.32)

os.makedirs(os.path.dirname(SALIDA), exist_ok=True)
fig.savefig(SALIDA, dpi=240, facecolor=SURFACE)
print("figura ->", SALIDA)
