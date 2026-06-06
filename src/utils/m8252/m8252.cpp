#include "m8252/m8252.h"
QVector<double> m8252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
