#include "g29166/m29166.h"
QVector<double> m29166::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
