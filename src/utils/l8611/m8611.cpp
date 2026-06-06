#include "l8611/m8611.h"
QVector<double> m8611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
