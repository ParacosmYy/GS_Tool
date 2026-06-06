#include "h15807/m15807.h"
QVector<double> m15807::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
