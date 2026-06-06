#include "e24544/m24544.h"
QVector<double> m24544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
