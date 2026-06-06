#include "g28286/m28286.h"
QVector<double> m28286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
