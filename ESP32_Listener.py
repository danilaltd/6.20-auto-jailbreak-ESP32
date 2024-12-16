from flask import Flask, request, jsonify
from flask_cors import CORS 
from datetime import datetime
import vlc
import requests
app = Flask(__name__)
CORS(app) 

import threading

esp_url = "http://1.1.2.1:80/"
host_url = '1.1.2.2'


@app.route('/message', methods=['POST'])
def handle_message():
    data = request.json
    message = data.get("message", "empty")
    print(datetime.now().strftime("[%d/%b/%Y %H:%M:%S]") + " " + message)
    with open('debug\log.txt', 'a') as file:
        print(datetime.now().strftime("[%d/%b/%Y %H:%M:%S]") + " " + message, file=file)
    return jsonify({"status": "got"}), 200
    
if __name__ == '__main__':
    app.run(host=host_url, port=5000)
    