# -*- coding: utf-8 -*-
import logging
from quart import Quart, render_template, Response, request
import time
import smbus
import asyncio
import cv2

# i2c engine driver address
engine_driver_address = 11

print('\nTubeFish ver 0.1')


""" ----- Web-app ----- """

web_interface_last_response_time = 0
web_interface_connection_status = False

print('starting web-app...')
web_app = Quart(__name__)


# Periodic functions
async def scheduler():
    while True:
        await i2c_periodic(0, 10)


@web_app.before_serving
async def startup():
    web_app.background_tasks = asyncio.ensure_future(scheduler())


@web_app.after_serving
async def shutdown():
    web_app.background_tasks = 0


@web_app.route("/", methods=['POST', 'GET'])
async def draw_main_page():
    return await render_template('main_page.html')


@web_app.route('/video_feed')
def video_feed():
    # Generation and sending video frame
    return Response(get_frame(), mimetype='multipart/x-mixed-replace; boundary=frame')


@web_app.route("/control", methods=['POST'])
# Getting control signals
async def get_control_signals():

    if engine_driver_connection_status:
        if request.method == 'POST':

            signal_dict = {}

            # Joystick's pose
            if request.args.get('controller') == '1':

                pos_x = int(request.args.get('x'))
                if pos_x > 100:
                    pos_x = 100
                elif pos_x < -100:
                    pos_x = -100

                pos_y = int(request.args.get('y'))
                if pos_y > 100:
                    pos_y = 100
                elif pos_y < -100:
                    pos_y = -100

                signal_dict.update(x=pos_x)
                signal_dict.update(y=pos_y)

                # Conversion joystick's pose to engine's signal
                if pos_x >= 0:
                    # first quarter
                    if pos_y >= 0:
                        horizontal_left = pos_y
                        horizontal_right = pos_y - pos_x
                    # fourth quarter
                    else:
                        horizontal_left = pos_y
                        horizontal_right = pos_y + pos_x
                else:
                    # second quarter
                    if pos_y >= 0:
                        horizontal_left = pos_y + pos_x
                        horizontal_right = pos_y
                    # third quarter
                    else:
                        horizontal_left = pos_y - pos_x
                        horizontal_right = pos_y

                # Conversion from [-100:100] to [0:200] for correct I2C transmit
                horizontal_left += 100
                horizontal_right += 100

                # I2C transmit
                i2c.write_i2c_block_data(engine_driver_address, 3, [horizontal_left, horizontal_right])

            # Depth joystick pose
            elif request.args.get('controller') == '2':

                pos_depth = int(request.args.get('d_level'))
                signal_dict.update(d_level=pos_depth)

                vertical_left = pos_depth
                vertical_right = pos_depth

                # Conversion from [-100:100] to [0:200] for correct I2C transmit
                vertical_left += 100
                vertical_right += 100

                # I2C transmit
                i2c.write_i2c_block_data(engine_driver_address, 1, [vertical_left, vertical_right])

            # Speed mode
            elif request.args.get('controller') == '3':

                # I2C transmit
                i2c.write_byte_data(engine_driver_address, 5, int(request.args.get('mode')))

    return Response(response=None, status=200)


@web_app.route("/get_status", methods=['POST'])
async def status_response():
    global web_interface_last_response_time, engine_driver_connection_status, i2c_array

    web_interface_last_response_time = time.time()
    system_status_array = [engine_driver_connection_status]
    for x in i2c_array:
        system_status_array.append(x)

    response = ['<label id=v_l>' + str(int(i2c_array[1])-100) + '</label>',
                '<label id=v_r>' + str(int(i2c_array[2])-100) + '</label>',
                '<label id=h_l>' + str(int(i2c_array[3])-100) + '</label>',
                '<label id=h_r>' + str(int(i2c_array[4])-100) + '</label>']
    return response


""" ----- I2C ----- """

i2c_array = ['Connection lost', 'Connection lost', 'Connection lost', 'Connection lost', 'Connection lost']
engine_driver_connection_status = False


# SMBus initialization
def i2c_init():
    print('starting I2C...')
    bus = smbus.SMBus(0)

    return bus


# Engine emergency stop
def i2c_emergency_stop():
    i2c.write_i2c_block_data(engine_driver_address, 1, [100, 100, 100, 100])


# I2C periodic request
async def i2c_periodic(cmd, registers_number):
    global engine_driver_connection_status, i2c_array, web_interface_last_response_time

    try:
        i2c_array = i2c.read_i2c_block_data(engine_driver_address, cmd, registers_number)
        engine_driver_connection_status = True
    except Exception as exc:
        if engine_driver_connection_status:
            engine_driver_connection_status = False
            print('connection lost')

    if engine_driver_connection_status:
        if time.time() - web_interface_last_response_time > 2:
            i2c_emergency_stop()

    await asyncio.sleep(1)


""" ----- CAMERA ----- """


# Camera initialization
def camera_init(cam_index):
    print('starting camera...')
    cam = False
    try:
        cam = cv2.VideoCapture(cam_index)  # embedded camera (CSI)
        cam.setExceptionMode(True)
    except cv2.error as e:
        print('[ERROR] starting camera: ')
        print(e)
    return cam


# Camera getting frame
def get_frame():
    while True:
        time.sleep(0.033)    # fps
        success, frame = camera.read()
        if success:
            # frame = cv2.resize(frame, (640, 480), interpolation=cv2.INTER_AREA)
            # frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            # _, frame = cv2.threshold(frame, 127, 255, cv2.THRESH_BINARY)
            _, buffer = cv2.imencode('.jpg', frame)
            yield (b'--frame\r\n'
                   b'Content-Type: image/png\r\n\r\n' + buffer.tobytes() + b'\r\n')


if __name__ == '__main__':

    # Camera starting
    camera = camera_init(0)

    # I2C (smbus) starting
    i2c = i2c_init()

    web_app.config['RESPONSE_TIMEOUT'] = 10000
    web_app.run(debug=False, host="0.0.0.0", port=80)



