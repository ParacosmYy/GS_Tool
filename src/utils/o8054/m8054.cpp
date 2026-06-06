#include "o8054/m8054.h"
QVector<double> m8054::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
