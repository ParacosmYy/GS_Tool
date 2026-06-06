#include "k29730/m29730.h"
QVector<double> m29730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
