#include "m24952/m24952.h"
QVector<double> m24952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
