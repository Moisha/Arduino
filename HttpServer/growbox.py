import sys
import threading
from http.server import BaseHTTPRequestHandler
from http.server import HTTPServer
import json
import psycopg2
from datetime import datetime
import win32serviceutil
import win32service
import win32event
import servicemanager
import socket
import time

conn = psycopg2.connect(database="growbox",
                        host="localhost",
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
        self.wfile.write('<body>Был получен GET-запрос.</body></html>'.encode())

    def do_POST(self):
        content_len = int(self.headers.get('Content-Length'))
        post_body = self.rfile.read(content_len)
        print(post_body)
        saveToPg(post_body)

        self.send_response(200)
        self.send_header("Content-type", "text/plain")
        self.end_headers()
        self.wfile.write('OK'.encode())

def run():
  server_address = ('', 9000)
  httpd = HTTPServer(server_address, HttpGrowBoxHandler)
  try:
      httpd.serve_forever()
  except KeyboardInterrupt:
      httpd.server_close()

class AppServerSvc (win32serviceutil.ServiceFramework):
    _svc_name_ = "growbox_http"
    _svc_display_name_ = "Growbox data reciever service"

    def __init__(self,args):
        win32serviceutil.ServiceFramework.__init__(self,args)
        self.hWaitStop = win32event.CreateEvent(None,0,0,None)
        socket.setdefaulttimeout(60)

    def SvcStop(self):
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING)
        win32event.SetEvent(self.hWaitStop)

    def SvcDoRun(self):
        servicemanager.LogMsg(servicemanager.EVENTLOG_INFORMATION_TYPE,
                          servicemanager.PYS_SERVICE_STARTED,
                          (self._svc_name_,''))
        self.main()

    def main(self):
        thread = threading.Thread(target=run)
        thread.start()
        rc = None
        while rc != win32event.WAIT_OBJECT_0:
            rc = win32event.WaitForSingleObject(self.hWaitStop, 1000)

if __name__ == '__main__':
    if (len(sys.argv) > 1) & (sys.argv[1] == 'console'):
        run()
#    else:
#        win32serviceutil.HandleCommandLine(AppServerSvc)
#    threading.Thread(target=run).start()
#    while(True):
#        print("working")
#        time.sleep(1)
# See PyCharm help at https://www.jetbrains.com/help/pycharm/
