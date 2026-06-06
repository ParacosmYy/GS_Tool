#include "r7857/m7857.h"
QVector<double> m7857::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
