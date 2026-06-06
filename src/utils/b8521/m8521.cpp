#include "b8521/m8521.h"
QVector<double> m8521::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
