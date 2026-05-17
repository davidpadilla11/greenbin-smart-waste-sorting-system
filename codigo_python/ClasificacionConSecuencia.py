import cv2
import datetime
import time
import base64
import openai
from PIL import Image
from io import BytesIO
import serial

api_key = "OPENAI_API_KEY"
openai.api_key = api_key

# Redimensiona y optimiza la imagen capturada antes de enviarla a la API de OpenAI. / Resize and optimize the captured image before sending it to the OpenAI API
def encode_image(image_path):
    max_size = (512, 512)
    with Image.open(image_path) as img:
        img = img.convert("RGB")
        img.thumbnail(max_size)
        buffered = BytesIO()
        img.save(buffered, format="JPEG", quality=85)
        return base64.b64encode(buffered.getvalue()).decode('utf-8')

# Enviar la imagen capturada a GPT-4o para la clasificación de residuos. / Send the captured image to GPT-4o for waste classification.
def generate_response(image_path):
    base64_image = encode_image(image_path)
    try:
        response = openai.ChatCompletion.create(
            model="gpt-4o",
            messages=[
                {"role": "system", "content": "Clasifica el residuo como orgánico, aprovechable, no aprovechable o riesgo biológico."},
                {
                    "role": "user",
                    "content": [
                        {
                            "type": "text",
                            "text": "Clasifica el residuo en la caneca y a qué tipo de compartimento pertenece: orgánico, aprovechable, no aprovechable o riesgo biológico."
                        },
                        {
                            "type": "image_url",
                            "image_url": {
                                "url": f"data:image/jpeg;base64,{base64_image}"
                            }
                        }
                    ]
                }
            ],
            max_tokens=300
        )
        assistant_response = response['choices'][0]['message']['content']
        return assistant_response
    except Exception as e:
        print(f"Error al generar la respuesta: {e}")
        return "Lo siento, hubo un problema al generar la respuesta."

# Enviar la categoría de residuos clasificados al Arduino Mega a través de comunicación serial / Send the classified waste category to the Arduino Mega via serial communication
def enviar_serial(arduino, dato):
    try:
        arduino.write(f"{dato}\n".encode())
        time.sleep(0.1)
    except Exception as e:
        print(f"Error al enviar datos al Arduino: {e}")

def main():
    # Inicializar la comunicación serial entre Jetson Nano y Arduino Mega / Initialize serial communication between Jetson Nano and Arduino Mega
    arduino = serial.Serial(port='/dev/ttyUSB0', baudrate=9600, timeout=1)

    contador_organico = 0
    contador_aprovechable = 0
    contador_no_aprovechable = 0
    contador_riesgo_biologico = 0

    # Inicializar la cámara web para la captura continua de vídeo / Initialize the webcam for continuous video capture
    camera = cv2.VideoCapture(0)
    if not camera.isOpened():
        print("Error: No se pudo acceder a la cámara.")
        arduino.close()
        return

    interval = 3
    last_time = time.time()
    deteccion_habilitada = True
    last_frame = None
    pausa_deteccion = False

    while True:
        # Si está en pausa, esperar el mensaje "FIN" del Arduino
        if pausa_deteccion:
            if arduino.in_waiting > 0:
                mensaje = arduino.readline().decode().strip()
                # Esperar a que el Arduino confirme el final de la secuencia de clasificación / Wait for the Arduino to confirm the end of the classification sequence
                if mensaje == "FIN":
                    print("Arduino completó la secuencia. Reiniciando sistema...")
                    camera.release()
                    arduino.close()
                    return  # Salir de la función para reiniciar completamente
            if 'frame' in locals():
                cv2.imshow("Video", frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
            continue

        ret, frame = camera.read()
        if not ret:
            print("Error: No se pudo leer el fotograma.")
            break
        
        # Convertir el fotograma actual a escala de grises para el análisis de movimiento / Convert the current frame to grayscale for motion analysis
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        # Aplica un desenfoque gaussiano para reducir el ruido de la imagen y mejorar la estabilidad de la detección de movimiento. / Apply a Gaussian blur to reduce image noise and improve motion detection stability.
        gray = cv2.GaussianBlur(gray, (21, 21), 0)

        if last_frame is None:
            # Si es la primera vez o tras reinicio, establecer el primer fotograma como referencia / If it's the first time or after reset, set the first frame as reference
            last_frame = gray
            last_time = time.time()
            continue

        current_time = time.time()

        if deteccion_habilitada and current_time - last_time >= interval:
            # Compara el fotograma actual con el fotograma anterior para detectar el movimiento / Compare the current frame with the previous frame to detect movement
            frame_delta = cv2.absdiff(last_frame, gray)
            # Aplicar umbralización para aislar las regiones de movimiento / Apply thresholding to isolate motion regions
            thresh = cv2.threshold(frame_delta, 30, 255, cv2.THRESH_BINARY)[1]
            thresh = cv2.dilate(thresh, None, iterations=2)

            # Detectar contornos de movimiento en la imagen umbralizada / Detect motion contours in the thresholded image
            contours, _ = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            movement_detected = False

            for contour in contours:
                # Ignorar los contornos pequeños para reducir las detecciones falsas / Ignore small contours to reduce false detections
                if cv2.contourArea(contour) < 1000:
                    continue
                movement_detected = True
                (x, y, w, h) = cv2.boundingRect(contour)
                # Dibujar un rectángulo de detección alrededor del objeto detectado / Draw detection rectangle around the detected object 
                cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

            if movement_detected:
                # Genera un nombre de archivo único para la imagen capturada / Generate a unique filename for the captured image
                timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
                filename = f"movement_{timestamp}.jpg"
                cv2.imwrite(filename, frame)
                print(f"Foto guardada: {filename}")

                print("Clasificando el residuo...")
                # Envía la imagen capturada para la clasificación de residuos basada en IA / Send the captured image for AI-based waste classification
                response = generate_response(filename)
                print(f"Clasificación: {response}")

                # Convierte el resultado de la clasificación de IA en un comando numérico para Arduino / Convert the AI classification result into a numeric command for Arduino
                if "no aprovechable" in response.lower():
                    contador_no_aprovechable += 1
                    enviar_serial(arduino, 3)
                elif "orgánico" in response.lower():
                    contador_organico += 1
                    enviar_serial(arduino, 1)
                elif "aprovechable" in response.lower():
                    contador_aprovechable += 1
                    enviar_serial(arduino, 2)
                elif "riesgo biológico" in response.lower():
                    contador_riesgo_biologico += 1
                    enviar_serial(arduino, 4)
                else:
                    print("No se pudo clasificar el residuo correctamente.")

                print(f"Contadores actualizados: Orgánico={contador_organico}, Aprovechable={contador_aprovechable}, No Aprovechable={contador_no_aprovechable}, Riesgo Biológico={contador_riesgo_biologico}")

                # Pausar el sistema de visión mientras se ejecuta el ciclo de clasificación mecánica / Pause the vision system while the mechanical classification cycle is executed
                deteccion_habilitada = False
                pausa_deteccion = True

            last_frame = gray
            last_time = current_time

        cv2.imshow("Video", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    camera.release()
    cv2.destroyAllWindows()
    arduino.close()

# Bucle principal para reiniciar el sistema desde cero al recibir "FIN" / Main loop to restart the system from scratch upon receiving "FIN"
while True:
    main()

