#include "l32511/m32511.h"
QVector<double> m32511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
