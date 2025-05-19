import sys
import pandas as pd
import numpy

import PyQt5 as QTcore

from PyQt5.QtWidgets import QMainWindow, QApplication, QFileDialog

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas

from matplotlib.figure import Figure

from vistas.interfaz_ui import Ui_MainWindow

class MiApp(QMainWindow):
    def __init__(self):
        super().__init__()
        print("Inicializando Ventana...")
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        # Conexiones de los botones
        self.ui.agregar.clicked.connect(self.cargar_archivo)
        self.ui.graficar.clicked.connect(self.graficar)

        self.datos = None  # Variable para almacenar el DataFrame


    def cargar_archivo(self):
        ruta, _ = QFileDialog.getOpenFileName(self, "Abrir archivo .txt", "", "Archivos TXT (*.txt);;CSV (*.csv)")
        if ruta:
            try:
                self.datos = pd.read_csv(ruta, delimiter=",")
                print("Archivo cargado correctamente.")
                print(self.datos.head())
            except Exception as e:
                print(f"Error al leer el archivo: {e}")

    def graficar(self):
        if self.datos is None:
            print("Primero debe cargar un archivo.")
            return

        # Limpiar cualquier gráfico anterior
        for i in reversed(range(self.ui.viscograf.count())):
            widget = self.ui.viscograf.itemAt(i).widget()
            if widget:
                widget.setParent(None)

        # Crear figura
        figura = Figure(figsize=(5, 4), dpi=100)
        canvas = FigureCanvas(figura)
        ax = figura.add_subplot(111)

        # Tomar datos del csv
        try:
            x = self.datos['Temperatura']
            y = self.datos['Viscosidad']
            ax.plot(x, y, marker='o', color='blue')
            ax.set_title("Viscosity vs. Temperature")
            ax.set_xlabel("Temperature (°C)")
            ax.set_ylabel("Viscosity (Pa·s)")
            ax.grid(True)
            self.ui.viscograf.addWidget(canvas)
        except KeyError:
            print("Las columnas 'Temperatura' y 'Viscosidad' no se encontraron en el archivo.")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    ventana = MiApp()
    ventana.show()
    sys.exit(app.exec_())
