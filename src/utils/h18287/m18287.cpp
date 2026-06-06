#include "h18287/m18287.h"
QVector<double> m18287::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
