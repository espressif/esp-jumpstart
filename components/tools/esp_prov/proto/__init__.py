import imp
import os

idf_path = os.environ['IDF_PATH']

constants_pb2 = imp.load_source("constants_pb2", idf_path + "/components/protocomm/python/constants_pb2.py")
sec0_pb2      = imp.load_source("sec0_pb2",      idf_path + "/components/protocomm/python/sec0_pb2.py")
sec1_pb2      = imp.load_source("sec1_pb2",      idf_path + "/components/protocomm/python/sec1_pb2.py")
session_pb2   = imp.load_source("session_pb2",   idf_path + "/components/protocomm/python/session_pb2.py")

wifi_constants_pb2 = imp.load_source("wifi_constants_pb2", idf_path + "/components/wifi_provisioning/python/wifi_constants_pb2.py")
wifi_config_pb2    = imp.load_source("wifi_config_pb2",    idf_path + "/components/wifi_provisioning/python/wifi_config_pb2.py")

custom_config_pb2  = imp.load_source("custom_config_pb2",  idf_path + "/examples/provisioning/custom_config/components/custom_provisioning/python/custom_config_pb2.py")
