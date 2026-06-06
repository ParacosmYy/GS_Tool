#include "f8165/m8165.h"
QVector<double> m8165::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
