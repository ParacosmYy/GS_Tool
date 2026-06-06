#include "k15910/m15910.h"
QVector<double> m15910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
