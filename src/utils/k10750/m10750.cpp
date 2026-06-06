#include "k10750/m10750.h"
QVector<double> m10750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
