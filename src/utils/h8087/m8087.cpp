#include "h8087/m8087.h"
QVector<double> m8087::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
