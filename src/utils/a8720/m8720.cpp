#include "a8720/m8720.h"
QVector<double> m8720::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
