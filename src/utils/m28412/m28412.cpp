#include "m28412/m28412.h"
QVector<double> m28412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
