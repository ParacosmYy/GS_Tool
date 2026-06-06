#include "f17485/m17485.h"
QVector<double> m17485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
