#include "f29685/m29685.h"
QVector<double> m29685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
