#include "h35487/m35487.h"
QVector<double> m35487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
