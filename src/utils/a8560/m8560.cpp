#include "a8560/m8560.h"
QVector<double> m8560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
