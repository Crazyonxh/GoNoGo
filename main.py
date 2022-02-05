# !user/bin/python
# coding=utf-8
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
        print('===================================!!!!!')
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
    # ================ SETUP ======================= #
    # 1.初始化所有的界面，完成控件与函数的连接
    # 2.初始化统计数据，读入参数，建立socket连接，绘图初始化
    def __init__(self):
        super().__init__()
        self.ser = Ser()
        r = 2
        # 创建主窗口
        self.resize(900*r,600*r)
        # 绘图框件创建
        self.pw1 = pg.PlotWidget(self)  
        self.pw2 = pg.PlotWidget(self)
        styles = {'color':'DDDDDD', 'font-size':'20px'}
        self.pw1.setLabel('bottom', 'time (second)', **styles)
        self.pw2.setLabel('bottom', 'trial #', **styles)
        self.pw1.addLegend((700*r,20*r))

        self.pw1.resize(860*r,200*r)
        self.pw1.move(20*r,20*r)
        self.pw2.resize(860*r,200*r)
        self.pw2.move(20*r,240*r)
        
        # trial和参数设置行
        self.port_combo = QComboBox(self)
        self.tn_label = QLabel('config path:', self)
        self.tn_line = QComboBox(self)
        self.th_label = QLabel('success/finished: 0/0',self)
        self.th_button = QPushButton('send',self)
        
        self.port_combo.resize(200*r,40*r)
        self.tn_label.resize(150*r,40*r)
        self.tn_line.resize(80*r,30*r)
        self.th_label.resize(200*r,40*r)
        self.th_button.resize(200*r,40*r)

        self.port_combo.move(20*r,460*r)
        self.tn_label.move(240*r,460*r)
        self.tn_line.move(350*r,465*r)
        self.th_label.move(460*r,460*r)
        self.th_button.move(680*r,460*r)

        self.tn_line.addItems(os.listdir('config/'))

        # 控制行
        self.s_button = QPushButton('start/pause',self)
        self.p_button = QPushButton('start/stop recording',self) 
        self.w_button = QPushButton('water',self) 
        self.o_button = QPushButton('water on/off',self)
        
        self.s_button.resize(200*r,40*r)
        self.p_button.resize(200*r,40*r)
        self.w_button.resize(200*r,40*r)
        self.o_button.resize(200*r,40*r)
        
        self.s_button.move(20*r,520*r)
        self.p_button.move(240*r,520*r)
        self.w_button.move(460*r,520*r)
        self.o_button.move(680*r,520*r)

        # 连接控制函数
        self.s_button.clicked.connect(self.change_running_state)
        self.p_button.clicked.connect(self.change_recording_state)
        self.w_button.clicked.connect(self.send_w)
        self.o_button.clicked.connect(self.send_o)
        self.th_button.clicked.connect(self.send)
        self.port_combo.addItems(self.ser.list_ports())
        self.port_combo.currentIndexChanged.connect(self.on_combo)

        # 统计数据
        self.is_running = True
        self.is_recording = False

        self.finished_trials = 0
        self.succ_trials = 0
        self.succ_trials_0 = 0

        self.load_params()
        self.paint_init()
        self.ser.build_connection()
        self.start_listenning()

        t = time.localtime(time.time())
        self.file_name = f'data/{t.tm_mon}_{t.tm_mday}_{t.tm_hour}_{t.tm_min}'
        self.f = open(self.file_name+'.txt', 'w')
        self.f_d = open(self.file_name+'_source.txt','w')
        self.on_message_recv('data path: ' + self.file_name)
        self.r_dic = {}

    def load_params(self):
        try:
            with open('config/' + self.tn_line.currentText(),'r') as f:
                self.dic = json.load(f)
        except:
            self.th_line.setText('Fail to load config file!')

    def paint_init(self):
        self.curves = []
        self.curves.append(self.pw1.plot(x = [], y=[],pen=pg.mkPen('B03A2E', width = 2),name='sound',fillLevel = 0, brush=(176,58,46,30)))
        self.curves.append(self.pw1.plot(x = [], y=[],pen=pg.mkPen('F4D03F', width = 2),name='window',fillLevel = 0, brush=(244,208,63,30)))
        self.curves.append(self.pw1.plot(x = [], y=[],pen=pg.mkPen('3498DB', width = 3),name='lick'))
        self.curves.append(self.pw1.plot(x = [], y=[],pen=pg.mkPen('28B463', width = 3),name='water'))
        self.T = []
        self.Y_lick = []
        self.Y_water = []

        self.trial_record = [0]
        self.record_curve = self.pw2.plot(y=self.trial_record,pen=pg.mkPen(width =1),symbol = '+', symbolBrush=('r'),symbolSize = 10)
        self.plot_pw2()
        self.pw2.setYRange(min=0,max=1)

    def on_combo(self):
        port = self.port_combo.currentText()
        self.ser.build_connection(port)
    
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
            elif line[0] == 'm': self.on_message_recv(line[1:])
        # 更新pw1的数据
        self.curves[2].setData(self.T, self.Y_lick)
        self.curves[3].setData(self.T, self.Y_water)

    def on_message_recv(self,line):
        self.th_label.setText(line)

    def on_data_recv(self,line):
        # 更新lick和water的数据
        lick, water, time = [int(c) for c in line.split(',')]
        self.T.append(time/1000)
        self.Y_lick.append(lick)
        self.Y_water.append(water)
    
    def on_trial_end(self,line):
        # 更新上一trial的统计数据
        succ0, succ, trial_left = [int(c) for c in line.split(',')]
        if self.is_recording:
            if succ0: self.succ_trials_0 += 1
            if succ: self.succ_trials += 1
            self.finished_trials += 1
            self.th_label.setText(f'succ0/succ/finished/left: \n {self.succ_trials_0}/{self.succ_trials}/{self.finished_trials}/{trial_left}')
            self.trial_record.append(succ)
            self.plot_pw2()
            
            record = f'{self.succ_trials_0},{self.succ_trials}/{self.finished_trials}\n'
            self.r_dic['T'] = self.T
            self.r_dic['Y_lick'] = self.Y_lick
            self.r_dic['Y_water'] = self.Y_water
            data = json.dumps(self.r_dic)
            nu = self.f.write(record); print(nu) # bug: 为什么不能储存？
            self.f_d.write(record + data + '\n')
    
    def plot_pw2(self):

        X = [i for i in range(self.finished_trials+1)]
        self.record_curve.setData(X,self.trial_record)
        m = max(0,self.finished_trials-30)
        self.pw2.setXRange(min=m, max = m+30)

    def on_trial_start(self,line):
        # 获取设定的时间, 绘制时间和声音窗口图
        start_time, sound_time, window_time, trial_end_time = [int(c) for c in line.split(',')]
        self.trial_end_time = trial_end_time
        trial_duration = trial_end_time - start_time
        sample_points = 1000
        T = np.array([start_time + i / sample_points * trial_duration for i in range(sample_points)])
        Y_window = np.zeros(sample_points)
        Y_window[np.where(np.logical_and(window_time < T, T  < window_time + self.dic['timeWindowDuration']))] = 1
        Y_sound = np.zeros(sample_points)
        Y_sound[np.where(np.logical_and(sound_time < T, T < sound_time + self.dic['soundDuration']))] = 1
        T = T / 1000
        self.curves[0].setData(T,Y_sound)
        self.curves[1].setData(T,Y_window)
        
        self.r_dic['T_window'] = list(T)
        self.r_dic['Y_window'] = list(Y_window)
        self.r_dic['Y_sound'] = list(Y_sound)
        # 清空数据
        self.T = []
        self.Y_lick = []
        self.Y_water = []

    def start_listenning(self):
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.recv_and_update)
        self.timer.start(20)

    # ==================按下按键时，发送指令到Arduino================== #
    def change_running_state(self):
        self.is_running = not self.is_running
        if self.is_running: self.ser.send('s')
        else: self.ser.send('p')

    def change_recording_state(self):
        self.is_recording = not self.is_recording
        if self.is_recording: self.th_label.setText('recording started.')
        else: self.th_label.setText('recording stopped.')

    def send_w(self):
        self.ser.send('w')
    
    def send_o(self):
        self.ser.send('o')
    
    def send(self):
        self.load_params()
        self.ser.send_config(self.dic)

if __name__=='__main__':
    app=QApplication(sys.argv)
    w=win()
    w.show()
    sys.exit(app.exec_())