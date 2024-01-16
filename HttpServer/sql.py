import psycopg2
from datetime import datetime
import numpy
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.ticker as ticker

matplotlib.use('agg')

def connectPg():
    return psycopg2.connect(database="growbox",
                            host="192.168.1.10",
                            user="postgres",
                            password="123",
                            port="5432")


def get_info_text(for_html):
    h1 = '<h3>' if for_html else ''
    h2 = '</h3>' if for_html else ''
    p1 = '<p>' if for_html else ''
    p2 = '</p>' if for_html else ''

    body = h1 + 'Growbox parameters' + h2 + '\r\n'
    try:
        sql = ('select dt, lamp_state, lamp_mode, watering_state, temperature, humidity, soil_humidity, soil_humidity_raw,' +
               '       (select dt from readings r1 where watering_state = 1 order by idr desc limit 1) as last_watering ' +
               '  from readings order by idr desc limit 1')
        print(sql)

        conn = connectPg()
        try:
            cur = conn.cursor()
            cur.execute(sql)
            col_names = [desc[0] for desc in cur.description]
            values = cur.fetchone()
            for i in range(len(col_names)):
                body += p1 + col_names[i] + ' = ' + str(values[i]) + p2 + '\r\n'

            print(body)
        finally:
            conn.close()

    except Exception as e:
        body = e.args[0]

    return body


def write_plot_to_iobytes(wfile, len, height):
    sql = (
            'select dt, lamp_state * 9 as lamp_state, watering_state * 9 + 10 as watering_state, temperature, humidity, ' +
            '       case when soil_humidity > 100 or soil_humidity < 0 then null else soil_humidity end as soil_humidity' +
            '  from readings ' +
            '  where dt > CURRENT_DATE - INTERVAL \'3 days\' ' +
            'order by idr desc limit 1000')

    conn = connectPg()
    try:
        cur = conn.cursor()
        cur.execute(sql)

        col_names = [desc[0] for desc in cur.description]
        values = numpy.array(list(cur.fetchall()))
        colors = ['k', 'c', 'r', 'b', 'g']

        fig, ax = plt.subplots(figsize=(len, height))
        ax.set_title("Growbox")
        for i in range(5):
            ax.plot(values[:, 0], values[:, i + 1], label=col_names[i + 1], color=colors[i])

        dates_fmt = mdates.DateFormatter('%d.%m %H:%M')
        ax.xaxis.set_major_formatter(dates_fmt)
        ax.xaxis.set_major_locator(ticker.LinearLocator(numticks = round(len / 1.5)))

        plt.grid(True, axis='y')

        ax.legend()
        fig.savefig(wfile, format='png')
        fig.close
    except:
        pass
