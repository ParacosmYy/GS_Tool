#include "i12708/m12708.h"
QVector<double> m12708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
