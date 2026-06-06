#include "a18880/m18880.h"
QVector<double> m18880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
