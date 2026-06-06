#include "m32432/m32432.h"
QVector<double> m32432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
