#include "m24172/m24172.h"
QVector<double> m24172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
