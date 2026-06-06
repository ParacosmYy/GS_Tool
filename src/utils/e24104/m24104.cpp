#include "e24104/m24104.h"
QVector<double> m24104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
