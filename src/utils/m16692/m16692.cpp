#include "m16692/m16692.h"
QVector<double> m16692::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
