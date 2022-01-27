#!/usr/bin/env python3
from __future__ import print_function
import sys
from ROOT import TChain
if sys.version_info.major == 2:
	from past.builtins import xrange
if sys.version_info.major == 3:
	xrange = range
import larcv


if len(sys.argv) < 2:
   print('Usage: python', sys.argv[0],
         'CONFIG_FILE [LARCV_FILE1 LARCV_FILE2 ...]')
   sys.exit(1)

# Instantiate, configure, and ensure it's kWrite mode
proc = larcv.ProcessDriver('ProcessDriver')
proc.configure(sys.argv[1])
if not proc.io().io_mode() == 1:
	print('Supera needs to be run with IOManager IO mode kWrite! exiting...')
	sys.exit(1)

# Set up input
ch = TChain('EDepSimEvents')
if len(sys.argv) > 2:
	for argv in sys.argv[2:]:
		if not argv.endswith('.root'):
			continue
		print('Adding input:', argv)
		ch.AddFile(argv)
print("Chain has", ch.GetEntries(), "entries")
#print("????", proc.batch_start_entry())
# sometimes num_entry overflows instead of being negative.  not sure why?
#event_range = (proc.batch_start_entry(), proc.batch_start_entry() + proc.batch_num_entry()) \
#	if proc.batch_num_entry() > 0 and proc.batch_num_entry() < 1e18 \
#	else (0, ch.GetEntries())
event_range = (0, ch.GetEntries())
print('Processing', event_range[1] - event_range[0], 'events')
sys.stdout.flush()

# Initialize and retrieve a list of processes that belongs to SuperaBase inherited module classes
proc.initialize()
supera_procs = []

ptrs = []


#module = proc.process_ptr(0)
#module = proc.process_ptr(1)
#module = proc.process_ptr(0)
#print("made it ")
#print(proc.process_names())
for name in proc.process_names():
	pid = proc.process_id(name)
	ptrs.append(proc.process_ptr(pid))
	#print(pid, type(pid))
	#print("hi kazu22")
	#module=
	#print("hi222")
	#mod2=proc.process_ptr(0)
	#print("hi322")
	#mod3=proc.process_ptr(0)
	#print("hi422")
	#print("name:",name)
	#print(dir(module))

	if getattr(ptrs[-1], 'is')('Supera'):
		print('Running a Supera module:', name)
		supera_procs.append(pid)

print("All the Supera Procs ", supera_procs)
# Event loop


for entry in xrange(*event_range):
	print("considering event:", entry)
	sys.stdout.flush()
	bytes = ch.GetEntry(entry)
	if bytes < 1:
		break
	ev = ch.Event

	proc.set_id(ev.RunId, 0, ev.EventId)
	# set event pointers
	for pid in supera_procs:
		#module2 = proc.process_ptr(pid) #pybind scope issue without 2
		print("pid", pid)
		print(type(ev))
		print(ev.__dict__.keys())
		ptrs[pid].SetEvent(ev)
		print("got here8")

	proc.process_entry()
proc.finalize()
