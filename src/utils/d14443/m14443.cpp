#include "d14443/m14443.h"
QVector<double> m14443::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
