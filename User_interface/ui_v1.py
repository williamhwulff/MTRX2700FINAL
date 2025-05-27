import sys
import struct
import serial
import time
import threading
from collections import deque
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QLabel, QHBoxLayout, QFrame
)
from PyQt5.QtCore import Qt, QTimer, pyqtSignal, QObject
import pyqtgraph as pg

# === Configuration ===
PORT             = '/dev/tty.usbmodem11303'
BAUD             = 115200
SENTINEL         = b'\xAA\x55'
STREAK_TYPE      = 1
BEEP_TYPE        = 2
POLL_INTERVAL_MS = 10
MAX_POINTS       = 500  # how many points to keep on screen

class SerialReader(QObject):
    data_received = pyqtSignal(int, float)  # msg_type, timestamp

    def __init__(self, port, baud):
        super().__init__()
        self.ser = serial.Serial(port, baud, timeout=1)
        self._running = True
        self.thread = threading.Thread(target=self.read_loop, daemon=True)
        self.thread.start()

    def stop(self):
        self._running = False
        self.ser.close()

    def read_packet(self):
        try:
            if self.ser.read(1) != SENTINEL[0:1]:
                return None
            if self.ser.read(1) != SENTINEL[1:2]:
                return None
            hdr = self.ser.read(4)
            if len(hdr) < 4:
                return None
            msg_type, length = struct.unpack('<HH', hdr)
            payload = self.ser.read(length)
            if len(payload) < length:
                return None
            return msg_type, payload
        except:
            return None

    def read_loop(self):
        while self._running:
            pkt = self.read_packet()
            if pkt:
                msg_type, payload = pkt
                if msg_type == BEEP_TYPE and len(payload) == 4:
                    self.data_received.emit(msg_type, time.time())
                elif msg_type == STREAK_TYPE and len(payload) == 4:
                    streak_val, = struct.unpack('<I', payload)
                    self.data_received.emit(msg_type, streak_val)

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Treasure Hunt")
        self.resize(800, 600)

        self.init_ui()
        self.setStyleSheet("background-color: #121212;")

        self.start_time = time.time()

        self.streak_value = 0

        # Draw a square-wave line for beeps
        self.beep_curve = self.plot_widget.plot(pen='r', stepMode=True)
        self.streak_curve = self.plot_widget.plot(pen=None, symbol='s', symbolBrush='y')
        self.beep_timestamps = deque(maxlen=MAX_POINTS)
        self.beep_values = deque(maxlen=MAX_POINTS)
        self.streak_timestamps = deque(maxlen=MAX_POINTS)

        # Setup serial
        self.reader = SerialReader(PORT, BAUD)
        self.reader.data_received.connect(self.handle_data)

        # Timer to refresh plot
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(POLL_INTERVAL_MS)

    def init_ui(self):
        layout = QVBoxLayout()

        title_label = QLabel("Treasure Hunt")
        title_label.setAlignment(Qt.AlignCenter)
        title_label.setStyleSheet("color: #FFD700; font-size: 32px; font-weight: bold; margin-bottom: 20px;")
        layout.addWidget(title_label)

        # Streak Display
        streak_frame = QFrame()
        streak_frame.setStyleSheet(
          "background-color: #222; color: #0f0; font-size: 28px; "
          "padding: 15px; border-radius: 10px; border: 2px solid #0f0;"
        )
        streak_layout = QHBoxLayout()
        self.streak_label = QLabel("Streak: 0")
        self.streak_label.setAlignment(Qt.AlignCenter)
        streak_layout.addWidget(self.streak_label)

        target_label = QLabel("Target Score: 5")
        target_label.setAlignment(Qt.AlignCenter)
        target_label.setStyleSheet("color: #0f0; font-size: 20px; margin-left: 30px;")
        streak_layout.addWidget(target_label)

        streak_frame.setLayout(streak_layout)

        # Beep Plot
        self.plot_widget = pg.PlotWidget(title="Beeps Over Time")
        self.plot_widget.setYRange(0, 1.2)
        self.plot_widget.setBackground('#111')
        self.plot_widget.showGrid(x=True, y=True)

        layout.addWidget(streak_frame)
        layout.addWidget(self.plot_widget)
        self.setLayout(layout)

    def handle_data(self, msg_type, timestamp):
        if msg_type == BEEP_TYPE:
            t = time.time() - self.start_time
            self.beep_timestamps.append(t)
            self.beep_values.append(1.0)
            self.beep_timestamps.append(t + 0.5)
            self.beep_values.append(0.0)
        elif msg_type == STREAK_TYPE:
            streak_val = int(timestamp)
            t = time.time() - self.start_time
            self.streak_timestamps.append(t)
            self.streak_value = streak_val
            self.streak_label.setText(f"Streak: {self.streak_value}")

    def update_plot(self):
        current_time = time.time() - self.start_time
        while self.beep_timestamps and self.beep_timestamps[0] < current_time - 10:
            self.beep_timestamps.popleft()
            self.beep_values.popleft()
        while self.streak_timestamps and self.streak_timestamps[0] < current_time - 10:
            self.streak_timestamps.popleft()

        # Prepare stepped square wave data for beep_curve (stepMode=True)
        x = list(self.beep_timestamps)
        y = list(self.beep_values)
        if x:
            # Append an extra x so len(x) == len(y) + 1 for stepMode
            x.append(current_time)
        self.beep_curve.setData(x, y)
        self.streak_curve.setData(self.streak_timestamps, [1.0] * len(self.streak_timestamps))
        self.plot_widget.setXRange(current_time - 10, current_time)

    def closeEvent(self, event):
        self.reader.stop()
        super().closeEvent(event)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = MainWindow()
    win.show()
    sys.exit(app.exec_())