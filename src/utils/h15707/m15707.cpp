#include "h15707/m15707.h"
QVector<double> m15707::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
