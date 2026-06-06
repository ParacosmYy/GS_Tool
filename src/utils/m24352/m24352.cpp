#include "m24352/m24352.h"
QVector<double> m24352::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
