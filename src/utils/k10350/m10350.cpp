#include "k10350/m10350.h"
QVector<double> m10350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
