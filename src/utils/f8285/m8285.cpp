#include "f8285/m8285.h"
QVector<double> m8285::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
