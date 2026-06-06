#include "k18750/m18750.h"
QVector<double> m18750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
