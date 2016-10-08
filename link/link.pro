TEMPLATE = subdirs

SUBDIRS += \
    tests \
    examples \
    src

tests.depends = src
examples.depends = src
