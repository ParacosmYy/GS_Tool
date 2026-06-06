#include "o8074/m8074.h"
QVector<double> m8074::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
