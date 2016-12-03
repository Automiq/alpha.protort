TEMPLATE = subdirs

SUBDIRS += \
    protocol \
    link \
    protolink \
    node \
    terminal \
    designer \
    parser \
    components

link.depends = protocol
protolink.depends = protocol
node.depends = link protolink
terminal.depends = link protolink
designer.depends = link protolink
