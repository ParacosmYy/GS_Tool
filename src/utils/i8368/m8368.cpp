#include "i8368/m8368.h"
QVector<double> m8368::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
