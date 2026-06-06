#include "p8815/m8815.h"
QVector<double> m8815::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
