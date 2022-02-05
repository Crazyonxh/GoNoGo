'''
Date: 2021-01-12 17:11:22
LastEditTime: 2021-02-22 22:42:24
LastEditors: Lekang
Organization: BBNC Lab@THU
Describtions: Python host for the go-nogo ethology system.
'''


from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QLineEdit, QPushButton, QComboBox
import sys
import json
import numpy as np
import pyqtgraph as pg
import serial 
import serial.tools.list_ports
from PyQt5.QtCore import QTimer
import time
import os

pg.setConfigOption('background', 'ECECEC')

class Ser:
    def __init__(self):
        self.buffer = ''
        self.ser = None

    def list_ports(self):
        self.ports = list(serial.tools.list_ports.comports())
        self.ports = [port.device for port in self.ports]
        return self.ports
    
    def build_connection(self, port = None, baud = 9600):
        if not port:
            try: 
                port = self.list_ports()[0]
            except:
                print('ERROR: NO PORT FOUND')
                sys.exit()
            if not port: return -1 # error: serial not connected
        self.ser = serial.Serial(port ,baud, timeout=5)
        return self.ser
    
    def send(self,s):
        if not self.ser: return 
        self.ser.write(str.encode(s+'\r\n',encoding = 'gbk'))
    
    def send_config(self,dic):
        print('Sending Config...')
        string = json.dumps(dic)
        print(string)
        self.send('c' + string)
    
    def recv(self):
        if self.ser.in_waiting:
            data = self.ser.read(self.ser.in_waiting).decode('gbk')
            data = self.buffer + data
            lines = data.split('\r\n')
            try:
                self.buffer = lines[-1] # 最后分割的一定是不完整的，暂时储存到buffer里
            except:
                self.buffer = ''
            lines = lines[:-1]
            return lines 
        else:
            return None

class win(QWidget):
    # ============================= SETUP =================================== #
    # 1.初始化所有的界面，完成控件与函数的连接
    # 2.初始化统计数据，读入参数，建立socket连接，绘图初始化
    def __init__(self):
        super().__init__()
        
        # PlotWidgets
        self.pw1 = pg.PlotWidget(self)  
        self.pw2 = pg.PlotWidget(self)
        styles = {'color':'DDDDDD', 'font-size':'20px'}
        self.pw1.setLabel('bottom', 'time (second)', **styles)
        self.pw2.setLabel('bottom', 'trial', **styles)
        
        # Serial
        self.ser = Ser()
        self.port_combo = QComboBox(self)
        self.port_combo.addItems(self.ser.list_ports())
        self.port = self.port_combo.currentText()
        self.port_combo.currentIndexChanged.connect(self.on_combo)
        
        self.connect_button = QPushButton('CON', self)
        self.connect_button.clicked.connect(self.connect)

        # Recording
        self.recording_button =  QPushButton('REC',self) 
        self.recording_button.clicked.connect(self.change_recording_state)
        self.is_recording = False
        self.trial_record = []

        # Params
        self.config_label = QLabel('config path:', self)
        self.config_line = QComboBox(self)
        self.config_button =  QPushButton('PARA',self)
        self.config_line.addItems(os.listdir('config/'))
        self.config_button.clicked.connect(self.send_params)
        
        # Terminal
        self.tm = QLabel('Waiting for connection', self)
        self.control = QLineEdit('', self)
        self.control.editingFinished.connect(self.send_cmd)

        self.setUI()
        self.paint_init()
        

    def setUI(self):
        r = 1
        self.pw1.addLegend((700*r,20*r))
        self.resize(900*r,600*r)

        self.pw1.resize(860*r,200*r)
        self.pw1.move(20*r,20*r)
        self.pw2.resize(860*r,200*r)
        self.pw2.move(20*r,240*r)

        self.port_combo.resize(200*r,40*r)
        self.connect_button.resize(80*r,40*r)
        self.recording_button.resize(80*r,40*r)
        self.config_label.resize(150*r,40*r)
        self.config_line.resize(80*r,30*r)
        self.config_button.resize(200*r,40*r)
        self.tm.resize(400*r, 40*r)
        self.control.resize(400*r, 30*r)

        self.port_combo.move(20*r,460*r)
        self.connect_button.move(240*r, 460*r)
        self.recording_button.move(340*r, 460*r)
        self.config_label.move(460*r,460*r)
        self.config_line.move(570*r,465*r) 
        self.config_button.move(680*r,460*r) 
        self.tm.move(20*r,520*r)
        self.control.move(460*r,520*r)

    def connect(self):
        self.ser.build_connection(self.port)

        t = time.localtime(time.time())
        self.file_name = f'data/{t.tm_mon}_{t.tm_mday}_{t.tm_hour}_{t.tm_min}'
        self.f = open(self.file_name+'.txt', 'w')
        self.log('data path: ' + self.file_name)

        self.T = []
        self.Y_lick = []
        self.Y_sound = []
        self.Y_window = []
        self.Y_actor = []

        self.start_listenning()

    def load_params(self):
        try:
            with open('config/' + self.config_line.currentText(),'r') as f:
                self.dic = json.load(f)
        except:
            self.log('Fail to load config file!')

    def paint_init(self):
        self.curves = []
        self.curves.append(self.pw1.plot(x = [], y=[],pen=pg.mkPen('B03A2E', width = 2),name='sound',fillLevel = 0, brush=(176,58,46,30)))
        self.curves.append(self.pw1.plot(x = [], y=[],pen=pg.mkPen('F4D03F', width = 2),name='window',fillLevel = 0, brush=(244,208,63,30)))
        self.curves.append(self.pw1.plot(x = [], y=[],pen=pg.mkPen('3498DB', width = 3),name='lick'))
        self.curves.append(self.pw1.plot(x = [], y=[],pen=pg.mkPen('28B463', width = 3),name='actor'))
        self.T = []
        self.Y_lick = []
        self.Y_actor = []

        self.trial_record = []
        self.record_curve = self.pw2.plot(y=self.trial_record,pen=pg.mkPen(width =1),symbol = '+', symbolBrush=('r'),symbolSize = 10)
        # self.plot_pw2()
        self.pw2.setYRange(min=0,max=3)

    def on_combo(self):
        self.port = self.port_combo.currentText()
    
    # ================ 如果有新数据输入，则更新界面 & 储存数据 ======================= #
    def recv_and_update(self):
        lines = self.ser.recv()
        if not lines: return 
        for line in lines:
            if not line:
                print(lines)
                continue
            if line[0] == 'e': self.on_trial_end(line[1:]) 
            elif line[0] == 's': self.on_trial_start(line[1:])
            elif line[0] == 't': self.on_data_recv(line[1:])
            elif line[0] == 'm': self.log(line[1:])
        # 更新pw1的数据
        self.curves[0].setData(self.T, self.Y_sound)
        self.curves[1].setData(self.T, self.Y_window)
        self.curves[2].setData(self.T, self.Y_lick)
        self.curves[3].setData(self.T, self.Y_actor)

    def log(self,line):
        self.tm.setText(line)

    def on_trial_start(self,line):
        # 获取设定的时间, 绘制时间和声音窗口图
        start_time, trial_end_time, flag = [int(c) for c in line.split(',')]
        
        # 准备下一周期的绘图
        self.pw1.setXRange(min=start_time/1000, max=trial_end_time/1000)
        self.T = []
        self.Y_lick = []
        self.Y_sound = []
        self.Y_window = []
        self.Y_actor = []
        
    def on_data_recv(self,line):
        # 更新lick和actor的数据
        lick, sound, actor, window, time = [int(c) for c in line.split(',')]
        self.T.append(time/1000)
        self.Y_lick.append(lick)
        self.Y_actor.append(actor)
        self.Y_window.append(window)
        self.Y_sound.append(sound)
    
    def on_trial_end(self,line):
        # 更新上一trial的统计数据
        res, trial_left = [int(c) for c in line.split(',')]
        if self.is_recording:
            self.trial_record.append(res)
            counts = [np.sum(np.array(self.trial_record)==i) for i in range(4)]
            print(counts)
            total = len(self.trial_record)
            log_info = f'{counts}/{total}\n'
            self.log(log_info)
            self.f.write(log_info)
            self.plot_pw2()
        
    def plot_pw2(self):
        X = [i for i in range(len(self.trial_record))]
        self.record_curve.setData(X,self.trial_record)
        m = max(0,len(self.trial_record)-30)
        self.pw2.setXRange(min=m, max = m+30)

    def start_listenning(self):
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.recv_and_update)
        self.timer.start(20)

    def change_recording_state(self):
        self.is_recording = not self.is_recording
        if self.is_recording: self.tm.setText('recording started.')
        else: self.tm.setText('recording stopped.')

    def send_cmd(self):
        text = self.control.text()
        print(len(text))
        self.ser.send(text)

    def send_params(self):
        self.load_params()
        self.ser.send_config(self.dic)

if __name__=='__main__':
    app=QApplication(sys.argv)
    w=win()
    w.show()
    sys.exit(app.exec_())