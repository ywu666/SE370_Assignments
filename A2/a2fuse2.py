# Yujia Wu - UPI: 827481772 / ywu660
# !/usr/bin/env python

from __future__ import print_function, absolute_import, division

import os
import sys

from collections import defaultdict
from errno import ENOENT
from time import time
from stat import S_IFDIR, S_IFREG
from fuse import FUSE, LoggingMixIn, Operations, FuseOSError
from memory import Memory


class A2Fuse2(LoggingMixIn, Operations):

    def __init__(self, root1, root2):
        self.root1 = root1
        self.root2 = root2
        self.files = {}
        self.data = defaultdict(bytes)
        self.fd = 0
        now = time()
        self.files['/'] = dict(st_mode=(S_IFDIR | 0o755), st_ctime=now,
                               st_mtime=now, st_atime=now, st_nlink=2)

    # Helpers
    # =======
    def _full_path(self, partial):
        if partial.startswith("/"):
            partial = partial[1:]

        paths = []
        path1 = os.path.join(self.root1, partial)
        path2 = os.path.join(self.root2, partial)
        paths.append(path1)
        paths.append(path2)

        return paths

    def create(self, path, mode, fi=None):
        self.files[path] = dict(st_mode=(S_IFREG | mode), st_nlink=1, st_uid=os.getuid(), st_gid=os.getuid(),
                                st_size=0, st_ctime=time(), st_mtime=time(),
                                st_atime=time())

        self.fd += 1
        return self.fd

    def getattr(self, path, fh=None):
        if path not in self.files:
            full_paths = self._full_path(path)
            for full_path in full_paths:
                if os.path.isfile(full_path):
                    st = os.lstat(full_path)
                    return dict((key, getattr(st, key)) for key in ('st_atime', 'st_ctime',
                                                                    'st_gid', 'st_mode', 'st_mtime', 'st_nlink',
                                                                    'st_size', 'st_uid'))
        else:
            if path not in self.files:
                raise FuseOSError(ENOENT)
            return self.files[path]

    def open(self, path, flags):
        if path not in self.files:
            full_path = self._full_path(path)
            for x in full_path:
                return os.open(x, flags)
        else:
            self.fd += 1
            return self.fd

    def read(self, path, length, offset, fh):
        if path not in self.files:
            os.lseek(fh, offset, os.SEEK_SET)
            return os.read(fh, length)
        else:
            return self.data[path][offset:offset + length]

    def readdir(self, path, fh):
        full_paths = self._full_path(path)
        dirents = ['.', '..']
        for full_path in full_paths:
            if os.path.isdir(full_path):
                dirents.extend(os.listdir(full_path))
        dirents.extend([x[1:] for x in self.files if x != '/'])
        for r in dirents:
            yield r

    def unlink(self, path):
        if path not in self.files:
            full_paths = self._full_path(path)
            for full_path in full_paths:
                return os.unlink(full_path)
        else:
            self.files.pop(path)

    def write(self, path, buf, offset, fh):
        if path not in self.files:
            os.lseek(fh, offset, os.SEEK_SET)
            return os.write(fh, buf)
        else:
            self.data[path] = self.data[path][:offset] + buf
            self.files[path]['st_size'] = len(self.data[path])
            return len(buf)

    # File methods
    # ============

    def flush(self, path, fh):
        if path not in self.files:
            return os.fsync(fh)

    # def release(self, path, fh):
    #     if path not in self.files:
    #         return os.close(fh)
    #     else:
    #         return 0
    #
    # def fsync(self, path, fdatasync, fh):
    #     if path not in self.files:
    #         return self.flush(path, fh)
    #     else:
    #         return 0


# override
def main(mountpoint, root1, root2):
    FUSE(A2Fuse2(root1, root2), mountpoint, nothreads=True, foreground=True)


if __name__ == '__main__':
    # logging.basicConfig(level=logging.DEBUG)
    main(sys.argv[3], sys.argv[2], sys.argv[1])
