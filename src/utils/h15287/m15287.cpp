#include "h15287/m15287.h"
QVector<double> m15287::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
