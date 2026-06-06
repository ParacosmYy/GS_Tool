#include "h8607/m8607.h"
QVector<double> m8607::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
