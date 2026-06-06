#include "b29601/m29601.h"
QVector<double> m29601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
