#include "e25064/m25064.h"
QVector<double> m25064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
