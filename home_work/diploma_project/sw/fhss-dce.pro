CONFIG+=ordered

TEMPLATE = subdirs

SUBDIRS += \
    comps/fhss-types \
    comps/qt-widget-mess \
    comps/fhss-protocol \  
    comps/qt-widget-dce \
    comps/qt-widget-exch-IQ \
    comps/eml \
    comps/fhss-logger \
    comps/prsgen \
    comps/qt-dce-panel \
    comps/qt-dce-plot \
    comps/qt-simple-plot \
    comps/qt-customplot \
    apps/fhss-dce

# !!!!! It is nessesary to use RELEASE regim for app working, otherwise there are udp packs lost.
