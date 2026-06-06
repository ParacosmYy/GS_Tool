#include "i35928/m35928.h"
QVector<double> m35928::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
