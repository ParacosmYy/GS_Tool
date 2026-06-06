#include "j24009/m24009.h"
QVector<double> m24009::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
