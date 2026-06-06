#include "l16291/m16291.h"
QVector<double> m16291::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
