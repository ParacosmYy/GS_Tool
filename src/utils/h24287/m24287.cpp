#include "h24287/m24287.h"
QVector<double> m24287::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
