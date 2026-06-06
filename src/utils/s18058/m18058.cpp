#include "s18058/m18058.h"
QVector<double> m18058::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
