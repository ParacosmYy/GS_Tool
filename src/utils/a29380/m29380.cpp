#include "a29380/m29380.h"
QVector<double> m29380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
