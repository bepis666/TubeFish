from flask import Flask, render_template, request

# Инициализация веб-приложения
app = Flask(__name__)

signal_status_dict = {'pin_red': False, 'pin_green': False, 'pin_blue': False}


@app.route("/", methods=['POST', 'GET'])
# Отрисовка корневой страницы
def draw_main_page():
    return render_template('main_page.html', RED=signal_status_dict['pin_red'], GREEN=signal_status_dict['pin_green'],
                           BLUE=signal_status_dict['pin_blue'])


@app.route("/control", methods=['POST'])
# Получение сигналов управления
def get_control_signals():

    if request.method == 'POST':

        signal_dict = {}

        # Положение джойстика
        if request.args.get('controller') == '1':
            signal_dict.update(x=request.args.get('x'))
            signal_dict.update(y=request.args.get('y'))

        # Положение ползунка глубины
        elif request.args.get('controller') == '2':
            signal_dict.update(d_level=request.args.get('d_level'))

        # Цветные кнопки
        if request.form.get('red_button'):
            signal_dict.update(red_button='pressed')

        elif request.form.get('green_button'):
            signal_dict.update(green_button='pressed')

        elif request.form.get('blue_button'):
            signal_dict.update(blue_button='pressed')

        print(signal_dict)
        return signal_dict
    else:
        signal_dict = {}
        return signal_dict


if __name__ == '__main__':
    app.run(debug=True)

