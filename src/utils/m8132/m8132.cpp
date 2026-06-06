#include "m8132/m8132.h"
QVector<double> m8132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
