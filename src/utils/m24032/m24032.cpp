#include "m24032/m24032.h"
QVector<double> m24032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
