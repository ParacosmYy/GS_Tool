#include "p8455/m8455.h"
QVector<double> m8455::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
