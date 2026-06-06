#include "a25280/m25280.h"
QVector<double> m25280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
