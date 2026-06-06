#include "k25450/m25450.h"
QVector<double> m25450::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
