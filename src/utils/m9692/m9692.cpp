#include "m9692/m9692.h"
QVector<double> m9692::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
