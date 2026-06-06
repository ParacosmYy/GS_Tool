#include "b18121/m18121.h"
QVector<double> m18121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
