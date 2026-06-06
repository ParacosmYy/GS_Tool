#include "i9348/m9348.h"
QVector<double> m9348::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
