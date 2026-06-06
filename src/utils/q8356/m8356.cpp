#include "q8356/m8356.h"
QVector<double> m8356::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
