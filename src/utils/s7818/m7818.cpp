#include "s7818/m7818.h"
QVector<double> m7818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
