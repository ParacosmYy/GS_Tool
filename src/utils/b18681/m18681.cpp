#include "b18681/m18681.h"
QVector<double> m18681::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
