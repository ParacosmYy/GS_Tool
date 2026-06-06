#include "a25260/m25260.h"
QVector<double> m25260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
