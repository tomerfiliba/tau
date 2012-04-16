import socket
import select
import heapq
import time


class MinHeap(object):
    def __init__(self, seq):
        self._heap = list(seq)
        heapq.heapify(self._heap)
    def push(self, item):
        heapq.heappush(self._heap, item)
    def pop(self, item):
        return heapq.heappop(self._heap)
    def peek(self):
        return self._heap[0]



class SelectReactor(object):
    def __init__(self):
        self._rlist = set()
        self._wlist = set()
        self._callbacks = []
        self._scheduled = MinHeap()
        self._running = False
    def call(self, func, *args, **kwargs):
        self._callbacks.append(partial(func, *args, **kwargs))
    def schedule(self, within, func, *args, **kwargs):
        dfr = Deferred()
        def scheduled():
            try:
                res = func(*args, **kwargs)
            except Exception as ex:
                dfr.throw(ex)
            else:
                dfr.set(res)
        self._scheduled.push((time.time() + within, scheduled))
        return dfr
    
    def register_read(self, file):
        self._rlist.add(file)
    def register_write(self, file):
        self._wlist.add(file)
    def unregister_read(self, file):
        self._rlist.discard(file)
    def unregister_write(self, file):
        self._wlist.discard(file)
    
    def main(self):
        if self._running():
            raise ValueError("already running")
        self._running = True
        while self._running:
            now = time.time()
            self._handle_io(now)
            self._handle_callbacks(now)
        self._handle_callbacks(now)
    
    def stop(self):
        def _stop(self):
            self._running = False
        self.call(_stop, self)
    
    def _handle_io(self, now):
        t, _ = self._scheduled.peek()
        timeout = min(max(t - now, 0), 1)
        rlist, wlist, _ = select.select(self._rlist, self._wlist, (), timeout)
        for fd in rlist:
            self.call.append(r.on_read)
        for fd in wlist:
            self.call.append(w.on_write)
    
    def _handle_callbacks(self, now):
        while self._scheduled.peek()[0] <= now:
            _, callinfo = self._scheduled.pop()
            self._callbacks.append(callinfo)
        callbacks = self._callbacks
        self._callbacks = []
        for func, args, kwargs in callbacks:
            func(args, kwargs)
    
    def run(self, func, *args, **kwargs):
        self.call(func, *args, **kwargs)
        self.main()


reactor = SelectReactor()

class Socket(object):
    IO_CHUNK = 16384
    def __init__(self, sock):
        self._sock = sock
        self._sock.setblocking(False)
        self._readbuf = b""
        self._writebuf = b""
        self._read_closed = False
        reactor.register_read(self)
    def close(self):
        reactor.unregister_read(self)
        reactor.unregister_write(self)
        self._sock.close()
    def fileno(self):
        return self._sock.fileno()
    def read(self, count):
        if self._readbuf:
            chunk = self._readbuf[:count]
            return chunk
        elif self._read_closed:
            return b""
        else:
            return Deferred()
    def write(self, data):
        reactor.register_write(self)
        self._writebuf += data
    def on_read(self):
        data = self._sock.recv(self.IO_CHUNK)
        if not data:
            self._read_closed = True
            reactor.unregister_read(self)
        else:
            self._readbuf += data
    def on_write(self):
        if not self._writebuf:
            reactor.unregister_write(self)
        chunk = self._writebuf[:self.IO_CHUNK]
        count = self._sock.write(chunk)
        self._writebuf = self._writebuf[count:]

def sleep(timeout):
    return reactor.schedule(timeout, lambda: None)

def accept():
    pass

def listen():
    pass

def main():
    pass

reactor.run(main)


















