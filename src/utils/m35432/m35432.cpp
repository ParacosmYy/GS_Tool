#include "m35432/m35432.h"
QVector<double> m35432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
