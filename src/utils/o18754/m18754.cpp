#include "o18754/m18754.h"
QVector<double> m18754::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
