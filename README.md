# GreenBin - Smart Waste Sorting System

## 1. Project Overview

GreenBin is an AI-based smart waste sorting system developed as a Mechatronics Engineering project focused on automated waste classification and sorting using computer vision, embedded systems, and electromechanical actuation.

The system combines image processing, artificial intelligence, Arduino-based control, and mechanical automation to automatically classify and distribute waste into different compartments.

The project was developed using an NVIDIA Jetson Nano running Linux for computer vision and AI processing, while an Arduino Mega handled the mechanical control of the system.

---

## 2. System Operation

The complete operation sequence of the system is described below:

1. A camera continuously monitors the waste input area.

2. Motion detection is performed using OpenCV.

3. When movement is detected, the system captures an image of the object.

4. The captured image is sent through the OpenAI API to GPT-4o, which analyzes the waste object and determines which compartment it should be sorted into.
   The AI system classifies the waste into one of the following categories:
   - Organic waste
   - Recyclable waste
   - Non-recyclable waste
   - Biological risk waste

5. The NVIDIA Jetson Nano sends a numeric command from 1 to 4 to the Arduino Mega through serial communication, where each number corresponds to a specific waste category.

   Example:

   1 = Organic waste  
   2 = Recyclable waste  
   3 = Non-recyclable waste  
   4 = Biological risk waste

7. The Arduino rotates the internal waste container using a NEMA stepper motor and a chain-driven transmission system until the correct compartment is aligned with the waste entry point.

8. Once positioned, linear actuators open the gate mechanism that allows the waste to pass into the corresponding compartment.

9. Limit switches are used to verify actuator position and prevent overtravel.

10. The waste falls into the corresponding compartment.

11. The actuators close the doors after the disposal process.

12. The system only allows one waste object per classification cycle to ensure correct positioning and disposal.

13. The Arduino sends a completion signal ("FIN") back to the Jetson Nano.

14. The system returns to standby mode waiting for the next classification cycle.

---

## 3. Hardware Components

### 3.1 Embedded Systems
- NVIDIA Jetson Nano Developer Kit
- Arduino Mega

### 3.2 Computer Vision
- KlipXtreme KWC-500 webcam (1080p USB camera)

### 3.3 AI and Software
- Python
- OpenCV
- OpenAI API
- GPT-4o
- Arduino IDE

### 3.4 Motion and Actuation
- NEMA 23 stepper motor
- DM542A stepper motor driver
- Chain-driven transmission mechanism
- CMC Engineering GmbH linear actuators
  - 24VDC
  - 500N force
  - 117 mm stroke
- Mechanical limit switches

### 3.5 Power System
- 12V DC power supply
- Emergency backup battery system (12V, 12Ah)
- YX850 automatic emergency switching module

### 3.6 Mechanical Structure
- Mechanical structure designed in SolidWorks
- Rotating multi-compartment waste container
- Chain and pulley transmission system
- Transparent acrylic side panels for internal visualization
- Aluminum structural frame

---

## 4. Software Architecture

The project was divided into two main control systems:

### 4.1 Python / Jetson Nano
Responsible for:
- Image acquisition using the webcam
- Motion detection using OpenCV
- Image processing
- Waste classification using the OpenAI API and GPT-4o
- Sending the waste category to the Arduino Mega through serial communication
- Coordinating the classification cycle

### 4.2 Arduino
Responsible for:
- Receiving the waste classification sent by the Jetson Nano
- Controlling all mechanical movements of the system
- Rotating the compartment system to the correct disposal position
- Controlling the opening and closing of the gate system
- Monitoring the limit switches during actuator movement
- Returning the system to the home position after each disposal cycle
- Sending the completion signal back to the Jetson Nano
- Serial communication with the Jetson Nano
---

## 5. Python-Based Vision System

The Python program running on the NVIDIA Jetson Nano was responsible for the computer vision, AI classification, and communication processes of the system.

The code was developed using multiple Python libraries, where each one handled a specific part of the classification cycle.

### Main Libraries Used

#### OpenCV (`cv2`)
Used for:
- Real-time video capture from the webcam
- Motion detection
- Image preprocessing
- Frame analysis
- Drawing detection rectangles
- Capturing and saving images

The system continuously analyzed video frames and compared them to detect movement inside the waste entry area.

Once movement was detected, the system automatically captured an image of the waste object.

#### OpenAI API (`openai`)
Used for:
- Sending the captured image to GPT-4o
- AI-based waste classification

The AI model analyzed the image and determined which waste category the object belonged to.

#### Pillow (`PIL`)
Used for:
- Image handling and optimization before classification

The captured image was resized and prepared before being sent to the OpenAI API.

#### PySerial (`serial`)
Used for:
- Serial communication between the Jetson Nano and the Arduino Mega

After the waste was classified, the Python program sent a numeric command from 1 to 4 to the Arduino, where each number represented a specific waste category.

#### Datetime (`datetime`)
Used for:
- Generating unique timestamps for captured images

This allowed the system to save each captured image with a unique filename.

#### Time (`time`)
Used for:
- Timing control
- Detection intervals
- Synchronization between classification cycles

#### Base64 (`base64`)
Used for:
- Preparing the captured image before sending it through the OpenAI API

### Motion Detection Process

The motion detection system was implemented using OpenCV frame differencing techniques.

The process worked as follows:

1. The webcam continuously captured video frames.
2. Each frame was converted to grayscale.
3. Gaussian blur was applied to reduce image noise.
4. The current frame was compared with the previous frame.
5. Thresholding and contour detection were used to identify movement areas.
6. Small contours were ignored to reduce false detections.
7. When significant movement was detected, the system captured an image and started the classification process.

### Classification and Synchronization

After classification:
- The Jetson Nano sent the waste category to the Arduino Mega.
- The Python program paused the detection system while the mechanical sorting process was executed.
- The Arduino performed the complete sorting sequence.
- Once the cycle was completed, the Arduino sent the message `"FIN"` back to the Jetson Nano.
- The Python program then restarted the detection and classification cycle.
---

## 6. Project Status

The system was successfully tested with different types of waste materials, including:
-Plastic containers
- Aluminum cans
-Organic waste
- Cardboard packaging
- Bottles
- Stones
- Cleaning sponge materials
- Face mask 

During development, several additional features were proposed and partially designed, but were not fully integrated into the final prototype due to project scope and time limitations. These included:
- Ultrasonic sensors for compartment fill-level monitoring
-Solar power support system
- Remote monitoring integration via Telegram

The final prototype successfully demonstrated the integration of computer vision, AI-based classification, embedded systems, and mechanical automation.

The project passed the final course evaluation with a score of 4.45 / 5.0.
