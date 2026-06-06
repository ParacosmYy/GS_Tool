#include "l37611/m37611.h"
QVector<double> m37611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
