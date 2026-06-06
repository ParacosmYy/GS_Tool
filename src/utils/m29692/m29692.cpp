#include "m29692/m29692.h"
QVector<double> m29692::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
