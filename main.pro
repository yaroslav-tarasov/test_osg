TEMPLATE = subdirs
DEV_ROOT = ./..

CONFIG += ordered

SUBDIRS =  \ 
          tests\osg_widget     \
          tests\visual         \
          visapp
          
visapp.depends = osg_widget
osg_widget.depends   = visual
