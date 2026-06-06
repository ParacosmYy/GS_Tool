#include "m15872/m15872.h"
QVector<double> m15872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
