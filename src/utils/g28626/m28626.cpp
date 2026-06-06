#include "g28626/m28626.h"
QVector<double> m28626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
