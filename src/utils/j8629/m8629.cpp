#include "j8629/m8629.h"
QVector<double> m8629::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
