#include "d24963/m24963.h"
QVector<double> m24963::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
