#include "m25292/m25292.h"
QVector<double> m25292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
