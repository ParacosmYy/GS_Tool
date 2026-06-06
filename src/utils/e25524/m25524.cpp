#include "e25524/m25524.h"
QVector<double> m25524::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
