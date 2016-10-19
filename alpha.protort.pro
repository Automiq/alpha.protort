TEMPLATE = subdirs

SUBDIRS += \
    protocol \
    link \
    node \
    terminal \
    designer \
    parser

link.depends = protocol
node.depends = link
terminal.depends = link
designer.depends = link
