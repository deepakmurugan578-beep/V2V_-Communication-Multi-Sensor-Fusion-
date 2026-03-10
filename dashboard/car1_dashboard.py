"""
╔══════════════════════════════════════════════════════════╗
║         V2V CAR1 LEADER — LIVE PYTHON DASHBOARD         ║
║                                                          ║
║  Serial format expected from Car1:                      ║
║  /*DIST,ZONE,TILT,SPD,DIR,MSG,CRASH,BRAKE,LINK*/        ║
║                                                          ║
║  Run: python car1_dashboard.py                          ║
║  Change COM_PORT below if needed                        ║
╚══════════════════════════════════════════════════════════╝
"""

import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib.patches import FancyBboxPatch, Circle
from matplotlib.animation import FuncAnimation
import matplotlib.patheffects as pe
import numpy as np
import threading
import time
from collections import deque

# ════════════════════════════════════════
#  CONFIG — change COM_PORT if needed
# ════════════════════════════════════════
COM_PORT   = "COM3"
BAUD_RATE  = 115200
MAX_POINTS = 100

# ════════════════════════════════════════
#  COLOR THEME — Red/Orange Leader theme
#  (different from Car2 cyan/green theme)
# ════════════════════════════════════════
BG          = "#080A0F"
CARD        = "#0D1018"
CARD2       = "#111520"
BORDER      = "#1E2535"
# Car1 accent colors — warm/aggressive
ORANGE      = "#FF6B2B"
ORANGE_DIM  = "#3A1800"
YELLOW      = "#FFD60A"
YELLOW_DIM  = "#332900"
RED         = "#FF2D55"
RED_DIM     = "#3A0010"
CYAN        = "#00E5FF"
CYAN_DIM    = "#003344"
GREEN       = "#00FF88"
GREEN_DIM   = "#003322"
BLUE        = "#4D9FFF"
BLUE_DIM    = "#001830"
PURPLE      = "#BF5FFF"
PURPLE_DIM  = "#1A0033"
WHITE       = "#F0F4FF"
GREY        = "#4A5568"
GREY2       = "#1E2535"

# ════════════════════════════════════════
#  SHARED DATA
# ════════════════════════════════════════
data = {
    "dist"      : 0,
    "zone"      : 0,
    "tilt"      : 0.0,
    "spd"       : 0,
    "direction" : 0,     # -1=left 0=fwd 1=right
    "msg"       : 1,     # 1=normal 2=warn 3=emergency
    "crash"     : 0,
    "brake"     : 0,
    "link"      : 0,
    "connected" : False,
    "msg_count" : 0,
    "last_update": 0,
    "raw"       : "",
}

hist_dist  = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
hist_spd   = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
hist_tilt  = deque([0.0]*MAX_POINTS, maxlen=MAX_POINTS)
hist_zone  = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)

# ════════════════════════════════════════
#  SERIAL READER THREAD
# ════════════════════════════════════════
def serial_reader():
    while True:
        try:
            ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
            data["connected"] = True
            print(f"Connected to {COM_PORT}")
            while True:
                try:
                    raw = ser.readline().decode("utf-8",
                                errors="ignore").strip()
                    # Parse /*...*/ format
                    if raw.startswith("/*") and raw.endswith("*/"):
                        inner = raw[2:-2]
                        parts = inner.split(",")
                        if len(parts) >= 9:
                            data["dist"]      = int(parts[0])
                            data["zone"]      = int(parts[1])
                            data["tilt"]      = float(parts[2])
                            data["spd"]       = int(parts[3])
                            data["direction"] = int(parts[4])
                            data["msg"]       = int(parts[5])
                            data["crash"]     = int(parts[6])
                            data["brake"]     = int(parts[7])
                            data["link"]      = int(parts[8])
                            data["msg_count"] += 1
                            data["last_update"] = time.time()
                            data["raw"] = raw
                            # History
                            d = data["dist"] if data["dist"] < 500 else 200
                            hist_dist.append(d)
                            hist_spd.append(data["spd"])
                            hist_tilt.append(data["tilt"])
                            hist_zone.append(data["zone"] * 60)
                    # Also parse legacy [CAR1] format as fallback
                    elif raw.startswith("[CAR1]"):
                        data["msg_count"] += 1
                        data["last_update"] = time.time()
                        data["raw"] = raw
                except Exception:
                    continue
        except serial.SerialException as e:
            data["connected"] = False
            print(f"Waiting for {COM_PORT}... {e}")
            time.sleep(2)

thread = threading.Thread(target=serial_reader, daemon=True)
thread.start()

# ════════════════════════════════════════
#  DRAWING HELPERS
# ════════════════════════════════════════

def draw_arc_gauge(ax, value, min_val, max_val,
                   label, unit, color, dim_color,
                   warn_val=None, crit_val=None):
    """Premium arc gauge with color zones."""
    ax.set_facecolor(CARD)
    ax.set_xlim(-1.3, 1.3)
    ax.set_ylim(-1.2, 1.15)
    ax.axis("off")

    START = 220
    SWEEP = 280
    N     = 200

    # Background track
    t0 = np.linspace(np.radians(START),
                     np.radians(START - SWEEP), N)
    ax.plot(np.cos(t0), np.sin(t0),
            color=GREY2, lw=10,
            solid_capstyle="round", zorder=1)

    pct = np.clip((value - min_val) /
                  (max_val - min_val), 0, 1)

    # Choose color based on warn/crit thresholds
    if crit_val is not None and value <= crit_val:
        arc_col = RED
    elif warn_val is not None and value <= warn_val:
        arc_col = YELLOW
    else:
        arc_col = color

    # Fill arc
    if pct > 0:
        t1 = np.linspace(np.radians(START),
                         np.radians(START - pct * SWEEP),
                         max(2, int(pct * N)))
        ax.plot(np.cos(t1), np.sin(t1),
                color=arc_col, lw=10,
                solid_capstyle="round",
                zorder=2, alpha=0.95)
        # Glow
        ax.plot(np.cos(t1), np.sin(t1),
                color=arc_col, lw=18,
                solid_capstyle="round",
                zorder=1, alpha=0.10)

    # Tick marks
    for i in range(6):
        t  = np.radians(START - i * SWEEP / 5)
        r1, r2 = 0.80, 0.94
        ax.plot([r1*np.cos(t), r2*np.cos(t)],
                [r1*np.sin(t), r2*np.sin(t)],
                color=GREY, lw=1.2, zorder=3)

    # Needle
    na = np.radians(START - pct * SWEEP)
    nx, ny = 0.66*np.cos(na), 0.66*np.sin(na)
    ax.annotate("",
        xy=(nx, ny), xytext=(0, 0),
        arrowprops=dict(
            arrowstyle="->,head_width=0.12,head_length=0.1",
            color=WHITE, lw=2.0))
    ax.add_patch(Circle((0, 0), 0.07,
                 facecolor=arc_col, zorder=6,
                 edgecolor=WHITE, linewidth=1.5))

    # Value text
    val_str = (f"{value:.1f}" if isinstance(value, float)
               else f"{int(value)}")
    ax.text(0, -0.30, val_str,
            ha="center", va="center",
            fontsize=17, fontweight="bold",
            color=WHITE, fontfamily="monospace",
            zorder=7)

    ax.text(0, -0.54, unit,
            ha="center", fontsize=8,
            color=arc_col, fontfamily="monospace")
    ax.text(0, -0.80, label,
            ha="center", fontsize=9,
            fontweight="bold",
            color=GREY, fontfamily="monospace")

    # Min / Max
    ax.text(-1.1, -0.25, str(min_val),
            ha="center", fontsize=7,
            color=GREY, fontfamily="monospace")
    ax.text(1.1, -0.25, str(max_val),
            ha="center", fontsize=7,
            color=GREY, fontfamily="monospace")


def draw_direction_panel(ax, direction, speed):
    """Car direction indicator with arrow."""
    ax.set_facecolor(CARD)
    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.5)
    ax.axis("off")

    # Title
    ax.text(0, 1.25, "DIRECTION",
            ha="center", fontsize=9,
            fontweight="bold",
            color=GREY, fontfamily="monospace")

    # Car body outline
    car_col = ORANGE if speed > 0 else GREY
    rect = FancyBboxPatch((-0.35, -0.55), 0.7, 1.1,
                          boxstyle="round,pad=0.1",
                          facecolor=CARD2,
                          edgecolor=car_col,
                          linewidth=2)
    ax.add_patch(rect)

    # Wheels
    for wx, wy in [(-0.45, 0.35), (0.45, 0.35),
                   (-0.45, -0.35), (0.45, -0.35)]:
        ax.add_patch(FancyBboxPatch(
            (wx - 0.12, wy - 0.18), 0.24, 0.36,
            boxstyle="round,pad=0.03",
            facecolor=GREY2,
            edgecolor=GREY,
            linewidth=1.5))

    # Direction arrow
    if direction == 0 and speed > 0:
        # Forward arrow
        ax.annotate("",
            xy=(0, 1.1), xytext=(0, 0.6),
            arrowprops=dict(
                arrowstyle="->,head_width=0.25,head_length=0.2",
                color=GREEN, lw=3))
        ax.text(0, -0.95, "FORWARD",
                ha="center", fontsize=8,
                fontweight="bold",
                color=GREEN, fontfamily="monospace")
    elif direction == -1:
        # Left arrow
        ax.annotate("",
            xy=(-1.2, 0), xytext=(-0.6, 0),
            arrowprops=dict(
                arrowstyle="->,head_width=0.25,head_length=0.2",
                color=YELLOW, lw=3))
        ax.text(0, -0.95, "TURN LEFT",
                ha="center", fontsize=8,
                fontweight="bold",
                color=YELLOW, fontfamily="monospace")
    elif direction == 1:
        # Right arrow
        ax.annotate("",
            xy=(1.2, 0), xytext=(0.6, 0),
            arrowprops=dict(
                arrowstyle="->,head_width=0.25,head_length=0.2",
                color=YELLOW, lw=3))
        ax.text(0, -0.95, "TURN RIGHT",
                ha="center", fontsize=8,
                fontweight="bold",
                color=YELLOW, fontfamily="monospace")
    else:
        # Stopped
        ax.text(0, 0, "■",
                ha="center", va="center",
                fontsize=24, color=RED)
        ax.text(0, -0.95, "STOPPED",
                ha="center", fontsize=8,
                fontweight="bold",
                color=RED, fontfamily="monospace")


def draw_zone_panel(ax, zone, dist):
    """Distance zone panel with 4 levels."""
    ax.set_facecolor(CARD)
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis("off")

    ax.text(0.02, 0.88, "DISTANCE ZONE",
            fontsize=8, fontweight="bold",
            color=GREY, fontfamily="monospace",
            transform=ax.transAxes)

    # Distance value large
    dist_str = f"{dist}cm" if dist < 500 else "---cm"
    ax.text(0.98, 0.88, dist_str,
            ha="right", fontsize=10,
            fontweight="bold",
            color=ORANGE, fontfamily="monospace",
            transform=ax.transAxes)

    zones = [
        (GREEN,  "SAFE",     "> 50cm"),
        (YELLOW, "WARNING",  "< 50cm"),
        (ORANGE, "DANGER",   "< 20cm"),
        (RED,    "CRITICAL", "< 11cm"),
    ]

    w   = 0.22
    gap = 0.02
    for i, (col, lbl, rng) in enumerate(zones):
        x      = 0.02 + i * (w + gap)
        active = (zone == i)

        rect = FancyBboxPatch(
            (x, 0.08), w, 0.62,
            boxstyle="round,pad=0.015",
            facecolor=col + "33" if active else CARD2,
            edgecolor=col if active else GREY2,
            linewidth=2.5 if active else 1,
            transform=ax.transAxes,
            clip_on=False)
        ax.add_patch(rect)

        if active:
            ax.plot([x, x + w], [0.70, 0.70],
                    color=col, lw=3,
                    transform=ax.transAxes,
                    solid_capstyle="round",
                    clip_on=False)

        alpha = 1.0 if active else 0.3
        ax.text(x + w/2, 0.50, lbl,
                ha="center", va="center",
                fontsize=7.5, fontweight="bold",
                color=col if active else GREY,
                fontfamily="monospace",
                alpha=alpha,
                transform=ax.transAxes)
        ax.text(x + w/2, 0.25, rng,
                ha="center", va="center",
                fontsize=6.5, color=GREY,
                fontfamily="monospace",
                alpha=alpha,
                transform=ax.transAxes)


def draw_alert_pill(ax, label, active,
                    on_color, on_bg,
                    icon_on="●", icon_off="○"):
    """Pill alert indicator."""
    ax.set_facecolor(CARD)
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis("off")

    col = on_color if active else GREY2
    bg  = on_bg    if active else CARD2

    rect = FancyBboxPatch(
        (0.05, 0.10), 0.90, 0.80,
        boxstyle="round,pad=0.06",
        facecolor=bg,
        edgecolor=col,
        linewidth=2.5 if active else 1,
        transform=ax.transAxes)
    ax.add_patch(rect)

    if active:
        ax.plot([0.05, 0.95], [0.90, 0.90],
                color=col, lw=2.5,
                transform=ax.transAxes,
                solid_capstyle="round")

    icon = icon_on if active else icon_off
    ax.text(0.5, 0.62, icon,
            ha="center", va="center",
            fontsize=16,
            color=col if active else GREY,
            transform=ax.transAxes)
    ax.text(0.5, 0.27, label,
            ha="center", va="center",
            fontsize=8, fontweight="bold",
            color=col if active else GREY,
            fontfamily="monospace",
            transform=ax.transAxes)


def draw_msg_panel(ax, msg_type):
    """Message type indicator."""
    ax.set_facecolor(CARD)
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis("off")

    msgs = {
        1: (GREEN,  "NORMAL",    "●"),
        2: (YELLOW, "WARNING",   "⚠"),
        3: (RED,    "EMERGENCY", "✕"),
    }
    col, lbl, icon = msgs.get(msg_type,
                               (GREY, "UNKNOWN", "?"))

    rect = FancyBboxPatch(
        (0.05, 0.10), 0.90, 0.80,
        boxstyle="round,pad=0.06",
        facecolor=col + "22",
        edgecolor=col,
        linewidth=2.5,
        transform=ax.transAxes)
    ax.add_patch(rect)

    ax.plot([0.05, 0.95], [0.90, 0.90],
            color=col, lw=2.5,
            transform=ax.transAxes,
            solid_capstyle="round")

    ax.text(0.5, 0.62, icon,
            ha="center", va="center",
            fontsize=16, color=col,
            transform=ax.transAxes)
    ax.text(0.5, 0.27, lbl,
            ha="center", va="center",
            fontsize=8, fontweight="bold",
            color=col, fontfamily="monospace",
            transform=ax.transAxes)


def draw_expert_graph(ax, history, label,
                      color, min_v, max_v,
                      unit="", show_warn=None):
    """Expert quality live graph."""
    ax.set_facecolor(CARD)
    for spine in ax.spines.values():
        spine.set_color(BORDER)
        spine.set_linewidth(0.8)
    ax.tick_params(colors=GREY, labelsize=7,
                   length=3, width=0.5)
    ax.xaxis.set_tick_params(labelbottom=False)

    x = np.arange(len(history))
    y = np.array(list(history), dtype=float)

    # Gradient fill
    if len(y) > 1:
        ax.fill_between(x, y, min_v,
                        alpha=0.10, color=color)
        ax.fill_between(x, y, min_v,
                        alpha=0.05, color=color,
                        where=y > (min_v + max_v) * 0.6)

    # Warning line
    if show_warn is not None:
        ax.axhline(show_warn, color=YELLOW,
                   lw=0.8, ls="--",
                   alpha=0.5, zorder=1)

    # Line glow + main
    if len(y) > 1:
        ax.plot(x, y, color=color,
                lw=0.8, alpha=0.2, zorder=2)
        ax.plot(x, y, color=color,
                lw=1.8, alpha=1.0, zorder=3)

    # Current dot
    if len(y) > 0:
        dot_col = RED if (show_warn and y[-1] <= show_warn) \
                  else color
        ax.plot(x[-1], y[-1], "o",
                color=dot_col,
                markersize=6,
                markeredgecolor=WHITE,
                markeredgewidth=1,
                zorder=5)

    ax.set_xlim(0, MAX_POINTS)
    ax.set_ylim(min_v, max_v * 1.08)
    ax.grid(True, color=GREY2,
            linestyle="--", alpha=0.3,
            linewidth=0.5, axis="y")
    ax.axhline((min_v + max_v) / 2,
               color=GREY2, lw=0.5,
               ls="--", alpha=0.4)

    cur = (f"{y[-1]:.1f}" if y[-1] != int(y[-1])
           else f"{int(y[-1])}")
    ax.set_title(
        f"  {label}   {cur}{unit}",
        color=color, fontsize=8.5,
        fontfamily="monospace",
        fontweight="bold", pad=5, loc="left")
    ax.set_ylabel(unit, color=GREY,
                  fontsize=7,
                  fontfamily="monospace")


# ════════════════════════════════════════
#  FIGURE LAYOUT
# ════════════════════════════════════════
plt.style.use("dark_background")
fig = plt.figure(figsize=(19, 10),
                 facecolor=BG)
fig.canvas.manager.set_window_title(
    "V2V  CAR1  LEADER  ─  LIVE DASHBOARD")

gs = gridspec.GridSpec(
    5, 6,
    figure=fig,
    height_ratios=[0.55, 2.2, 1.2, 1.8, 1.8],
    hspace=0.55, wspace=0.42,
    left=0.04, right=0.97,
    top=0.95, bottom=0.04)

# ── Header ──
ax_hdr = fig.add_subplot(gs[0, :])
ax_hdr.set_facecolor(CARD)
ax_hdr.axis("off")
for spine in ax_hdr.spines.values():
    spine.set_color(ORANGE)
    spine.set_linewidth(1.2)

title_t = ax_hdr.text(
    0.5, 0.68,
    "V2V  CAR1  LEADER  ─  LIVE DASHBOARD",
    ha="center", va="center",
    fontsize=15, fontweight="bold",
    color=ORANGE, fontfamily="monospace",
    transform=ax_hdr.transAxes)
title_t.set_path_effects([
    pe.withStroke(linewidth=6,
                  foreground=ORANGE_DIM)])

status_t = ax_hdr.text(
    0.5, 0.18, "◌  CONNECTING...",
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

# ── Row 1: Gauges + Direction ──
ax_dist  = fig.add_subplot(gs[1, 0])
ax_tilt  = fig.add_subplot(gs[1, 1])
ax_spd   = fig.add_subplot(gs[1, 2])
ax_dir   = fig.add_subplot(gs[1, 3])
ax_msg   = fig.add_subplot(gs[1, 4])
ax_link  = fig.add_subplot(gs[1, 5])

# ── Row 2: Zone bar + alerts ──
ax_zone  = fig.add_subplot(gs[2, :4])
ax_crash = fig.add_subplot(gs[2, 4])
ax_brake = fig.add_subplot(gs[2, 5])

# ── Row 3: Graphs ──
ax_gr1   = fig.add_subplot(gs[3, :3])
ax_gr2   = fig.add_subplot(gs[3, 3:])

# ── Row 4: Graphs ──
ax_gr3   = fig.add_subplot(gs[4, :3])
ax_gr4   = fig.add_subplot(gs[4, 3:])


# ════════════════════════════════════════
#  ANIMATION UPDATE
# ════════════════════════════════════════
def update(frame):
    now  = time.strftime("%H:%M:%S")
    live = (time.time() - data["last_update"] < 2.0
            and data["msg_count"] > 0)

    # ── Header ──
    time_t.set_text(f"⏱  {now}")
    msg_t.set_text(f"PKT  {data['msg_count']:06d}")

    if not data["connected"]:
        status_t.set_text(
            f"◌  PORT {COM_PORT} NOT FOUND — CHECK USB")
        status_t.set_color(RED)
    elif not live:
        status_t.set_text(
            "◌  WAITING FOR CAR1 DATA...")
        status_t.set_color(YELLOW)
    elif data["crash"]:
        status_t.set_text(
            "✕  CRASH DETECTED — EMERGENCY STOP!")
        status_t.set_color(RED)
    elif data["zone"] == 3:
        status_t.set_text(
            "✕  CRITICAL ZONE — COLLISION STOP!")
        status_t.set_color(RED)
    elif data["zone"] == 2:
        status_t.set_text(
            "⚠  DANGER ZONE — SLOWING DOWN")
        status_t.set_color(ORANGE)
    elif data["zone"] == 1:
        status_t.set_text(
            "⚠  WARNING ZONE — MEDIUM SPEED")
        status_t.set_color(YELLOW)
    elif data["link"]:
        status_t.set_text(
            "●  CAR2 LINKED — PLATOON ACTIVE")
        status_t.set_color(GREEN)
    else:
        status_t.set_text(
            "●  RUNNING — WAITING FOR CAR2")
        status_t.set_color(ORANGE)

    # ── Gauges ──
    ax_dist.cla()
    # Distance gauge — color flips when close
    dist_val = min(data["dist"], 200) \
               if data["dist"] < 500 else 0
    draw_arc_gauge(ax_dist,
                   dist_val,
                   0, 200,
                   "DISTANCE", "cm",
                   ORANGE, ORANGE_DIM,
                   warn_val=20,
                   crit_val=11)

    ax_tilt.cla()
    draw_arc_gauge(ax_tilt,
                   data["tilt"],
                   -90, 90,
                   "TILT ANGLE", "deg",
                   PURPLE, PURPLE_DIM)

    ax_spd.cla()
    spd_colors = [GREEN, YELLOW, ORANGE, RED]
    spd_col    = spd_colors[data["zone"]]
    draw_arc_gauge(ax_spd,
                   data["spd"],
                   0, 255,
                   "MOTOR SPEED", "pwm",
                   spd_col, GREEN_DIM)

    # ── Direction panel ──
    ax_dir.cla()
    draw_direction_panel(ax_dir,
                         data["direction"],
                         data["spd"])

    # ── Message type ──
    ax_msg.cla()
    draw_msg_panel(ax_msg, data["msg"])

    # ── Link status ──
    ax_link.cla()
    draw_alert_pill(ax_link,
                    "CAR2 LINK",
                    bool(data["link"]),
                    GREEN, GREEN_DIM,
                    "⬡", "⬡")

    # ── Zone panel ──
    ax_zone.cla()
    draw_zone_panel(ax_zone,
                    data["zone"],
                    data["dist"])

    # ── Crash + Brake pills ──
    ax_crash.cla()
    draw_alert_pill(ax_crash,
                    "CRASH",
                    bool(data["crash"]),
                    RED, RED_DIM,
                    "✕", "○")

    ax_brake.cla()
    draw_alert_pill(ax_brake,
                    "BRAKING",
                    bool(data["brake"]),
                    ORANGE, ORANGE_DIM,
                    "■", "○")

    # ── Graphs ──
    ax_gr1.cla()
    draw_expert_graph(ax_gr1, hist_dist,
                      "DISTANCE",
                      ORANGE, 0, 200,
                      "cm", show_warn=20)

    ax_gr2.cla()
    draw_expert_graph(ax_gr2, hist_spd,
                      "MOTOR SPEED",
                      YELLOW, 0, 255, "pwm")

    ax_gr3.cla()
    draw_expert_graph(ax_gr3, hist_tilt,
                      "TILT ANGLE",
                      PURPLE, -90, 90, "deg")

    ax_gr4.cla()
    draw_expert_graph(ax_gr4, hist_zone,
                      "ZONE LEVEL",
                      RED, 0, 200, "")

    fig.canvas.draw_idle()


# ════════════════════════════════════════
#  LAUNCH
# ════════════════════════════════════════
print("╔════════════════════════════════════════╗")
print("║   V2V CAR1 LEADER DASHBOARD v1.0      ║")
print(f"║   Port : {COM_PORT}                        ║")
print(f"║   Baud : {BAUD_RATE}                     ║")
print("║   Close window to exit                ║")
print("╚════════════════════════════════════════╝")

ani = FuncAnimation(fig, update,
                    interval=120,
                    cache_frame_data=False)
plt.show()
