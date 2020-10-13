from tkinter import ttk
from tkinter import *
import compiler


class Proyecto:
    def __init__(self, window):
        self.wind = window
        self.wind.title('Proyecto 1 Base de datos')

        frame = LabelFrame(self.wind, text='Base de datos')
        frame.grid(row=0, column=0)

        # Instrucciones
        Label(frame, text='Ingresar Instrucciones: ').grid(row=1, column=0)
        Instrucciones = self.inst = Text(frame)
        self.inst.grid(row=1, column=1)

        def super_crack():
            text_input = ""
            text_input += text_input + Instrucciones.get("1.0", END)
            text_input = text_input[0:-1]
            compiler.compile(text_input)

        ttk.Button(frame, text="Run", command=super_crack).grid(row=2, columnspan=2, sticky=W + E)


if __name__ == '__main__':
    window = Tk()
    application = Proyecto(window)
    window.mainloop()