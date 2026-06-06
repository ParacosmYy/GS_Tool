#include "n32053/m32053.h"
QVector<double> m32053::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
