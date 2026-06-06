#include "h8627/m8627.h"
QVector<double> m8627::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
