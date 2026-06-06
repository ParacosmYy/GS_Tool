#include "l21011/m21011.h"
QVector<double> m21011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
