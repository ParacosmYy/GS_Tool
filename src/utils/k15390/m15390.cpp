#include "k15390/m15390.h"
QVector<double> m15390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
