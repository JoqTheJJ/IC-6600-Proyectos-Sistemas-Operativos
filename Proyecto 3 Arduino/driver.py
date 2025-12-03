import serial
import time
from pycaw.pycaw import AudioUtilities
import screen_brightness_control as sbc


#Nombre puerto
puerto = "COM6"
baud = 9600

puerto_serial = serial.Serial(puerto, baud, timeout=1)
time.sleep(2)




#Volumen
device = AudioUtilities.GetSpeakers()
volume = device.EndpointVolume

#Flag cambio
last_vol = -1
last_bri = -1

print("Driver [ubu] iniciado")



while True:
    try:
        if puerto_serial.in_waiting > 0:
            linea = puerto_serial.readline().decode().strip()

            #Estructura "volumen,brillo\n" num = 0-100**
            if "," not in linea:
                continue

            volumen, brillo = linea.split(",")
            vol = int(volumen)
            bri = int(brillo)


            #Cambios Volumen
            if vol != last_vol:
                real_vol = vol / 100.0
                volume.SetMasterVolumeLevelScalar(real_vol, None)
                last_vol = vol


            #Cambios brillo
            if bri != last_bri:
                sbc.set_brightness(bri)
                last_bri = bri


            print(f"Volumen:{vol} Brillo:{bri}")

    except Exception as e:
        print("Error: ", e)
        time.sleep(5)