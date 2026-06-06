#include "m24072/m24072.h"
QVector<double> m24072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
