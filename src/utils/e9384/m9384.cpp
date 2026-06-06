#include "e9384/m9384.h"
QVector<double> m9384::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
