# -*- coding: utf-8 -*-
import logging
from functools import wraps
from logging.handlers import MemoryHandler

try:
    from logging import NullHandler
except ImportError:
    class NullHandler(logging.Handler):
        def emit(self, record):
            pass

try:
    unicode
    _unicode = True
except NameError:
    _unicode = False

_pongo_logs = None


def init_ab_logs(pongo_logs):
    global _pongo_logs
    _pongo_logs = pongo_logs


# noinspection PyBroadException
class PongoLogHandler(logging.Handler):
    def __init__(self, pongo_logs):
        logging.Handler.__init__(self)
        self.pongo_logs = pongo_logs

    def flush(self):
        self.acquire()
        try:
            self.pongo_logs.flush()
            pass
        finally:
            self.release()

    def emit(self, record):
        try:
            msg = self.format(record)
            stream = self.pongo_logs
            fs = "%s\n"
            if not _unicode:
                stream.write(fs % msg)
            else:
                try:
                    if (isinstance(msg, unicode) and
                            getattr(stream, 'encoding', None)):
                        ufs = u'%s\n'
                        try:
                            stream.write(ufs % msg)
                        except UnicodeEncodeError:
                            stream.write((ufs % msg).encode(stream.encoding))
                    else:
                        stream.write(fs % msg)
                except UnicodeError:
                    stream.write(fs % msg.encode("UTF-8"))
            self.flush()
        except (KeyboardInterrupt, SystemExit):
            raise
        except:
            self.handleError(record)


def log_if_errors(name, target_handler=None, flush_level=None, capacity=None):
    fmt = '%(asctime)-15s %(message)s'
    if target_handler is None:
        target_handler = PongoLogHandler(_pongo_logs)
        target_handler.setFormatter(logging.Formatter(fmt))
    if flush_level is None:
        flush_level = logging.ERROR
    if capacity is None:
        capacity = 100
    handler = MemoryHandler(capacity, flushLevel=flush_level, target=target_handler)
    handler.setFormatter(logging.Formatter(fmt))
    logger = logging.getLogger(name)

    def decorator(fn):
        @wraps(fn)
        def wrapper(*args, **kwargs):
            logger.addHandler(handler)
            try:
                return fn(log=logger, *args, **kwargs)
            except Exception:
                logger.exception('call failed')
                raise
            finally:
                super(MemoryHandler, handler).flush()
                logger.removeHandler(handler)

        return wrapper

    return decorator


class LoggingContext(object):
    def __init__(self, logger, level=None, handler=None, close=True):
        self.logger = logger
        self.level = level
        self.handler = handler
        self.close = close

    def __enter__(self):
        if self.level is not None:
            self.old_level = self.logger.level
            self.logger.setLevel(self.level)
        if self.handler:
            self.logger.addHandler(self.handler)

    def __exit__(self, et, ev, tb):
        if self.level is not None:
            self.logger.setLevel(self.old_level)
        if self.handler:
            self.logger.removeHandler(self.handler)
        if self.handler and self.close:
            self.handler.close()
            # implicit return of None => don't swallow exceptions