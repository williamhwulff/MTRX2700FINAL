import sys
import struct
import serial
import time
import threading
from collections import deque
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QLabel, QHBoxLayout, QFrame, QComboBox, QGridLayout, QProgressBar,
    QStackedWidget, QPushButton, QSpacerItem, QSizePolicy
)
from PyQt5.QtCore import Qt, QTimer, pyqtSignal, QObject
from PyQt5.QtGui import QPixmap, QFont
import pyqtgraph as pg

# === Configuration ===
PORT             = '/dev/tty.usbmodem1303'
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

        self.window_size = 10  # default window size in seconds
        self.total_beeps = 0

        self.init_pages()
        self.setStyleSheet("background-color: #1e1e1e;")

        self.start_time = time.time()

        self.streak_value = 0

        # Setup serial
        self.reader = SerialReader(PORT, BAUD)
        self.reader.data_received.connect(self.handle_data)

        # Timer to refresh plot
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(POLL_INTERVAL_MS)

    def init_pages(self):
        self.stack = QStackedWidget()

        # Home page with buttons
        home_page = QWidget()
        home_layout = QVBoxLayout()

        # Home page header with logo and title
        home_header_layout = QHBoxLayout()
        logo_label = QLabel()
        logo_pix = QPixmap("logo.png")
        if not logo_pix.isNull():
            logo_label.setPixmap(logo_pix.scaledToHeight(40, Qt.SmoothTransformation))
        home_header_layout.addWidget(logo_label)
        home_title = QLabel("Treasure Hunt")
        home_title.setStyleSheet("color: #FFD700; font-size: 24px; font-weight: bold; margin-left: 10px;")
        home_header_layout.addWidget(home_title)
        home_header_layout.addStretch()
        home_layout.addLayout(home_header_layout)
        # Add spacing below header
        home_layout.addSpacing(20)

        home_layout.setAlignment(Qt.AlignTop | Qt.AlignHCenter)
        home_layout.setContentsMargins(40, 20, 40, 20)
        home_layout.setSpacing(15)

        btn_instructions = QPushButton("Instructions")
        btn_treasure = QPushButton("Treasure's Jingle")
        btn_riddle = QPushButton("Translate the Riddle")

        for btn in (btn_instructions, btn_treasure, btn_riddle):
            btn.setStyleSheet(
                "font-size: 18px; color: #e0e0e0; "
                "background-color: #333333; padding: 10px; "
                "border: 2px solid #FFD700; border-radius: 5px;"
            )
            btn.setFixedWidth(200)

        btn_layout = QVBoxLayout()
        btn_layout.setAlignment(Qt.AlignHCenter)
        btn_layout.setSpacing(15)
        btn_layout.addWidget(btn_instructions)
        btn_layout.addWidget(btn_treasure)
        btn_layout.addWidget(btn_riddle)
        home_layout.addLayout(btn_layout)

        home_page.setLayout(home_layout)

        # Instruction page (with header and back arrow)
        instruction_page = QWidget()
        instruction_layout = QVBoxLayout()
        # Instruction page header
        instr_header_layout = QHBoxLayout()
        instr_header_layout.addStretch()
        instr_logo = QLabel()
        instr_logo_pix = QPixmap("logo.png")
        if not instr_logo_pix.isNull():
            instr_logo.setPixmap(instr_logo_pix.scaledToHeight(30, Qt.SmoothTransformation))
        instr_header_layout.addWidget(instr_logo)
        instr_title = QLabel("Treasure Hunt")
        instr_title.setAlignment(Qt.AlignCenter)
        instr_title.setStyleSheet("color: #FFD700; font-size: 24px; font-weight: bold;")
        instr_header_layout.addWidget(instr_title)
        back_button_instr = QPushButton("← Back")
        back_button_instr.setStyleSheet("color: #e0e0e0; font-size: 16px;")
        instr_header_layout.addWidget(back_button_instr)
        back_button_instr.clicked.connect(lambda: self.stack.setCurrentIndex(0))
        instruction_layout.addLayout(instr_header_layout)
        instruction_page.setLayout(instruction_layout)

        # Treasure page
        treasure_page = QWidget()
        self.init_treasure_ui(treasure_page)

        # Riddle page (with header and back arrow)
        riddle_page = QWidget()
        riddle_layout = QVBoxLayout()
        # Riddle page header
        rid_header_layout = QHBoxLayout()
        rid_header_layout.addStretch()
        rid_logo = QLabel()
        rid_logo_pix = QPixmap("logo.png")
        if not rid_logo_pix.isNull():
            rid_logo.setPixmap(rid_logo_pix.scaledToHeight(30, Qt.SmoothTransformation))
        rid_header_layout.addWidget(rid_logo)
        rid_title = QLabel("Treasure Hunt")
        rid_title.setAlignment(Qt.AlignCenter)
        rid_title.setStyleSheet("color: #FFD700; font-size: 24px; font-weight: bold;")
        rid_header_layout.addWidget(rid_title)
        back_button_rid = QPushButton("← Back")
        back_button_rid.setStyleSheet("color: #e0e0e0; font-size: 16px;")
        rid_header_layout.addWidget(back_button_rid)
        back_button_rid.clicked.connect(lambda: self.stack.setCurrentIndex(0))
        riddle_layout.addLayout(rid_header_layout)

        # Riddle page content: large image in bottom-right
        content_layout = QHBoxLayout()
        content_layout.addStretch()  # push image to right
        image_label = QLabel()
        riddle_pix = QPixmap("riddle_image.png")  # replace with your image filename
        if not riddle_pix.isNull():
            # scale as needed, preserving aspect ratio
            image_label.setPixmap(riddle_pix.scaled(400, 400, Qt.KeepAspectRatio, Qt.SmoothTransformation))
        content_layout.addWidget(image_label, alignment=Qt.AlignBottom | Qt.AlignRight)
        riddle_layout.addLayout(content_layout)

        riddle_page.setLayout(riddle_layout)

        self.stack.addWidget(home_page)         # index 0
        self.stack.addWidget(instruction_page)  # index 1
        self.stack.addWidget(treasure_page)     # index 2
        self.stack.addWidget(riddle_page)       # index 3

        btn_instructions.clicked.connect(lambda: self.stack.setCurrentIndex(1))
        btn_treasure.clicked.connect(lambda: self.stack.setCurrentIndex(2))
        btn_riddle.clicked.connect(lambda: self.stack.setCurrentIndex(3))

        layout = QVBoxLayout()
        layout.addWidget(self.stack)
        self.setLayout(layout)

    def init_treasure_ui(self, parent):
        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(20, 20, 20, 20)
        main_layout.setSpacing(15)

        # Header with logo, title, and back arrow
        header_layout = QHBoxLayout()
        logo_label = QLabel()
        pixmap = QPixmap("logo.png")
        if not pixmap.isNull():
            logo_label.setPixmap(pixmap.scaledToHeight(50, Qt.SmoothTransformation))
        header_layout.addWidget(logo_label)

        title_label = QLabel("Treasure Hunt")
        title_label.setAlignment(Qt.AlignCenter)
        title_label.setStyleSheet("color: #FFD700; font-size: 32px; font-weight: bold; margin-left: 10px;")
        header_layout.addWidget(title_label)
        header_layout.addStretch()
        # Add back arrow button to header (top right)
        back_button = QPushButton("← Back")
        back_button.setStyleSheet("color: #e0e0e0; font-size: 16px;")
        header_layout.addWidget(back_button)
        back_button.clicked.connect(lambda: self.stack.setCurrentIndex(0))
        main_layout.addLayout(header_layout)

        # Streak Display and Window ComboBox
        streak_frame = QFrame()
        streak_frame.setStyleSheet(
          "background-color: #222; color: #FFD700; font-size: 28px; "
          "padding: 15px; border-radius: 10px; border: 2px solid #FFD700;"
        )
        target_label = QLabel("Target Score: 5")
        target_label.setAlignment(Qt.AlignCenter)
        target_label.setStyleSheet("color: #FFD700; font-size: 20px; margin-left: 30px;")

        window_label = QLabel("Window:")
        window_label.setStyleSheet("color: #FFD700; font-size: 20px; margin-left: 30px;")

        self.window_combo = QComboBox()
        self.window_combo.addItems(["10", "15", "20"])
        self.window_combo.setCurrentText(str(self.window_size))
        self.window_combo.setStyleSheet(
            "font-size: 20px; color: #e0e0e0; background-color: #333; "
            "border: 1px solid #FFD700; border-radius: 5px;"
        )
        self.window_combo.currentTextChanged.connect(self.on_window_size_changed)

        streak_counter_label = QLabel("Streak: 0")
        streak_counter_label.setAlignment(Qt.AlignCenter)
        streak_counter_label.setStyleSheet("color: #FFD700; font-size: 20px; margin-left: 30px;")
        self.streak_counter_label = streak_counter_label

        streak_outer_layout = QVBoxLayout()
        self.streak_progress = QProgressBar()
        self.streak_progress.setRange(0, 5)
        self.streak_progress.setValue(0)
        self.streak_progress.setAlignment(Qt.AlignCenter)
        self.streak_progress.setStyleSheet("""
            QProgressBar {
                border: 2px solid #FFD700;
                border-radius: 10px;
                text-align: center;
                color: #0000FF;
                font-size: 28px;
                background-color: #222;
            }
            QProgressBar::chunk {
                background-color: #FFD700;
                border-radius: 10px;
            }
        """)
        streak_outer_layout.addWidget(self.streak_progress)

        control_layout = QHBoxLayout()
        control_layout.addWidget(target_label)
        control_layout.addStretch()
        control_layout.addWidget(self.streak_counter_label)
        control_layout.addWidget(window_label)
        control_layout.addWidget(self.window_combo)
        streak_outer_layout.addLayout(control_layout)

        streak_frame.setLayout(streak_outer_layout)
        main_layout.addWidget(streak_frame)

        # Main content layout with plot and metrics
        content_layout = QGridLayout()

        # Beep Plot
        self.plot_widget = pg.PlotWidget(title="Beeps Over Time")
        self.plot_widget.setYRange(0, 1.2)
        self.plot_widget.setBackground('#111')
        grid_pen = pg.mkPen(color=(224,224,224,50), style=Qt.DashLine)
        self.plot_widget.showGrid(x=True, y=True, alpha=0.3)
        self.plot_widget.setAntialiasing(True)
        self.plot_widget.setLabel('bottom', 'Time', units='s')
        self.plot_widget.setLabel('left', 'Event')
        self.plot_widget.addLegend()

        content_layout.addWidget(self.plot_widget, 0, 0)

        # Metrics panel on right
        metrics_frame = QFrame()
        metrics_frame.setStyleSheet(
            "background-color: #222; color: #0f0; font-size: 18px; "
            "padding: 15px; border-radius: 10px; border: 2px solid #0f0;"
        )
        metrics_layout = QVBoxLayout()
        self.total_beeps_label = QLabel("Total Beeps: 0")
        self.avg_interval_label = QLabel("Avg Interval: N/A")
        self.total_beeps_label.setStyleSheet("color: #e0e0e0;")
        self.avg_interval_label.setStyleSheet("color: #e0e0e0;")
        metrics_layout.addWidget(self.total_beeps_label)
        metrics_layout.addWidget(self.avg_interval_label)
        metrics_layout.addStretch()
        metrics_frame.setLayout(metrics_layout)

        content_layout.addWidget(metrics_frame, 0, 1)

        main_layout.addLayout(content_layout)
        parent.setLayout(main_layout)

        # Draw a square-wave line for beeps
        self.beep_curve = self.plot_widget.plot(pen='r', stepMode=True)
        self.streak_curve = self.plot_widget.plot(pen=None, symbol='s', symbolBrush='b')
        self.beep_timestamps = deque(maxlen=MAX_POINTS)
        self.beep_values = deque(maxlen=MAX_POINTS)
        self.streak_timestamps = deque(maxlen=MAX_POINTS)

    def on_window_size_changed(self, text):
        try:
            self.window_size = int(text)
        except ValueError:
            self.window_size = 10

    def handle_data(self, msg_type, timestamp):
        if msg_type == BEEP_TYPE:
            t = time.time() - self.start_time
            self.beep_timestamps.append(t)
            self.beep_values.append(1.0)
            self.beep_timestamps.append(t + 0.5)
            self.beep_values.append(0.0)
            self.total_beeps += 1
        elif msg_type == STREAK_TYPE:
            streak_val = int(timestamp)
            t = time.time() - self.start_time
            self.streak_timestamps.append(t)
            self.streak_value = streak_val
            self.streak_progress.setValue(self.streak_value)
            self.streak_counter_label.setText(f"Streak: {self.streak_value}")

    def update_plot(self):
        current_time = time.time() - self.start_time
        while self.beep_timestamps and self.beep_timestamps[0] < current_time - self.window_size:
            self.beep_timestamps.popleft()
            self.beep_values.popleft()
        while self.streak_timestamps and self.streak_timestamps[0] < current_time - self.window_size:
            self.streak_timestamps.popleft()

        # Prepare stepped square wave data for beep_curve (stepMode=True)
        x = list(self.beep_timestamps)
        y = list(self.beep_values)
        if x:
            # Append an extra x so len(x) == len(y) + 1 for stepMode
            x.append(current_time)
        self.beep_curve.setData(x, y)
        self.streak_curve.setData(self.streak_timestamps, [1.0] * len(self.streak_timestamps))
        self.plot_widget.setXRange(current_time - self.window_size, current_time)

        # Update metrics
        self.total_beeps_label.setText(f"Total Beeps: {self.total_beeps}")
        if len(self.beep_timestamps) > 1:
            intervals = [self.beep_timestamps[i+1] - self.beep_timestamps[i] for i in range(len(self.beep_timestamps)-1)]
            avg_interval_ms = int(sum(intervals) / len(intervals) * 1000)
            self.avg_interval_label.setText(f"Avg Interval: {avg_interval_ms} ms")
        else:
            self.avg_interval_label.setText("Avg Interval: N/A")

    def closeEvent(self, event):
        self.reader.stop()
        super().closeEvent(event)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setFont(QFont("Segoe UI", 10))
    win = MainWindow()
    win.show()
    sys.exit(app.exec_())