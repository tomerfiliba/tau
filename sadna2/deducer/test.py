import socket
import guify


@guify.expose_class
class ChatClient(object):
    history = guify.attribute(guify.ListOf(str))
    
    def __init__(self, host, port, username, password):
        self.sock = socket.socket()
        self.sock.connect((host, port))
        self.sock.send("%s\n%s\n" % (username, password))
        if self.sock.recv(100) != "OK":
            raise ValueError("server denied")
        self.history = []

    @guify.expose_method(line = str)
    def send(self, line):
        pass
    
    


