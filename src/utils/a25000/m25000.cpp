#include "a25000/m25000.h"
QVector<double> m25000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
