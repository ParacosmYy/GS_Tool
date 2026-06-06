#include "m25092/m25092.h"
QVector<double> m25092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
