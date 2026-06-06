#include "k18030/m18030.h"
QVector<double> m18030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
