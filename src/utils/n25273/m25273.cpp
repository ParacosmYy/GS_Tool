#include "n25273/m25273.h"
QVector<double> m25273::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
