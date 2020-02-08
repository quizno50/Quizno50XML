CXXFLAGS=-Wall -O2 -fPIC
LDFLAGS=-shared
DESTDIR=/usr/local
INCLUDE_DIR=include
LIB_DIR=lib
INSTALL=env install
Q50XML_VER_MAJ=0
Q50XML_VER_MIN=1
Q50XML_VER_REV=0
Q50XML_LIB_NAME=libQuizno50XML.so.${Q50XML_VER_MAJ}.${Q50XML_VER_MIN}.${Q50XML_VER_REV}
Q50XML_OBJS=FileString.o Quizno50XML.o
Q50XML_HEADERS=Quizno50XML.hpp

${Q50XML_LIB_NAME}: ${Q50XML_OBJS} ${Q50XML_HEADERS}
	$(CXX) -o $@ ${Q50XML_OBJS} $(LDFLAGS)

install: ${Q50XML_LIB_NAME} ${Q50XML_HEADERS}
	${INSTALL} -m 644 ${Q50XML_LIB_NAME} ${DESTDIR}/${LIB_DIR}
	${INSTALL} -m 644 ${Q50XML_HEADERS} ${DESTDIR}/${INCLUDE_DIR}

.PHONY: install