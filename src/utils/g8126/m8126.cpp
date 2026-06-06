#include "g8126/m8126.h"
QVector<double> m8126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
