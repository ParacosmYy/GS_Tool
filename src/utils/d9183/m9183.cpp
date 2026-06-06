#include "d9183/m9183.h"
QVector<double> m9183::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
