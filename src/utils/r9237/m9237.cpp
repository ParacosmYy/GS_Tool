#include "r9237/m9237.h"
QVector<double> m9237::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
