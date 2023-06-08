# RT-Thread building script for bridge

import os
from building import *
Import('rtconfig')

cwd = GetCurrentDir()
group = []

metal_src = Split('''
libmetal/lib/device.c
libmetal/lib/dma.c
libmetal/lib/init.c
libmetal/lib/io.c
libmetal/lib/irq.c
libmetal/lib/log.c
libmetal/lib/shmem.c
libmetal/lib/softirq.c
libmetal/lib/version.c
''')

openamp_src = Split('''
open-amp/lib/proxy/rpmsg_retarget.c
open-amp/lib/remoteproc/elf_loader.c
open-amp/lib/remoteproc/remoteproc.c
open-amp/lib/remoteproc/remoteproc_virtio.c
open-amp/lib/remoteproc/rsc_table_parser.c
open-amp/lib/rpmsg/rpmsg.c
open-amp/lib/rpmsg/rpmsg_virtio.c
open-amp/lib/service/rpmsg/rpc/rpmsg_rpc_client.c
open-amp/lib/service/rpmsg/rpc/rpmsg_rpc_server.c
open-amp/lib/version.c
open-amp/lib/virtio/virtio.c
open-amp/lib/virtio/virtqueue.c
''')

rtt_port = Split('''
rtt_port/device.c
rtt_port/init.c
rtt_port/io.c
rtt_port/irq.c
rtt_port/time.c
''')

rtt_openamp_demo = []
if GetDepend('AMP_DEMO_ENABLE') == True:
    rtt_openamp_demo += Glob("rtt_openamp_demo/openamp_*.c")

if GetDepend('AMP_DEMO_ECHO') == False:
    SrcRemove(rtt_openamp_demo, ["openamp_app_echo.c"])

if GetDepend('AMP_DEMO_MATRIX') == False:
    SrcRemove(rtt_openamp_demo, ["openamp_app_matrix_multiply.c"])

if GetDepend('AMP_DEMO_PTY') == False:
    SrcRemove(rtt_openamp_demo, ["openamp_app_vuart.c"])

src = openamp_src + metal_src + rtt_port + rtt_openamp_demo
CPPPATH = [
    cwd + '/open-amp/lib/include/',
    cwd + '/rtt_port',
    cwd + '/rtt_openamp_demo/',
]
CPPDEFINES = ['METAL_INTERNAL', 'OPENAMP_VERSION_MAJOR=1', 'OPENAMP_VERSION_MINOR=0', 'OPENAMP_VERSION_PATCH=0', 'OPENAMP_VERSION=\\"1.0.0\\"']

group = DefineGroup('OpenAMP', src, depend = [''], CPPPATH = CPPPATH, CPPDEFINES = CPPDEFINES)

Return('group')
