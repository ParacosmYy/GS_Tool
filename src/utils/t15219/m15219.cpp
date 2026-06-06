#include "t15219/m15219.h"
QVector<double> m15219::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
