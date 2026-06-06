#include "i25768/m25768.h"
QVector<double> m25768::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
