#include "a24000/m24000.h"
QVector<double> m24000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
