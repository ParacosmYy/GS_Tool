#include "l8291/m8291.h"
QVector<double> m8291::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
