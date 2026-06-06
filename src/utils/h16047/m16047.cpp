#include "h16047/m16047.h"
QVector<double> m16047::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
