#include "m35692/m35692.h"
QVector<double> m35692::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
