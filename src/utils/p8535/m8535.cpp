#include "p8535/m8535.h"
QVector<double> m8535::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
