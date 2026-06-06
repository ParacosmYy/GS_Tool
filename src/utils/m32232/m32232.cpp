#include "m32232/m32232.h"
QVector<double> m32232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
