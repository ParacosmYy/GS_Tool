#include "f18685/m18685.h"
QVector<double> m18685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
