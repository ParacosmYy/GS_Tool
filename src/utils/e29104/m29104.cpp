#include "e29104/m29104.h"
QVector<double> m29104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
