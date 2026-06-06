#include "h25287/m25287.h"
QVector<double> m25287::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
