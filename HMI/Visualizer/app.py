import sys
import os
from datetime import datetime
import pandas as pd
from vistas.interfaz_ui import *
from PyQt5 import QtCore, QtWidgets
from PyQt5.QtWidgets import QTableWidgetItem, QFileDialog, QMessageBox
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure


class MiApp(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        print("Inicializando Ventana...")
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        # Conexiones de los botones
        self.ui.agregar.clicked.connect(self.cargar_archivo)
        self.ui.graficar.clicked.connect(self.graficar)

        self.datos = None  # Variable para almacenar el DataFrame
        self.archivos_recientes = []  # Lista para historial de archivos

        # Configurar tabla de archivos recientes
        self.ui.tabla_recientes.setColumnCount(4)
        self.ui.tabla_recientes.setHorizontalHeaderLabels(["Nombre", "Fecha", "Tipo", "Tamaño"])

    def cargar_archivo(self):
        ruta, _ = QFileDialog.getOpenFileName(
            self,
            "Abrir archivo .txt",
            "",
            "Archivos TXT (*.txt);;CSV (*.csv)"
        )
        if ruta:
            try:
                self.datos = pd.read_csv(ruta, delimiter=",")
                print("Archivo cargado correctamente.")
                print(self.datos.head())

                self.mostrar_en_tabla()

                info_archivo = {
                    "ruta": ruta,
                    "nombre": os.path.basename(ruta),
                    "fecha": datetime.now().strftime("%d/%m/%Y"),
                    "tipo": "CSV/TXT",
                    "tamano": f"{os.path.getsize(ruta) // 1024} KB"
                }
                self.archivos_recientes.append(info_archivo)
                self.agregar_a_tabla_recientes(info_archivo)

            except Exception as e:
                print(f"Error al leer el archivo: {e}")

    def agregar_a_tabla_recientes(self, info):
        fila = self.ui.tabla_recientes.rowCount()
        self.ui.tabla_recientes.insertRow(fila)
        self.ui.tabla_recientes.setItem(fila, 0, QTableWidgetItem(info["nombre"]))
        self.ui.tabla_recientes.setItem(fila, 1, QTableWidgetItem(info["fecha"]))
        self.ui.tabla_recientes.setItem(fila, 2, QTableWidgetItem(info["tipo"]))
        self.ui.tabla_recientes.setItem(fila, 3, QTableWidgetItem(info["tamano"]))

    def graficar(self):
        fila = self.ui.tabla_recientes.currentRow()
        if fila == -1:
            if self.datos is None:
                print("No hay datos cargados ni archivo seleccionado.")
                return
            else:
                datos = self.datos
        else:
            try:
                ruta = self.archivos_recientes[fila]["ruta"]
                datos = pd.read_csv(ruta, delimiter=",")
            except Exception as e:
                QMessageBox.critical(self, "Error", f"No se pudo cargar el archivo: {e}")
                return

        for i in reversed(range(self.ui.viscograf.count())):
            widget = self.ui.viscograf.itemAt(i).widget()
            if widget:
                widget.setParent(None)

        figura = Figure(figsize=(5, 4), dpi=100)
        canvas = FigureCanvas(figura)
        ax = figura.add_subplot(111)

        try:
            x = datos['Temperatura']
            y = datos['Viscosidad']
            ax.plot(x, y, marker='o', color='blue')
            ax.set_title("Viscosity vs. Temperature")
            ax.set_xlabel("Temperature (\u00b0C")
            ax.set_ylabel("Viscosity (Pa\u00b7s)")
            ax.grid(True)
            self.ui.viscograf.addWidget(canvas)
        except KeyError:
            QMessageBox.warning(self, "Error", "Columnas 'Temperatura' o 'Viscosidad' no encontradas.")

    def mostrar_en_tabla(self):
        if self.datos is None:
            return

        self.ui.tabla_datos.setRowCount(0)
        self.ui.tabla_datos.setColumnCount(0)

        num_filas = len(self.datos.index)
        num_columnas = len(self.datos.columns)

        self.ui.tabla_datos.setRowCount(num_filas)
        self.ui.tabla_datos.setColumnCount(num_columnas)
        self.ui.tabla_datos.setHorizontalHeaderLabels(self.datos.columns)

        for i in range(num_filas):
            for j in range(num_columnas):
                valor = str(self.datos.iat[i, j])
                self.ui.tabla_datos.setItem(i, j, QtWidgets.QTableWidgetItem(valor))


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    ventana = MiApp()
    ventana.show()
    sys.exit(app.exec_())
