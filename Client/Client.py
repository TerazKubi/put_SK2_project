import socket
import threading
from tkinter import *
from tkinter import simpledialog, messagebox
from functools import partial



class Client:
    def __init__(self, port):
        self.port = port

        self.window = Tk()
        self.window.title('Tic Tac Toe extended')
        self.window.geometry("500x500+100+100")

        self.socket = None

        self.buttons = []
        self.frames = []

        self.entry = None

        self.running = False

        self.receive_thread = None

        self.marks = {'1': 'x', '2': 'o'}
        self.turns = {'1': 'Yours', '0': 'Opponent'}
        self.fontColor = {'1': "green", '0': "red"}
        self.win_msg = {"WIN": "YOU WON!!!", "LOST": "YOU LOST :c"}

        self.myMark = '0'
        self.nextMoveIndex = 10
        self.swins = [0, 0, 0, 0, 0, 0, 0, 0, 0]
        self.move_to_show = []
        self.game_finished = False

        self.start_gui_init()



    def start_gui_init(self):
        l = Label(self.window, text="Enter server IP address:").pack()
        self.entry = Entry(self.window)
        self.entry.pack()
        address_button = Button(self.window, text="Connect", command=self.connect_to_server)
        address_button.pack()
        self.window.protocol("WM_DELETE_WINDOW", self.stop)
        self.window.mainloop()

    def connect_to_server(self):
        address = self.entry.get()
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.socket.connect((address, self.port))
        except:
            print("CONNECTION ERROR")
            return

        for child in self.window.winfo_children():
            child.destroy()

        self.gui_init()

        self.running = True
        self.init_thread()

    def init_thread(self):
        self.receive_thread = threading.Thread(target=self.receive)
        self.receive_thread.start()

    def gui_init(self):
        self.nav_frame = Frame(master=self.window, width=500, height=30, highlightbackground="black", highlightthicknes=2)
        self.nav_frame.place(x=0, y=0)

        self.game_frame = Frame(master=self.window, width=500, height=470)
        self.game_frame.place(x=0, y=30)

        self.nav_label = Label(master=self.nav_frame, text="")
        self.mark_label = Label(master=self.nav_frame, text="Mark:")
        self.turn_label = Label(master=self.nav_frame, text="Turn:")
        self.nav_label.place(x=1, y=1)
        self.mark_label.place(x=400, y=1)
        self.turn_label.place(x=270, y=1)

        self.buttons_init()



    def buttons_init(self):
        frame_pos_x = 25
        frame_pos_y = 10

        FRAME_WIDTH = 150
        FRAME_HEIGHT = 150

        BTN_HEIGHT = 40
        BTN_WIDTH = 40

        for i in range(1, 10):
            frame = Frame(master=self.game_frame,
                          highlightbackground="black", highlightthicknes=1)

            frame.place(x=frame_pos_x, y=frame_pos_y, width=FRAME_WIDTH, height=FRAME_HEIGHT)

            frame_pos_x += FRAME_WIDTH + 2

            if i % 3 == 0:
                frame_pos_x = 25
                frame_pos_y += FRAME_HEIGHT + 2

            #add buttons to frame
            buttons = []

            btn_pos_x = 11
            btn_pos_y = 11

            for j in range(1, 10):

                btn = Button(master=frame, text="", fg='black',
                             command=partial(self.button_onclick, i-1, j-1))
                btn.place(x=btn_pos_x, y=btn_pos_y, width=BTN_WIDTH, height=BTN_HEIGHT)

                btn_pos_x += BTN_WIDTH + 2

                if j % 3 == 0:
                    btn_pos_x = 11
                    btn_pos_y += BTN_HEIGHT + 2

                buttons.append(btn)

            self.buttons.append(buttons)
            self.frames.append(frame)

    def button_onclick(self, i, j):
        if self.game_finished:
            return
        msg_to_send = "M" + str(i) + str(j) + self.myMark
        self.socket.send(msg_to_send.encode(encoding="utf-8"))

    def receive(self):
        while self.running:
            try:
                msg = self.socket.recv(4096)
                msg = msg.decode(encoding="utf-8")
                msg = msg.rstrip("\x00")
                self.handle_message(msg)
            except:
                print("closed")

    def stop(self):
        self.running = False
        self.socket.close()
        self.window.destroy()
        exit(0)

    def handle_message(self, msg):
        msg = msg.split(" ")
        print("MSG: ", msg)

        if msg[0] == '':
            self.show_message("SERVER ERROR")
            self.running = False
            self.socket.close()
            print("SERVER ERROR")
            self.game_finished = True
            return

        if msg[0] == "MOVE":
            self.move_to_show = [int(msg[1]), int(msg[2]), self.marks[msg[3]]]

        elif msg[0] == "TURN":
            self.turn_label["text"] = "Turn: " + self.turns[msg[1]]
            self.turn_label.config(fg=self.fontColor[msg[1]])

            if len(self.move_to_show) == 0:
                return

            if self.swins[self.move_to_show[0]] == 0:
                self.buttons[self.move_to_show[0]][self.move_to_show[1]]["text"] = self.move_to_show[2]

            if int(msg[1]) == 0 and self.nextMoveIndex != 10:
                self.change_frame_color_inactive(self.nextMoveIndex)

            self.nextMoveIndex = self.move_to_show[1]
            print("\n")

            if int(msg[1]) == 1 and self.swins[self.nextMoveIndex] == 0:
                self.change_frame_color_active(self.nextMoveIndex)

        elif msg[0] == "MARK":
            self.myMark = msg[1]
            self.mark_label["text"] = "Mark: " + self.marks[msg[1]]

        elif msg[0] == "SWIN":
            index = int(msg[1])
            mark = msg[2]
            self.swins[index] = 1
            self.change_frame_swin(index, mark)
            print("SWINS: ", self.swins)
        elif msg[0] == "SDRAW":
            index = int(msg[1])
            self.swins[index] = 3
            self.change_frame_sdraw(index)
        elif msg[0] == "BWIN":
            self.show_message(self.win_msg[msg[1]])
            self.turn_label["text"] = "Turn: "
            self.turn_label.config(fg="black")
            self.game_finished = True
            if msg[1] == "WIN":
                self.change_frame_color_inactive(self.nextMoveIndex)
                self.nav_label.config(fg="green")
            if msg[1] == "LOST":
                self.nav_label.config(fg="red")
        elif msg[0] == "BDRAW":
            self.show_message("DRAW!!!")
        elif msg[0] == "SERVERMSG":
            self.handle_server_msg(msg[1])
        else:
            self.show_message(msg)

    def show_message(self, msg):
        self.nav_label["text"] = msg

    def change_frame_color_active(self, index):
        self.frames[index].config(highlightbackground="red", highlightthicknes=3)

    def change_frame_color_inactive(self, index):
        self.frames[index].config(highlightbackground="black", highlightthicknes=1)

    def change_frame_swin(self, index, mark):
        for child in self.frames[index].winfo_children():
            child.destroy()
        label = Label(master=self.frames[index], text=self.marks[mark])
        label.place(x=75, y=75)

    def change_frame_sdraw(self, index):
        for child in self.frames[index].winfo_children():
            child.destroy()

    def handle_server_msg(self, msg):
        if msg == "opponent_left":
            self.show_message("Opponent left")
            self.refresh_gui()
        elif msg == "not_turn":
            self.show_message("Wait for your turn")
        elif msg == "no_opponent":
            self.show_message("Wait for opponent.")
        elif msg == "invalid_move1":
            self.show_message("There is already placed mark.")
        elif msg == "invalid_move2":
            self.show_message("Make a move in red square.")

    def refresh_gui(self):
        self.myMark = '0'
        self.nextMoveIndex = 10
        self.swins = [0, 0, 0, 0, 0, 0, 0, 0, 0]
        self.move_to_show = []
        self.game_finished = False

        self.turn_label["text"] = "Turn: "
        self.turn_label.config(fg="black")

        for frame in self.game_frame.winfo_children():
            frame.destroy()

        self.buttons = []
        self.frames = []

        self.buttons_init()