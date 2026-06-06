#include "f25805/m25805.h"
QVector<double> m25805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
