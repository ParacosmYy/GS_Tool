#include "h8387/m8387.h"
QVector<double> m8387::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
