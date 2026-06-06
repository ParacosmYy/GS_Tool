#include "g29706/m29706.h"
QVector<double> m29706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
