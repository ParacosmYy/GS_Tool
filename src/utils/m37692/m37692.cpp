#include "m37692/m37692.h"
QVector<double> m37692::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
