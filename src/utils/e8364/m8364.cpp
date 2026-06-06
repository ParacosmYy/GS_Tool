#include "e8364/m8364.h"
QVector<double> m8364::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
