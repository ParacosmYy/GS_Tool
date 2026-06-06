#include "e25384/m25384.h"
QVector<double> m25384::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
