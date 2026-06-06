#include "t25299/m25299.h"
QVector<double> m25299::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
