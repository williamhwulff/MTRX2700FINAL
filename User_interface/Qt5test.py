# update_test.py

import sys
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout
from PyQt5.QtCore import QTimer, Qt

class StreakWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Streak Monitor")
        self.streak = 0

        # Centered label
        self.label = QLabel(f"Streak: {self.streak}")
        self.label.setAlignment(Qt.AlignCenter)

        # Layout
        layout = QVBoxLayout(self)
        layout.addWidget(self.label)

        # Timer to increment once per second
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.simulate_update)
        self.timer.start(1000)

    def simulate_update(self):
        self.streak += 1
        self.label.setText(f"Streak: {self.streak}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    w = StreakWindow()
    w.resize(200, 100)
    w.show()
    sys.exit(app.exec_())