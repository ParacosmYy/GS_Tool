#include "b25601/m25601.h"
QVector<double> m25601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
