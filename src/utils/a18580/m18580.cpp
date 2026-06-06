#include "a18580/m18580.h"
QVector<double> m18580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
