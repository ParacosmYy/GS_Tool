#include "e8224/m8224.h"
QVector<double> m8224::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
