#include "m33232/m33232.h"
QVector<double> m33232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
