#include "a24100/m24100.h"
QVector<double> m24100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
