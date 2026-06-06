#include "f24105/m24105.h"
QVector<double> m24105::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
