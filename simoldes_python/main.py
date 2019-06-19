#!/usr/bin/env python
from server import Server
import json
import time
import azure

azure.CONNECTION_STRING = "HostName=Simoldes4-IOTHUB.azure-devices.net;DeviceId=RPI_UMINHO;SharedAccessKey=4WxzRU+9H7kpQCOYvDdEWf54JVkwwduH5G1qtJZIL90="

def success(request):
    try:
        json_req = json.loads(request)
        Azure = azure.AzureIoT_Microservice()
        Azure.send_message(request)
        i = 0
        while Azure.result == "EMPTY" and i <10:
            time.sleep(1)
            i += 1
        if str(Azure.result) == "OK":
            return "Successfully sent to the Cloud"
        else:
            return "Could not send to the Cloud"
    except Exception as e:
        print str(e)
        return "JSON Syntax Error"


def error(request):
    print "error"
    return ""

if __name__ == "__main__":
    s = Server(port=5050, hostname="127.0.0.1")
    s.start(success, error)
