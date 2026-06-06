#include "l25011/m25011.h"
QVector<double> m25011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
