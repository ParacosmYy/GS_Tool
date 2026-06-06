#include "a25200/m25200.h"
QVector<double> m25200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
