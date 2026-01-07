# ebpf-profiling

Small example using eBPF and USDTs to do profiling on multiple processes.
Our design is a ringbuffer that contains events.
Every event has a timestamp in it.
Our `loader.cpp` attaches to each USDT in `/proc/`, and when it hits one of the probes an event fires. 
We pass a "sequence number" to the probe to track a "buffer" passing through a system consisting of multiple processes.
The processes are "source", "processor" and "sink".
We then print it, outlook looks as follows:

```bash
(dev) niels@personal-laptop:~/repositories/ebpf-profiling$ ./build.sh
[   0] Found probe beg_global                          in 'source'
[   1] Found probe beg_awe                             in 'processor'
[   2] Found probe end_awe                             in 'processor'
[   3] Found probe beg_writing                         in 'processor'
[   4] Found probe end_writing                         in 'processor'
[   5] Found probe end_global                          in 'sink'

Trace started. Ctrl-C to stop.

[  11832166705679] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 1
[  11832166741817] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 1
[  11832192230862] [PID:  32764] processor    buffer_tracker:end_writing     | Seq: 0
[  11832216822256] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 2
[  11832216852303] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 2
[  11832242348652] [PID:  32764] processor    buffer_tracker:beg_awe         | Seq: 1
[  11832257421818] [PID:  32764] processor    buffer_tracker:end_awe         | Seq: 1
[  11832266929095] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 3
[  11832266951718] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 3
[  11832267518546] [PID:  32764] processor    buffer_tracker:beg_writing     | Seq: 1
[  11832317821785] [PID:  32764] processor    buffer_tracker:end_writing     | Seq: 1
[  11832317822136] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 4
[  11832317822506] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 4
[  11832368124283] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 5
[  11832368127679] [PID:  32764] processor    buffer_tracker:beg_awe         | Seq: 2
[  11832368129162] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 5
[  11832383232835] [PID:  32764] processor    buffer_tracker:end_awe         | Seq: 2
[  11832393329713] [PID:  32764] processor    buffer_tracker:beg_writing     | Seq: 2
[  11832418484509] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 6
[  11832418485220] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 6
[  11832443497327] [PID:  32764] processor    buffer_tracker:end_writing     | Seq: 2
[  11832468632204] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 7
[  11832468632695] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 7
[  11832493625926] [PID:  32764] processor    buffer_tracker:beg_awe         | Seq: 3
[  11832508723378] [PID:  32764] processor    buffer_tracker:end_awe         | Seq: 3
[  11832518708786] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 8
[  11832518741127] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 8
[  11832518823963] [PID:  32764] processor    buffer_tracker:beg_writing     | Seq: 3
[  11832569699310] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 9
[  11832569699761] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 9
[  11832569744095] [PID:  32764] processor    buffer_tracker:end_writing     | Seq: 3
[  11832620327372] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 10
[  11832620327552] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 10
[  11832620373298] [PID:  32764] processor    buffer_tracker:beg_awe         | Seq: 4
[  11832635473215] [PID:  32764] processor    buffer_tracker:end_awe         | Seq: 4
[  11832645570814] [PID:  32764] processor    buffer_tracker:beg_writing     | Seq: 4
[  11832670677018] [PID:  32765] sink         buffer_tracker:end_global      | Seq: 11
[  11832670677519] [PID:  32763] source       buffer_tracker:beg_global      | Seq: 11
[  11832695767192] [PID:  32764] processor    buffer_tracker:end_writing     | Seq: 4
```
