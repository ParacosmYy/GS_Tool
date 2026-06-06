#include "f34685/m34685.h"
QVector<double> m34685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
