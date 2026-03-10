import serial
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.gridspec as gridspec
from matplotlib.patches import FancyBboxPatch, Circle, Arc, FancyArrowPatch
from matplotlib.animation import FuncAnimation
from matplotlib.colors import LinearSegmentedColormap
import matplotlib.patheffects as pe
import numpy as np
import threading
import time
from collections import deque

# ════════════════════════════════════════
#  CONFIG
# ════════════════════════════════════════
COM_PORT   = "COM3"
BAUD_RATE  = 115200
MAX_POINTS = 100

# ════════════════════════════════════════
#  PREMIUM DARK THEME
# ════════════════════════════════════════
BG          = "#060912"
CARD        = "#0C1120"
CARD2       = "#0E1526"
BORDER      = "#1C2845"
CYAN        = "#00E5FF"
CYAN_DIM    = "#005566"
GREEN       = "#00FF88"
GREEN_DIM   = "#004422"
RED         = "#FF2D55"
RED_DIM     = "#440011"
ORANGE      = "#FF6B2B"
ORANGE_DIM  = "#442200"
YELLOW      = "#FFD60A"
YELLOW_DIM  = "#443300"
PURPLE      = "#BF5FFF"
PURPLE_DIM  = "#2A0044"
BLUE        = "#4D9FFF"
BLUE_DIM    = "#001A44"
WHITE       = "#F0F4FF"
GREY        = "#4A5568"
GREY2       = "#2D3748"

# ════════════════════════════════════════
#  SHARED DATA
# ════════════════════════════════════════
data = {
    "link"        : 0,
    "car1_dist"   : 0,
    "car1_zone"   : 0,
    "car1_spd"    : 0,
    "ir_fl"       : 0,
    "ir_fr"       : 0,
    "tilt"        : 0.0,
    "car2_spd"    : 0,
    "crash"       : 0,
    "emergency"   : 0,
    "braking"     : 0,
    "connected"   : False,
    "msg_count"   : 0,
    "last_update" : 0,
}

hist_dist  = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
hist_spd2  = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
hist_tilt  = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
hist_spd1  = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)

# ════════════════════════════════════════
#  SERIAL THREAD
# ════════════════════════════════════════
def serial_reader():
    while True:
        try:
            ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
            data["connected"] = True
            print(f"✅ Connected to {COM_PORT}")
            while True:
                try:
                    line = ser.readline().decode("utf-8").strip()
                    if line.startswith("/*") and line.endswith("*/"):
                        inner = line[2:-2]
                        parts = inner.split(",")
                        if len(parts) >= 11:
                            data["link"]       = int(parts[0])
                            data["car1_dist"]  = int(parts[1])
                            data["car1_zone"]  = int(parts[2])
                            data["car1_spd"]   = int(parts[3])
                            data["ir_fl"]      = int(parts[4])
                            data["ir_fr"]      = int(parts[5])
                            data["tilt"]       = float(parts[6])
                            data["car2_spd"]   = int(parts[7])
                            data["crash"]      = int(parts[8])
                            data["emergency"]  = int(parts[9])
                            data["braking"]    = int(parts[10])
                            data["msg_count"] += 1
                            data["last_update"]= time.time()
                            hist_dist.append(min(data["car1_dist"], 200))
                            hist_spd2.append(data["car2_spd"])
                            hist_tilt.append(data["tilt"])
                            hist_spd1.append(data["car1_spd"])
                except:
                    continue
        except serial.SerialException:
            data["connected"] = False
            print(f"⚠ Waiting for {COM_PORT}...")
            time.sleep(2)

thread = threading.Thread(target=serial_reader, daemon=True)
thread.start()

# ════════════════════════════════════════
#  DRAWING HELPERS
# ════════════════════════════════════════

def styled_card(ax, edge_color=BORDER, title=None, title_color=None):
    """Draw a styled card background with optional title."""
    ax.set_facecolor(CARD)
    for spine in ax.spines.values():
        spine.set_color(edge_color)
        spine.set_linewidth(1.2)
    if title:
        col = title_color or GREY
        ax.set_title(title, color=col, fontsize=8,
                     fontfamily="monospace", pad=5,
                     fontweight="bold", loc="left")


def draw_arc_gauge(ax, value, min_val, max_val,
                   label, unit, color, dim_color):
    """Premium arc gauge with glow needle."""
    ax.set_facecolor(CARD)
    ax.set_xlim(-1.25, 1.25)
    ax.set_ylim(-1.15, 1.15)
    ax.axis("off")

    START = 220
    SWEEP = 280
    N     = 200

    # ── Track ──
    angles = np.linspace(np.radians(START),
                         np.radians(START - SWEEP), N)
    ax.plot(np.cos(angles), np.sin(angles),
            color=GREY2, lw=10,
            solid_capstyle="round", zorder=1)

    # ── Colored fill ──
    pct   = np.clip((value - min_val) / (max_val - min_val), 0, 1)
    if pct > 0:
        fill_a = np.linspace(np.radians(START),
                             np.radians(START - pct * SWEEP),
                             max(2, int(pct * N)))
        ax.plot(np.cos(fill_a), np.sin(fill_a),
                color=color, lw=10,
                solid_capstyle="round", zorder=2,
                alpha=0.95)
        # Glow layer
        ax.plot(np.cos(fill_a), np.sin(fill_a),
                color=color, lw=16,
                solid_capstyle="round", zorder=1,
                alpha=0.12)

    # ── Tick marks ──
    for i in range(6):
        t    = np.radians(START - i * SWEEP / 5)
        r1, r2 = 0.82, 0.95
        ax.plot([r1*np.cos(t), r2*np.cos(t)],
                [r1*np.sin(t), r2*np.sin(t)],
                color=GREY, lw=1.2, zorder=3)

    # ── Needle ──
    needle_a = np.radians(START - pct * SWEEP)
    nx, ny   = 0.68 * np.cos(needle_a), 0.68 * np.sin(needle_a)
    ax.annotate("",
                xy=(nx, ny), xytext=(0, 0),
                arrowprops=dict(
                    arrowstyle="->,head_width=0.12,head_length=0.1",
                    color=WHITE, lw=2.0))
    # Center dot
    ax.add_patch(Circle((0, 0), 0.07,
                 facecolor=color, zorder=6,
                 edgecolor=WHITE, linewidth=1.5))

    # ── Value ──
    ax.text(0, -0.32,
            f"{value:.1f}" if isinstance(value, float) else f"{int(value)}",
            ha="center", va="center",
            fontsize=17, fontweight="bold",
            color=WHITE, fontfamily="monospace",
            zorder=7)

    # ── Unit + Label ──
    ax.text(0, -0.56, unit,
            ha="center", fontsize=8,
            color=color, fontfamily="monospace")
    ax.text(0, -0.82, label,
            ha="center", fontsize=9,
            fontweight="bold",
            color=GREY, fontfamily="monospace")

    # ── Min / Max ──
    ax.text(-1.1, -0.28, str(min_val),
            ha="center", fontsize=7,
            color=GREY, fontfamily="monospace")
    ax.text(1.1, -0.28, str(max_val),
            ha="center", fontsize=7,
            color=GREY, fontfamily="monospace")


def draw_ir_panel(ax, fl, fr):
    """Clean IR sensor panel showing both sensors."""
    ax.set_facecolor(CARD)
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 4)
    ax.axis("off")

    # Title
    ax.text(5, 3.6, "IR SENSORS",
            ha="center", va="center",
            fontsize=9, fontweight="bold",
            color=GREY, fontfamily="monospace")

    sensors = [
        (2.2, fl,  "FRONT LEFT"),
        (7.8, fr, "FRONT RIGHT"),
    ]

    for cx, detected, lbl in sensors:
        col = RED if detected else GREEN
        dim = RED_DIM if detected else GREEN_DIM
        st  = "BLOCKED" if detected else "CLEAR"

        # Outer ring glow
        if detected:
            for r, a in [(0.95, 0.08), (0.82, 0.15), (0.7, 0.25)]:
                ax.add_patch(Circle((cx, 1.8), r,
                             facecolor="none",
                             edgecolor=col,
                             linewidth=1,
                             alpha=a))

        # Main circle
        ax.add_patch(Circle((cx, 1.8), 0.65,
                     facecolor=dim,
                     edgecolor=col,
                     linewidth=2.5,
                     zorder=3))

        # Icon
        ax.text(cx, 1.85, "●" if detected else "○",
                ha="center", va="center",
                fontsize=16, color=col,
                fontfamily="monospace", zorder=4)

        # Status
        ax.text(cx, 0.85, st,
                ha="center", va="center",
                fontsize=8, fontweight="bold",
                color=col, fontfamily="monospace")
        ax.text(cx, 0.38, lbl,
                ha="center", va="center",
                fontsize=7, color=GREY,
                fontfamily="monospace")


def draw_zone_bar(ax, zone):
    """Sleek zone indicator bar."""
    ax.set_facecolor(CARD)
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis("off")

    ax.text(0.02, 0.82, "DISTANCE ZONE",
            fontsize=8, fontweight="bold",
            color=GREY, fontfamily="monospace",
            transform=ax.transAxes)

    zones = [
        (GREEN,  "SAFE",     "Zone 0",  "< 60cm"),
        (YELLOW, "WARNING",  "Zone 1",  "< 30cm"),
        (ORANGE, "DANGER",   "Zone 2",  "< 15cm"),
        (RED,    "CRITICAL", "Zone 3",  "< 11cm"),
    ]

    w = 0.22
    gap = 0.02
    for i, (col, label, sub, rng) in enumerate(zones):
        x     = 0.02 + i * (w + gap)
        active= (zone == i)

        # Card bg
        rect = FancyBboxPatch(
            (x, 0.06), w, 0.62,
            boxstyle="round,pad=0.015",
            facecolor=col + "33" if active else CARD2,
            edgecolor=col if active else GREY2,
            linewidth=2 if active else 1,
            transform=ax.transAxes,
            clip_on=False)
        ax.add_patch(rect)

        # Glow top line
        if active:
            ax.plot([x, x + w], [0.68, 0.68],
                    color=col, lw=2.5,
                    transform=ax.transAxes,
                    solid_capstyle="round",
                    clip_on=False)

        alpha = 1.0 if active else 0.3
        ax.text(x + w/2, 0.50, label,
                ha="center", va="center",
                fontsize=8, fontweight="bold",
                color=col if active else GREY,
                fontfamily="monospace",
                alpha=alpha,
                transform=ax.transAxes)
        ax.text(x + w/2, 0.33, sub,
                ha="center", va="center",
                fontsize=7, color=GREY,
                fontfamily="monospace",
                alpha=alpha,
                transform=ax.transAxes)
        ax.text(x + w/2, 0.17, rng,
                ha="center", va="center",
                fontsize=6.5, color=GREY,
                fontfamily="monospace",
                alpha=alpha,
                transform=ax.transAxes)


def draw_alert_pill(ax, label, active,
                    on_color, on_bg, icon_on, icon_off):
    """Pill-shaped alert indicator."""
    ax.set_facecolor(CARD)
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis("off")

    col = on_color if active else GREY2
    bg  = on_bg    if active else CARD2

    rect = FancyBboxPatch(
        (0.06, 0.12), 0.88, 0.76,
        boxstyle="round,pad=0.06",
        facecolor=bg,
        edgecolor=col,
        linewidth=2.2 if active else 1,
        transform=ax.transAxes)
    ax.add_patch(rect)

    # Top glow line
    if active:
        ax.plot([0.06, 0.94], [0.88, 0.88],
                color=col, lw=2,
                transform=ax.transAxes,
                solid_capstyle="round")

    icon = icon_on if active else icon_off
    ax.text(0.5, 0.62, icon,
            ha="center", va="center",
            fontsize=16,
            color=col if active else GREY,
            transform=ax.transAxes)
    ax.text(0.5, 0.26, label,
            ha="center", va="center",
            fontsize=8, fontweight="bold",
            color=col if active else GREY,
            fontfamily="monospace",
            transform=ax.transAxes)


def draw_expert_graph(ax, history, label,
                      color, min_v, max_v, unit,
                      show_fill=True):
    """Expert-quality live graph."""
    ax.set_facecolor(CARD)
    for spine in ax.spines.values():
        spine.set_color(BORDER)
        spine.set_linewidth(0.8)
    ax.tick_params(colors=GREY, labelsize=7,
                   length=3, width=0.5)
    ax.xaxis.set_tick_params(labelbottom=False)

    x = np.arange(len(history))
    y = np.array(list(history), dtype=float)

    # ── Gradient fill ──
    if show_fill and len(y) > 1:
        ax.fill_between(x, y, min_v,
                        alpha=0.12, color=color,
                        interpolate=True)
        ax.fill_between(x, y, min_v,
                        alpha=0.06, color=color,
                        interpolate=True,
                        where=y > (min_v + max_v) * 0.5)

    # ── Line with glow ──
    if len(y) > 1:
        ax.plot(x, y, color=color,
                lw=0.8, alpha=0.25, zorder=2)
        ax.plot(x, y, color=color,
                lw=1.8, alpha=1.0, zorder=3)

    # ── Current value dot ──
    if len(y) > 0:
        ax.plot(x[-1], y[-1], "o",
                color=color,
                markersize=6,
                markeredgecolor=WHITE,
                markeredgewidth=1,
                zorder=5)

    # ── Horizontal reference lines ──
    mid = (min_v + max_v) / 2
    ax.axhline(mid, color=GREY2,
               lw=0.6, ls="--", alpha=0.5)

    ax.set_xlim(0, MAX_POINTS)
    ax.set_ylim(min_v, max_v * 1.05)
    ax.grid(True, color=GREY2,
            linestyle="--", alpha=0.3,
            linewidth=0.5, axis="y")

    # ── Current value in title ──
    cur = f"{y[-1]:.1f}" if y[-1] != int(y[-1]) \
          else f"{int(y[-1])}"
    ax.set_title(
        f"  {label}   {cur}{unit}",
        color=color, fontsize=8.5,
        fontfamily="monospace",
        fontweight="bold", pad=5, loc="left")

    ax.set_ylabel(unit, color=GREY,
                  fontsize=7,
                  fontfamily="monospace")


# ════════════════════════════════════════
#  FIGURE SETUP
# ════════════════════════════════════════
plt.style.use("dark_background")
fig = plt.figure(figsize=(19, 10),
                 facecolor=BG)
fig.canvas.manager.set_window_title(
    "V2V  CAR2  FOLLOWER  ─  LIVE DASHBOARD")

gs = gridspec.GridSpec(
    5, 6,
    figure=fig,
    height_ratios=[0.55, 2.2, 1.2, 1.8, 1.8],
    hspace=0.55, wspace=0.38,
    left=0.04, right=0.97,
    top=0.95, bottom=0.04)

# ── Header ──
ax_hdr = fig.add_subplot(gs[0, :])
ax_hdr.set_facecolor(CARD)
ax_hdr.axis("off")
for spine in ax_hdr.spines.values():
    spine.set_color(CYAN)
    spine.set_linewidth(1)

title_t = ax_hdr.text(
    0.5, 0.68,
    "V2V  CAR2  FOLLOWER  ─  LIVE DASHBOARD",
    ha="center", va="center",
    fontsize=15, fontweight="bold",
    color=CYAN, fontfamily="monospace",
    transform=ax_hdr.transAxes)
title_t.set_path_effects([
    pe.withStroke(linewidth=6,
                  foreground=CYAN_DIM)])

status_t = ax_hdr.text(
    0.5, 0.18,
    "● CONNECTING...",
    ha="center", va="center",
    fontsize=8.5, color=YELLOW,
    fontfamily="monospace",
    transform=ax_hdr.transAxes)

time_t = ax_hdr.text(
    0.015, 0.5, "",
    ha="left", va="center",
    fontsize=8, color=GREY,
    fontfamily="monospace",
    transform=ax_hdr.transAxes)

msg_t = ax_hdr.text(
    0.985, 0.5, "",
    ha="right", va="center",
    fontsize=8, color=GREY,
    fontfamily="monospace",
    transform=ax_hdr.transAxes)

# ── Row 1: Gauges ──
ax_g_dist = fig.add_subplot(gs[1, 0])
ax_g_spd1 = fig.add_subplot(gs[1, 1])
ax_g_tilt = fig.add_subplot(gs[1, 2])
ax_g_spd2 = fig.add_subplot(gs[1, 3])
ax_ir     = fig.add_subplot(gs[1, 4:6])

# ── Row 2: Zone bar + alerts ──
ax_zone   = fig.add_subplot(gs[2, :4])
ax_link   = fig.add_subplot(gs[2, 4])
ax_crash  = fig.add_subplot(gs[2, 5])

# ── Row 3: Graphs ──
ax_gr1    = fig.add_subplot(gs[3, :3])
ax_gr2    = fig.add_subplot(gs[3, 3:])

# ── Row 4: Graphs ──
ax_gr3    = fig.add_subplot(gs[4, :3])
ax_gr4    = fig.add_subplot(gs[4, 3:])

# ════════════════════════════════════════
#  ANIMATION
# ════════════════════════════════════════
def update(frame):
    now      = time.strftime("%H:%M:%S")
    live     = (time.time() - data["last_update"] < 2.0
                and data["msg_count"] > 0)

    # ── Header ──
    time_t.set_text(f"⏱  {now}")
    msg_t.set_text(f"PKT  {data['msg_count']:06d}")

    if not data["connected"]:
        status_t.set_text(f"◌  PORT {COM_PORT} NOT FOUND — CHECK USB")
        status_t.set_color(RED)
    elif not live:
        status_t.set_text("◌  WAITING FOR CAR2 DATA...")
        status_t.set_color(YELLOW)
    elif data["crash"]:
        status_t.set_text("✕  CRASH DETECTED — EMERGENCY STOP")
        status_t.set_color(RED)
    elif data["emergency"]:
        status_t.set_text("⚠  CAR1 EMERGENCY — CAR2 HALTED")
        status_t.set_color(RED)
    elif data["braking"]:
        status_t.set_text("⚠  CAR1 BRAKING — CAR2 SLOWING")
        status_t.set_color(ORANGE)
    elif data["link"]:
        status_t.set_text("●  LINKED — PLATOON MODE ACTIVE")
        status_t.set_color(GREEN)
    else:
        status_t.set_text("◌  WAITING FOR CAR1 SIGNAL...")
        status_t.set_color(YELLOW)

    # ── Gauges ──
    ax_g_dist.cla()
    draw_arc_gauge(ax_g_dist,
                   min(data["car1_dist"], 200),
                   0, 200,
                   "CAR1 DISTANCE", "cm",
                   CYAN, CYAN_DIM)

    ax_g_spd1.cla()
    draw_arc_gauge(ax_g_spd1,
                   data["car1_spd"],
                   0, 255,
                   "CAR1 SPEED", "pwm",
                   BLUE, BLUE_DIM)

    ax_g_tilt.cla()
    draw_arc_gauge(ax_g_tilt,
                   data["tilt"],
                   -90, 90,
                   "CAR2 TILT", "deg",
                   PURPLE, PURPLE_DIM)

    ax_g_spd2.cla()
    # Speed color changes with zone
    spd_col = [GREEN, YELLOW, ORANGE, RED][data["car1_zone"]]
    draw_arc_gauge(ax_g_spd2,
                   data["car2_spd"],
                   0, 255,
                   "CAR2 SPEED", "pwm",
                   spd_col, GREEN_DIM)

    # ── IR Panel ──
    ax_ir.cla()
    draw_ir_panel(ax_ir,
                  data["ir_fl"],
                  data["ir_fr"])

    # ── Zone bar ──
    ax_zone.cla()
    draw_zone_bar(ax_zone, data["car1_zone"])

    # ── Alert pills ──
    ax_link.cla()
    draw_alert_pill(ax_link,
                    "CAR1 LINKED",
                    bool(data["link"]),
                    GREEN, GREEN_DIM,
                    "⬡", "⬡")

    ax_crash.cla()
    draw_alert_pill(ax_crash,
                    "CRASH",
                    bool(data["crash"]),
                    RED, RED_DIM,
                    "✕", "○")

    # ── Graphs ──
    ax_gr1.cla()
    draw_expert_graph(ax_gr1, hist_dist,
                      "CAR1 DISTANCE",
                      CYAN, 0, 200, "cm")

    ax_gr2.cla()
    draw_expert_graph(ax_gr2, hist_spd1,
                      "CAR1 SPEED",
                      BLUE, 0, 255, "pwm")

    ax_gr3.cla()
    draw_expert_graph(ax_gr3, hist_spd2,
                      "CAR2 SPEED",
                      GREEN, 0, 255, "pwm")

    ax_gr4.cla()
    draw_expert_graph(ax_gr4, hist_tilt,
                      "CAR2 TILT",
                      PURPLE, -90, 90, "°")

    fig.canvas.draw_idle()

# ════════════════════════════════════════
#  LAUNCH
# ════════════════════════════════════════
print("╔════════════════════════════════════════╗")
print("║   V2V CAR2 EXPERT DASHBOARD v2.0      ║")
print(f"║   Port : {COM_PORT}                        ║")
print(f"║   Baud : {BAUD_RATE}                     ║")
print("║   Close window to exit                ║")
print("╚════════════════════════════════════════╝")

ani = FuncAnimation(fig, update,
                    interval=120,
                    cache_frame_data=False)
plt.show()
