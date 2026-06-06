#include "b18521/m18521.h"
QVector<double> m18521::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
