#include "b10601/m10601.h"
QVector<double> m10601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
