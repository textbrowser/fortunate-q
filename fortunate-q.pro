unix {
purge.commands = find . -name '*~' -exec rm {} \\;
}

CONFIG		+= qt release warn_on
DEFINES         +=
LANGUAGE	 = C++
QMAKE_CLEAN	+= fortunate-q
QT		+= core network

contains(QMAKE_HOST.arch, armv7l) {
QMAKE_CXXFLAGS_RELEASE += -march=armv7
}

contains(QMAKE_HOST.arch, ppc) {
QMAKE_CXXFLAGS_RELEASE += -mcpu=powerpc -mtune=powerpc
}

QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_RELEASE -= -O2

android {
QMAKE_CXXFLAGS_RELEASE += -Wall \
                          -Wcast-qual \
                          -Wenum-compare \
                          -Wextra \
                          -Wfloat-equal \
                          -Wformat=2 \
                          -Woverloaded-virtual \
                          -Wpointer-arith \
                          -Wstack-protector \
                          -Wstrict-overflow=1 \
                          -Wundef \
                          -fPIC \
                          -fstack-protector-all \
                          -funroll-loops \
                          -fwrapv \
                          -std=c++17
} else:freebsd-* {
QMAKE_CXXFLAGS_RELEASE += -Wall \
                          -Wcast-align \
                          -Wcast-qual \
                          -Werror \
                          -Wextra \
                          -Wformat=2 \
                          -Woverloaded-virtual \
                          -Wpointer-arith \
                          -Wstack-protector \
                          -Wstrict-overflow=5 \
                          -Wundef \
                          -fPIE \
                          -fstack-protector-all \
                          -funroll-loops \
                          -fwrapv \
                          -std=c++17
} else:macx {
QMAKE_CXXFLAGS_RELEASE += -Wall \
                          -Wcast-align \
                          -Wcast-qual \
                          -Wenum-compare \
                          -Wextra \
                          -Wformat=2 \
                          -Woverloaded-virtual \
                          -Wpointer-arith \
                          -Wstack-protector \
                          -Wstrict-overflow=5 \
                          -Wundef \
                          -fPIE \
                          -fstack-protector-all \
                          -funroll-loops \
                          -fwrapv \
                          -std=c++17
} else:win32 {
QMAKE_CXXFLAGS_RELEASE += -Wall \
                          -Wcast-align \
                          -Wcast-qual \
                          -Wdouble-promotion \
                          -Wenum-compare \
                          -Wextra \
                          -Wformat=2 \
                          -Wl,-z,relro \
                          -Wno-class-memaccess \
                          -Wno-deprecated-copy \
                          -Woverloaded-virtual \
                          -Wpointer-arith \
                          -Wstack-protector \
                          -Wstrict-overflow=1 \
                          -Wundef \
                          -fPIE \
                          -funroll-loops \
                          -fwrapv \
                          -pie \
                          -std=c++17
} else {
QMAKE_CXXFLAGS_RELEASE += -Wall \
                          -Wcast-qual \
                          -Wdangling-reference \
                          -Wdouble-promotion \
                          -Wenum-compare \
                          -Wextra \
                          -Wfloat-equal \
                          -Wformat=2 \
                          -Wl,-z,relro \
                          -Wlogical-op \
                          -Wno-class-memaccess \
                          -Wno-deprecated-copy \
                          -Wold-style-cast \
                          -Woverloaded-virtual \
                          -Wpointer-arith \
                          -Wstack-protector \
                          -Wstrict-overflow=1 \
                          -Wundef \
                          -fPIE \
                          -fstack-protector-all \
                          -funroll-loops \
                          -fwrapv \
                          -pie \
                          -std=c++17
}

greaterThan(QT_MAJOR_VERSION, 5) {
QMAKE_CXXFLAGS_RELEASE += -std=c++17
QMAKE_CXXFLAGS_RELEASE -= -std=c++11
}

QMAKE_DISTCLEAN     += -r .qmake* \
                       -r Temporary \
                       -r html \
                       -r latex

unix {
QMAKE_EXTRA_TARGETS += purge
}

HEADERS	       += fortunate-q.h fortunate-q-sample-class.h
INCLUDEPATH    += .
MOC_DIR         = Temporary/moc
OBJECTS_DIR     = Temporary/obj
PROJECTNAME     = fortunate-q
QMAKE_STRIP	= echo
RCC_DIR         = Temporary/rcc
SOURCES	       += fortunate-q-test.cc
TARGET		= fortunate-q
TEMPLATE	= app
TRANSLATIONS    =
