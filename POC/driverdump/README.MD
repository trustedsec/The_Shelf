# DriverDump

This capability was written up as a POC of abusing the old process explorer driver to grab a privledged handle to lsass and then dump it.

There are a few blogs on this topic, I don't remember the original source I followed to get the idea. The actual dumping process comes from https://github.com/fortra/nanodump/blob/main/source/nanodump.c.  That was used rather then the built in dbghlp api because dbghlp will attempt to duplicate the handle you provide it, which will fail in this case.

Apache 2 License to line up with using nanodump as a major contributer to the codebase.

This is being retired as it was more a thought experiment to see if it was possible and to play with using the driver ioctl. It was not used in our own operations.


