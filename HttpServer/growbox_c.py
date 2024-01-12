from http.server import BaseHTTPRequestHandler
from http.server import HTTPServer
import json
import psycopg2
from datetime import datetime
import numpy
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import io
from PIL import Image



def connectPg():
    return psycopg2.connect(database="growbox",
                            host="192.168.1.10",
                            user="postgres",
                            password="123",
                            port="5432")


def dtToPgString(unix_dt):
    dt = datetime.utcfromtimestamp(int(unix_dt))
    return "'" + dt.strftime('%Y%m%d %H:%M:%S') + "'"


def saveToPg(post_body):
    props = json.loads(post_body);
    # props = json.loads(b'{"dt": "1704899625",\r\n "dtStr": "2024-01-10T15:13:45", "lampMode": 0, "lampState": 1, "wateringState": 0, "soilHumidity": 315.0 }');

    fields = ['raw_dt', 'dt', 'lamp_state', 'lamp_mode', 'watering_state']
    values = [props['dt'], dtToPgString(props['dt']), props['lampState'], props['lampMode'], props['wateringState']]

    if 'humidity' in props:
        fields += ['humidity']
        values += [props['humidity']]

    if 'temperature' in props:
        fields += ['temperature']
        values += [props['temperature']]

    if 'soilHumidity' in props:
        fields += ['soil_humidity']
        values += [props['soilHumidity']]

    field_list = ", ".join(fields)

    conn = connectPg()
    try:
        sql = 'insert into readings (' + field_list + ') select ' + ", ".join(values)
        cur = conn.cursor()
        cur.execute(sql)
        conn.commit()
    finally:
        conn.close()


class HttpGrowBoxHandler(BaseHTTPRequestHandler):
    def getInfoBody(self):
        body = '<h3>Growbox parameters</h3>'
        try:
            sql = ('select dt, lamp_state, lamp_mode, watering_state, temperature, humidity, soil_humidity,' +
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
                    body += '<p>' + col_names[i] + ' = ' + str(values[i]) + '</p>\r\n'

                print(body)
            finally:
                conn.close()

        except Exception as e:
            body = e.args[0]

        return body

    def get_writeValues(self):
        self.wfile.write('<html><head><meta charset="utf-8">'.encode())
        self.wfile.write('<title>Growbox HTTP server</title></head>'.encode())
        self.wfile.write(('<body>' + self.getInfoBody() + '</body></html>').encode())

    def get_writePlot(self):
        sql = (
                    'select dt, lamp_state * 100 as lamp_state, watering_state * 95 + 2 as watering_state, temperature, humidity, soil_humidity' +
                    '  from readings order by idr desc limit 1000')

        conn = connectPg()
        try:
            cur = conn.cursor()
            cur.execute(sql)

            col_names = [desc[0] for desc in cur.description]
            values = numpy.array(list(cur.fetchall()))
            colors = ['k', 'c', 'r', 'b', 'g']

            fig, ax = plt.subplots(figsize=(20, 10))
            ax.set_title("Growbox")
            for i in range(5):
                ax.plot(values[:, 0], values[:, i + 1], label=col_names[i + 1],
                        color=colors[i])  # Plot some data on the axes.

            dates_fmt = mdates.DateFormatter('%d.%M %H:%m')
            ax.xaxis.set_major_formatter(dates_fmt)

            ax.legend()
            fig.savefig(self.wfile, format='png')
        except:
            pass


    def get_isPlot(self):
        return self.path == '/plot'

    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "image/png" if self.get_isPlot() else "text/html")
        self.end_headers()
        if (self.get_isPlot()):
            self.get_writePlot()
        else:
            self.get_writeValues()


    def do_POST(self):
        response_code = 200
        response_text: str = "OK"
        try:
            content_len = int(self.headers.get('Content-Length'))
            post_body = self.rfile.read(content_len)
            print(post_body)
            saveToPg(post_body)
        except Exception as e:
            response_code = 500
            response_text = e.args[0]

        self.send_response(response_code)
        self.send_header("Content-type", "text/plain")
        self.end_headers()
        self.wfile.write(response_text.encode())


def run():
    server_address = ('', 9000)
    httpd = HTTPServer(server_address, HttpGrowBoxHandler)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        httpd.server_close()


if __name__ == '__main__':
    run()
