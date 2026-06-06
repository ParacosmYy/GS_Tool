#include "e32384/m32384.h"
QVector<double> m32384::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
