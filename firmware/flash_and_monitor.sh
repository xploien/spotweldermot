#!/bin/bash

# Obtener la ruta donde reside este script (la raíz del proyecto)
BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Si CMake pasó una ruta la usamos ($1), si no, usamos el archivo de la raíz
ELF_FILE=${1:-"$BASE_DIR/firmware.elf"}

if [ ! -f "$ELF_FILE" ]; then
    echo " Error: No se encontró el binario en: $ELF_FILE"
    exit 1
fi

echo "Flashing: $ELF_FILE..."
wlink -v flash "$ELF_FILE"

if [ $? -ne 0 ]; then
    echo " Error at flashing aborting."
    exit 1
fi

echo " Flash successful" 

# Iniciamos el monitor serial embebido
python3 - << 'EOF'
import serial
import serial.tools.list_ports
import sys
import time

def main():
    ports = list(serial.tools.list_ports.comports())
    # Prioridad a chips WCH (USB ID 1a86)
    wch_ports = sorted([p.device for p in ports if "1a86" in p.hwid], reverse=True)
    
    if not wch_ports:
        wch_ports = sorted([p.device for p in ports if "ttyACM" in p.device], reverse=True)

    if not wch_ports:
        print(" Error: No se encontró dispositivo serial (/dev/ttyACM*)")
        return

    port = wch_ports[0]
    print(f"  Connected to: {port} @ 115200. (Ctrl+C to exit)\n")
    
    try:
        with serial.Serial(port, 115200, timeout=0.1) as ser:
            ser.reset_input_buffer()
            while True:
                if ser.in_waiting:
                    # Leemos y decodificamos ignorando errores de caracteres parciales
                    data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                    print(data, end='', flush=True)
                time.sleep(0.01)
    except KeyboardInterrupt:
        print("\n\n Monitor serial cerrado.")
    except Exception as e:
        print(f"\n  Port Error: {e}")

if __name__ == "__main__":
    main()
EOF
