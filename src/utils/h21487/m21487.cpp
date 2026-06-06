#include "h21487/m21487.h"
QVector<double> m21487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
