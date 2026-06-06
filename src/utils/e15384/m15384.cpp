#include "e15384/m15384.h"
QVector<double> m15384::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
