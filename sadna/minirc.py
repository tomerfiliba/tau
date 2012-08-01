import socket

# https://github.com/epw/pyirc/blob/master/pyirc.py
# http://sourceforge.net/projects/pyirclib/files/pyirclib/0.4.3/

class IRCClient(object):
    def __init__(self, host, nick, ident = "Anonymous", realname = "Anonymous", port = 6667):
        self.host = host
        self.nick = nick
        self.ident = ident
        self.realname = realname
        self.port = port
        self.sock = socket.socket()
        self.sock.connect((host, port))
        self.sockfile = self.sock.makefile("r+")
        self._login()
    
    def _login(self):
        self.sockfile.write("NICK %s\r\n" % (self.nick,))
        self.sockfile.write("USER %s %s bla :%s\r\n" % (self.ident, self.host, self.realname))        
    
    def close(self):
        self.sock.shutdown(socket.SHUT_RDWR)
        self.sock.close()
    
    def fileno(self):
        return self.sock.fileno()
    
    def join(self, channel):
        self.sockfile.write("JOIN #%s\r\n" % (channel,))
    def leave(self, channel):
        self.sockfile.write("PART #%s\r\n" % (channel,))
        
    def action(self, nick, msg):
        self.s.send ("PRIVMSG %s :ACTION %s\r\n" % (self.nick, msg))
            
    def privmsg(self, message):
        self.sock.send("PRIVMSG %s :%s\r\n" % (self.nick, message))
    
    def list_channels(self):
        self.sockfile.write("PART #%s\r\n")
    
    def quit(self):
        self.sockfile.write("QUIT\r\n")


if __name__ == "__main__":
    c = IRCClient("irc.freenode.net", "sheker8")
    c.join("python")






