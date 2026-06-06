#include "l18051/m18051.h"
QVector<double> m18051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
