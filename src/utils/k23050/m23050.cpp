#include "k23050/m23050.h"
QVector<double> m23050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
