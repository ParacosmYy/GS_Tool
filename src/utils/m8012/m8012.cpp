#include "m8012/m8012.h"
QVector<double> m8012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
