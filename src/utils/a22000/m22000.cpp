#include "a22000/m22000.h"
QVector<double> m22000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
