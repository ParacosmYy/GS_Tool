#include "m24692/m24692.h"
QVector<double> m24692::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
