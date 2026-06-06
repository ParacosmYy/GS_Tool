#include "k18670/m18670.h"
QVector<double> m18670::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
