#include "f8225/m8225.h"
QVector<double> m8225::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
