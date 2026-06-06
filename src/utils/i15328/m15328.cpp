#include "i15328/m15328.h"
QVector<double> m15328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
