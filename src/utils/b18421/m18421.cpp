#include "b18421/m18421.h"
QVector<double> m18421::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
