#include "m8472/m8472.h"
QVector<double> m8472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
