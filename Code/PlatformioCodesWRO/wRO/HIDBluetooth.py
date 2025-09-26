import sys
import serial
import serial.tools.list_ports
import threading
import time
import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
import matplotlib
matplotlib.use('TkAgg')
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

class HIDBluetoothApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("HID Bluetooth Monitor")
        self.geometry("900x600")
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

        # Serial
        self.serial_port = None
        self.serial_thread = None
        self.running = False
        self.data_buffer = []
        self.max_points = 100

        # Variables
        self.velocidad = tk.StringVar()
        self.distancia = tk.StringVar()
        self.angulo_actual = tk.StringVar()
        self.angulo_objetivo = tk.StringVar()
        self.dist_izq = tk.DoubleVar()
        self.dist_der = tk.DoubleVar()

        # UI
        self.create_widgets()
        self.after(1000, self.update_plot)

    def create_widgets(self):
        frame = ttk.Frame(self)
        frame.pack(side=tk.TOP, fill=tk.X, padx=10, pady=10)

        ttk.Label(frame, text="Velocidad:").grid(row=0, column=0, sticky=tk.W)
        ttk.Label(frame, textvariable=self.velocidad).grid(row=0, column=1, sticky=tk.W)
        ttk.Label(frame, text="Distancia recorrida:").grid(row=1, column=0, sticky=tk.W)
        ttk.Label(frame, textvariable=self.distancia).grid(row=1, column=1, sticky=tk.W)
        ttk.Label(frame, text="Ángulo actual:").grid(row=2, column=0, sticky=tk.W)
        ttk.Label(frame, textvariable=self.angulo_actual).grid(row=2, column=1, sticky=tk.W)
        ttk.Label(frame, text="Ángulo objetivo:").grid(row=3, column=0, sticky=tk.W)
        ttk.Label(frame, textvariable=self.angulo_objetivo).grid(row=3, column=1, sticky=tk.W)
        ttk.Label(frame, text="Distancia sensor izquierdo:").grid(row=4, column=0, sticky=tk.W)
        ttk.Label(frame, textvariable=self.dist_izq).grid(row=4, column=1, sticky=tk.W)
        ttk.Label(frame, text="Distancia sensor derecho:").grid(row=5, column=0, sticky=tk.W)
        ttk.Label(frame, textvariable=self.dist_der).grid(row=5, column=1, sticky=tk.W)

        # Serial port selection
        self.port_var = tk.StringVar()
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_menu = ttk.Combobox(frame, textvariable=self.port_var, values=ports, state="readonly")
        self.port_menu.grid(row=0, column=3, padx=10)
        self.port_menu.set(ports[0] if ports else "")
        ttk.Button(frame, text="Conectar", command=self.connect_serial).grid(row=1, column=3, padx=10)
        ttk.Button(frame, text="Desconectar", command=self.disconnect_serial).grid(row=2, column=3, padx=10)
        # Botón para mover 10cm
        ttk.Button(frame, text="Mover 10cm", command=self.send_move10).grid(row=3, column=3, padx=10)

        # Plot
        self.fig = Figure(figsize=(6,3), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_title("Distancia sensores (cm)")
        self.ax.set_xlabel("Tiempo")
        self.ax.set_ylabel("Distancia (cm)")
        self.line_izq, = self.ax.plot([], [], label="Izquierdo")
        self.line_der, = self.ax.plot([], [], label="Derecho")
        self.ax.legend()
        self.canvas = FigureCanvasTkAgg(self.fig, master=self)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)
    def send_move10(self):
        if self.serial_port and self.serial_port.is_open:
            try:
                self.serial_port.write(b'MOVE10\n')
            except Exception as e:
                messagebox.showerror("Error", f"No se pudo enviar el comando: {e}")

    def connect_serial(self):
        port = self.port_var.get()
        if not port:
            messagebox.showerror("Error", "Selecciona un puerto COM")
            return
        try:
            self.serial_port = serial.Serial(port, 115200, timeout=1)
            self.running = True
            self.serial_thread = threading.Thread(target=self.read_serial, daemon=True)
            self.serial_thread.start()
        except Exception as e:
            messagebox.showerror("Error", f"No se pudo abrir el puerto: {e}")

    def disconnect_serial(self):
        self.running = False
        if self.serial_port:
            self.serial_port.close()
            self.serial_port = None

    def read_serial(self):
        while self.running and self.serial_port:
            try:
                line = self.serial_port.readline().decode(errors='ignore').strip()
                if line:
                    self.parse_line(line)
            except Exception:
                pass

    def parse_line(self, line):
        # Espera líneas tipo:
        # Pulsos: 123 | Heading: 45.0 | Servo: 90 | PWM: 120 | US Izq: 23 cm | US Der: 45 cm
        try:
            if "Pulsos:" in line:
                parts = line.split("|")
                for part in parts:
                    if "Pulsos:" in part:
                        self.distancia.set(part.split(":")[1].strip())
                    elif "Heading:" in part:
                        self.angulo_actual.set(part.split(":")[1].strip())
                    elif "Servo:" in part:
                        self.angulo_objetivo.set(part.split(":")[1].strip())
                    elif "US Izq:" in part:
                        self.dist_izq.set(float(part.split(":")[1].replace("cm", "").strip()))
                    elif "US Der:" in part:
                        self.dist_der.set(float(part.split(":")[1].replace("cm", "").strip()))
                # Para el plot
                self.data_buffer.append((self.dist_izq.get(), self.dist_der.get()))
                if len(self.data_buffer) > self.max_points:
                    self.data_buffer.pop(0)
        except Exception:
            pass

    def update_plot(self):
        izq = [d[0] for d in self.data_buffer]
        der = [d[1] for d in self.data_buffer]
        self.line_izq.set_data(range(len(izq)), izq)
        self.line_der.set_data(range(len(der)), der)
        self.ax.relim()
        self.ax.autoscale_view()
        self.canvas.draw()
        self.after(200, self.update_plot)

    def on_closing(self):
        self.running = False
        if self.serial_port:
            self.serial_port.close()
        self.destroy()

if __name__ == "__main__":
    app = HIDBluetoothApp()
    app.mainloop()
