bulkredis
=========

A bulk loader that goes from a Python dictionary into Redis' mass
insert protocol.

This C Python module will use C for handling the walking of a Python
data structure, writing to a temporary file, and forking a process to
use redis-cli's '--pipe' option to bulk insert into Redis. This
process is faster than walking the structure in Python using the Redis
module's pipeline methods.

_Insert professional perf comparison here_

Note that this is a fairly tailored solution. However, it is rather
simple and straight forward to work with. Given a dictionary of
dictionaries, the top level key is the key in Redis. The next level
key is the member to be stored in Redis. The final element in the
child dictionary becomes an optional score or field parameter to
Redis; depending on type. Floats are interpreted as scores in a
ZSET. Strings are interpreted as fields in a HASH.

Potentially, more parameters and data structure types may be
supported. Also, note a C function may be written with variable
argument support to handle Redis' mass insert generically. Here it is
in Python:

```
def bulkredis(file, *args)
    block = "*%s\r\n" % len(args)
    for i in args:
        block += "$%s\r\n%s\r\n" % (len(str(i)), str(i))
    file.write(block)
```

Finally, the first two parameters to the module's execute method are
'hostname' and 'port' respectively. The third parameter is the
temporary file name. The last reference to the dictionary is passed
in. The return value is a copy of the return value from the libc
system() function, unless an error is encountered, in which case, None
is returned.

For example:

```
import bulkredis
import sys

test_dict = {
    'key1': {
        'member1': None,
        'member2': None
    },
    'key2': {
        'member3': 1.1,
        'member4': 2.0
    },
    'key3': {
        'member5': 'field1',
        'member6': 'field2'
    }
}

ret = bulkredis.execute('localhost', '6379', '/tmp/file.txt', test_dict)
sys.exit(ret)
```

N.B.: Key names are specifically converted via Python's str()
builtin. A dict like {1: {100: None}} is fine. Also, None is used
instead of a Boolean, since None is 16 bytes, where as True is
24 bytes. That adds up slowly.
