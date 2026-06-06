#include "m18692/m18692.h"
QVector<double> m18692::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
