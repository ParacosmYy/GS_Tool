#include "f8005/m8005.h"
QVector<double> m8005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
