#Remember to put await.mp3, fail.mp3

from flask import Flask, request, jsonify
from flask_cors import CORS 
from datetime import datetime
import vlc
import requests
app = Flask(__name__)
CORS(app) 

import threading
import time

esp_url = "http://1.1.2.1:80/"
host_url = '1.1.1.2'
#host_url = '1.1.2.3'
debug = 'debug\log.txt'
class ResettableTimer:
    def __init__(self, timeout, callback):
        self.timeout = timeout
        self.callback = callback
        self.timer = None

    def start(self):
        self.cancel() 
        self.timer = threading.Timer(self.timeout, self.callback)
        self.timer.start()

    def reset(self):
        print("Timer reset")
        self.start()

    def cancel(self):
        if self.timer is not None:
            self.timer.cancel()
            self.timer = None

def send_to(url, data):
    while True:
        try:
            response = requests.post(url, json=data, timeout=10)
            response.raise_for_status()  
            with open(debug, 'a') as file:
                print("Success:", response.json(), file=file)
            print("Success:", response.json())  
            break
        except requests.exceptions.RequestException as e:
            print(f"Error: {e}. Retrying in 5 seconds...")
            with open(debug, 'a') as file:
                print(f"Error: {e}. Retrying in 5 seconds...", file=file)
            time.sleep(5)  

def send_stop(status):
    with open(debug, 'a') as file:
        print("Send stop: ", status, file=file)
    print("Send stop: ", status)  
    url = esp_url + "/stop"
    data = {"stop": status}        
    send_to(url, data)

def timer_out():
    print(datetime.now().strftime("[%d/%b/%Y %H:%M:%S]") + " " + "timer out")
    with open(debug, 'a') as file:
        print(datetime.now().strftime("[%d/%b/%Y %H:%M:%S]") + " " + "timer out", file=file)
    fail_message()        
        
def send_reboot():
    with open(debug, 'a') as file:
        print("Send reboot", file=file)
    print("Send reboot")  
    url = esp_url + "/reboot"
    data = {"reboot": True}
    send_to(url, data)

def send_await():
    with open(debug, 'a') as file:
        print("Send await", file=file)
    print("Send await")  
    url = esp_url + "/await"
    data = {"await": True}
    send_to(url, data)      

def reset_timer():
    print("reset")
    timer.reset()

def error_message(message):
    print(message)
    send_reboot()
    fail_message()

def await_message():
    send_await()
    print("await")
    p = vlc.MediaPlayer("await.mp3")
    p.play()

def fail_message():
    print(datetime.now().strftime("[%d/%b/%Y %H:%M:%S]") + " " + "fail") 
    with open(debug, 'a') as file:
        print(datetime.now().strftime("[%d/%b/%Y %H:%M:%S]") + " " + "fail", file=file)
    for _ in range(5):
        p = vlc.MediaPlayer("fail.mp3")
        p.play()   
        time.sleep(1)
        duration = p.get_length() / 1000
        time.sleep(duration)
        time.sleep(1)

@app.route('/message', methods=['POST'])
def handle_message():
    data = request.json
    message = data.get("message", "empty")
    print(datetime.now().strftime("[%d/%b/%Y %H:%M:%S]") + " " + message)
    with open(debug, 'a') as file:
        print(datetime.now().strftime("[%d/%b/%Y %H:%M:%S]") + " " + message, file=file)
    if (message in ("empty", "error", "failed to find rthdr <-> master sock overlap", "failed to find slave", "failed to find overlapped socket, how even?", "failed to fake pktopts 1", "failed to fake pktopts 2", "failed jit test = 0x")) or ("failed jit test = 0x" in message):
        error_message(message)
    elif message == "start":
        send_stop(False)   
    elif message == "stop":
        send_stop(True);  
    elif message == "await":
        await_message()
    elif message == "reboot":
        send_reboot()    
    elif message == "fail":
        fail_message()     
    elif ("ESP:" not in message) or (message in "ESP: launch"):
        reset_timer()    
    else:
        print("esp message")
    return jsonify({"status": "got"}), 200
    
if __name__ == '__main__':
    timer = ResettableTimer(180, timer_out)
    timer.start()
    app.run(host=host_url, port=5000)
    