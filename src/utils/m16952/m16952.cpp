#include "m16952/m16952.h"
QVector<double> m16952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
