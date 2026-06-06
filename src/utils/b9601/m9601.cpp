#include "b9601/m9601.h"
QVector<double> m9601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
