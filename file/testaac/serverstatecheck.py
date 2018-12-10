#!/usr/bin/python
# -*- coding: UTF-8 -*-

import MySQLdb
import urllib2
import json
import time,datetime
import traceback
import logging

TRANSCODE_DB_NAME      = "testtranscode_4"
TRANSCODE_DB_IP      = "192.168.0.85"
TRANSCODE_DB_USERNAME     = "root"
TRANSCODE_DB_PASSWORD     = "1234"
MAX_IDLE_DURATION = 600 #秒
CHECK_SLEEP_DURATION = 1 #秒

HTTP_REQUEST_SERVER_IP = "192.168.3.86"
HTTP_REQUEST_SERVER_PORT = 8080
HTTP_REQUEST_CLUSTER_ID = 123

class ServerStateCheckShutdown:
	logger = logging.getLogger("ServerStateCheckShutdown")
	logger.setLevel(level = logging.INFO)
	handler = logging.FileHandler("ServerStateCheckShutdown.log")
	handler.setLevel(logging.INFO)
	formatter = logging.Formatter('%(asctime)s-%(filename)s-%(lineno)s-%(funcName)s:%(message)s')
	handler.setFormatter(formatter)
	logger.addHandler(handler)

	def __init__( self):
		print "__init__"
		# 打开数据库连接
		self.db = MySQLdb.connect(TRANSCODE_DB_IP, TRANSCODE_DB_USERNAME, TRANSCODE_DB_PASSWORD, TRANSCODE_DB_NAME, charset='utf8' )
		# 使用cursor()方法获取操作游标 
		self.cursor = self.db.cursor()
		
	def __del__(self):
		print "__del__"
   		self.cursor.close()
   		self.db.close()

   	def log(self,msg):
		self.logger.warning(msg)

	def query_unprocessed_tasks(self):
		try:
			querytasksql = "SELECT * from onair_trans_task where status = 0"
			self.cursor.execute(querytasksql)
			results = self.cursor.fetchall()
			return len(results)
		except:
			self.log("Error: query_unprocessed_tasks:%s" %(traceback.format_exc()))
			return -1

	def http_request(self, serverip):
		try:
			url = "http://%s:%d/cc2.0/api/deleteInstanceBatch/hw/?clusters_id=%d&num=1&intranetIp=%s" \
					%(HTTP_REQUEST_SERVER_IP,HTTP_REQUEST_SERVER_PORT,HTTP_REQUEST_CLUSTER_ID,serverip)
			req = urllib2.Request(url)
			self.log(req)
			res_data = urllib2.urlopen(req)
			res = res_data.read()
			self.log(res)
			jsondata = json.loads(res)
			if jsondata.has_key("code") and jsondata["code"] == true:
				self.delete_server_from_nodes(serverip)
			return 0
		except:
			self.log("Error: http_request:%s" %(traceback.format_exc()))
			return -1

	def set_server_hidden(self,serverip):
		try:
			querytasksql = "update onair_node_state set hidden=0 where address = \"%s\"" %(serverip)
			self.cursor.execute(querytasksql)
			self.db.commit()
			return 0
		except:
			self.log("Error: set_server_hidden:%s" %(traceback.format_exc()))
			return -1

	def delete_server_from_nodes(self,serverip):
		try:
			querytasksql = "delete from onair_node_state where address = \"%s\"" %(serverip)
			self.cursor.execute(querytasksql)
			self.db.commit()
			return 0
		except:
			self.log("Error: delete_server_from_nodes:%s" %(traceback.format_exc()))
			return -1

	def query_one_idle_server(self):
		try:
			queryjobsql = "select finish_time,exec_server,status \
							from onair_exec_job \
							where exec_server not in \
							(select DISTINCT exec_server from onair_exec_job where `status` = 1 or `status` = 3 ) \
							ORDER BY finish_time DESC \
							LIMIT 1"
			# 执行SQL语句
			self.cursor.execute(queryjobsql)
			# 获取所有记录列表
			results = self.cursor.fetchall()
			if len(results) <= 0:
				return ""

			row = results[0]
			finish_time = row[0]
			exec_server = row[1]
			status = row[2]
			self.log("exec_server=%s status=%d finish_time=" % (exec_server, status))
			self.log(finish_time)

			if exec_server == "0.0.0.0":
				return ""

			timediff = datetime.datetime.now() - finish_time
			self.log("timediff = %u" %(timediff.total_seconds()))
			if timediff.total_seconds() <= MAX_IDLE_DURATION:
				return ""

			return exec_server
		except:
			self.log("Error: query_one_idle_server: %s" %(traceback.format_exc()))
			return ""

def main():

	while(True):
		time.sleep(CHECK_SLEEP_DURATION)

		sscs = ServerStateCheckShutdown()

		if sscs.query_unprocessed_tasks() > 0:
			del sscs
			continue

		idle_server_ip = sscs.query_one_idle_server()

		#http request
		if idle_server_ip == "":
			del sscs
			continue

		sscs.set_server_hidden(idle_server_ip)
		sscs.http_request(idle_server_ip)
		
		del sscs

if __name__ == "__main__":

		main()
