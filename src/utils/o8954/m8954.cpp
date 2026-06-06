#include "o8954/m8954.h"
QVector<double> m8954::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
