#include "m8412/m8412.h"
QVector<double> m8412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
