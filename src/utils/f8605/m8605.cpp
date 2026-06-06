#include "f8605/m8605.h"
QVector<double> m8605::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
