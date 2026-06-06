#include "m20812/m20812.h"
QVector<double> m20812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
