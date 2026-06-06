#include "b18401/m18401.h"
QVector<double> m18401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
