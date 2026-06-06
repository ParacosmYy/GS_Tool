#include "m8592/m8592.h"
QVector<double> m8592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
