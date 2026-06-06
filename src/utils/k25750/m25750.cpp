#include "k25750/m25750.h"
QVector<double> m25750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
