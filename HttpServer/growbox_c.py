from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from sql import *

def dtToPgString(unix_dt):
    dt = datetime.utcfromtimestamp(int(unix_dt))
    return "'" + dt.strftime('%Y%m%d %H:%M:%S') + "'"


def check_float_prop(val, max_val = 100):
    v = float(val)
    return (v >= 0) & (v <= max_val)


def appendField(props, propName, fieldName, fields, values, checkFloat = False, max_val = 100):
    if (propName in props) and (not checkFloat or check_float_prop(props[propName], max_val)):
        fields += [fieldName]
        values += [props[propName]]


def saveToPg(post_body):
    props = json.loads(post_body);
    # props = json.loads(b'{"dt": "1704899625",\r\n "dtStr": "2024-01-10T15:13:45", "lampMode": 0, "lampState": 1, "wateringState": 0, "soilHumidity": 315.0 }');

    device = device_condition(props['source'])
    dt_now = f"'{datetime.now().strftime('%Y%m%d %H:%M:%S')}'"

    fields = ['id_device', 'dt_server']
    values = [device, dt_now]

    if 'dt' in props:
        fields += ['dt']
        values += [dtToPgString(props['dt'])]

    appendField(props, 'dt', 'raw_dt', fields, values, True, 1)
    appendField(props, 'lampState', 'lamp_state', fields, values, True, 1)
    appendField(props, 'lampMode', 'lamp_mode', fields, values, True, 1)
    appendField(props, 'humidifierState', 'humidifier_state', fields, values, True, 1)
    appendField(props, 'humidity', 'humidity', fields, values, True)
    appendField(props, 'targetHumidity', 'humidity_target', fields, values, True)
    appendField(props, 'temperature', 'temperature', fields, values, True)
    appendField(props, 'co2', 'co2', fields, values, True, 2000)
    appendField(props, 'co2State', 'co2_state', fields, values, True, 1)
    appendField(props, 'fanState', 'fan_state', fields, values, True, 1)

    field_list = ", ".join(fields)
    value_list = ", ".join(values)

    conn = connectPg()
    try:
        sql = f"insert into readings ({field_list}) select {value_list}"
        cur = conn.cursor()
        cur.execute(sql)
        conn.commit()
    finally:
        conn.close()


class HttpGrowBoxHandler(BaseHTTPRequestHandler):
    def getInfoBody(self):
        return get_info_text(True, "")


    def get_writeValues(self):
        self.wfile.write('<html><head><meta charset="utf-8">'.encode())
        self.wfile.write('<title>Growbox HTTP server</title></head>'.encode())
        self.wfile.write(('<body>' + self.getInfoBody() + '</body></html>').encode())


    def get_writePlot(self):
        source = 0
        cnt = 1000
        path_parts = self.path.split("/")

        if len(path_parts) > 2 and path_parts[2].isnumeric():
            source = path_parts[2]

        if len(path_parts) > 3 and path_parts[3].isnumeric():
            cnt = int(path_parts[3])

        write_plot_to_iobytes(self.wfile, 20, 10, cnt, source)


    def get_isPlot(self):
        return self.path.startswith('/plot')


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
            print(e.args[0])

        self.send_response(response_code)
        self.send_header("Content-type", "text/plain")
        self.end_headers()
        self.wfile.write(response_text.encode())


def run():
    server_address = ('', 9000)
    httpd = ThreadingHTTPServer(server_address, HttpGrowBoxHandler)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        httpd.server_close()


if __name__ == '__main__':
    run()
