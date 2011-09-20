puts @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
load ./libtclhwloc0.1.so
hwloc create topo0
catch {
    puts "a cpubind:[topo0 cpubind get]"
    puts "b cpubind -pid 1 :[topo0 cpubind get -pid 1]"
    puts "c cpubind -pid 1 -thread :[topo0 cpubind get -pid 1 -thread]"
    puts "d Setting cpubind to 0"
    topo0 cpubind set "0"
    puts "e Setting cpubind to 1"
    topo0 cpubind set -pid [pid] "1"
    topo0 cpubind set -pid [pid] -process "1"
    puts "f cpubind:[topo0 cpubind get]"

    puts "g Overloading a single core for 20secs"
    set running 1
    after 20000 [list set running 0]
    while {$running} { incr a ; update }
} msg
puts $msg
topo0 destroy

# -*- tcl -*-
puts @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
load ./libtclhwloc0.1.so
if [catch {
hwloc create topo0
puts "membind:[topo0 membind get]"
puts "membind -pid 1 :[topo0 membind get -pid 1]"
puts "membind -pid 1 -type thread :[topo0 membind get -pid 1 -type thread]"
puts "Setting membind to 0"
topo0 membind set "0" default
puts "Setting membind to 1"
topo0 membind set -pid [pid] "1" default
topo0 membind set -pid [pid] -type process "1" default
puts "membind:[topo0 membind get]"

} out err] { puts $err }
topo0 destroy
