#include "t24219/m24219.h"
QVector<double> m24219::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
