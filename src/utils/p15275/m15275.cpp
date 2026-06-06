#include "p15275/m15275.h"
QVector<double> m15275::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
