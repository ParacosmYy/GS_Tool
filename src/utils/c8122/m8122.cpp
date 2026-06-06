#include "c8122/m8122.h"
QVector<double> m8122::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
