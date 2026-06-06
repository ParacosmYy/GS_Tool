#include "m8052/m8052.h"
QVector<double> m8052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
