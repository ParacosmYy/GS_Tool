#include "m28692/m28692.h"
QVector<double> m28692::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
