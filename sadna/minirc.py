import socket

# https://github.com/epw/pyirc/blob/master/pyirc.py
# http://sourceforge.net/projects/pyirclib/files/pyirclib/0.4.3/

class IRCClient(object):
    def __init__(self, host, nick, ident, realname, port = 6667):
        self.host = host
        self.nick = nick
        self.ident = ident
        self.realname = realname
        self.port = port
        self.sock = socket.socket()
        self.sock.connect((host, port))
        self._login()
    
    def _login(self):
        self.sock.send("NICK %s\r\n" % (self.nick,))
        self.sock.send("USER %s %s bla :%s\r\n" % (self.ident, self.host, self.realname))        
    
    def close(self):
        self.sock.shutdown(socket.SHUT_RDWR)
        self.sock.close()
    
    def fileno(self):
        return self.sock.fileno()
    
    def join(self, channel):
        self.sock.send("JOIN :%s\r\n" % (chan,))
    
    def action(self):
        pass
    
    def say(self, message):
        pass

HOST="irc.freenode.net"
PORT=6667