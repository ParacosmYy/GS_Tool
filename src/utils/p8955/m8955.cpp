#include "p8955/m8955.h"
QVector<double> m8955::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
