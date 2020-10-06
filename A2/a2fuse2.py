# Yujia Wu - UPI: 827481772 / ywu660
# !/usr/bin/env python

from __future__ import print_function, absolute_import, division

import logging

import os
import sys

from fuse import FUSE, LoggingMixIn
from passthrough import Passthrough
from memory import Memory


class A2Fuse2(LoggingMixIn, Passthrough):

    def __init__(self, root1, root2):
        self.root1 = root1
        self.root2 = root2
        self.memory = Memory()

    # Helpers
    # =======
    def _full_path(self, partials):
        paths = []
        for partial in partials:
            if partial.startswith("/"):
                partial = partial[1:]
            path = os.path.join(self.root, partial)
            paths.append(path)
        return paths

    def getattr(self, path, fh=None):
        if path in self.memory.files:
            Passthrough.getattr(self, path, fh)
        else:
            self.memory.getattr(self, path, fh)

    def readdir(self, path, fh):
        full_paths = self._full_path(self, path)
        dirents = ['.', '..']
        for full_path in full_paths:
            if full_path == "/":
                if os.path.isdir(full_path):
                    dirents.extend(os.listdir(full_path))
                dirents.extend([x[1:] for x in self.files if x != '/'])
            elif full_path not in self.memory.files:
                if os.path.isdir(full_path):
                    dirents.extend(os.listdir(full_path))
            else:
                dirents.extend([x[1:] for x in self.files if x != '/'])
            for r in dirents:
                yield r

    def open(self, path, flags):
        if path not in self.memory.files:
            Passthrough.open(self, path, flags)
        else:
            self.memory.open(self, path, flags)

    def create(self, path, mode):
        # create in memory
        return self.memory.create(self, path, mode)

    def unlink(self, path):
        if path not in self.memory.files:
            Passthrough.unlink(self, path)
        else:
            self.memory.unlink(self, path)

    def write(self, path, buf, offset, fh):
        if path not in self.memory.files:
            Passthrough.write(self, path, buf, offset, fh)
        else:
            self.memory.write(self, path, buf, offset, fh)

    def read(self, path, length, offset, fh):
        if path not in self.memory.files:
            Passthrough.read(self, path, length, offset, fh)
        else:
            self.memory.read(self, path, length, offset, fh)

    def flush(self, path, fh):
        if path not in self.memory.files:
            return Passthrough.flush(self, path, fh)


# override
def main(mountpoint, root1, root2):
    FUSE(A2Fuse2(root1, root2), mountpoint, nothreads=True, foreground=True)


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    main(sys.argv[3], sys.argv[2], sys.argv[1])
