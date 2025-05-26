# streak_ui.py

import sys
import struct
import time
import serial

from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout
from PyQt5.QtCore    import QTimer, Qt

from matplotlib.figure          import Figure
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas

# ——— Configuration ———
PORT             = '/dev/tty.usbmodem11303'  # ← your VCP port
BAUD             = 115200
SENTINEL         = b'\xAA\x55'
STREAK_TYPE      = 1
BEEP_TYPE        = 2
POLL_INTERVAL_MS = 50    # how often to poll serial (ms)
BEEP_PULSE_MS    = 100   # how long to draw each beep high (ms)

def read_packet(ser):
    """Read one framed packet, or return None."""
    b = ser.read(1)
    if not b or b != SENTINEL[0:1]:
        return None
    if ser.read(1) != SENTINEL[1:2]:
        return None
    hdr = ser.read(4)
    if len(hdr) < 4:
        return None
    msg_type, length = struct.unpack('<HH', hdr)
    payload = ser.read(length)
    if len(payload) < length:
        return None
    return msg_type, payload

class StreakWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Streak & Beep Monitor")

        # Data state
        self.streak    = 0
        self.last_beep = None
        self.beep_times= []

        # UI labels
        self.streak_label = QLabel(f"Streak: {self.streak}", self)
        self.streak_label.setAlignment(Qt.AlignCenter)

        self.beep_label   = QLabel("Last beep: N/A", self)
        self.beep_label.setAlignment(Qt.AlignCenter)

        # Plot canvas
        self.figure = Figure(figsize=(6,2))
        self.ax     = self.figure.add_subplot(111)
        self.ax.set_ylim(-0.1, 1.1)
        self.ax.set_xlabel("Time (s)")
        self.ax.set_ylabel("Beep")
        self.ax.set_title("Beep Events")
        self.canvas = FigureCanvas(self.figure)

        # Layout
        layout = QVBoxLayout(self)
        layout.addWidget(self.streak_label)
        layout.addWidget(self.beep_label)
        layout.addWidget(self.canvas)

        # Open serial port
        try:
            self.ser = serial.Serial(PORT, BAUD, timeout=0.01)
        except Exception as e:
            self.ser = None
            self.streak_label.setText(f"Serial error:\n{e}")

        # Polling timer
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.poll_serial)
        self.timer.start(POLL_INTERVAL_MS)

    def poll_serial(self):
        if not self.ser or not self.ser.is_open:
            return

        try:
            pkt = read_packet(self.ser)
        except serial.SerialException as e:
            self.streak_label.setText(f"Serial error:\n{e}")
            self.timer.stop()
            return

        if not pkt:
            return

        msg_type, payload = pkt

        if msg_type == STREAK_TYPE:
            new_streak, = struct.unpack('<I', payload)
            if new_streak != self.streak:
                self.streak = new_streak
                self.streak_label.setText(f"Streak: {self.streak}")

        elif msg_type == BEEP_TYPE:
            ts_ms, = struct.unpack('<I', payload)
            self.last_beep = ts_ms
            self.beep_label.setText(f"Last beep: {self.last_beep} ms")
            self.beep_times.append(ts_ms / 1000.0)
            self.update_plot()

    def update_plot(self):
        """Redraw the square-wave beep plot."""
        x_vals = [0]
        y_vals = [0]
        for t in self.beep_times:
            x_vals += [t, t, t + BEEP_PULSE_MS/1000.0, t + BEEP_PULSE_MS/1000.0]
            y_vals += [0, 1, 1, 0]

        self.ax.clear()
        self.ax.step(x_vals, y_vals, where='post')
        self.ax.set_ylim(-0.1, 1.1)
        self.ax.set_xlabel("Time (s)")
        self.ax.set_ylabel("Beep")
        self.ax.set_title("Beep Events")
        self.canvas.draw()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    w = StreakWindow()
    w.resize(600, 300)
    w.show()
    sys.exit(app.exec_())