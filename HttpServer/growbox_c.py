from http.server import BaseHTTPRequestHandler
from http.server import HTTPServer
import json
import psycopg2
from datetime import datetime

conn = psycopg2.connect(database="growbox",
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

    sql = 'insert into readings (' + field_list + ') select ' + ", ".join(values)
    cur = conn.cursor()
    cur.execute(sql)
    conn.commit()

class HttpGrowBoxHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write('<html><head><meta charset="utf-8">'.encode())
        self.wfile.write('<title>Growbox HTTP server</title></head>'.encode())

        body = ''
        try:
            sql = 'select dt, lamp_state, lamp_mode, watering_state, temperature, humidity, soil_humidity, (select dt from readings r1 where watering_state = 1 order by idr desc limit 1) as last_watering from readings order by idr desc limit 1'
            print(sql)

            cur = conn.cursor()
            cur.execute(sql)
            col_names = [desc[0] for desc in cur.description]
            values = cur.fetchone()
            for i in range(len(col_names)):
                body += '<p>' + col_names[i] + ' = ' + str(values[i]) + '</p>\r\n'

            body = '<h3>Growbox parameters</h3>' + body
            print(body)

        except Exception as e:
            body = e.args[0]
            pass

        self.wfile.write(('<body>' + body + '</body></html>').encode())

    def do_POST(self):
        responce_code = 200
        responce_text = "OK"
        try:
            content_len = int(self.headers.get('Content-Length'))
            post_body = self.rfile.read(content_len)
            print(post_body)
            saveToPg(post_body)
        except Exception as e:
            responce_code = 500
            responce_text = e
            pass

        self.send_response(responce_code)
        self.send_header("Content-type", "text/plain")
        self.end_headers()
        self.wfile.write(responce_text.encode())

def run():
  server_address = ('', 9000)
  httpd = HTTPServer(server_address, HttpGrowBoxHandler)
  try:
      httpd.serve_forever()
  except KeyboardInterrupt:
      httpd.server_close()

if __name__ == '__main__':
    run()
